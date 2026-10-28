/*
 * NeighborTable.h
 *
 *  Created on: Sep 6, 2020
 *      Author: awillig
 */

#ifndef BEACONING_NEIGHBORTABLE_NEIGHBORTABLE_H_
#define BEACONING_NEIGHBORTABLE_NEIGHBORTABLE_H_

#include <map>
#include <list>
#include <iostream>
#include <omnetpp.h>
#include "../base/BeaconingBase.h"
#include "../base/BeaconReport_m.h"
#include "../neighbortable/PacketLossEstimator.h"
#include <inet/common/geometry/common/Coord.h>
#include <inet/common/geometry/common/EulerAngles.h>
#include <inet/linklayer/common/MacAddress.h>


using namespace omnetpp;


// ================================================================
// ================================================================

class NeighborTableEntry {

 protected:
  NodeIdentifier            nbId;
  simtime_t                 lastBeacon;
  Coord                     lastPosition;
  Coord                     lastVelocity;
  uint32_t                  numObservedBeacons;
  EwmaPacketLossEstimator   plrEst;
  double                    estCollsionTime;
  int                       lastI;
  int                       lastJ;
  std::string               lastRoadId;
  double                    lastT;
  double                    distance;
  double       pSpeed;
  int         targetI;
  int         targetJ;
  int         switchToI;
  int         switchToJ;
  double      targetT;
  double       nextAlgorithmTime;
  bool         isSwitching;
  std::string  targetRoadId;

 public:
  NeighborTableEntry();
  NeighborTableEntry(NodeIdentifier id);
  NeighborTableEntry (NodeIdentifier id, simtime_t lastB, Coord lastPos, Coord lastVel, double alpha, double time, int i, int j, std::string rId, double t);
  NeighborTableEntry (MacAddress id, simtime_t lastB, Coord lastPos, Coord lastVel, double alpha, double time, int i, int j, std::string rId, double t, int ti, int tj, int tt, bool is, double nat, int si, int sj, double pS, std::string tId);

  NodeIdentifier getNeighborId() const {return nbId;};
  simtime_t  getLastBeaconReceptionTime() const {return lastBeacon;};
  uint32_t   getLastSequenceNumber() const {return plrEst.getLastSequenceNumber();};
  Coord      getLastPosition() const {return lastPosition;};
  Coord      getLastVelocity() const {return lastVelocity;};
  double     getEstCollsionTime() const {return estCollsionTime;};
  double     getPacketLossRate() const {return plrEst.getCurrentPacketLossRate();};
  uint32_t   getNumObservedBeacons() const {return numObservedBeacons;};
  int        getLastI() const {return lastI;};
  int        getLastJ() const {return lastJ;};
  double     getLastT() const {return lastT;}
  std::string getLastRoadId() const {return lastRoadId;};
  std::string getTargetRoadId() const {return targetRoadId;};
  double     getDistance() const {return distance;};
  int        getTargetI() const {return targetI;};
  int        getTargetJ() const {return targetJ;};
  double     getTargetT() const {return targetT;};
  int        getSwitchI() const {return switchToI;};
  int        getSwitchJ() const {return switchToJ;};
  bool       getIsSwitching() const {return isSwitching;};
  double     getPSpeed() const {return pSpeed;};
  double     getNextAlgorithmTime() const {return nextAlgorithmTime;};

  EwmaPacketLossEstimator& getEstimator() {return plrEst;};

  void setDistance(double d) {distance=d;};
  void setLastBeaconReceptionTime(simtime_t lastt) {lastBeacon=lastt;};
  void setLastPosition(Coord lastp) {lastPosition=lastp;};
  void setLastVelocity(Coord lastv) {lastVelocity=lastv;};
  void incrNumObservedBeacons() {numObservedBeacons += 1;};
  void setEstCollsionTime(double time) {estCollsionTime=time;};
  void setLastI(int i) {lastI = i;};
  void setLastJ(int j) {lastJ = j;};
  void setLastT(double t) {lastT = t;};
  void setTargetI(int ti) {targetI = ti;};
  void setTargetJ(int tj) {targetJ = tj;};
  void setSwitchI(int si) {switchToI = si;};
  void setSwitchJ(int sj) {switchToJ = sj;};
  void setTargetT(double t) {targetT = t;};
  void setPSpeed(double s) {pSpeed = s;};
  void setIsSwitching(bool is) {isSwitching = is;};
  void setNextAlgorithmTime(double nat) {nextAlgorithmTime = nat;}
  void setLastRoadId(std::string id) {lastRoadId = id;}
  void setTargetRoadId(std::string id) {targetRoadId = id;}

  friend std::ostream& operator<<(std::ostream& os, const NeighborTableEntry& nte);
};


// ================================================================
// ================================================================

class NeighborTable : public cSimpleModule {
 protected:

  std::map<NodeIdentifier, NeighborTableEntry>  nbTable;

  double     ewmaAlpha;
  simtime_t  timeoutTime;
  simtime_t  cleanupPeriod;

  cMessage  *pMsgTableCleanup;

 public:

  virtual void initialize(int stage);
  virtual void finish();
  virtual void handleMessage(cMessage *msg);
  virtual int numInitStages () const override { return inet::NUM_INIT_STAGES;};

 public:

  NeighborTable () { ewmaAlpha = 0.95; timeoutTime = 10.0; };
  NeighborTable (double alpha, double timeout);

  double getEwmaAlpha () const {return ewmaAlpha;};
  void   setEwmaAlpha (double alpha);

  double getTimeoutTime () const {return timeoutTime.dbl();};
  void   setTimeoutTime (simtime_t timeout);

  double getCleanupPeriod () const {return cleanupPeriod.dbl();};
  void   setCleanupPeriod (simtime_t period);

  void recordBeacon(const BeaconReport* beaon);
  void recordBeacon(const Beacon* beaon);
  void cleanupTable (void);

  bool entryAvailable (NodeIdentifier id) {return nbTable.find(id) != nbTable.end();};
  NeighborTableEntry& operator[] (const NodeIdentifier& id) {return nbTable[id];};
  size_t size (void) const {return nbTable.size();};
  std::list<NodeIdentifier> getNeighborIdList (void);
  NeighborTableEntry getEntry(NodeIdentifier id) {return nbTable[id];};
  friend std::ostream& operator<<(std::ostream& os, const NeighborTable& nt);
};





#endif /* BEACONING_NEIGHBORTABLE_NEIGHBORTABLE_H_ */
