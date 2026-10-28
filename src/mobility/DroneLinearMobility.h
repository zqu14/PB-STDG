//
// Author: Emin Ilker Cetinbas (niw3_at_yahoo_d0t_com)
// Copyright (C) 2005 Emin Ilker Cetinbas
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef MOBILITY_DRONELINEARMOBILITY_H_
#define MOBILITY_DRONELINEARMOBILITY_H_

#include "inet/common/INETDefs.h"
#include "inet/mobility/base/MovingMobilityBase.h"
#include "../roadSystem/RoadSystem.h"
#include "MovingInfo_m.h"
#include <omnetpp.h>
using namespace omnetpp;
using namespace inet;


#ifndef KEEP
#define KEEP        -9999
#endif

class DroneLinearMobility : public MovingMobilityBase
{
  protected:
    bool sent_to_rs = false;
    double baseT = 0;
    double baseNextT = 0;
    double currentT = 0;
    double currentTotalSLength = 0;
    double currentSLength = 0;
    int currentPartSize = 0;
    bool is_final = false;
    bool end_switch = false;
    simtime_t halfSwitch;
    bool halfChangedLane = false;
    bool START_SWITCHING = false;
    std::vector<Vector3d> SWITCH_PATH;
    int SWITCH_INDEX = 0;
    bool IS_SWTICHING = false;
    double __thisTargetT;
    double speed;
    Coord direction;
    cMessage* recordPosition;
    simtime_t recordPositionPeriod;
    double lastSetSpeed;
    int n=0;
    int currentI;
    int currentJ;
    int _currentI;
    int _currentJ;
    RoadSystem* rs = nullptr;
    Vector3d nextTarget;
    Vector3d __nextTarget;
    std::string currentPart;
    std::string endPart;
    simtime_t nextTurn;
    int __nextT=-1;
    int nextT=-1;
    Lane nextLane;
    int currentClose=1;
    bool switching = false;
    bool arrive = false;
    bool isShortSwitch = false;
    bool enableEmitPosition = true;
    bool enableEmitSwitch = true;
    bool startSwitching = false;
    cGate* gateToP = nullptr;
    cGate* gatePIn = nullptr;

  protected:

    void updateCurrentT(double time);
    void updateCurrentTInfo();

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;

  public:
    void takeNewTarget(Vector3d v, double nt, double tt);
    void setNexTPart(std::string id, int t, int i, int j);
    void setNextTurnTime();
    void setCurrentClose(int t);
    void emitPosition();
    int getCurrI() {return currentI;}
    int getCurrJ() {return currentJ;}
    std::string getcurrRoadId() {return currentPart;}
    double getCurrentT();
    void switchLane(Lane l, double speed, double upper);
    virtual double getMaxSpeed() const override { return speed; }
    DroneLinearMobility();
    void setSpeed(double speed);
    void handleMessage(cMessage* msg);
    bool isArrival();
    void sendInfo(int i);
    virtual ~DroneLinearMobility();
    virtual void move() override;
    bool getIsSwitching(){return IS_SWTICHING or end_switch;} ;
};


#endif /* MOBILITY_DRONELINEARMOBILITY_H_ */

