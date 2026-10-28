//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "Processor.h"
#include <inet/common/InitStages.h>
#include "../neighbortable/NeighborTable.h"
#include <inet/common/packet/Packet.h>
#include "../../lbp/LocalBroadcastProtocol.h"
#include <inet/common/ModuleAccess.h>
#include "../../mobility/MovingInfo_m.h"

Define_Module(Processor);
simsignal_t countId = cComponent::registerSignal("overtakeCount");
simsignal_t perferredSpeedId = cComponent::registerSignal("perferredSpeedValue");
simsignal_t speedCostId = cComponent::registerSignal("speedCost");
simsignal_t switchCostId = cComponent::registerSignal("switchCost");
simsignal_t positionCostId = cComponent::registerSignal("positionCost");
simsignal_t totalCostId = cComponent::registerSignal("totalCost");


using namespace inet;

Processor::Processor ()
{
}

void Processor::finish()
{
}

Processor::~Processor()
{
    cancelAndDelete(startPlan);
}

// ----------------------------------------------------


void Processor::initialize (int stage)
{
  if (stage == INITSTAGE_LOCAL)
    {
      gidBeaconsIn = findGate("beaconsIn");
      gateToM = gate("toM");
      gateMIn = gate("mIn");
      gateToRB = gate("toRB");

      minimumSafetyDistance = par("minimumSafetyDistance");
      maxNeighborRange      = par("maxNeighborRange");
      clTimeThreshold       = par("clTimeThreshold");
      overtakeSpeed         = par("overtakeSpeed");
      otherLaneSafetyRange  = par("E2");
      targetLaneSafetyRange = par("E1");


      KAPPA_Speed = par("K2");
      KAPPA_Position = par("K1");
      KAPPA_Switch = par("K3");
      KAPPA_SINTERVAL = par("KI");
      E_SPEED = par("E3");
      E_TARGET = par("E2");
      E_NEIGHBOR = par("E3");

      startPlan             = new cMessage("startPlan");

      fisrtPart         = par("fisrtPart").stringValue();
      endPart           = par("endPart").stringValue();
      currentPart       = fisrtPart;
      currentI          = par("currentI");
      currentJ          = par("currentJ");
    }

  if (stage == INITSTAGE_LAST)
  {
      cModule *host = getContainingNode(this);
      assert(host);
      LocalBroadcastProtocol *lbp = check_and_cast<LocalBroadcastProtocol*>(host->getSubmodule("lbp"));
      assert(lbp);
      ownIdentifier  =  lbp->getOwnMacAddress();
      rb = check_and_cast<RenewalBeaconing*>(host->getSubmodule("rb"));
      mobility  = check_and_cast<DroneLinearMobility*>(host->getSubmodule("mobility"));
      nbTable = check_and_cast<NeighborTable*>(host->getSubmodule("nbt"));
      assert(mobility);
      cGate* destIn = mobility->gate("pIn");
      cGate* destOut = mobility->gate("toP");
      gateToM->connectTo(destIn);
      destOut->connectTo(gateMIn);

      preferredSpeed = rb->getPreferredSpeedd();
      maxSpeed = rb->getMaxSpeed();
      emit(perferredSpeedId, preferredSpeed);
      rs = check_and_cast<RoadSystem *>(getParentModule()->getParentModule()->getSubmodule("roadSystem"));
      rp = rs->getRoadPath(fisrtPart, endPart);
      setNextNextTarget();
      rb->setTarget(rp.getFromLane().getI(), rp.getFromLane().getJ(), rp.getFromT());
      rb->setTargetRoadId(rp.getNextId());
      //scheduleAt(simTime()+1.3, startPlan);
      setNextRun(1.3);
  }
}


void Processor::setNextRun(double time) {
    if (!std::isfinite(time) || time < 0)
        throw cRuntimeError("Processor: delay must be nonnegative and finite");
    if (startPlan->isScheduled()) cancelEvent(startPlan);
    scheduleAt(simTime()+time, startPlan);
    rb->setAlgorithmTime(simTime().dbl()+time);
}

void Processor::moveToNextPart() {
    Lane next = rp.getToLane();
    std::string _id = rp.getNextId();
    currentPart = _id;
    double _t = rp.getToT();
    int totalt = rs->getLaneCoord(Lane(0, 0), _id).rows()-1;
    totalt = round(totalt*_t);
    Vector3d nextT = rs->getWayPoint(totalt+1, _id, next);
    mobility->setNexTPart(_id, totalt, next.getI(), next.getJ());
    rp.toNextPoint();
    rb->setTarget(rp.getFromLane().getI(), rp.getFromLane().getJ(), rp.getFromT());
    rb->setTargetRoadId(rp.getNextId());
    setNextNextTarget();
}




void Processor::setNextNextTarget() {
    if (currentPart == endPart) {
        mobility->takeNewTarget(Vector3d(0,0,0), 9999, 9999);
        return;
    }
    Lane next = rp.getToLane();
    std::string _id = rp.getNextId();
    double _t = rp.getToT();
    int totalt = rs->getLaneCoord(Lane(0, 0), _id).rows()-1;
    totalt = round(totalt*_t);
    double _t1 = rp.getFromT();
    int totalt2 = round(rs->getLaneCoord(Lane(0, 0), currentPart).rows() * _t1);;
    Vector3d nextT = rs->getWayPoint(totalt+1, _id, next);
    mobility->takeNewTarget(nextT, totalt2, _t1);
}

AlgorithmOutput Processor::findNextLane() {

    double t = mobility->getCurrentT();
    std::vector<Lane> allNeighbors = rs->getNeighborLane(Lane(currentI, currentJ), currentPart, t, 1);
    allNeighbors.push_back(Lane(currentI, currentJ));
    std::vector<double> allCosts;
    std::vector<Lane> nextLs;
    std::vector<double> speedDiffs;
    AlgorithmOutput output;
    double min = MAX;
    EV << "\nProcessor::runAlgorithm: Start printing cost:\n";
    EV << "Lane          speed          collision          Position      Total\n";
    for(auto&i : allNeighbors) {
        double speedCost = getSpeedCost(i);
        double collisionCost = getCollisionCost(i);
        double positionCost = getPositionCost(i);

        // define f(dT)
        double coeff_p = 1;
        if (rp.getFromT() > 0) {
            double disToTarget = -1;
            disToTarget = rs->getDistance(currentPart, Lane(currentI, currentJ), t, rp.getFromT());
            if (disToTarget <= 15) coeff_p = 20.0;
            else if (disToTarget <= 30) coeff_p = (-2.0/3.0)*disToTarget+30.0;
            else coeff_p = 10.0 * 30.0 / disToTarget;
        }
        double positionCoeff = coeff_p * KAPPA_Position;
        // end define


        // for one singel lane, simple slow down scenario
        //if (Lane(currentI, currentJ) != i && overtakeSpeed == 2) {
        //    collisionCost = 999999;
        //    positionCost = 999999;
        //}


        double totalCost = KAPPA_Speed * speedCost*speedCost + KAPPA_Switch * collisionCost + positionCoeff * positionCost;
        EV << "Lane (" << i.getI() << ", " << i.getJ() << ")   ";
        EV << speedCost << "        " << collisionCost << "        " << positionCost << "       "    << totalCost << endl;
        allCosts.push_back(totalCost);
        speedDiffs.push_back(speedCost);
        if (i == Lane(currentI, currentJ)) {
            output.l = i;
            output.speed = preferredSpeed - speedCost;
        }
        if (totalCost < min) {
            min = totalCost;
        }
    }
    std::vector<double> _speedDiffs;
    for(int i=0;i<allNeighbors.size();i++) {
        if (allCosts[i] == min) {
            nextLs.push_back(allNeighbors[i]);
            _speedDiffs.push_back(speedDiffs[i]);
        }
    }
    if (nextLs.size() == 0 or min > 10000) {
        output.speed = 0;
        return output;
    }
    if (std::find(nextLs.begin(), nextLs.end(), Lane(currentI, currentJ)) != nextLs.end()) return output;
    int randomIndex = std::min(static_cast<int>(nextLs.size()) - 1, std::max(0, static_cast<int>(par("random01").doubleValue()*nextLs.size())));
    output.speed = preferredSpeed - _speedDiffs[randomIndex];
    output.l = nextLs[randomIndex];
    return output;
}

