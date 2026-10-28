/*
 * NeighborTable.cc
 *
 *  Created on: Sep 6, 2020
 *      Author: awillig
 */


#include "NeighborTable.h"
#include <inet/common/packet/Packet.h>

Define_Module(NeighborTable);


using namespace inet;

// ===============================================================================
// ===============================================================================
//
// class NeighborTableEntry
//
// ===============================================================================
// ===============================================================================

// ----------------------------------------------------

NeighborTableEntry::NeighborTableEntry()
  :nbId(nullIdentifier),
   lastBeacon(simTime()),
   lastPosition(Coord(0,0,0)),
   lastVelocity(Coord(0,0,0)),
   numObservedBeacons(0),
   plrEst(0.95),
   estCollsionTime(-1),
   lastI(0),
   lastJ(0),
   lastT(0),
   lastRoadId("none"),
   distance(-99999),
   targetI(9999),
   targetJ(9999),
   targetT(-1),
   isSwitching(false),
   nextAlgorithmTime(-1),
   switchToI(9999),
   switchToJ(9999),
   pSpeed(15),
   targetRoadId("none")
{
}

// ----------------------------------------------------

NeighborTableEntry::NeighborTableEntry(MacAddress id)
  :nbId(id),
   lastBeacon(simTime()),
   lastPosition(Coord(0,0,0)),
   lastVelocity(Coord(0,0,0)),
   numObservedBeacons(0),
   plrEst(0.95),
   estCollsionTime(-1),
   lastI(0),
   lastJ(0),
   lastT(0),
   lastRoadId("none"),
   distance(-99999),
   targetI(9999),
   targetJ(9999),
   targetT(-1),
   isSwitching(false),
   nextAlgorithmTime(-1),
   switchToI(9999),
   switchToJ(9999),
   pSpeed(15),
   targetRoadId("none")
{
}


// ----------------------------------------------------

NeighborTableEntry::NeighborTableEntry (MacAddress id, simtime_t lastB, Coord lastPos, Coord lastVel, double alpha, double time, int i, int j, std::string rId, double t, int ti, int tj, int tt, bool is, double nat, int si, int sj,  double pS, std::string tId)
  :nbId(id),
   lastBeacon(lastB),
   lastPosition(lastPos),
   lastVelocity(lastVel),
   numObservedBeacons(1),
   plrEst(alpha),
   estCollsionTime(time),
   lastI(i),
   lastJ(j),
   lastRoadId(rId),
   lastT(t),
   distance(-99999),
   targetI(ti),
   targetJ(tj),
   targetT(tt),
   isSwitching(is),
   nextAlgorithmTime(nat),
   switchToI(si),
   switchToJ(sj),
   pSpeed(pS),
   targetRoadId(tId)
{
}
// ----------------------------------------------------

NeighborTableEntry::NeighborTableEntry (MacAddress id, simtime_t lastB, Coord lastPos, Coord lastVel, double alpha, double time, int i, int j, std::string rId, double t)
  :nbId(id),
   lastBeacon(lastB),
   lastPosition(lastPos),
   lastVelocity(lastVel),
   numObservedBeacons(1),
   plrEst(alpha),
   estCollsionTime(time),
   lastI(i),
   lastJ(j),
   lastRoadId(rId),
   lastT(t),
   distance(-99999),
   targetI(9999),
   targetJ(9999),
   targetT(-1),
   isSwitching(false),
   nextAlgorithmTime(-1),
   switchToI(9999),
   switchToJ(9999),
   pSpeed(15),
   targetRoadId("none")
{
}
// ----------------------------------------------------

std::ostream& operator<<(std::ostream& os, const NeighborTableEntry& nte)
{
  int     oldprec = os.precision();
  double  plr     = nte.plrEst.getCurrentPacketLossRate();
  os.precision(2);
  os << "NB{id=" << nte.nbId
     << ", obs=" << nte.numObservedBeacons
     << ", plr=" << plr
    //<< ", lastb=" << nte.lastBeacon
    //<< ", lastp=" << nte.lastPosition
    //<< ", lastv=" << nte.lastVelocity
     << "}";
  os.precision(oldprec);
  return os;
}


// ===============================================================================
// ===============================================================================
//
// class NeighborTable
//
// ===============================================================================
// ===============================================================================

// ----------------------------------------------------

NeighborTable::NeighborTable (double alpha, double timeout)
{
  setEwmaAlpha(alpha);
  setTimeoutTime(timeout);
}


// ----------------------------------------------------

void NeighborTable::setEwmaAlpha (double alpha)
{
  assert ((0 <= alpha) && (alpha <= 1));
  ewmaAlpha = alpha;

  for (auto it = nbTable.begin(); it != nbTable.end(); ++it)
    {
      (it->second).getEstimator().setAlpha(alpha);
    }
}


// ----------------------------------------------------

void NeighborTable::setTimeoutTime (simtime_t timeout)
{
  assert ((0 < timeout.dbl()));
  timeoutTime = timeout;
}

// ----------------------------------------------------

void NeighborTable::setCleanupPeriod (simtime_t period)
{
  assert ((0 < period.dbl()));
  cleanupPeriod = period;
}



// ----------------------------------------------------


void NeighborTable::initialize (int stage)
{
  if (stage == INITSTAGE_LOCAL)
    {
      double alpha   =   par ("plrEstimatorAlpha");
      double timeout =   par ("tableTimeoutValue");
      double period  =   par ("tableCleanupPeriod");

      setEwmaAlpha (alpha);
      setTimeoutTime (timeout);
      setCleanupPeriod (period);

      pMsgTableCleanup = new cMessage("NeighborTable::Cleanup");
      assert(pMsgTableCleanup);
      scheduleAt(simTime() + cleanupPeriod, pMsgTableCleanup);
    }
}


