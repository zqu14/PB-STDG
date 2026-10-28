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

#include "inet/common/INETMath.h"
#include "DroneLinearMobility.h"
#include <algorithm>
#include <cmath>

using namespace inet;

Define_Module(DroneLinearMobility);

simsignal_t speedId = cComponent::registerSignal("speed");
simsignal_t speedIntervalId = cComponent::registerSignal("speedInterval");
simsignal_t positionXId = cComponent::registerSignal("positionX");
simsignal_t positionYId = cComponent::registerSignal("positionY");
simsignal_t positionZId = cComponent::registerSignal("positionZ");
simsignal_t endSwitchId = cComponent::registerSignal("endSwitch");
simsignal_t startSwitchId = cComponent::registerSignal("startSwitch");

DroneLinearMobility::DroneLinearMobility()
{
    speed = 0;
}

void DroneLinearMobility::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);

    EV_TRACE << "initializing LinearMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {

        gateToP = gate("toP");
        gatePIn = gate("pIn");

        speed = par("speed");
        if (!std::isfinite(speed) || speed < 0)
            throw cRuntimeError("DroneLinearMobility: speed must be nonnegative and finite");
        stationary = (speed == 0);
        rad heading = deg(fmod(par("initialMovementHeading").doubleValue(), 360));
        rad elevation = deg(fmod(par("initialMovementElevation").doubleValue(), 360));
        direction.x = par("initialDX");
        direction.y = par("initialDY");
        direction.z = par("initialDZ");
        currentI = par("initialI");
        currentJ = par("initialJ");
        updateInterval = 0.05;
        currentPart = par("currentPart").stdstringValue();
        endPart = par("endPart").stdstringValue();
        lastVelocity = direction * speed;
        lastUpdate = simTime();
        recordPosition = new cMessage("recordPosition");
        recordPositionPeriod = par("recordPositionPeriod");
        if (recordPositionPeriod <= SIMTIME_ZERO)
            throw cRuntimeError("DroneLinearMobility: recordPositionPeriod must be positive");
        lastSetSpeed = 0;
        scheduleAt(simTime() + recordPositionPeriod, recordPosition);
        rs = check_and_cast<RoadSystem *>(getParentModule()->getParentModule()->getSubmodule("roadSystem"));
        nextTarget = rs->getWayPoint(n+1, currentPart, Lane(currentI, currentJ));
        lastPosition.x = par("initialX");
        lastPosition.y = par("initialY");
        lastPosition.z = par("initialZ");
        setNextTurnTime();
        emitPosition();
        currentPartSize = rs->getLaneCoord(Lane(currentI, currentJ), currentPart).rows()-1;
        updateCurrentTInfo();
    }
}
void DroneLinearMobility::updateCurrentTInfo() {
    if (currentPartSize <= 0 || n < 0 || n > currentPartSize)
        throw cRuntimeError("DroneLinearMobility: invalid waypoint index");
    currentSLength = 0;
    baseT = (double)n/(double)currentPartSize;
    baseNextT = (double)(n+1)/(double)currentPartSize;
    currentT = baseT;
    currentTotalSLength = n < currentPartSize ? rs->getSLength(currentPart, Lane(currentI, currentJ), n) : 0;

}

void DroneLinearMobility::emitPosition() {
    if (enableEmitPosition) {
        emit(positionXId, lastPosition.x);
        emit(positionYId, lastPosition.y);
        emit(positionZId, lastPosition.z);
    }
}



