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


#include <iostream>
#include <fstream>
#include <inet/common/ModuleAccess.h>
#include "NodeGenerator2.h"
#include "inet/visualizer/scene/NetworkNodeCanvasVisualizer.h"
#include <inet/mobility/contract/IMobility.h>
#include "inet/common/scenario/ScenarioManager.h"
#include <algorithm>
#include <random>
Define_Module(NodeGenerator2);

simsignal_t generateCountId = cComponent::registerSignal("generateCount");
simsignal_t failCountId = cComponent::registerSignal("failCount");


void NodeGenerator2::nextGenerateFromTable() {

    double time = generateTimeTable.begin()->first;
    LaneInfo li = generateTimeTable.begin()->second;
    generateTimeTable.erase(time);

    simtime_t nextTime = simTime()+par("renewalDistribution");
    generateTimeTable[nextTime.dbl()] = li;

    time = generateTimeTable.begin()->first;
    if (currentNum < maxNum) {
        //std::cout << "===================================================================\n";
        //std::cout << "start generate"<< li.id << "   " <<    currentNum << "\n";
        generate(li);
        //std::cout << "===================================================================\n";
        scheduleAt(time, startGenerating1);
    }
}



void NodeGenerator2::addAllLaneGeneration() {
    //std::cout << "=========================start input all lanes\n";
    std::vector<EntryInfo> eis = rs->getAllEntry();
    for (auto&i : eis) {
        LaneInfo li;
        li.id = i.id;
        li.startId = i.pointId;
        li.x = i.coord.coeff(0,0);
        li.y = i.coord.coeff(1,0);
        li.z = i.coord.coeff(2,0);
        li.i = i.i;
        li.j = i.j;
        li.dx = i.direction.coeff(0, 0);
        li.dy = i.direction.coeff(1, 0);
        li.dz = i.direction.coeff(2, 0);
        simtime_t nextTime = simTime()+par("renewalDistribution");
        generateTimeTable[nextTime.dbl()] = li;
        //std::cout << li.id << "  " << nextTime.dbl() << "  "<< simTime().dbl() << endl;

        LastTime[li.id] = 0;
        LastSpeed[li.id] = 1000;
    }


    simtime_t time = generateTimeTable.begin()->first;
    if (currentNum < maxNum) {
         scheduleAt(simTime()+time, startGenerating1);
    }

}




void NodeGenerator2::initialize(int stage) {
    if (stage == 0) {
        currentNum = 0;
        centerX = par("centerX");
        centerZ = par("centerZ");
        maxNum = par("maxNumOfNodes");
        startGenerating1 = new cMessage("startGenerating1");
        count = new cMessage("count");
        radius = par("radius").doubleValue();
        failGenerateCount = 0;
        failCount = 0;
        generateCount = 0;
    }
    if (stage == INITSTAGE_LAST)
    {
        cModule *host = getParentModule();
        rs = check_and_cast<RoadSystem *>(host->getSubmodule("roadSystem"));
        addAllLaneGeneration();
        scheduleAt(simTime(), count);
    }
}

int NodeGenerator2::numInitStages() const {
    return 13;
}
void NodeGenerator2::handleMessage(cMessage* msg) {
    if (msg == startGenerating1) {
        nextGenerateFromTable();
    } else if (msg == count) {
        countNode();
        scheduleAt(simTime()+1, count);
    } else {
        error("NodeGenerator::handleMessage Unknown message");
    }
}

void NodeGenerator2::countNode() {
    emit(generateCountId, generateCount);
    emit(failCountId, failCount);
    failCount = 0;
    generateCount = 0;
}


