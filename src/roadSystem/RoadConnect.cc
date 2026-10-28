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

#include "RoadConnect.h"

bool RoadConnect::checkPartConnect(std::string id2, int fromi, int toi, int fromj, int toj) {
    if(next.find(id2) != next.end()) {
        if (next[id2][0] == fromi and
            next[id2][1] == fromj and
            next[id2][2] == toi and
            next[id2][3] == toj) return true;
    }
    return false;
}


std::map<std::string, std::vector<int>> RoadConnect::getNext() {
    return next;
}

void RoadConnect::addNext(std::string id, int fromi, int fromj, int toi, int toj) {
    next[id] = {fromi, fromj, toi, toj};
}

void RoadConnect::addNextT(std::string id, double fromT, double toT) {
    nextT[id] = {fromT, toT};
}

std::map<std::string, std::vector<double>> RoadConnect::getNextT() {
    return nextT;
}


bool RoadConnect::checkFinal(std::string dest) {

    if (next.count(dest) == 1) return true;
    return false;
}

RoadConnect::RoadConnect() {
    // TODO Auto-generated constructor stub

}

RoadConnect::~RoadConnect() {
    // TODO Auto-generated destructor stub
}

