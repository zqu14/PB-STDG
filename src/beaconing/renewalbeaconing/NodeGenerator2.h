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

#ifndef BEACONING_RENEWALBEACONING_NODEGENERATOR2_H_
#define BEACONING_RENEWALBEACONING_NODEGENERATOR2_H_
#include <omnetpp.h>
#include <vector>
#include "../../roadSystem/RoadSystem.h"

using namespace omnetpp;

struct LaneInfo {
    std::string startId;
    std::string id;
    double x;
    double y;
    double z;
    double dx;
    double dy;
    double dz;
    int i;
    int j;
};



class NodeGenerator2 : public cSimpleModule {

    typedef struct nodePosi nodePosi;
    public:
        void handleMessage(cMessage* msg);
        void initialize(int stage);
        int numInitStages() const;
        void generate(LaneInfo li);
        NodeGenerator2();
        virtual ~NodeGenerator2();
        nodePosi getRandomPosition();
        nodePosi _getRandomPosition();
        bool checkPositionConflict(nodePosi pos);
        nodePosi _getLanePosition(int index);
        void addAllLaneGeneration();
        void countNode();
        void nextGenerateFromTable();


        virtual void finish() override;
    private:

        RoadSystem *rs       = nullptr;
        // parametes used in generation
        std::map<std::string, double> LastTime;
        std::map<std::string, double> LastSpeed;
        std::map<double, LaneInfo> generateTimeTable;

        int failGenerateCount;
        double radius;
        int maxNum;
        int currentNum;
        cMessage* startGenerating1;
        cMessage* count;
        double centerX;
        double centerZ;

        int failCount;
        int generateCount;

};

//int NodeGenerator::currentNum = 0;

#endif /* BEACONING_RENEWALBEACONING_NODEGENERATOR2_H_ */