void DroneLinearMobility::setNextTurnTime() {
    Vector3d current = {lastPosition.x, lastPosition.y, lastPosition.z};
    Vector3d distance = nextTarget.array() - current.array();
    if (lastVelocity.x != 0 and abs(lastVelocity.x) >= abs(lastVelocity.y) and abs(lastVelocity.x) >= abs(lastVelocity.z)) {
        nextTurn = simTime() + distance.coeff(0, 0) / lastVelocity.x;
    } else if (lastVelocity.y != 0 and abs(lastVelocity.y) >= abs(lastVelocity.x) and abs(lastVelocity.y) >= abs(lastVelocity.z) ) {
        nextTurn = simTime() + distance.coeff(1, 0) / lastVelocity.y;
    } else if (lastVelocity.z != 0 and abs(lastVelocity.z) >= abs(lastVelocity.x) and abs(lastVelocity.z) >= abs(lastVelocity.y) ) {
        nextTurn = simTime() + distance.coeff(2, 0) / lastVelocity.z;
    } else {
        nextTurn = simTime();
    }
}


void DroneLinearMobility::handleMessage(cMessage* msg)
{

    if (strcmp(msg->getName(), "move") == 0)
    {
        MovingMobilityBase::handleMessage(msg);
    }

    else if (msg == recordPosition)
    {
        scheduleAt(simTime() + recordPositionPeriod, recordPosition);
    }
    else
    {
        EV << msg->getName() << endl;
        EV << "DroneLinearMobility::handleMessage: got improper message type" << endl;
        endSimulation();
    }
}


void DroneLinearMobility::takeNewTarget(Vector3d v, double nt, double tt) {
    __thisTargetT = tt;
    __nextTarget = v;
    __nextT = nt;
}

double DroneLinearMobility::getCurrentT() {
    return currentT;
}

void DroneLinearMobility::updateCurrentT(double time) {
    if (n >= currentPartSize) { currentT = 1; return; }
    if (currentTotalSLength <= 0)
        throw cRuntimeError("DroneLinearMobility: segment length must be positive");
    currentSLength += std::max(0.0, time) * speed;
    const double fraction = std::min(0.99, currentSLength / currentTotalSLength);
    currentT = baseT + (baseNextT - baseT) * fraction;

}


void DroneLinearMobility::move() {
    if (arrive) return;
    if (speed == 0) return;
    double elapsedTime = (simTime() - lastUpdate).dbl();
    if ((IS_SWTICHING or end_switch) and simTime() >= halfSwitch and halfChangedLane) {
        halfChangedLane = false;
        currentI = _currentI;
        currentJ = _currentJ;
        sendInfo(1);
    }
    double turnTime = (simTime()-nextTurn).dbl();
    if (turnTime >= 0 or START_SWITCHING) {
        Vector3d _nextTarget = {1,1,1};
        emitPosition();
        if (IS_SWTICHING) {
            _nextTarget = SWITCH_PATH[SWITCH_INDEX];
            if (START_SWITCHING) turnTime = elapsedTime;
            SWITCH_INDEX++;
            bool one_path = false;
            if (START_SWITCHING) one_path = true;
            START_SWITCHING = false;

            if (!one_path) {
                n++;
                updateCurrentTInfo();
            }

            if (SWITCH_INDEX >= SWITCH_PATH.size()) {
                Vector3d nn = rs->getWayPoint(n+1, currentPart, Lane(_currentI, _currentJ));
                Vector3d leng = nn.array() - _nextTarget.array();
                if (leng.norm() > 1) {    // original > 3
                    is_final = true;
                }
                IS_SWTICHING = false;
                end_switch = true;
                SWITCH_PATH.clear();
            }

        } else {
            bool already_sent = false;
            if (end_switch) {
                end_switch = false;
                sendInfo(4);
                already_sent = true;
                if ((double)(n+1)/(double)currentPartSize >= __thisTargetT) return;
            }
            if (!is_final) {
                n++;
                updateCurrentTInfo();
            }
            if (currentT >= __thisTargetT and !already_sent) {
                _nextTarget = __nextTarget;
                sendInfo(0);
            } else {
               if ((n+1) >= rs->getLaneCoord(Lane(currentI,currentJ), currentPart).rows()) {
                    arrive = true;
                    //if (endPart == currentPart) right_finish = true;
                    return;
               }
               _nextTarget = rs->getWayPoint(n+1, currentPart, Lane(currentI, currentJ));
            }

            is_final = false;
        }
        Vector3d d = _nextTarget.array() - nextTarget.array();
        Coord _lastPosition  = Coord(nextTarget.coeff(0, 0), nextTarget.coeff(1, 0), nextTarget.coeff(2, 0));
        nextTarget = _nextTarget;
        d.normalize();
        direction = Coord(d.coeff(0,0), d.coeff(1,0), d.coeff(2,0));
        double _speed = speed;
        if (IS_SWTICHING or end_switch) _speed = sqrt(speed*speed+10*10);
        lastVelocity = direction * _speed;
        lastPosition =_lastPosition + lastVelocity * turnTime;
        setNextTurnTime();
        updateCurrentT(turnTime);
    } else {
        lastPosition += lastVelocity * elapsedTime;
        updateCurrentT(elapsedTime);
        EV << n << "    "  << currentT << "   "  << nextTarget << "   "   << nextTurn << endl;
     }



}



