/*
 * RoadPath.cpp
 *
 *  Created on: Apr 11, 2024
 *      Author: zqu14
 */

#include "RoadPath.h"

RoadPath::RoadPath() {
    // TODO Auto-generated constructor stub

}

RoadPath::~RoadPath() {
    // TODO Auto-generated destructor stub
}


void RoadPath::reset() {
    index = 0;
}

void RoadPath::setNext(std::string id, Lane fromlane, Lane tolane, Vector3d coord, double fromt, double tot) {
    nexts.push_back(id);
    fromLane.push_back(fromlane);
    toLane.push_back(tolane);
    nextsCoord.push_back(coord);
    fromT.push_back(fromt);
    toT.push_back(tot);
    roadPoint++;
}

void RoadPath::toNextPoint() {
    //if (index<roadPoint)
    index++;
}

std::string RoadPath::getNextId() {
    if (index<=roadPoint) return nexts[index];
    return "None";
}
Lane RoadPath::getFromLane() {
    if (index<=roadPoint) return fromLane[index];
    return Lane(-9999,-9999);
}
Lane RoadPath::getToLane() {
    if (index<=roadPoint) return toLane[index];
    return Lane(-9999,-9999);
}
Vector3d RoadPath::getNextCoord() {
    if (index<=roadPoint) return nextsCoord[index];
    return Vector3d(0,0,0);
}
double RoadPath::getFromT() {
    if (index<=roadPoint) return fromT[index];
    return -9999;
}
double RoadPath::getToT() {
    if (index<=roadPoint) return toT[index];
    return -9999;
}
void RoadPath::setStart(std::string start) {
    this->start = start;
}

std::string RoadPath::getStr() {

    std::string str = "Start --> " + start + "\n";
    for (int i=0; i<=roadPoint; i++) {
        Vector3d v = getNextCoord();
        str = str + "        | (" + std::to_string(getFromLane().getI()) + "," + std::to_string(getFromLane().getJ()) + ") "
                + std::to_string(getFromT()) + "\n";
        str = str + "        |\n";
        str = str + "        |\n";
        str = str + "        | (" + std::to_string(v[0]) + "," + std::to_string(v[1]) + "," + std::to_string(v[2]) + ")\n";
        str = str + "        |\n";
        str = str + "        |\n";
        str = str + "        V (" + std::to_string(getToLane().getI()) + "," + std::to_string(getToLane().getJ()) + ") "
                + std::to_string(getToT()) + "\n";
        str = str + "        " + getNextId();
        if (i==roadPoint) str = str + " --> End";
        str = str + "\n";
        index++;
    }
    str = str + "Total Length: " + std::to_string(length) + "\n";
    str = str + "===================path end=================\n";
    index = 0;
    return str;

}

