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

#include<string>
#include<Eigen/Dense>
#include "../beaconing/processor/LaneUtil.h"
using namespace Eigen;

#ifndef ROADSYSTEM_ROADPART_H_
#define ROADSYSTEM_ROADPART_H_

class RoadPart {
public:
    RoadPart();
    virtual ~RoadPart();
    void setID(std::string id);
    void addCurve(MatrixXd V);
    MatrixXd getCurve();
    void addLane(int i, int j, double start, double end);
    void addLane(int i, int j, double start, double end, double inter);
    std::map< Lane, std::vector< std::vector<double> > > getLanes();
    std::string getLaneStr();
    virtual MatrixXd getE1();
    virtual MatrixXd getE2();
    std::string getType();

protected:
    std::map<Lane, std::vector<double>> sLengths;
    std::string type;
    std::string id;
    MatrixXd points;
    bool first_curve = true;
    std::map< Lane, std::vector< std::vector<double> > > lanes;
};

#endif /* ROADSYSTEM_ROADPART_H_ */
