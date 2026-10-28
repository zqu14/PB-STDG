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
#include <omnetpp.h>
#include "ArrivalChecker.h"
#include "../mobility/DroneLinearMobility.h"
#include "../beaconing/processor/Processor.h"
#include <inet/physicallayer/neighborcache/NeighborListNeighborCache.h>
#include <inet/physicallayer/contract/packetlevel/IRadio.h>
#include <vector>
#include <set>
#include <inet/common/ModuleAccess.h>
using namespace std;

Define_Module(ArrivalChecker);
simsignal_t collisionCountId = cComponent::registerSignal("collisionCount");
simsignal_t allId = cComponent::registerSignal("allFinishNode");
simsignal_t rightId = cComponent::registerSignal("rightFinishNode");
simsignal_t simTimeId = cComponent::registerSignal("simTimeValue");
simsignal_t collisionNodeId = cComponent::registerSignal("collisionNode");
simsignal_t collisionVecId = cComponent::registerSignal("collisionVec");
simsignal_t nodeCountId = cComponent::registerSignal("nodeCount");
simsignal_t lossCountId = cComponent::registerSignal("lossCount");
simsignal_t arriveCountId = cComponent::registerSignal("arriveCount");
simsignal_t aveSeqGapId = cComponent::registerSignal("aveSeqGap");
simsignal_t aveTimeGapId = cComponent::registerSignal("aveTimeGap");



void ArrivalChecker::initialize() {
    startTime = clock();
    check = new cMessage("check");
    count = new cMessage("count");
    minimumSafetyDistance = par("minimumSafetyDistance");
    numOfTarget = par("numOfTarget");
    constrainZ = par("constrainZ").doubleValue();
    scheduleAt(simTime()+CL_CHECK_PERIOD, check);
    scheduleAt(simTime(), count);
    currentNumofTarget = 0;
    arriveCount = 0;
}


void ArrivalChecker::handleMessage(cMessage* msg) {
    if (msg == check) {
       checkCollision();
       scheduleAt(simTime()+CL_CHECK_PERIOD, check);
    } else if (msg == count) {
       countNode();
       scheduleAt(simTime()+NODE_COUNT_CHECK_PERIOD, count);
    } else {
        error("ArrivalChecker::handleMessage unknown message");
    }
}

void ArrivalChecker::countNode() {

    int c = 0;
    int loss = 0;
    for (int i=0; i<numOfTarget;i++) {
        cModule *module = getParentModule()->getSubmodule("nodes", i);
        if (module != nullptr) {
            DroneLinearMobility * mobility = check_and_cast<DroneLinearMobility *>(module->getSubmodule("mobility"));
            if (!mobility->isArrival()) {
                c += 1;
            }
            Processor *p = check_and_cast<Processor *>(module->getSubmodule("p"));
            loss += p->getPacketLoss();
        }
    }
    emit(lossCountId, loss);
    emit(nodeCountId, c);
    emit(arriveCountId, arriveCount);
    arriveCount = 0;
}



