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

#ifndef ROADSYSTEM_ROADCONNECT_H_
#define ROADSYSTEM_ROADCONNECT_H_

#include<string>
#include<map>
#include<vector>

class RoadConnect {
public:

    RoadConnect();
    virtual ~RoadConnect();

    void addNext(std::string id, int fromi, int fromj, int toi, int toj);
    std::map<std::string, std::vector<int>> getNext();
    std::map<std::string, std::vector<double>> getNextT();
    bool checkFinal(std::string dest);
    void addNextT(std::string id, double fromT, double toT);
    bool checkPartConnect(std::string id2, int fromi, int toi, int fromj, int toj);


private:

    std::string id;
    std::map<std::string, std::vector<int>> next;
    std::map<std::string, std::vector<double>> nextT;

};

#endif /* ROADSYSTEM_ROADCONNECT_H_ */
