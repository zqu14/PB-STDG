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

#include "RoadPart.h"

RoadPart::RoadPart() {
    // TODO Auto-generated constructor stub

}

RoadPart::~RoadPart() {
    // TODO Auto-generated destructor stub
}

MatrixXd RoadPart::getE1() {
    std::cout << "error!" << endl;
    MatrixXd d;
    return d;
}

MatrixXd RoadPart::getE2() {
    std::cout << "error!" << endl;
    MatrixXd d;
    return d;
}

void RoadPart::setID(std::string id) {
    this->id = id;
}

std::string RoadPart::getType() {
    return type;
}


MatrixXd RoadPart::getCurve() {
    return points;
}

std::string RoadPart::getLaneStr() {

    std::string s = "Lane Info for " + id + " :\n";

    for (auto& x : lanes) {
        s = s + "--(" + std::to_string(x.first.getI()) + "," + std::to_string(x.first.getJ()) + "): ";
        for (const auto& y : x.second) {
            s = s + std::to_string(y[0]) + " - " + std::to_string(y[1]);
            if (y.size()==3) s = s + " inter: " + std::to_string(y[2]);
            s = s + ", ";
        }
        s = s + "\n";
    }

    return s;

}

std::map< Lane, std::vector< std::vector<double> > > RoadPart::getLanes() {
    return lanes;
}


void RoadPart::addLane(int i, int j, double start, double end){
    Lane l = Lane(i, j);
    if (lanes.find(l) == lanes.end()) {
        std::vector< std::vector<double> > temp;
        lanes[l] = temp;
    }
    std::vector<double> v = {start, end};
    lanes[l].push_back(v);
}

void RoadPart::addLane(int i, int j, double start, double end, double inter){
    Lane l = Lane(i, j);
    if (lanes.find(l) == lanes.end()) {
        std::vector< std::vector<double> > temp;
        lanes[l] = temp;
    }
    std::vector<double> v = {start, end, inter};
    lanes[l].push_back(v);
}



void RoadPart::addCurve(MatrixXd V) {

    if (first_curve) {
        points.resize(V.rows(), V.cols());
        points << V;
        first_curve = false;
    } else {
        int r1 = points.rows();
        double tailX = points.coeff(r1-1, 0);
        double tailY = points.coeff(r1-1, 1);
        double tailZ = points.coeff(r1-1, 2);
        double headX = V.coeff(0, 0);
        double headY = V.coeff(0, 1);
        double headZ = V.coeff(0, 2);
        if (std::abs(tailX-headX) < 0.001 and std::abs(tailY-headY) < 0.001 and std::abs(tailZ-headZ) < 0.001 ){
            MatrixXd newPoints(r1+V.rows()-1, points.cols());
            V.block(0, 0, V.rows()-1, 3) = V.bottomRows(V.rows()-1);
            V.conservativeResize(V.rows()-1,3);
            newPoints << points,V;
            points = newPoints;
        } else {
            MatrixXd newPoints(r1+V.rows(), points.cols());
            newPoints << points, V;
            points = newPoints;
        }
    }
}