void Processor::makeDecision(AlgorithmOutput output) {
    Lane newLane = output.l;
    double newSpeed = output.speed;
    if (newLane == Lane(currentI, currentJ) and newLane != rp.getFromLane()) {
        //-----------------------------------------------------------------------------------
        if (rs->checkIsTurnLane(currentPart, newLane)) {
            double closeT = rs->getRoadClose(currentPart, newLane, mobility->getCurrentT(), 1);
            double dis2 = rs->getDistance(currentPart, newLane, mobility->getCurrentT(), closeT);
            if (dis2 < newSpeed*KAPPA_STOP) newSpeed = 0;
            EV << "Stop before the inter" << endl;
            EV << "Distance: " << dis2 << endl;
            EV << "T: " << mobility->getCurrentT() << endl;
            EV << "targetT: " << closeT << endl;
        //-----------------------------------------------------------------------------------
        } else {
            double dis2 = -1;
            if (rp.getFromT() > 0) dis2 = rs->getDistance(currentPart, newLane, mobility->getCurrentT(), rp.getFromT());
            double maxMargin = preferredSpeed*KAPPA_STOP;
            if (dis2 < maxMargin and dis2 > 0) newSpeed = 0;
            EV << "Distance: " << dis2 << endl;
            EV << "T: " << mobility->getCurrentT() << endl;
            EV << "targetT: " << rp.getFromT() << endl;
  //---------------------------------------------------------
  //    an approach to increase priority, that will not affect
  //    any metrics but can avoid some drone wait too long
  //---------------------------------------------------------
          if (waitTime > 5 and couldMove and newSpeed == 0) {
              if (dis2 > 3 and dis2 < maxMargin) {
                  dis2 -= 2;
                  newSpeed = dis2/2.0;
                  if (newSpeed > preferredSpeed/2.0) newSpeed = preferredSpeed/2.0;
              }
          }
        }
    }
    if (newSpeed != 0) {
        waitTime = 0;
        lastRun = 0;
    }
    else {
        if (lastRun == 0) lastRun = simTime();
        else {
            waitTime += (simTime() - lastRun).dbl();
            lastRun = simTime();
        }
    }
    //--------------------------------------------------------

    mobility->setSpeed(newSpeed);
    if (newLane != Lane(currentI, currentJ) and newSpeed != 0) {
        laneSwitchTo = newLane;
        double upper = rs->getRoadClose(currentPart, laneSwitchTo, mobility->getCurrentT());
        double targetUpper = rp.getFromT();
        if (upper > targetUpper and targetUpper > 0) upper = targetUpper;
        EV << "Upper: " << upper << endl;
        mobility->switchLane(laneSwitchTo, newSpeed, upper);
        switchCounter++;
        rb->setSwitchLane(laneSwitchTo.getI(), laneSwitchTo.getJ());
        lastSwitch = simTime();
        //std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    }
}


void Processor::runAlgorithm() {

    isStopByTargetLaneDrone = false;
    if (mobility->isArrival()) return;
    runCounter++;

    if (overtakeSpeed == 2 and rs->getPartType(currentPart) != "ramp") {
        couldMove = true;

        couldMove = false; //TODO CLOSE THE FEATURE on 13/7/2026
        getGroupCost();
        return;
    }


    if (mode == "blind") {
        return; // Abandon part
    } else {
        couldMove = true;
        getCandidateNodes();
        EV << "--------------------------------------------------------------------------------------\n";
        EV << "Processor::runAlgorithm: Start printing candidate nodes:\n";
        EV << "ID:" << ownIdentifier << endl;
        EV << "Current Lane (" << currentI << ", " << currentJ << ")     Target Lane (" <<rp.getFromLane().getI() << ","
                << rp.getFromLane().getJ() << ")\n";
        EV << "Current Part: " << currentPart << endl;
        EV << "---------------------In neighbor lane-----------------------\n";
        for(auto l: neighborNodes) {
            Lane lane = l.first;
            EV << "In Lane (" << lane.getI() << ", " << lane.getJ() << "): ";
            for (auto n : l.second) EV << n << ", ";
            EV << "\n";
        }
        EV << "----------------In neighbor neighbor lane--------------------\n";
        for(auto l: nNeighborNodes) {
                Lane lane = l.first;
                EV << "In Lane (" << lane.getI() << ", " << lane.getJ() << "): ";
                for (auto n : l.second) EV << n << "--" << (*nbTable)[n].getDistance() << ", ";
                EV << "\n";
        }
        AlgorithmOutput output = findNextLane();
        EV << "----------------Algorithm Output--------------------\n";
        EV << "Lane: (" << output.l.getI() << ", " << output.l.getJ() << ")" << endl;
        EV << "Speed: " << output.speed << endl;

        EV << "-----------------To next target-----------------------\n";
        makeDecision(output);
        clearCandidateNodes();
        EV <<"current speed: "  << mobility->getMaxSpeed() << endl;

        if (rs->getNeighborLane(Lane(currentI, currentJ), currentPart, mobility->getCurrentT()).size() == 0) setNextRun(KAPPA_SINTERVAL);
        else if (mobility->getMaxSpeed()  == 0) setNextRun(KAPPA_SINTERVAL);
        else setNextRun(2);

        if (mobility->getIsSwitching()) {
            rb->setAlgorithmTime(simTime().dbl()+1); // one second for switching
        }
        EV << "--------------------------------------------------------------------------------------\n";
    }
}


// ----------------------------------------------------


void Processor::handleMovingInfo(MovingInfo* mi) {

    EV << "receive type: " << mi->getType() << endl;
    if (mi->getType()==1) {
        currentI = laneSwitchTo.getI();
        currentJ = laneSwitchTo.getJ();
    } else if (mi->getType()==0) {
        currentI = rp.getToLane().getI();
        currentJ = rp.getToLane().getJ();
        moveToNextPart();
        cancelEvent(startPlan);
        setNextRun(0);
        //scheduleAt(simTime(), startPlan);
    } else {
        cancelEvent(startPlan);
        setNextRun(0);
        //scheduleAt(simTime(), startPlan);
    }
}

// ------------------------------------------------------------------------------------------------------------------------------------

int Processor::getPacketLoss()
{
    int loss = 0;
    for (auto it = nblostTable.begin(); it != nblostTable.end(); ++it)
    {
        auto key = it->first;
        loss += it->second;
        nblostTable[key] = 0;
    }
    return loss;
}

void Processor::updateClTime(MacAddress nb) {
    int itI                = (*nbTable)[nb].getLastI();
    int itJ                = (*nbTable)[nb].getLastJ();
    std::string itRoadId   = (*nbTable)[nb].getLastRoadId();
    if (itRoadId == currentPart) { //TODO need to consider the beacon delay
        // update distance
        double itT             = (*nbTable)[nb].getLastT();
        double dis;
        if (itT >= mobility->getCurrentT()) dis = rs->getDistance(itRoadId, Lane(itI,itJ), mobility->getCurrentT(), itT);
        else dis = -1 * rs->getDistance(itRoadId, Lane(itI,itJ), itT, mobility->getCurrentT());
        (*nbTable)[nb].setDistance(dis);
        // update clTime
        if (itI == mobility->getCurrI() and itJ == mobility->getCurrJ()) {
            double speedDiff;
            double clTime = -1;
            if (itT >= mobility->getCurrentT()) speedDiff = mobility->getMaxSpeed() - (*nbTable)[nb].getLastVelocity().length();
            else speedDiff = (*nbTable)[nb].getLastVelocity().length() - mobility->getMaxSpeed();
            if (speedDiff > 0.00001) clTime = abs(dis) / abs(speedDiff);
            if (clTime >= 1000) clTime = -1;
            (*nbTable)[nb].setEstCollsionTime(clTime);
            if (dis>0 and clTime < 1 and clTime > 0 and !mobility->getIsSwitching()) {
                EV <<"  stop" << endl;
                EV << (*nbTable)[nb].getLastVelocity().length() << "   "  << mobility->getMaxSpeed() << endl;
                EV << nb << "     "  << clTime << "   "  << dis << endl;
                mobility->setSpeed(0);
            }
        } else {
            (*nbTable)[nb].setEstCollsionTime(-1);
        }
    }
    else {
        int isConnect = rs->checkConnect(currentPart, itRoadId, mobility->getCurrI(), itI, mobility->getCurrJ(), itJ);
        double fT, bT, fS, bS, cFT, cTT;
        std::string fId, bId;
        int fi,fj,bi,bj;
        double clTime = -1;
        if (isConnect == 1) {
            fT = mobility->getCurrentT();
            bT = (*nbTable)[nb].getLastT();
            fId = currentPart;bId = itRoadId;
            fi = mobility->getCurrI();fj = mobility->getCurrJ();
            bi = itI;bj = itJ;
            fS = mobility->getMaxSpeed();
            bS = (*nbTable)[nb].getLastVelocity().length();
        } else if (isConnect == 2) {
            bT = mobility->getCurrentT();
            fT = (*nbTable)[nb].getLastT();
            bId = currentPart;fId = itRoadId;
            bi = mobility->getCurrI();bj = mobility->getCurrJ();
            fi = itI;fj = itJ;
            bS = mobility->getMaxSpeed();
            fS = (*nbTable)[nb].getLastVelocity().length();
        } else {
            if (rs->checkAConnectToB(currentPart, itRoadId)) {
                fT = mobility->getCurrentT();
                bT = (*nbTable)[nb].getLastT();
                fId = currentPart;bId = itRoadId;
                fi = mobility->getCurrI();fj = mobility->getCurrJ();
                bi = itI;bj = itJ;
                fS = mobility->getMaxSpeed();
                bS = (*nbTable)[nb].getLastVelocity().length();
            } else {
                (*nbTable)[nb].setDistance(-99999);
                (*nbTable)[nb].setEstCollsionTime(clTime);
                return;
            }
        }
        auto ts = rs->getConnectT(fId, bId);
        cFT = ts[0];
        cTT = ts[1];
        double dis1 = rs->getDistance(fId, Lane(fi,fj), fT, cFT);

        if (abs(dis1) >= 30) {
            (*nbTable)[nb].setDistance(-99999);
            (*nbTable)[nb].setEstCollsionTime(clTime);
            return;
        }

        double speedDiff = fS - bS;
        double dis2;
        if (bT >= cTT) {
            dis2 = rs->getDistance(bId, Lane(bi,bj), cTT, bT);
            (*nbTable)[nb].setDistance(dis1+dis2);
            if (speedDiff > 0.00001) clTime = abs(dis1+dis2) / abs(speedDiff);
            if (clTime >= 1000) clTime = -1;

            if (isConnect==1 and dis1+dis2>0 and clTime < 1 and clTime > 0 and !mobility->getIsSwitching()) {
                EV <<"  stop" << endl;
                EV << (*nbTable)[nb].getLastVelocity().length() << "   "  << mobility->getMaxSpeed() << endl;
                EV << nb << "     "  << clTime << "   "  <<dis1+dis2 << endl;
                mobility->setSpeed(0);
            }
        } else {
            dis2 = rs->getDistance(bId, Lane(bi,bj), bT, cTT);
            (*nbTable)[nb].setDistance(abs(dis2-dis1));
            if ((dis1 >= dis2 and speedDiff > 0) or (dis1 <= dis2 and speedDiff < 0)) clTime = abs(dis2-dis1) / abs(speedDiff);
            if (clTime >= 1000) clTime = -1;
        }
        (*nbTable)[nb].setEstCollsionTime(clTime);
    }
}

double Processor::getSpeedCost(Lane l) {
    double t = mobility->getCurrentT();
    double cost = 0;
    double minDis = MAX;

    bool isCurrent = false;
    if (l == Lane(currentI, currentJ)) isCurrent = true;

    for(auto&it : neighborNodes[l]) {
        double otherT = (*nbTable)[it].getLastT();
        double distance = (*nbTable)[it].getDistance();
        if (distance < minDis and distance > 0) {
            minDis = distance;
            if (distance <= E_SPEED) cost = preferredSpeed - (*nbTable)[it].getLastVelocity().length();   // E_SPEED = 30
            if (distance <= E_SPEED and isCurrent) couldMove = false;
            double itD = rs->getDistance(currentPart, Lane(currentI, currentJ), otherT, rp.getFromT());
            if (!isCurrent and itD >0 and itD < 1) couldMove = false;
            if (cost < 0 ) cost = 0;
        }
    }
    return cost;
}


double Processor::getCollisionCost(Lane l) {
    double t = mobility->getCurrentT();
    double inter = rs->getRoadInter(currentPart, Lane(currentI, currentJ), t);
    if (l == Lane(currentI, currentJ)) return 0;
    if (neighborNodes[l].size() != 0) {
        for (auto &nb: neighborNodes[l]) {
            if (inter > 0 and l==rp.getFromLane() and (*nbTable)[nb].getLastT() < inter) continue;
            double itDis = (*nbTable)[nb].getDistance();
            if (itDis <= E_TARGET and itDis > -1 * E_TARGET) {
                isStopByTargetLaneDrone = true;
                return MAX; // E_TARGET = 16
            }
        }
    }
    double cost = 0;
    bool isFirst = true;
    double mySpeed = mobility->getMaxSpeed();
    if (mode == "simple") {
        auto nnls = rs->getNeighborLane(l, currentPart, t);
        for (auto ll: nnls) {
            if (nNeighborNodes[ll].size() != 0) {
                    for (auto &nb: nNeighborNodes[ll]) {
                        if (abs((*nbTable)[nb].getDistance()) <= E_NEIGHBOR) {
                            cost = cost + 1;
                            //std::cout << abs((*nbTable)[nb].getDistance()) << ", " << E_NEIGHBOR << endl;
                            double itT = (*nbTable)[nb].getLastT();
                            if (t < itT ) { isFirst = false; }
                            else if (t== itT and ownIdentifier < nb) isFirst = false;
                            if (abs((*nbTable)[nb].getDistance()) <= 3 and mySpeed < (*nbTable)[nb].getLastVelocity().length()) isFirst = false;
                        }
                    }
            }
        }
    }
    if (isFirst) cost = 0;
    return cost;
}


double Processor::getPositionCost(Lane l) {
    double closeT = rs->getRoadClose(currentPart, l, mobility->getCurrentT());
    if (closeT!=1.0 and (rp.getFromLane() != l or rp.getFromT() > closeT)) {
        double closeDistance = rs->getDistance(currentPart, Lane(mobility->getCurrI(), mobility->getCurrJ()), mobility->getCurrentT(), closeT);
        if (closeDistance < preferredSpeed * 2) {
            return MAX;
        }
    }
    // check whether this lane is a non-target turning lane
    bool isEnd = (currentPart == endPart);
    if (rs->checkIsTurnLane(currentPart, l))
    {
        if (rp.getFromLane() == l) return 0;
        return MAX;
    }
    // other cases
    if (mode == "simple") {
        double dis = rs->getDistance(currentPart, Lane(currentI, currentJ), mobility->getCurrentT(),rp.getFromT());
        int min = 9999;
        for (auto _l : rs->getAllSwitchLane(currentPart, mobility->getCurrentT())) {
            int ldis = LaneUtil::getDistance(_l, l);
            if (ldis < min) min = ldis;
        }
        min = 5-min; // 5 is magic //TODO changed from 4 to 5 for 3 tiers density test
        if (min <0 ) min = 0;
        if (isEnd) { // do not have next turning lane,
            return min; // TODO min -> 0  changed in 6/9/2026  0->min
        } else {
            return LaneUtil::getDistance(l, rp.getFromLane());
        }
    }
    return 0;

}
void Processor::recordBeacon (BeaconReport* beacon)
{
  assert(beacon);
  MacAddress  id      = beacon->getSenderId();
  uint32_t    seqno   = beacon->getSeqno();
  if (nbTable->entryAvailable(id)) {

      beaconSeqGap += seqno - nbSeqTable[id];
      beaconRecTotal += 1;
      beaconTimeGap += beacon->getGapTime();

      if (seqno - nbSeqTable[id] > 1) {
          nblostTable[id] += (seqno - nbSeqTable[id] -1);
      }
      nbSeqTable[id] = seqno;
  } else {
      nbSeqTable[id] = seqno;
      nblostTable[id] = 0;
  }
  nbTable->recordBeacon(beacon);
  updateClTime(id);

  Lane itLane = Lane((*nbTable)[id].getLastI(), (*nbTable)[id].getLastJ());
  std::string itPart = (*nbTable)[id].getLastRoadId();
  double itT = (*nbTable)[id].getLastT();
  double beaconTimeGap = simTime().dbl()- (*nbTable)[id].getLastBeaconReceptionTime().dbl();
  double itDis = rs->getDistance(itPart, itLane, 0, itT);
  double itActualDis = itDis + beaconTimeGap * getItSpeed(id);
  double itActualT = rs->getT(itPart, itLane, itActualDis);
  (*nbTable)[id].setLastT(itActualT);
}

void Processor::getCandidateNodes() {
    double t = mobility->getCurrentT();
    std::vector<MacAddress> emptyVector;
    std::vector<Lane> allNeighborLanes;

    allNeighborLanes = rs->getNeighborLane(Lane(currentI, currentJ), currentPart, t);

    for(auto &l : allNeighborLanes) {
        neighborNodes[l] = emptyVector;
        std::vector<Lane> allNNeighborLanes = rs->getNeighborLane(l, currentPart, t);
        for(auto &lll : allNNeighborLanes) nNeighborNodes[lll] = emptyVector;
    }
    neighborNodes[Lane(currentI,currentJ)] = emptyVector;
    for (auto & it : nbTable->getNeighborIdList()) {
        Lane itLane = Lane((*nbTable)[it].getLastI(), (*nbTable)[it].getLastJ());
        std::string itPart = (*nbTable)[it].getLastRoadId();
        if(neighborNodes.find(itLane)!=neighborNodes.end() and itPart==currentPart) neighborNodes[itLane].push_back(it);
        if(nNeighborNodes.find(itLane)!=nNeighborNodes.end() and itPart==currentPart) nNeighborNodes[itLane].push_back(it);
        if (rs->checkConnect(currentPart, itPart, currentI, itLane.getI(), currentJ, itLane.getJ()) == 1) {
            neighborNodes[Lane(currentI, currentJ)].push_back(it);
        }
        if (itPart == rp.getNextId() and rs->getPartType(itPart)== "ramp") {
            if (neighborNodes.find(rp.getFromLane())!= neighborNodes.end()) neighborNodes[rp.getFromLane()].push_back(it);
        }
    }
}

void Processor::clearCandidateNodes() {
    neighborNodes.clear();
    nNeighborNodes.clear();
}

void Processor::handleMessage(cMessage *msg)
{
  if (dynamic_cast<BeaconReport*>(msg) && msg->arrivedOn(gidBeaconsIn))
  {
      BeaconReport* beacon = (BeaconReport*) msg;
      recordBeacon(&(*beacon));
      delete msg;
      //EV << mobility->getCurrentT()<< endl;
      return;
  } else if (msg == startPlan)
  {
      runAlgorithm();
      send(new cMessage("SBN"), gateToRB);
      return;
  } else if (dynamic_cast<MovingInfo*>(msg)) {
      MovingInfo* mi = (MovingInfo*) msg;
      handleMovingInfo(mi);
      delete msg;
      return;
  }
  EV << "Processor::handleMessage: got improper message type" << endl;
  endSimulation();
}


void Processor::printTable()
{
    EV << "Processor::handleMessage: Starting print Table for node "<< ownIdentifier << endl;
    EV << "----------------------------------------------------------------------------------" << endl;
    EV << "Address              Position                   Velocity              Estimate Collision time        i,j     roadID      t       distance"       << endl;
    auto listNeighbor = nbTable->getNeighborIdList();
    for (auto nb : listNeighbor)
    {
        EV << nb << "    "
           << (*nbTable)[nb].getLastPosition() << "    "
           << (*nbTable)[nb].getLastVelocity() << "       "
           << (*nbTable)[nb].getEstCollsionTime() << "     " <<
           (*nbTable)[nb].getLastI() <<"," << (*nbTable)[nb].getLastJ() << "  " <<
           (*nbTable)[nb].getLastRoadId() <<"    "<< (*nbTable)[nb].getLastT() <<
           "      " << (*nbTable)[nb].getDistance() << endl;

    }
    EV << "Processor::handleMessage: Printing finished"<< endl;
}


//==================================================================================================================================================


void Processor::getCompetitors() {

    std::map<MacAddress, double> tmap;

    int myIndexOfBlock = rs->getIndexOfBlock(currentPart, mobility->getCurrentT());
    for (auto & it : nbTable->getNeighborIdList()) {
        if ((*nbTable)[it].getLastRoadId() != currentPart) continue;

        double timeGap = (*nbTable)[it].getNextAlgorithmTime() - simTime().dbl();
        bool itIsSwitching = (*nbTable)[it].getIsSwitching();

        if (std::isnan((*nbTable)[it].getLastT())) {

            std::cout << it << "   " <<  (*nbTable)[it].getNextAlgorithmTime() << "   "  << simTime() <<  endl;
            std::cout << (*nbTable)[it].getLastI() << "   " <<  (*nbTable)[it].getLastJ()<< "   "  << (*nbTable)[it].getLastRoadId() <<  endl;
        }


        int itIndexOfBlock = rs->getIndexOfBlock(currentPart, (*nbTable)[it].getLastT());
        Lane itLane = Lane((*nbTable)[it].getLastI(), (*nbTable)[it].getLastJ());
        Lane itSwitchLane = Lane((*nbTable)[it].getSwitchI(), (*nbTable)[it].getSwitchJ());

        if (abs(itIndexOfBlock) == abs(myIndexOfBlock)) {
            if (!itIsSwitching && timeGap <= 0.5 && timeGap>=0) {  //TODO GAP < 0 ALSO CONSIDER COMPETITOR
                competitors.push_back((*nbTable)[it].getNeighborId());
                finished[(*nbTable)[it].getNeighborId()] = false;
                tmap[(*nbTable)[it].getNeighborId()] = (*nbTable)[it].getLastT();

                //if (timeGap < 0) {
                //    double itT = (*nbTable)[it].getLastT();
                //    double itDis = rs->getDistance(currentPart, itLane, 0, itT);
                //    double myDis = rs->getDistance(currentPart, Lane(currentI, currentJ), 0, mobility->getCurrentT());
                //    double itPreviousDis = itDis + timeGap * getItSpeed(it);
                //    double myPreviousDis = myDis + timeGap * mobility->getMaxSpeed();
                //    algorithmLog += std::to_string(itPreviousDis) +" " + std::to_string(myPreviousDis);
                //    double itRelativeDis = myDis - myPreviousDis + itPreviousDis;
                //    double itAlgorithmT = rs->getT(currentPart, itLane, itRelativeDis);
                //    algorithmLog += "!!! --- FIND HAZARD DRONE IN SAME BLOCK id: " + it.str() + " change T from "
                //            + std::to_string(itT) + " to " + std::to_string(itAlgorithmT) + " --- !!!\n";
                //    tmap[(*nbTable)[it].getNeighborId()] = itAlgorithmT;
                //}

            }
            directHazard[itLane].push_back((*nbTable)[it].getNeighborId());
            if (itIsSwitching) directHazard[itSwitchLane].push_back((*nbTable)[it].getNeighborId());
            if (timeGap < 0) {
                potentialHazard.push_back((*nbTable)[it].getNeighborId());
                algorithmLog += "!!! --- FIND HAZARD DRONE IN SAME BLOCK id: " + it.str() + " --- !!!\n";
            }
        }

        if (itIndexOfBlock<0 and abs(itIndexOfBlock)-1==abs(myIndexOfBlock)) {
            if (!itIsSwitching && timeGap <= 0.5) potentialHazard.push_back((*nbTable)[it].getNeighborId());
            directHazard[itLane].push_back((*nbTable)[it].getNeighborId());
            if (itIsSwitching) directHazard[itSwitchLane].push_back((*nbTable)[it].getNeighborId());
        }

        if (abs(itIndexOfBlock)==abs(myIndexOfBlock)-1) {
            directHazard[itLane].push_back((*nbTable)[it].getNeighborId());
            if (itIsSwitching) directHazard[itSwitchLane].push_back((*nbTable)[it].getNeighborId());
        }


    }
    competitors.push_back(ownIdentifier);
    directHazard[Lane(currentI, currentJ)].push_back(ownIdentifier);

    tmap[ownIdentifier] = mobility->getCurrentT();
    std::sort(competitors.begin(), competitors.end(), [&tmap](const MacAddress&x,const MacAddress&y){
        return tmap[x] != tmap[y] ? tmap[x] > tmap[y] : x < y;
    } );


    algorithmLog += "--------------------------------------------------------\n competitors priority:\n";
    for (auto x: competitors) {
        algorithmLog += "---- " + x.str() + "  t = " + std::to_string(tmap[x]) + "\n";
    }
    algorithmLog += "--------------------------------------------------------\n direct Hazard:\n";
    for (auto x: directHazard) {
        Lane l = x.first;
        algorithmLog += "---- lane(" + std::to_string(l.getI()) + "," + std::to_string(l.getJ()) + "): ";
        for (auto y: x.second) algorithmLog += y.str() + " ";
        algorithmLog += "\n";
    }
    algorithmLog += "--------------------------------------------------------\n potential Hazard:\n";
    for (auto x: potentialHazard) {
        algorithmLog += "---- " + x.str() + "\n";
    }

}
void Processor::initCostMatrix() {
    for (auto nb: competitors) {
        Lane itLane; std::vector<Lane> allNeighbors;
        if (nb == ownIdentifier) {
            itLane = Lane(currentI, currentJ);
            allNeighbors = rs->getNeighborLane(itLane, currentPart, mobility->getCurrentT(), 1);
        }
        else {
            itLane = Lane((*nbTable)[nb].getLastI(), (*nbTable)[nb].getLastJ());
            allNeighbors = rs->getNeighborLane(itLane, currentPart, (*nbTable)[nb].getLastT(), 1);
        }
        costMatrix[nb] = std::map<Lane, std::vector<double>>();
        for (auto l : allNeighbors) costMatrix[nb][l] = {0,0,0};
        costMatrix[nb][itLane] = {0,0,0};
    }
}


void Processor::getGroupCost() {
    clearAlgorithmMatrix();

    algorithmLog += "--------------------------------------------------------\nStart calculate cost at time "
            + std::to_string(simTime().dbl()) + " at block: " + std::to_string(rs->getIndexOfBlock(currentPart, mobility->getCurrentT()))
            + " in road " + currentPart + ": \n";

    algorithmLog += "speed: " + std::to_string(mobility->getMaxSpeed()) + "\n" + "lane: " + std::to_string(currentI) + "," + std::to_string(currentJ)
    + "\n" + "target lane: " + std::to_string(rp.getFromLane().getI())
            + "," + std::to_string(rp.getFromLane().getJ()) + "\n" + "t: " + std::to_string(mobility->getCurrentT()) + "\n"
            + mobility->getCurrentPosition().str() + "\n";
    getCompetitors();
    initCostMatrix();

    getGroupPositionCost();
    getGroupSpeedCost();


    for (auto competitor: competitors) {
        algorithmLog += "--------------------------------------------------------\n collision cost for:" + competitor.str() + "\n";
        for (auto& item : costMatrix[competitor]) {
            Lane l = item.first;
            auto& cost = item.second;
            cost[2] = getCollisionCost(l, competitor);
            algorithmLog += "---- cost: " + std::to_string(cost[2]) + "\n";
        }
        algorithmLog += "--------------------------------------------------------\n total cost for:" + competitor.str() + "\n";
        for (auto& item : costMatrix[competitor]) {
            Lane l = item.first;
            auto& cost = item.second;
            algorithmLog += "-------- in lane( " + std::to_string(l.getI()) + "," + std::to_string(l.getJ()) + "): ";
            double total = KAPPA_Speed * cost[0]*cost[0] + KAPPA_Position * cost[1] +  KAPPA_Switch* cost[2];
            algorithmLog += "speed: " + std::to_string(cost[0]) + " position: " + std::to_string(cost[1]) +
                    " collision: " + std::to_string(cost[2]) + " total: " + std::to_string(total) + "\n";
        }
        makeItDecision(competitor);
        if (competitor == ownIdentifier) {
            break;
        }
    }


    //new code 15/6

    if (itDecisionSpeed == 0 and itDecisionLane == Lane(currentI, currentJ)) {
        if (lastRun == 0) lastRun = simTime();
        else {
            waitTime += (simTime() - lastRun).dbl();
            lastRun = simTime();
        }
        if (waitTime > 5 and couldMove) {
            waitTime = 0;
            lastRun = 0;

            double dis2 = -1;
            if (rp.getFromT() > 0) dis2 = rs->getDistance(currentPart, itDecisionLane, mobility->getCurrentT(), rp.getFromT());
            double maxMargin = preferredSpeed*KAPPA_STOP;
            if (dis2 > 3 and dis2 < maxMargin) {
                dis2 -= 2;
                itDecisionSpeed = dis2/2.0;
                if (itDecisionSpeed > preferredSpeed/2.0) itDecisionSpeed = preferredSpeed/2.0;
                EV << ownIdentifier << "   " << std::to_string(itDecisionSpeed) << endl;
            }
        }
    } else {
        waitTime = 0;
        lastRun = 0;
    }

    //new code end




    mobility->setSpeed(itDecisionSpeed);
    if (itDecisionLane != Lane(currentI, currentJ) and itDecisionSpeed != 0) {
        laneSwitchTo = itDecisionLane;
        double upper = rs->getRoadClose(currentPart, laneSwitchTo, mobility->getCurrentT());
        double targetUpper = rp.getFromT();
        if (upper > targetUpper and targetUpper > 0) upper = targetUpper;
        mobility->switchLane(laneSwitchTo, itDecisionSpeed, upper);
        switchCounter++;
        rb->setSwitchLane(laneSwitchTo.getI(), laneSwitchTo.getJ());
        lastSwitch = simTime();
    }
    double nat;

    if (rs->getNeighborLane(Lane(currentI, currentJ), currentPart, mobility->getCurrentT()).size() == 0) nat = KAPPA_SINTERVAL;
    else if (mobility->getMaxSpeed()  == 0) nat = KAPPA_SINTERVAL;
    else nat = 2;
    setNextRun(nat);

    if (mobility->getIsSwitching()) {
        rb->setAlgorithmTime(simTime().dbl()+1); // one second for switching
        nat = 1;
    }
    algorithmLog += "NEXT ALGORITHM RUN AT: " + std::to_string(nat+simTime().dbl()) + "\n";
    algorithmLog += "--------------------------------------------------------\n";
    for (auto nb : nbTable->getNeighborIdList())
    {
        algorithmLog += nb.str() + "    "
           + (*nbTable)[nb].getLastPosition().str() + "    "
           + std::to_string((*nbTable)[nb].getLastI()) + ","
           + std::to_string((*nbTable)[nb].getLastJ())  + "       "
           + std::to_string((*nbTable)[nb].getTargetI()) + ","
           + std::to_string((*nbTable)[nb].getTargetJ())  + "       "
           + std::to_string((*nbTable)[nb].getIsSwitching()) + "     " +
           std::to_string((*nbTable)[nb].getNextAlgorithmTime()) + "     " +
           (*nbTable)[nb].getLastRoadId() + "    "+ std::to_string(rs->getIndexOfBlock((*nbTable)[nb].getLastRoadId(), (*nbTable)[nb].getLastT())) + "    "
           + std::to_string((*nbTable)[nb].getLastT()) + "     " + std::to_string((*nbTable)[nb].getLastVelocity().length()) + "    "
           + std::to_string((*nbTable)[nb].getPSpeed()) + "     " + std::to_string(getItSpeed(nb)) + "\n";
   //     if (nb == ownIdentifier) std::cout << ownIdentifier << "\n" << algorithmLog << endl;
    }
    //std::cout << algorithmLog << endl;
    EV << algorithmLog << endl;
}





void Processor::makeItDecision(MacAddress it) {


    std::map<Lane, double> totalCosts;
    double minCost = 10000;
    Lane minLane;
    Lane currentLane;
    bool hasValidLane = false;
    double pSpeed;
    double currentT;
    double targetT;
    double distanceToTarget = -1;
    if (it == ownIdentifier) {
        pSpeed = preferredSpeed;
        targetT = rp.getFromT();
        currentT = mobility->getCurrentT();
        currentLane = Lane(currentI,currentJ);
    } else {
        targetT = (*nbTable)[it].getTargetT();
        pSpeed = (*nbTable)[it].getPSpeed();
        currentT = (*nbTable)[it].getLastT();
        currentLane = Lane((*nbTable)[it].getLastI(),(*nbTable)[it].getLastJ());
    }
    if (targetT > 0) distanceToTarget = rs->getDistance(currentPart, currentLane, currentT, targetT);
    minLane = currentLane;


    double coeff_p = 1;
    if (targetT> 0) {
        if (distanceToTarget <= 15) coeff_p = 20.0;
        else if (distanceToTarget <= 30) coeff_p = (-2.0/3.0)*distanceToTarget+30.0;
        else coeff_p = 10.0 * 30.0 / distanceToTarget;
    }
    double positionCoeff = coeff_p * KAPPA_Position;

    algorithmLog += "--------------------------------------------------------\n making decision for: " + it.str() + "\n";
    algorithmLog += "---- compare total cost:\n";
    for (auto& item : costMatrix[it]) {
        Lane l = item.first;
        auto& cost = item.second;
        totalCosts[l] = KAPPA_Speed * cost[0]*cost[0] + positionCoeff * cost[1] + KAPPA_Switch * cost[2];
        algorithmLog += "-------- lane(" + std::to_string(l.getI()) + "," + std::to_string(l.getJ()) + "): " + std::to_string(totalCosts[l]) + "\n";
        if (totalCosts[l] < minCost) {
            algorithmLog += "------------ set to output lane\n";
            hasValidLane = true;
            minCost = totalCosts[l];
            minLane = l;
        } else if (totalCosts[l] == minCost and totalCosts[l] < 10000) {
            if (minLane < l and minLane != currentLane) minLane = l;
            if (l == currentLane) minLane = l;
            algorithmLog += "------------ lane("+ std::to_string(minLane.getI()) + "," + std::to_string(minLane.getJ()) + "): is current lane or has larger index\n";
        }
    }
    if (!hasValidLane) algorithmLog += "---- no valid lane found:\n";

// new code
//    std::vector<Lane> nextLs;
//    for (auto& item : totalCosts) {
//        Lane l = item.first;
//        auto& cost = item.second;
//        if (cost  == minCost) {
 //           nextLs.push_back(l);
//        }
//    }
//    if (minCost >= 10000) minLane = currentLane;
//    else if (std::find(nextLs.begin(), nextLs.end(), currentLane) != nextLs.end()) minLane = currentLane;
//    else {
//        int randomIndex = floor(par("random01").doubleValue()*nextLs.size());
//        minLane = nextLs[randomIndex];
//    }

// end



    itDecisionLane = minLane;
    itDecisionSpeed = pSpeed - costMatrix[it][minLane][0];



    if (!hasValidLane) itDecisionSpeed = 0;


    if (itDecisionLane == Lane(currentI, currentJ) and itDecisionLane != rp.getFromLane())  {

        if (rs->checkIsTurnLane(currentPart, itDecisionLane)) {
            double closeT = rs->getRoadClose(currentPart, itDecisionLane, currentT, 1);
            double dis2 = rs->getDistance(currentPart, itDecisionLane, currentT, closeT);
            if (dis2 < itDecisionSpeed*KAPPA_STOP) {
                algorithmLog += "---- distance to close point: " + std::to_string(dis2) + " < margin - SET SPEED TO 0\n";
                itDecisionSpeed = 0;
            }

        } else {
            double maxMargin = preferredSpeed*KAPPA_STOP;
            if (distanceToTarget < maxMargin and distanceToTarget > 0) {
                algorithmLog += "---- distance to target: " + std::to_string(distanceToTarget) + " < margin - SET SPEED TO 0\n";
                itDecisionSpeed = 0;
            }
        }

    }
    algorithmLog += "---- lane: (" + std::to_string(itDecisionLane.getI()) + "," + std::to_string(itDecisionLane.getJ()) + "), speed: " + std::to_string(itDecisionSpeed) + "\n";

    if (it == ownIdentifier) return;

    algorithmLog += "--------------------------------------------------------\n updating cost matrix\n";

    bool isSwitch = currentLane != itDecisionLane;
    for (auto competitor: competitors) {

        if (finished[competitor] or competitor == it) continue;
        algorithmLog += "---- for:" + competitor.str() + "\n";


        for (auto& item : costMatrix[competitor]) {
            Lane l = item.first;
            auto& cost = item.second;
            if (l == itDecisionLane) {
                if (isSwitch) {
                    algorithmLog += "-------- add to direct hazard\n";
                    directHazard[l].push_back(it);
                }

                double dis; double ps = preferredSpeed;
                if (competitor == ownIdentifier) {
                    dis = rs->getDistance(currentPart, l, mobility->getCurrentT() ,currentT);
                } else {
                    dis = rs->getDistance(currentPart, l, (*nbTable)[competitor].getLastT() ,currentT);
                    ps = (*nbTable)[competitor].getPSpeed();
                }

                if (dis < E_SPEED){
                    double scost = ps - itDecisionSpeed;
                    if (scost > cost[0]) {
                        algorithmLog += "-------- update speed cost in lane(" + std::to_string(l.getI()) + "," + std::to_string(l.getJ())+
                                ") from " + std::to_string(cost[0]) + " to " + std::to_string(scost) + "\n";
                        cost[0] = scost;
                    }
                }
            }
        }
    }
    finished[it] = true;



}


















void Processor::getGroupSpeedCost() {

    algorithmLog += "--------------------------------------------------------\n speed cost:\n";
    std::map<MacAddress, std::map<MacAddress, double>> disMatrix;

    for (auto nb: competitors) {
        disMatrix[nb] = std::map<MacAddress, double>{};
        double currT;double dis;double targetT;
        double myCurrT = mobility->getCurrentT();
        Lane targetLane;
        std::string targetId;
        Lane myLane =  Lane(currentI, currentJ);
        if (nb != ownIdentifier) {
            currT = (*nbTable)[nb].getLastT();
            if (myCurrT >= currT) dis = rs->getDistance(currentPart, myLane, currT, myCurrT);
            else dis = -1 * rs->getDistance(currentPart, myLane, myCurrT, currT);
            disMatrix[nb][ownIdentifier] = dis;
            targetId = (*nbTable)[nb].getTargetRoadId();
            targetT = (*nbTable)[nb].getTargetT();
            targetLane = Lane((*nbTable)[nb].getTargetI(),(*nbTable)[nb].getTargetJ());
        } else {
            currT = myCurrT;
            //disMatrix[nb][ownIdentifier] = 0;
            targetId = rp.getNextId();
            targetT = rp.getFromT();
            targetLane = rp.getFromLane();
        }
        for (auto & it : nbTable->getNeighborIdList()) {
            std::string itPart = (*nbTable)[it].getLastRoadId();
            double itT = (*nbTable)[it].getLastT();
            if (itPart != currentPart) {
                if (itPart == targetId and rs->getPartType(itPart)== "ramp") {
                    disMatrix[nb][it] = rs->getDistance(itPart, Lane(0,0),0, itT) + rs->getDistance(currentPart, targetLane, currT,targetT);}
                continue;}
            if (it == nb) continue;
            Lane itLane = Lane((*nbTable)[it].getLastI(), (*nbTable)[it].getLastJ());
            if (itT >= currT) dis = rs->getDistance(currentPart, itLane, currT, itT);
            else dis = -1 * rs->getDistance(currentPart, itLane, itT, currT);
            disMatrix[nb][it] = dis;
        }
    }

    for (auto nb: competitors) {
        double pSpeed;
        if (nb != ownIdentifier) pSpeed = (*nbTable)[nb].getPSpeed();
        else pSpeed = preferredSpeed;
        algorithmLog += "---- for " + nb.str() + " with preferred speed " + std::to_string(pSpeed) + "\n";
        for (auto& item : costMatrix[nb]) {
            auto& l = item.first;
            auto& cost = item.second;
            cost[0] = getSpeedCost(l, disMatrix[nb], pSpeed, nb);
            algorithmLog += "-------- in lane( " + std::to_string(l.getI()) + "," + std::to_string(l.getJ()) + "): " + std::to_string(cost[0]) + "\n";
        }
    }
}








void Processor::getGroupPositionCost() {

    algorithmLog += "--------------------------------------------------------\n position cost:\n";
    for (auto nb: competitors) {
        double currT;double targetT;Lane targetLane;Lane currLane;
        if (nb != ownIdentifier) {
            currT = (*nbTable)[nb].getLastT();
            targetT = (*nbTable)[nb].getTargetT();
            targetLane = Lane((*nbTable)[nb].getTargetI(), (*nbTable)[nb].getTargetJ());
            currLane = Lane((*nbTable)[nb].getLastI(), (*nbTable)[nb].getLastJ());
        } else {
            currT = mobility->getCurrentT();
            targetT = rp.getFromT();
            targetLane = rp.getFromLane();
            currLane = Lane(currentI, currentJ);
        }
        algorithmLog += "---- for " + nb.str() + " at lane(" + std::to_string(currLane.getI()) + "," + std::to_string(currLane.getJ())
        + ") with target lane(" + std::to_string(targetLane.getI()) + "," + std::to_string(targetLane.getJ()) + "): \n";
        for (auto& item : costMatrix[nb]) {
            auto& l = item.first;
            auto& cost = item.second;
            cost[1] = getPositionCost(l, currT, targetT, currLane, targetLane);
            algorithmLog += "-------- in lane( " + std::to_string(l.getI()) + "," + std::to_string(l.getJ()) + "): " + std::to_string(cost[1]) + "\n";
        }
    }
}

//===========================================================================================================================================
double Processor::getPositionCost(Lane l, double currT, double targetT, Lane currLane, Lane targetLane) {
    double closeT = rs->getRoadClose(currentPart, l, currT);
    if (closeT!=1.0 and (targetLane != l or targetT > closeT)) {
        double closeDistance = rs->getDistance(currentPart, currLane, currT, closeT);
        if (closeDistance < 15 * 2) {
            return MAX;
        }
    }
    bool isEnd = (targetT == -9999);
    if (rs->checkIsTurnLane(currentPart, l))
    {
        if (targetLane == l) return 0;
        return MAX;
    }
    double dis = rs->getDistance(currentPart, currLane, currT, targetT);
    int min = 9999;
    for (auto _l : rs->getAllSwitchLane(currentPart, currT)) {
        int ldis = LaneUtil::getDistance(_l, l);
        if (ldis < min) min = ldis;
    }
    min = 5-min;
    if (min <0 ) min = 0;
    if (isEnd) return min;  //TODO min -> 0 // changed on 2026/6/9 0->min
    return LaneUtil::getDistance(l, targetLane);
}


double Processor::getSpeedCost(Lane l, const std::map<MacAddress, double>& dm, double pSpeed, NodeIdentifier competitor) {
    double cost = 0;
    double minDis = MAX;
    Lane targetLane = rp.getFromLane();
    Lane currentLane = Lane(currentI, currentJ);
    if (competitor != ownIdentifier) {
        targetLane = Lane((*nbTable)[competitor].getTargetI(), (*nbTable)[competitor].getTargetJ());
        currentLane = Lane((*nbTable)[competitor].getLastI(), (*nbTable)[competitor].getLastJ());
    }
    for (auto& item : dm) {
        auto& it = item.first;
        auto& itDistance = item.second;
        Lane itLane;

        // new code
        if (competitor == ownIdentifier){
            itLane = Lane((*nbTable)[it].getLastI(), (*nbTable)[it].getLastJ());
            if (itLane == Lane(currentI, currentJ)) {
                if (itDistance <= E_SPEED and itDistance > 0) couldMove = false;
            }
        }
        // new code end 15/6/2026


        if (it != ownIdentifier) {
            Lane switchLane = Lane(9999,9999);
            std::string itPart = (*nbTable)[it].getLastRoadId();
            itLane = Lane((*nbTable)[it].getLastI(), (*nbTable)[it].getLastJ());
            if ((*nbTable)[it].getIsSwitching()) switchLane = Lane((*nbTable)[it].getSwitchI(), (*nbTable)[it].getSwitchJ());
            if (itPart != currentPart and l!=targetLane) continue;
            if (itPart == currentPart) {
                if (l == currentLane and itLane != l and switchLane != l) continue;
                if (l != currentLane and not (switchLane == l or (switchLane == Lane(9999,9999) and itLane == l)) ) continue;
            }
        } else if (Lane(currentI, currentJ) != l) continue;
        //std::cout << "------------comparing speed with " << it << " in lane (" << l.getI() << "," << l.getJ() << ")" << endl;
        if (itDistance < minDis and itDistance > 0) {
            minDis = itDistance;
            double itSpeed;
            if (it == ownIdentifier) itSpeed = mobility->getMaxSpeed();
            else itSpeed = getItSpeed(it);// (*nbTable)[it].getLastVelocity().length();
            if (itDistance <= E_SPEED) cost = pSpeed - itSpeed;   // E_SPEED = 30
            if (cost < 0 ) cost = 0;
        }
    }
    return cost;
}


double Processor::getCollisionCost(Lane l, MacAddress competitor) {

    Lane currLane = Lane(currentI,currentJ);
    Lane targetLane = rp.getFromLane();
    double currT = mobility->getCurrentT();
    double targetT = rp.getFromT();
    double currSpeed = mobility->getMaxSpeed();
    if (competitor != ownIdentifier) {
        currLane = Lane((*nbTable)[competitor].getLastI(), (*nbTable)[competitor].getLastJ());
        targetLane = Lane((*nbTable)[competitor].getTargetI(), (*nbTable)[competitor].getTargetJ());
        currT = (*nbTable)[competitor].getLastT();
        targetT = (*nbTable)[competitor].getTargetT();
        currSpeed = getItSpeed(competitor);

    }

    if (l == currLane) {
        algorithmLog += "---- in lane(" + std::to_string(l.getI()) + "," + std::to_string(l.getJ()) + "): \n-------- RETURN 0\n"; return 0;
    }
    double inter = rs->getRoadInter(currentPart, currLane, currT);
    int currIndexOfBlock = rs->getIndexOfBlock(currentPart, currT);

    algorithmLog += "---- in lane(" + std::to_string(l.getI()) + "," + std::to_string(l.getJ()) + "):\n";
    algorithmLog += "-------- direct hazard: \n";

    for (auto &nb: directHazard[l]) {

        if (nb == competitor) continue;

        algorithmLog += "---------- " + nb.str();

        double itT = mobility->getCurrentT();
        double itTarget = rp.getFromT();
        if (nb != ownIdentifier) {
            itT  = (*nbTable)[nb].getLastT();
            itTarget = (*nbTable)[nb].getTargetT();
        }

        if (targetT > currT and itT > targetT) {
            algorithmLog +=  " SKIP - currT: " + std::to_string(currT) + " < currTarget: " + std::to_string(targetT) + " < itT: " + std::to_string(itT)  + "\n";
            continue;
        }
        if (itTarget > itT and currT > itTarget) {
            algorithmLog += " SKIP - itT: " + std::to_string(itT) + " < itTarget: " + std::to_string(itTarget) + " < currT: " + std::to_string(currT) + "\n";
            continue;
        }
        if (inter > 0 and l==targetLane and itT < inter) {
            algorithmLog +=  " SKIP - itT: " + std::to_string(itT) + " < inter:" + std::to_string(inter)  + "\n";
            continue;
        }

        double itDis;
        if(itT > currT) itDis = rs->getDistance(currentPart, l, currT, itT);
        else itDis = rs->getDistance(currentPart, l, itT, currT);
        if (itDis <= E_TARGET) {
            algorithmLog += " RETURN MAX - distance: " + std::to_string(itDis) + " <= epsilon: " + std::to_string(E_TARGET) + "\n";
            return MAX;
        }
        algorithmLog += " SKIP - distance: " + std::to_string(itDis) + " > epsilon: " + std::to_string(E_TARGET) + "\n";
    }

    double cost = 0;
    algorithmLog += "-------- potential hazard: \n";
    for (auto &nb: potentialHazard) {

        double itT  = (*nbTable)[nb].getLastT();
        double itTarget = (*nbTable)[nb].getTargetT();
        Lane itLane = Lane((*nbTable)[nb].getLastI(), (*nbTable)[nb].getLastJ());
        int itIndexOfBlock = rs->getIndexOfBlock(currentPart, itT);
        int laneDistance = LaneUtil::getDistance(l, itLane);
        if (laneDistance != 1) continue;

        if (targetT > currT and itT > targetT) {
            algorithmLog += " SKIP - currT: " + std::to_string(currT) + " < currTarget: " + std::to_string(targetT) + " < itT: " +  std::to_string(itT) + "\n";
            continue;
        }
        if (itTarget > itT and currT > itTarget) {
            algorithmLog += " SKIP - itT: " + std::to_string(itT) + " < itTarget: " + std::to_string(itTarget) + " < currT: " +  std::to_string(currT) + "\n";
            continue;
        }
        if (inter > 0 and l==targetLane and itT < inter) {
            algorithmLog += " SKIP - itT: " + std::to_string(itT) + " < inter:" + std::to_string(inter) + "\n";
            continue;
        }

        algorithmLog += "---------- " + nb.str();

// new code to avoid next algorithm time < 0 lock
        //if (abs(currIndexOfBlock) == abs(itIndexOfBlock) and getItSpeed(nb) < 1 and getItSpeed(nb) >= currSpeed) {
          if (abs(currIndexOfBlock) == abs(itIndexOfBlock) and getItSpeed(nb) <= currSpeed and getItSpeed(nb) < 1 ) {

            if (currT > itT or (itT == currT and competitor > nb)) {
                algorithmLog += " SKIP IGNORE BEHIND POTENTIAL- itT: " + std::to_string(itT) + "  currT:" + std::to_string(currT) +"\n";
                continue;
            }
        }
// end
        double itDis;
        if(itT > currT) itDis = rs->getDistance(currentPart, l, currT, itT);
        else itDis = rs->getDistance(currentPart, l, itT, currT);

        algorithmLog += " distance: " + std::to_string(itDis) + " epsilon: " + std::to_string(E_TARGET) + " RESULT: ";

        if (itDis <= E_NEIGHBOR) { // TODO use E_TARGET instead of E_NEIGHBOR
             cost = cost + 1;
             algorithmLog += "+1\n";
        } else  algorithmLog += "+0\n";
    }
    algorithmLog += "-------- RETURN "+ std::to_string(cost) + "\n";
    return cost;
}





/***
 * In previous algorithm collision cost = direct cost + concurrent cost
 *
 * Now direct cost = check other's (isSwitching and switchToLane) AND the candidate lane's drone
 *
 * The concurrent cost will be (no longer calculated) OR (only calculated for drones in ahead block and dis<=15 ), this will be checked in later stage
 *
 * But the isFirst should be considered carefully
 */
void Processor::getGroupCollisoinCost() {
    std::cout << "--------------------------------------------------------\n collision cost:" << endl;
    std::map<Lane, std::vector<Lane>> neighborLaneMap;
    double nextBlockPos = (abs(rs->getIndexOfBlock(currentPart, mobility->getCurrentT()))+1) * 50 ; //50 is block size
    for (auto item: rs->getRoadParts()[currentPart]->getLanes()) {
        auto lane = item.first;
        double nextBlockT = nextBlockPos / rs->getLength(currentPart, lane);
        if (nextBlockT >= 1) continue;
        std::vector<Lane> neighborLane = rs->getNeighborLane(lane, currentPart, nextBlockT, 1);
        if (!neighborLane.empty()) {
            neighborLaneMap[lane] = neighborLane;
        }
    }


    std::map<Lane, std::vector<MacAddress>> aheadHazards;
    std::map<Lane, std::vector<MacAddress>> directHazards;
    int currIndexOfBlock = rs->getIndexOfBlock(currentPart, mobility->getCurrentT());
    for (auto & it : nbTable->getNeighborIdList()) {

        if ((*nbTable)[it].getLastRoadId() != currentPart) continue;

        int itIndexOfBlock = rs->getIndexOfBlock(currentPart, (*nbTable)[it].getLastT());

        bool isAhead = itIndexOfBlock<0 and abs(itIndexOfBlock)-1 == currIndexOfBlock;
        bool isCurr = abs(itIndexOfBlock) == abs(currIndexOfBlock);
        Lane itLane =  Lane((*nbTable)[it].getLastI(), (*nbTable)[it].getLastJ());

        if (isAhead or isCurr) {
            Lane affectedLane = itLane;
            if ((*nbTable)[it].getIsSwitching()) {
                affectedLane =  Lane((*nbTable)[it].getSwitchI(), (*nbTable)[it].getSwitchJ());
            }
            directHazards[affectedLane].push_back(it);
        }

        if (isAhead) {
            if ((*nbTable)[it].getNextAlgorithmTime()-simTime().dbl() <= 0.5) { // only consider who will run algorithm
                aheadHazards[itLane].push_back(it);
                for(auto l: neighborLaneMap[itLane]) aheadHazards[l].push_back(it);
            }
        }
    }
    directHazards[Lane(currentI, currentJ)].push_back(ownIdentifier);

    for (auto item: aheadHazards) {
        auto l = item.first;
        auto nl = item.second;
        std::cout << "---- " << l.getI() << "," << l.getJ() << endl;
        for (auto ll: nl) std::cout  << "--------" << ll<< endl;
    }



}





