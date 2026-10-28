/*
 * RoadPath.h
 *
 *  Created on: Apr 11, 2024
 *      Author: zqu14
 */

#ifndef ROADSYSTEM_ROADPATH_H_
#define ROADSYSTEM_ROADPATH_H_

#include <vector>
#include <string>
#include <stdexcept>
#include "Bezier.h"
#include <Eigen/Dense>
#include"RoadPart.h"
#include"RoadConnect.h"
#include<map>
#include<iostream>

class RoadPath {
public:
    RoadPath();
    virtual ~RoadPath();
    void setNext(std::string id, Lane fromlane, Lane tolane, Vector3d coord, double fromt, double tot);
    void toNextPoint();
    void reset();
    std::string getNextId();
    Lane getFromLane();
    Lane getToLane();
    Vector3d getNextCoord();
    double getFromT();
    double getToT();
    std::string getStr();
    void setStart(std::string start);
    void addLength(double l){length += l;};
    double getLength(){return length;};

private:
    double length = 0.0;
    int index = 0;
    int roadPoint = -1;
    std::string start;
    std::vector<std::string> nexts;
    std::vector<Lane> fromLane;
    std::vector<Lane> toLane;
    std::vector<Vector3d> nextsCoord;
    std::vector<double> fromT;
    std::vector<double> toT;

};

#endif /* ROADSYSTEM_ROADPATH_H_ */
