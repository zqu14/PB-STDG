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

#ifndef ROADSYSTEM_ROADSYSTEM_H_
#define ROADSYSTEM_ROADSYSTEM_H_

#include"RoadPath.h"

struct EntryInfo {
    std::string id;
    std::string pointId;
    Vector3d coord;
    Vector3d direction;
    int i;
    int j;
};




class RoadSystem : public cSimpleModule{

public:

    RoadSystem();
    virtual ~RoadSystem();
    void initialize();
    void setRF(std::string s1, std::string s2);
    Vector3d getRF1();
    Vector3d getRF2();
    std::vector<std::string> getPath(std::string start, std::string end, double t);
    std::vector< std::vector<std::string> > getPaths(std::string start, std::string end, double t, std::map<std::string,double> searched);
    void setSystem();
    void addRoadPart(std::string id, RoadPart* r);
    void setRadius(double r);
    std::map<std::string, RoadPart*> getRoadParts();
    void addConnects(int fromi, int fromj, int toi, int toj, std::string from_id, std::string to_id, double fromt, double tot);
    std::string getConnectStr();
    MatrixXd getLaneCoord(Lane lane, std::string id);
    std::vector<Lane> getNeighborLane(Lane lane, std::string id, double t, int mode);
    std::vector<Lane> getNeighborLane(Lane lane, std::string id, double t);
    std::vector<Lane> getNeighborLane(Lane lane, std::string id);
    std::vector<Lane> getAllSwitchLane(std::string id, double t);
    RoadPath getRoadPath(std::string start, std::string end);
    double getRoadClose(std::string id, Lane lane, double t);
    double getRoadClose(std::string id, Lane lane, double t, int n);
    double getRoadInter(std::string id, Lane lane, double t);
    double getRoadOpen(std::string id, Lane lane, double t);
    Lane getRandomNextCloseLane(std::string id, Lane l, double t, Lane ld);
    Lane getNeighborCompleteLane(std::string id, Lane lane, double t);
    Vector3d getWayPoint(int n, std::string id, Lane l);
    void addEntry(std::string id);
    void addExit(std::string id);
    std::vector<EntryInfo> getAllEntry();
    std::string getRandomExit();
    std::string getRandomExit(std::string start);
    std::vector<Vector3d> getSuitableSwitchPoint(std::string id, Lane l1, Lane l2, double t, double speed, int upper);
    Vector3d getNeighborPoint(std::string id, Lane l1, Lane l2, double t);
    double getSLength(std::string id, Lane l, int n);
    double getLength(std::string id, Lane l) {return laneLengths[id][l];};
    double getDistance(std::string id, Lane l, double t1, double t2);
    int checkConnect(std::string id1, std::string id2, int i1, int i2, int j1, int j2);
    bool checkAConnectToB(std::string ida, std::string idb);
    std::vector<double> getConnectT(std::string id1, std::string id2);
    bool checkIsTurnLane(std::string id, Lane l);
    void setLaneBlock(std::string id, Lane l, bool b) {laneBlocks[id][l]=b;};
    bool getLaneBlock(std::string id, Lane l) {return laneBlocks[id][l];};;
    void initExitPool();
    std::string getPartType(std::string id) {return roadParts[id]->getType();};
    int getIndexOfBlock(std::string id, double t);
    double getT(std::string id, Lane l, double dis);

private:
    std::map<std::string, std::vector<std::vector<std::string>>> exitPool;
    std::vector<std::string> entryId;
    std::vector<std::string> exitId;
    std::map<std::string, RoadPart*> roadParts;
    std::map<std::string, RoadConnect> roadConnects;
    double r;
    Vector3d rf1 = {0,0,1};
    Vector3d rf2 = {0,1,0};
    std::map<std::string, std::map<Lane, MatrixXd>> laneCoords;
    std::map<std::string, std::map<Lane, VectorXd>> laneSLengths;
    std::map<std::string, std::map<Lane, double>> laneLengths;
    std::map<std::string, std::map<Lane, bool>> laneBlocks;
    double E_BLOCK = 50;
};


#endif /* ROADSYSTEM_ROADSYSTEM_H_ */