void ArrivalChecker::checkCollision() {
    set<int> collided;
    MatrixXd positions;
    positions.resize(numOfTarget, 3);
    for (int i=0; i<numOfTarget;i++) {
        cModule *module = getParentModule()->getSubmodule("nodes", i);
        if (module != nullptr) {
            DroneLinearMobility *mobility = check_and_cast<DroneLinearMobility *>(module->getSubmodule("mobility"));
            Coord p = mobility->getCurrentPosition();
            positions.row(i) = Vector3d(p.x, p.y, p.z);

            int li = mobility->getCurrI();
            int lj = mobility->getCurrJ();

            std::string itID = mobility->getcurrRoadId();
            if (itID == "road_one" or itID == "road_two" or itID == "road_three" or itID == "road_four") {
                std::vector<int> ij = {li,lj};
                if (densityCount.find(ij) == densityCount.end()) {
                    densityCount[ij] = 0;
                }
                if (simTime() > 100) densityCount[ij]++;
                if (li==0 and lj==0) emit(rightId, true);
                emit(allId, true);
            }


            if (mobility->isArrival()) {


                Processor *p1 = check_and_cast<Processor *>(module->getSubmodule("p"));
                beaconGap += p1->getSeqGap();
                beaconDelay += p1->getTimeGap();
                beaconAmount += p1->getBeaconAmount();


                currentNumofTarget++;
                arriveCount++;
                //if (mobility->getRightFinish()) emit(rightId, true);
                //emit(allId, true);
                std::cout << "delete " << i << " at " << simTime() << endl;


                arc += p1->getRunCounter();
                sc += p1->getSwitchCounter();


                module->callFinish();
                module->deleteModule();
                if (currentNumofTarget == numOfTarget) {
                    endSimulation();
                }
                positions.row(i) = Vector3d(-1,-1,-1);
            }
        } else {
            positions.row(i) = Vector3d(-1,-1,-1);
        }
    }

    for (int i=0; i<numOfTarget;i++) {
        Vector3d p1 = positions.row(i);
        if (!p1.isApprox(Vector3d(-1,-1,-1))) {
            for (int j=i+1;j<numOfTarget;j++){
                Vector3d p2 = positions.row(j);
                if (p2.isApprox(Vector3d(-1,-1,-1))) continue;
                Vector3d disV = p1.array()-p2.array();
                double dis = disV.norm();
                if (dis <= minimumSafetyDistance) {
                    /***
                    if (p2->getIsStopByTargetLaneDrone() or p1->getIsStopByTargetLaneDrone()) {

                        double t1 = p1->getLastSwitchTime();
                        double t2 = p2->getLastSwitchTime();
                        DroneLinearMobility *m1 = check_and_cast<DroneLinearMobility *>(module1->getSubmodule("mobility"));
                        DroneLinearMobility *m2 = check_and_cast<DroneLinearMobility *>(module2->getSubmodule("mobility"));
                        double v1 = m1->getMaxSpeed();
                        double v2 = m2->getMaxSpeed();
                        std::cout << "ArrivalChecker:: node " << i << " and node " << j << " collided and will be removed" << endl;
                        std::cout << "ArrivalChecker:: --at time " << simTime() << " and distance is " << dis << endl;
                        std::cout << "ArrivalChecker:: --v1 " << v1 << " and v1 " << v2 << endl;
                        std::cout << "ArrivalChecker:: --t1 " << t1 << " and t2 " << t2 << endl;
                        emit(rightId, simTime().dbl());
                        emit(rightId, t1);
                        emit(rightId, t2);
                        emit(rightId, v1);
                        emit(rightId, v2);

                    }
                    ***/



                    emit(collisionCountId, true);
                    collided.insert(i);
                    collided.insert(j);
                    emit(collisionNodeId,i);
                    emit(collisionNodeId,j);
                    //std::cout << "--------------------------------" << endl;
                    //std::cout << "ArrivalChecker:: node " << i << " and node " << j << " collided and will be removed" << endl;
                    //std::cout << "ArrivalChecker:: --at time " << simTime() << " and distance is " << dis << endl;
                }
            }
        }
    }
    // delete the collided nodes detected above
    set<int>::iterator it;
    for (it=collided.begin();it!=collided.end();it++) {
        cModule *module = getParentModule()->getSubmodule("nodes", *it);
        if (module != nullptr) {

            Processor *p1 = check_and_cast<Processor *>(module->getSubmodule("p"));
            beaconGap += p1->getSeqGap();
            beaconDelay += p1->getTimeGap();
            beaconAmount += p1->getBeaconAmount();
            
            arc += p1->getRunCounter();
            sc += p1->getSwitchCounter();

            //std::cout << p1->getAlgorithm() << endl;

            module->callFinish();
            module->deleteModule();
        }
    }
}


ArrivalChecker::~ArrivalChecker() {
    cancelAndDelete(check);
    cancelAndDelete(count);
}

void ArrivalChecker::finish()
{
    for (int i=0; i<numOfTarget;i++) {
        cModule *module = getParentModule()->getSubmodule("nodes", i);
        if (module != nullptr) {
            Processor *p1 = check_and_cast<Processor *>(module->getSubmodule("p"));
            arc += p1->getRunCounter();
            sc += p1->getSwitchCounter();

	}
    }

    for (const auto& p: densityCount) {
        auto k = p.first;
        auto v = p.second;
        std::string ij = std::to_string(k[0]) + std::to_string(k[1]);
        recordScalar(ij.c_str(), v);
    }
    
    recordScalar("arc", arc);
    recordScalar("sc", sc);

    emit(simTimeId, clock()-startTime);
    emit(aveSeqGapId, beaconGap / beaconAmount);
    emit(aveTimeGapId, beaconDelay / beaconAmount);

    cSimpleModule::finish();
}