//TODO speed is the value toward the line, need another speed toward tangent
void DroneLinearMobility::switchLane(Lane l, double speed, double upper) {

    if (sent_to_rs and speed != 0) {
        rs->setLaneBlock(currentPart, Lane(currentI,currentJ), false);
        sent_to_rs = false;
    }
    halfSwitch = simTime() + 0.5;
    halfChangedLane = true;
    START_SWITCHING = true;
    SWITCH_INDEX = 0;
    int u = round((rs->getLaneCoord(l, currentPart).rows()-1) * upper);

    SWITCH_PATH = rs->getSuitableSwitchPoint(currentPart, Lane(currentI, currentJ), l, currentT, speed, u);
    double dis = rs->getDistance(currentPart, Lane(currentI, currentJ), currentT, upper);

    if (dis < speed * 1) { //TODO 1 is one second, magic number
        if (dis < 0.1) setSpeed(0.1);
        else setSpeed(dis);
        EV << "original speed: " << speed <<  ", actual speed: " << dis << endl;
    }

    double _speed = sqrt(speed*speed+10*10);
    lastVelocity = direction * _speed;

    nextTarget = Vector3d(lastPosition.x,lastPosition.y,lastPosition.z);
    _currentI = l.getI();
    _currentJ = l.getJ();
    IS_SWTICHING = true;

}




void DroneLinearMobility::setNexTPart(std::string id, int t, int i, int j) {
    currentPart = id;
    n = t;
    currentI = i;
    currentJ = j;
    currentPartSize = rs->getLaneCoord(Lane(currentI, currentJ), currentPart).rows()-1;
    updateCurrentTInfo();
}

bool DroneLinearMobility::isArrival()
{
    return arrive;
}

void DroneLinearMobility::setSpeed(double speed)
{
    if (!std::isfinite(speed) || speed < 0)
        throw cRuntimeError("DroneLinearMobility: speed must be nonnegative and finite");


    if (sent_to_rs and speed != 0) {
        rs->setLaneBlock(currentPart, Lane(currentI,currentJ), false);
        sent_to_rs = false;
    }


    if (speed == 0 and rs->getDistance(currentPart, Lane(currentI,currentJ), 0, currentT) < 10) {
        rs->setLaneBlock(currentPart, Lane(currentI,currentJ), true);
        sent_to_rs = true;
    }
    this->speed = speed;
    lastVelocity = direction * speed;
    setNextTurnTime();
}

void DroneLinearMobility::sendInfo(int i){
    MovingInfo* mi = new MovingInfo();
    mi->setType(i);
    take(mi);
    send(mi, gateToP);
}


DroneLinearMobility::~DroneLinearMobility()
{
    cancelAndDelete(recordPosition);
}

void DroneLinearMobility::finish()
{
    double currentTime =  simTime().dbl();
    double timeDiff = currentTime - lastSetSpeed;
    emitPosition();
    MovingMobilityBase::finish();
}