// ----------------------------------------------------

void NeighborTable::finish()
{
}

// ----------------------------------------------------

void NeighborTable::handleMessage(cMessage *msg)
{
  if (msg == pMsgTableCleanup)
    {
      cleanupTable();
      scheduleAt (simTime()+cleanupPeriod, pMsgTableCleanup);
      return;
    }

  EV << "NeighborTable::handleMessage: got improper message type" << endl;
  endSimulation();
  //auto packet = check_and_cast<Packet *>(msg);
  //const auto& beacon    = packet->popAtFront<SwarmBeacon>();  // ### Check msg-getKind() before doing this
  //recordBeacon(&(*beacon));
  //delete msg;
}

// ----------------------------------------------------
void NeighborTable::recordBeacon (const Beacon* beacon)
{
  assert(beacon);
  MacAddress  id      = beacon->getSenderId();
  simtime_t   rxTime  = simTime();
  Coord       pos     = beacon->getCurrPosition();
  Coord       vel     = beacon->getCurrVelocity();
  uint32_t    seqno   = beacon->getSeqno();
  int         i       = beacon->getCurrI();
  int         j       = beacon->getCurrJ();
  std::string rId     = beacon->getCurrentRoadId();
  double      t       = beacon->getCurrT();
  if (entryAvailable(id))
    {
      auto entry = nbTable[id];
      entry.setLastBeaconReceptionTime (rxTime);
      entry.setLastPosition (pos);
      entry.setLastVelocity(vel);
      entry.incrNumObservedBeacons();
      entry.getEstimator().recordObservation(seqno, rxTime);
      entry.setLastI(i);
      entry.setLastJ(j);
      entry.setLastRoadId(rId);
      entry.setLastT(t);
      nbTable[id] = entry;
    }
  else
    {
      NeighborTableEntry newEntry = NeighborTableEntry(id, rxTime, pos, vel, ewmaAlpha, -1, i, j, rId, t);
      nbTable[id] = newEntry;
    }
  std::cout << 1111111111 << endl;
}

void NeighborTable::recordBeacon (const BeaconReport* beacon)
{
  assert(beacon);
  MacAddress  id      = beacon->getSenderId();
  simtime_t   rxTime  = simTime();
  Coord       pos     = beacon->getSenderPosition();
  Coord       vel     = beacon->getSenderVelocity();
  uint32_t    seqno   = beacon->getSeqno();
  int         i       = beacon->getCurrI();
  int         j       = beacon->getCurrJ();
  std::string rId     = beacon->getCurrentRoadId();
  std::string tId     = beacon->getTargetRoadId();
  double      t       = beacon->getCurrT();
  //if (t <= 0) t = 0.0001;
  int         targetI = beacon->getTargetI();
  int         targetJ = beacon->getTargetJ();
  double      targetT = beacon->getTargetT();
  bool        iS      = beacon->getIsSwitching();
  double      nAT     = beacon->getNextAlgorithmTime();
  int         switchI = beacon->getSwitchToI();
  int         switchJ = beacon->getSwitchToJ();
  double      pS      = beacon->getPSpeed();
  if (entryAvailable(id))
    {
      auto entry = nbTable[id];
      entry.setLastBeaconReceptionTime (rxTime);
      entry.setLastPosition (pos);
      entry.setLastVelocity(vel);
      entry.incrNumObservedBeacons();
      entry.getEstimator().recordObservation(seqno, rxTime);
      entry.setLastI(i);
      entry.setLastJ(j);
      entry.setLastRoadId(rId);
      entry.setLastT(t);
      entry.setTargetI(targetI);
      entry.setTargetJ(targetJ);
      entry.setTargetT(targetT);
      entry.setIsSwitching(iS);
      entry.setNextAlgorithmTime(nAT);
      entry.setSwitchI(switchI);
      entry.setSwitchJ(switchJ);
      entry.setPSpeed(pS);
      entry.setTargetRoadId(tId);
      nbTable[id] = entry;


      if (std::isnan(t)) {
          std::cout << id << " " << t << " " << nAT << " " << pS << "  "<< simTime() << ""   << rId << endl;
       }

    }
  else
    {
      NeighborTableEntry newEntry = NeighborTableEntry(id, rxTime, pos, vel, ewmaAlpha, -1, i, j, rId, t, targetI, targetJ, targetT, iS, nAT, switchI, switchJ, pS, tId);
      nbTable[id] = newEntry;
    }
}

// ----------------------------------------------------


void NeighborTable::cleanupTable (void)
{
    simtime_t currTime = simTime();

    for (auto it = nbTable.begin(); it != nbTable.end();)
    {
        auto entry = it->second;
        simtime_t diff = currTime - entry.getLastBeaconReceptionTime();
        if (diff > timeoutTime)
        {
            nbTable.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

// ----------------------------------------------------

std::list<NodeIdentifier> NeighborTable::getNeighborIdList(void)
{
    std::list<NodeIdentifier> result;
    for (auto it = nbTable.begin(); it != nbTable.end(); ++it)
    {
        result.push_front(it->first);
    }
    return result;
}


// ----------------------------------------------------

std::ostream& operator<<(std::ostream& os, const NeighborTable& nt)
{
  int oldprec = os.precision();
  os.precision(2);
  os << "NeighborTable[alpha=" << nt.ewmaAlpha
     << ", timeout=" << nt.timeoutTime
     << ", neighbors=";
  for (auto it = nt.nbTable.begin(); it != nt.nbTable.end(); ++it)
    {
      os << it->second;
      if (std::next(it) != nt.nbTable.end())
    os << ", ";
    }
  os << "]";
  os.precision(oldprec);
  return os;
}