void NodeGenerator2::generate(LaneInfo li) {

    double numOfNodes = par("numOfNodesDistribution");
    std::string id = li.id;
    double laneX = li.x;
    double laneY = li.y;
    double laneZ = li.z;
    double dx = li.dx;
    double dy = li.dy;
    double dz = li.dz;
    std::string startId = li.startId;
    int lanei = li.i;
    int lanej = li.j;
    std::string exit = rs->getRandomExit(startId);
    while (numOfNodes > 0) {
        if (currentNum >= maxNum) return;
        // record the information of generated node to prevent the immediate collision
        double thisLaneLastSpwanTime = LastTime[id];
        double thisLaneLastSpwanSpeed = LastSpeed[id];
        double maxSpeed = par("maxSpeedDistribution").doubleValue();
        double preferredSpeed = maxSpeed * par("preferredSpeedPercent").doubleValue();
        // estimate the next collision time, if this time is too short, drop this generation
        double lastDronePosition = (simTime().dbl()-thisLaneLastSpwanTime) * thisLaneLastSpwanSpeed;
        double collisionTime = (lastDronePosition-0.5) / (preferredSpeed-thisLaneLastSpwanSpeed);
        if (rs->getLaneBlock(startId, Lane(lanei,lanej)) || (collisionTime >0 && collisionTime <= 1.4) || (lastDronePosition <= 7 && thisLaneLastSpwanSpeed!=0)) {
            failGenerateCount++;
            failCount++;
            numOfNodes--;
            continue;
        } else {
            LastTime[id] = simTime().dbl();
            LastSpeed[id] = preferredSpeed;
            generateCount++;
        }
        cModuleType *moduleType = cModuleType::get("swarmstack.beaconing.renewalbeaconing.RenewalBeaconingNodeSafety");
        cModule *mod = moduleType->create("nodes", this->getParentModule(),maxNum,currentNum);
        currentNum++;
        numOfNodes--;
        // set up parameters and gate sizes before we set up its submodules

        mod->par("osgModel") = "3d/drone.ive.5.scale.0,0,90.rot";
        mod->getDisplayString().parse("p=200,100;i=misc/node_s");
        mod->finalizeParameters();

        // create internals, and schedule it
        mod->buildInside();
        auto *mobility = mod->getSubmodule("mobility");

        // if the mobility parameters have been set in .ini file, do not initialize it
        if (mobility->par("speed").doubleValue() == 0) {
            mobility->par("speed") = preferredSpeed;
            mobility->par("initialX") = laneX;
            mobility->par("initialY") = laneY;
            mobility->par("initialZ") = laneZ;
            mobility->par("initialDX") = dx;
            mobility->par("initialDY") = dy;
            mobility->par("initialDZ") = dz;
            mobility->par("initialI") = lanei;
            mobility->par("initialJ") = lanej;
            mobility->par("currentPart") = startId;
            mobility->par("endPart") = exit;
            mod->getSubmodule("rb")->par("preferredSpeed") = preferredSpeed;
            mod->getSubmodule("rb")->par("maxSpeed") = maxSpeed;
        } else {
            if (mobility->par("speed").doubleValue() == -1) mobility->par("speed") = 0.0;
            mod->getSubmodule("rb")->par("preferredSpeed") = mobility->par("speed").doubleValue();
            mod->getSubmodule("rb")->par("maxSpeed") = mobility->par("speed").doubleValue();
        }
        // initial start and end road
        auto *processor = mod->getSubmodule("p");
        processor->par("fisrtPart") = startId;
        processor->par("endPart") = exit;
        processor->par("currentI") = lanei;
        processor->par("currentJ") = lanej;

        //std::cout << "time: " << simTime().dbl() << " id: "<< id << " xyz: "<< laneX << " " <<
        //        laneY << " " << laneZ << endl;

        // Send signal to simulator and put the new node into simulation network
        mobility->par("initialMovementHeading") = 0.0;
        inet::cPreModuleInitNotification pre;
        pre.module = mod;
        emit(POST_MODEL_CHANGE, &pre);
        mod->callInitialize();
        inet::cPostModuleInitNotification post;
        post.module = mod;
        emit(POST_MODEL_CHANGE, &post);
        mod->scheduleStart(simTime());
    }


}



NodeGenerator2::NodeGenerator2() {
    // TODO Auto-generated constructor stub

}

NodeGenerator2::~NodeGenerator2() {
    cancelAndDelete(startGenerating1);
}

void NodeGenerator2::finish() {
    //std::cout << currentNum << endl;
    //std::cout << failGenerateCount << endl;
}


