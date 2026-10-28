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

#ifndef ARRIVALCHECK_ARRIVALCHECKER_H_
#define ARRIVALCHECK_ARRIVALCHECKER_H_

#ifndef CL_CHECK_PERIOD
#define CL_CHECK_PERIOD        0.01
#endif

#ifndef NODE_COUNT_CHECK_PERIOD
#define NODE_COUNT_CHECK_PERIOD        1
#endif



#include <omnetpp.h>
using namespace omnetpp;
class ArrivalChecker : public cSimpleModule {
public:
    virtual ~ArrivalChecker();
    void handleMessage(cMessage* msg);
    void initialize();
    void checkCollision();
    void countNode();
    virtual void finish() override;
private:
    std::map<std::vector<int>, int> densityCount;
    double startTime;
    cMessage* check;
    cMessage* count;
    double constrainZ;
    double minimumSafetyDistance;
    int arriveCount;
    int numOfTarget;
    int currentNumofTarget;
    int arc=0; // algorithm run counter
    int sc=0;  // switch counter


    double beaconGap = 0;
    double beaconDelay = 0;
    double beaconAmount = 0;

};

#endif /* ARRIVALCHECK_ARRIVALCHECKER_H_ */
