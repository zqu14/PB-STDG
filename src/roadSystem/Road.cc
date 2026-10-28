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

#include "Road.h"
#include <string>
#include <cmath>
#include <iostream>

Road::Road() {
    // TODO Auto-generated constructor stub
    points.resize(1, 3);
    type = "road";
}



void Road::checkHeadTail() {
    int r = points.rows();
    double tailX = points.coeff(r-1, 0);
    double tailY = points.coeff(r-1, 1);
    double tailZ = points.coeff(r-1, 2);
    double headX = points.coeff(0, 0);
    double headY = points.coeff(0, 1);
    double headZ = points.coeff(0, 2);

    if (std::abs(tailX-headX) < 0.001 and std::abs(tailY-headY) < 0.001 and std::abs(tailZ-headZ) < 0.001 ){
        points.block(r-1, 0, 0, 3) = points.bottomRows(0);
        points.conservativeResize(r-1,3);
    }
}


MatrixXd Road::getE1() {
    return e1;
}

MatrixXd Road::getE2() {
    return e2;
}

void Road::construct() {
    checkHeadTail();
    calculateT();
    calculateE1();
}

void Road::calculateE2() {
    MatrixXd v;
    v.resize(1,3);
    for(int i = 0; i < e1.rows(); i++){

        Vector3d a = e1.row(i);
        Vector3d b = t.row(i);

        if (i==0) {
            v.row(0) = a.cross(b);
        } else {
            v.conservativeResize(v.rows()+1, v.cols());
            v.row(v.rows()-1) = a.cross(b);
        }

    }
    e2 = e1.array() * 1/2 + std::sqrt(3)/2*v.array();
}



void Road::calculateE1() {
    e1.resize(1,3);
    Vector3d t0 = t.row(0);

    if (std::abs(t.coeff(0, 0)-0)<=0.00000001 and std::abs(t.coeff(0, 1)-0)<=0.00000001) {  // TODO check whether == r1
        r2 << t0.cross(r2);
        e1 << r2;
        last_e1 << r2;
    } else {
        r1 << t0.cross(r1);
        e1.row(0)= r1.normalized();
        last_e1 << r1.normalized();
    }

    for (int i=0; i<points.rows()-1; i++) {
        Vector3d thisE1 = twoReflect(i).normalized();
        e1.conservativeResize(e1.rows()+1, e1.cols());
        e1.row(e1.rows()-1) = thisE1;
    }
    calculateE2();
}

Vector3d Road::twoReflect(int i) {

    Vector3d p1 = points.row(i);
    Vector3d p2 = points.row(i+1);

    Vector3d N = reflectN(p1, p2);



    double plane = reflectPlane(p1, p2, N);

    // get first reflected e1 and t1 at point2

    Vector3d _e1 = last_e1;
    Vector3d p_e1_t = reflectPT(N, plane, p1, _e1);
    Vector3d _e1_t = reflectVT(p_e1_t, p2);

    Vector3d t1 = t.row(i);
    Vector3d p_t1_t = reflectPT(N, plane, p1, t1);
    Vector3d t1_t = reflectVT(p_t1_t, p2);


    // get second reflected e2

    Vector3d t2 = t.row(i+1);
    Vector3d p_t2 = p2 + t2;
    N = reflectN(p_t1_t, p_t2);
    plane = reflectPlane(p_t1_t, p_t2, N);
    Vector3d p_e2 = reflectPoint(N, plane, p_e1_t);
    Vector3d e2 = p_e2 - p2;

    e2 = e2.unaryExpr([](double d) {
      if (std::abs(d) <= 0.0000001) {
          return 0.0;
      } else return d;
    });
    last_e1 = e2;
    return e2;
}


Vector3d Road::reflectN(Vector3d p1, Vector3d p2) {
    return (p1-p2).normalized();
}

double Road::reflectPlane(Vector3d p1, Vector3d p2, Vector3d N) {

    Vector3d first = ((p1+p2)/2);
    auto a = first.dot(N);
    return a;
}


Vector3d Road::reflectPoint(Vector3d N, double plane, Vector3d p){
    double value = p.dot(N) - plane;
    if (value > 0) {
        return p - 2 * std::abs(value) * N;
    } else {
        return p + 2 * std::abs(value) * N;
    }
}



Vector3d Road::reflectPT(Vector3d N, double plane, Vector3d p1, Vector3d V){

    Vector3d p = p1 + V;
    return reflectPoint(N, plane, p);
}

Vector3d Road::reflectVT(Vector3d pt, Vector3d p2){
    return pt - p2;
}



void Road::calculateT() {

    MatrixXd p1 = points;
    MatrixXd p2 = points;
    int r = points.rows();
    p1.block(0, 0, r-1, 3) = p1.bottomRows(r-1);
    p1.conservativeResize(r-1,3);
    p2.block(r-1, 0, 0, 3) = p2.bottomRows(0);
    p2.conservativeResize(r-1,3);

    MatrixXd tempT(points.rows()-1, 3);
    tempT = p1-p2;
    tempT = tempT.unaryExpr([](double d) {
      if (std::abs(d) <= 0.0000001) {
          return 0.0;
      } else return d;
    });

    // first t
    t.resize(1,3);
    t << tempT.row(0).normalized();

    // middle t
    for (int i=0; i<r-2; i++) {
        int index = i+1;
        Vector3d right = tempT.row(index);
        Vector3d left = tempT.row(index-1);
        VectorXd tangent;
        if (right.isApprox(left, 0.0000001)) {
            tangent = right.normalized();
        } else {
            Vector3d n = right.normalized() - left.normalized();
            Vector3d zero;
            zero << 0.0, 0.0, 0.0;
            if (n.isApprox(zero, 0.0000001)) {
                tangent = right.normalized();
            } else {
                Vector3d f = right.cross(left);
                tangent = f.cross(n).normalized();
            }
        }
        t.conservativeResize(t.rows()+1, t.cols());
        t.row(t.rows()-1) = tangent;

    }
    // last t
    t.conservativeResize(t.rows()+1, t.cols());
    t.row(t.rows()-1) = t.row(t.rows()-2);

    //std::cout << t.rows() << std::endl;

}






Road::~Road() {
    // TODO Auto-generated destructor stub
}

