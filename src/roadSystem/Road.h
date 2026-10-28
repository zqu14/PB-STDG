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

#ifndef ROADSYSTEM_ROAD_H_
#define ROADSYSTEM_ROAD_H_

#include <map>
#include <vector>
#include "RoadPart.h"
#include <string>
#include<Eigen/Dense>
using namespace Eigen;


class Road :public RoadPart{
public:
    Road();
    virtual ~Road();
    virtual MatrixXd getE1() override;
    virtual MatrixXd getE2() override;
    void calculateE1();
    void calculateE2();
    void checkHeadTail();
    void calculateT();
    Vector3d twoReflect(int i);
    double reflectPlane(Vector3d p1, Vector3d p2, Vector3d N);
    Vector3d reflectN(Vector3d p1, Vector3d p2);
    Vector3d reflectPoint(Vector3d N, double plane, Vector3d p);
    Vector3d reflectPT(Vector3d N, double plane, Vector3d p1, Vector3d V);
    Vector3d reflectVT(Vector3d pt, Vector3d p2);
    void construct();

private:
    Vector3d r1 = {0,0,1};
    Vector3d r2 = {0,1,0};
    MatrixXd e1;
    MatrixXd e2;
    MatrixXd t;
    Vector3d last_e1;

};

#endif /* ROADSYSTEM_ROAD_H_ */
