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

#include "RoadSystem.h"
#include <cstring>
Define_Module(RoadSystem);

void RoadSystem::initialize() {
    E_BLOCK  = par("E_BLOCK");
}

MatrixXd RoadSystem::getLaneCoord(Lane lane, std::string id) {
    return laneCoords[id][lane];
}

std::vector<Lane> RoadSystem::getNeighborLane(Lane lane, std::string id, double t, int mode) {
    std::map< Lane, std::vector< std::vector<double> >> lanes =  roadParts[id]->getLanes();
    std::vector<Lane> nlanes = LaneUtil::getNeighborLane(lane);
    std::vector<Lane> result;
    for (Lane& l : nlanes) {
        if (lanes.find(l) == lanes.end()) {
            continue;
        } else {
            for(std::vector<double>& v : lanes[l]) {
                double start = v[0];
                double end =  v[1];
                if (v.size()==3) start = v[2];
                if (t >= start and t <= end) {
                    result.push_back(l);
                    break;
                }
            }
        }
    }
    return result;
}
std::vector<Lane> RoadSystem::getNeighborLane(Lane lane, std::string id, double t) {

    std::map< Lane, std::vector< std::vector<double> >> lanes =  roadParts[id]->getLanes();
    std::vector<Lane> nlanes = LaneUtil::getNeighborLane(lane);
    std::vector<Lane> result;

    for (Lane& l : nlanes) {
        if (lanes.find(l) == lanes.end()) {
            continue;
        } else {
            for(std::vector<double>& v : lanes[l]) {
                if (t >= v[0] and t <= v[1]) {
                    result.push_back(l);
                    break;
                }
            }
        }
    }
    return result;
}

std::vector<Lane> RoadSystem::getNeighborLane(Lane lane, std::string id) {

    std::map< Lane, std::vector< std::vector<double> >> lanes =  roadParts[id]->getLanes();
    std::vector<Lane> nlanes = LaneUtil::getNeighborLane(lane);
    std::vector<Lane> result;

    for (Lane& l : nlanes) {
        if (lanes.find(l) == lanes.end()) {
            continue;
        } else {
            result.push_back(l);
        }
    }
    return result;
}


std::vector< std::vector<std::string> > RoadSystem::getPaths(std::string start, std::string end, double t, std::map<std::string,double> searched) {
    std::vector< std::vector<std::string> > paths;
    if (searched.find(start) != searched.end() and searched[start] <= t) return paths;
    if (start == end) {
        std::vector<std::string> path;
        path.push_back(start);
        paths.push_back(path);
        return paths;
    }
    if (roadConnects.count(start) == 0) return paths;
    searched[start]=t;
    for (auto &i : roadConnects[start].getNext())
    {
        auto to_id = i.first;
        auto c_lane = i.second;
        double t1 = roadConnects[start].getNextT()[to_id][0];
        double t2 = roadConnects[start].getNextT()[to_id][1];
        if(t < t1) {
            std::vector< std::vector<std::string> > ps = getPaths(to_id, end, t2, searched);
            if (ps.size() > 0 ) {
                for (auto& path : ps) {
                    if (path.size() > 0) {
                        //if (std::find(path.begin(), path.end(), start) == path.end()) {
                            path.insert(path.begin(), start);
                            paths.push_back(path);



                        //} else continue;
                    }
                }
            }
        } else continue;
    }
    return paths;
}

RoadPath RoadSystem::getRoadPath(std::string start, std::string end) {
    std::map<std::string,double> searched;
    std::vector<std::vector<std::string>> paths =  getPaths(start,end,0, searched);
    RoadPath shortest;
    double shortestLength = 99999;
    for (auto&path : paths) {
        RoadPath roadPath;
        bool is_first_node = true;
        std::string last_node = "";
        double currentT = 0;
        for (auto&node : path) {
            if (is_first_node) {
                is_first_node = false;
                roadPath.setStart(node);
            } else {
                std::string id = node;
                auto laneinfo = roadConnects[last_node].getNext()[node];
                Lane fl = Lane(laneinfo[0],laneinfo[1]);
                Lane tl = Lane(laneinfo[2],laneinfo[3]);
                auto tinfo = roadConnects[last_node].getNextT()[node];
                double ft = tinfo[0];
                double tt = tinfo[1];
                Vector3d coord;
                if (roadParts[node]->getType() == "ramp") {
                    coord = roadParts[node]->getCurve().row(0);
                } else {
                    MatrixXd coords = roadParts[last_node]->getCurve();
                    coord = coords.row(coords.rows()-1);
                }
                roadPath.setNext(node, fl, tl, coord, ft, tt);
                double partLength = getDistance(last_node, Lane(0,0), currentT, ft);
                roadPath.addLength(partLength);

                //std::cout << "+++ addLength: " << partLength << " from: " << last_node << "   " << currentT << " to " << ft << endl;

                currentT = tt;
            }
            last_node = node;
        }
        if (last_node == " ") last_node = start;
        double partLength = getDistance(last_node, Lane(0,0), currentT, 1);
        roadPath.addLength(partLength);
        //std::cout << "+++ addLength: " << partLength << " from: " << last_node << "   " << currentT << " to " << 1 << endl;
        //std::cout << "===========================================" << endl;
        //for (auto&yyy : path) {
        //    std::cout << yyy <<"->";
        //}
        //std::cout << "     length: " << roadPath.getLength() << endl;
        if (roadPath.getLength() < shortestLength)  {
            shortestLength = roadPath.getLength();
            shortest = roadPath;
        }
    }
    //std::cout << "++++++++++++++++++++++++++++++++++++++" << endl;
    //std::cout << shortest.getStr() << endl;
    return shortest;
}

bool RoadSystem::checkAConnectToB(std::string ida, std::string idb) {
    auto next = roadConnects[ida].getNext();
    if (next.find(idb) != next.end()) return true;
    return false;
}


std::vector<std::string> RoadSystem::getPath(std::string start, std::string end, double t) {
    std::vector<std::string> path;
    //std::cout << start << " to "<< end<< " at " << t << endl;

    if (start == end) {
        path.push_back(start);
        return path;
    }
    if (roadConnects.count(start) == 0) throw std::runtime_error("RoadSystem::getPath -- Cannot load road in system invalid destination or false road system init");;
    for (auto &i : roadConnects[start].getNext())
    {
        auto to_id = i.first;
        auto c_lane = i.second;
        double t1 = roadConnects[start].getNextT()[to_id][0];
        double t2 = roadConnects[start].getNextT()[to_id][1];

        if(t < t1) {
            std::vector<std::string> p = getPath(to_id, end, t2);
            if (p.size() > 0 ) {
                path.push_back(start);
                path.insert(path.end(), p.begin(), p.end());
                return path;
            }
        } else {
            continue;
        }
    }
    return path;
}

double RoadSystem::getRoadClose(std::string id, Lane lane, double t) {
    auto lllll = roadParts[id]->getLanes()[lane];
    for (auto &i :lllll) {
        if (t>=i[0] and t <i[1]) return i[1];
    }
    return 1.0;
}

double RoadSystem::getRoadClose(std::string id, Lane lane, double t, int n) {
    auto lllll = roadParts[id]->getLanes()[lane];
    for (auto &i :lllll) {
        double end = i[1];
        if (i.size() == 3) end = i[2];
        if (t>=i[0] and t < end) return i[1];
    }
    return 1.0;
}


double RoadSystem::getRoadInter(std::string id, Lane lane, double t) {
    auto lllll = roadParts[id]->getLanes()[lane];
        for (auto &i :lllll) {
            if (t>=i[0] and t < i[1] and i.size()==3) return i[2];
        }
        return -9999;
}




double RoadSystem::getRoadOpen(std::string id, Lane lane, double t) {
    auto lllll = roadParts[id]->getLanes()[lane];
    for (auto &i :lllll) {
        if (t>i[0] and t<=i[1]) return i[0];
    }
    return 0.0;
}

Lane RoadSystem::getNeighborCompleteLane(std::string id, Lane lane, double t) {
    std::map< Lane, std::vector< std::vector<double> >> lanes =  roadParts[id]->getLanes();
    std::vector<Lane> nlanes = LaneUtil::getNeighborLane(lane);
    std::vector<Lane> result;

    for (Lane& l : nlanes) {
        if (lanes.find(l) == lanes.end()) {
            continue;
        } else {
            if (getRoadClose(id, l, t) == 1.0) result.push_back(l);
        }
    }
    if (result.size()==0) return lane;
    std::srand(par("randomExitDistribution"));
    int random_pos = std::rand() % result.size();
    return result[random_pos];

}




void RoadSystem::setRadius(double r) {
    this->r = r;
}

//TODO
//---------------------------new code without test -------------------------------------------------------------------------------------------------------------------

double RoadSystem::getSLength(std::string id, Lane l, int n) {
    return laneSLengths[id][l][n];
}


void RoadSystem::addRoadPart(std::string id, RoadPart* r) {
    roadParts[id] = r;
    std::map<Lane, MatrixXd> m;
    for(auto &i : r->getLanes()) {
        MatrixXd center = r->getCurve();
        MatrixXd e1 = r->getE1();
        MatrixXd e2 = r->getE2();
        Lane l = i.first;
        if (l == Lane(0,0)) m[l] = center;
        else m[l] = center.array() + e1.array()*l.getI()*this->r + e2.array()*l.getJ()*this->r;
    }
    laneCoords[id] = m;

    std::map<Lane, VectorXd> m2;
    std::map<Lane, double> m3;
    std::map<Lane, bool> m4;
    for(auto &i : m) {
        double totalLength = 0;
        Lane l = i.first;
        int rows = i.second.rows();
        VectorXd slengths(rows-1);
        for (int j=0;j<rows-1;j++) {
            Vector3d from =  i.second.row(j);
            Vector3d to =  i.second.row(j+1);
            Vector3d dis = to.array() - from.array();
            totalLength = totalLength + dis.norm();
            slengths(j) = dis.norm();
        }
        m2[l] = slengths;
        m3[l] = totalLength;
        m4[l] = false;
    }

    laneSLengths[id] = m2;
    laneLengths[id] = m3;
    laneBlocks[id] = m4;

    //for(auto &i : r->getLanes()) {
    //    Lane l = i.first;
    //    std::cout << "lane" <<l.getI()<<l.getJ()<< "length: " << laneLengths[id][l] << endl;
    //}



}


Vector3d RoadSystem::getNeighborPoint(std::string id, Lane l1, Lane l2, double t) {
    int total = roadParts[id]->getCurve().rows()-1;
    int currentN = floor(total * t);
    Vector3d ToLaneFloorCoord = getWayPoint(currentN, id, l2);
    Vector3d ToLaneNextCoord = getWayPoint(currentN+1, id, l2);
    double floorT = (double)currentN/(double)total;
    double nextT = (double)(currentN+1)/(double)total;
    double percentage = (t - floorT) / (nextT - floorT);
    Vector3d result = ToLaneFloorCoord.array() + (ToLaneNextCoord.array() - ToLaneFloorCoord.array()) * percentage;
    return result;
}

//TODO currently use one second
std::vector<Vector3d> RoadSystem::getSuitableSwitchPoint(std::string id, Lane l1, Lane l2, double t, double speed, int upper) {


    std::vector<Vector3d> results;
    Vector3d neighborPoint = getNeighborPoint(id, l1, l2, t);
    int n = floor((roadParts[id]->getCurve().rows()-1) * t + 0.000001) + 1;
    if (n > upper) {
        std::cout << n << "  " << upper << "  " << id << "   "  << t << endl;
        std::cout << l1.getI() <<", " << l1.getJ() << endl;
        std::cout << l2.getI() <<", " << l2.getJ() << endl;
        throw cRuntimeError("RoadSystem::getSuitableSwitchPoint: t could not larger than upper");
    }

    Vector3d nextPoint;
    Vector3d fromPoint = neighborPoint;
    double total = speed * 1;
    double rest = total; // TODO 1 is one second
    double p=0;
    bool isSpaceEnough = true;
    while(1) {
        nextPoint = getWayPoint(n, id, l2);
        Vector3d distanceVector = fromPoint.array() - nextPoint.array();
        double distance = distanceVector.norm();
        if (distance > rest or upper==n) {
            if (distance < rest) isSpaceEnough = false;
            break;
        }
        rest = rest - distance;

        p = (total - rest) / total;
        Vector3d thisPoint = getWayPoint(n, id, l1);
        Vector3d interPoint = thisPoint.array() + (nextPoint.array() - thisPoint.array()) * p;
        results.push_back(interPoint);

        n++;
        fromPoint = nextPoint;
    }

    Vector3d finialPoint;
    if (upper == n and !isSpaceEnough) {
        finialPoint = nextPoint;
    } else {
        Vector3d dis = nextPoint.array() - fromPoint.array();
        double d = dis.norm();
        finialPoint = fromPoint.array() + (rest/d) * dis.array();
    }

    results.push_back(finialPoint);
    return results;




}

std::vector<Lane> RoadSystem::getAllSwitchLane(std::string id, double t) {
    std::vector<Lane> result;
     auto c = roadConnects[id];
     for (auto item: c.getNext()) {
         auto _id = item.first;
         if (c.getNextT()[_id][0] > t) {
             result.push_back(Lane(item.second[0], item.second[1]));
         }
     }
     return result;
}


int RoadSystem::getIndexOfBlock(std::string id, double t) {
    double dis = getDistance(id, Lane(0,0), 0, t);
    int index = dis/E_BLOCK;

    if ((dis - index * E_BLOCK) <= 15) {             // 50, 30 are currently magic numbers for block size and buffer size
        index = index * -1;
    }
    return index;
}




//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------




std::map<std::string, RoadPart*> RoadSystem::getRoadParts() {
    return roadParts;
}

void RoadSystem::addConnects(int fromi, int fromj, int toi, int toj, std::string from_id, std::string to_id, double fromt, double tot) {
    if (roadConnects.find(from_id) == roadConnects.end()) {
        roadConnects[from_id] = RoadConnect();
    }
    roadConnects[from_id].addNext(to_id, fromi, fromj, toi, toj);
    roadConnects[from_id].addNextT(to_id, fromt, tot);
}

std::string RoadSystem::getConnectStr() {

    std::string s = "Connects: \n";
    for (auto& x : roadConnects) {
        std::string from_id = x.first;
        s = s + "--From " + from_id + ":\n";
        for (auto& y : x.second.getNext()) {
            std::string to_id = y.first;
            auto ls = y.second;
            double fromt = x.second.getNextT()[to_id][0];
            double tot = x.second.getNextT()[to_id][1];
            s = s + "----" + "(" + std::to_string(ls[0]) + "," + std::to_string(ls[1]) + ") at " + std::to_string(fromt) + " to "
            + to_id + " lane (" + std::to_string(ls[2]) + "," + std::to_string(ls[3]) + ") at " + std::to_string(tot) + "\n";
        }
    }
    return s;
}


void RoadSystem::setRF(std::string s1, std::string s2) {
    int length = s1.size();
    char delim = ',';
    //std::vector<std::string> pointStr = split(s1.substr(1,length-2), delim);
    //rf1 << std::stod(pointStr[0]), std::stod(pointStr[1]), std::stod(pointStr[2]);
    //length = s2.size();
    //pointStr = split(s2.substr(1,length-2), delim);
    //rf2 << std::stod(pointStr[0]), std::stod(pointStr[1]), std::stod(pointStr[2]);
}


void RoadSystem::addEntry(std::string id) {
    entryId.push_back(id);
}
void RoadSystem::addExit(std::string id) {
    exitId.push_back(id);
}



std::vector<EntryInfo> RoadSystem::getAllEntry() {
    std::vector<EntryInfo> entries;
    for (auto&i : entryId) {
        for (auto&k : roadParts[i]->getLanes()){
            Lane l = k.first;
            auto oi = k.second;
            bool valid = false;
            for(std::vector<double>& v : oi) {
                if (0 >= v[0] and 0 <= v[1]) {
                    valid = true;
                    break;
                }
            }
            if (!valid) continue;
            EntryInfo j;
            j.pointId = i;
            std::string id = i + std::to_string(l.getI()) + std::to_string(l.getJ());
            j.i = l.getI();
            j.j = l.getJ();
            j.id = id;
            j.coord = getLaneCoord(l, i).row(0);
            Vector3d tt = getLaneCoord(l, i).row(1);
            j.direction = tt - j.coord;
            j.direction.normalize();
            entries.push_back(j);
        }
    }
    return entries;
}


Vector3d RoadSystem::getWayPoint(int n, std::string id, Lane l) {
    return getLaneCoord(l, id).row(n);
}



void RoadSystem::initExitPool() {
    for (auto entry: entryId) {
        exitPool[entry] = std::vector<std::vector<std::string>>();
        std::vector<std::string> added;
        std::vector<std::string> pool;
        // hop zero, only itself
        if (std::find(exitId.begin(),exitId.end(),entry) != exitId.end()) pool.push_back(entry);
        exitPool[entry].push_back(pool);
        for(auto i: pool) added.push_back(i);
        pool.clear();
        // hop one
        std::vector<std::string> oneHop;
        for(auto next: roadConnects[entry].getNext()) {
            std::string id = next.first;
            oneHop.push_back(id);
            if (std::find(exitId.begin(),exitId.end(),id) != exitId.end()) pool.push_back(id);
        }
        exitPool[entry].push_back(pool);
        for(auto i: pool) added.push_back(i);
        pool.clear();
        // hop two
        std::vector<std::string> twoHop;
        for(auto onehop: oneHop) {
            for(auto next: roadConnects[onehop].getNext()) {
                std::string id = next.first;
                twoHop.push_back(id);
                if (std::find(exitId.begin(),exitId.end(),id) != exitId.end()) pool.push_back(id);
            }
        }
        exitPool[entry].push_back(pool);
        for(auto i: pool) added.push_back(i);
        pool.clear();
        // hop three
        for(auto twohop: twoHop) {
            for(auto next: roadConnects[twohop].getNext()) {
                std::string id = next.first;
                if (std::find(exitId.begin(),exitId.end(),id) != exitId.end()) pool.push_back(id);
            }
        }
        exitPool[entry].push_back(pool);
        for(auto i: pool) added.push_back(i);
        pool.clear();
        //others
        for(auto exit: exitId) {
            std::map<std::string,double> searched;
            if (getPaths(entry, exit, 0, searched).size() == 0) continue;
            if (std::find(added.begin(),added.end(),exit) == added.end()) pool.push_back(exit);
        }
        exitPool[entry].push_back(pool);
    }

    if (0) {
        std::cout << "============================= exit pool: \n";
        for (auto ids: exitPool) {
            std::string id = ids.first;
            auto pool = ids.second;
            std::cout << "-------- " << id << ": \n";
            for(int i=0;i<pool.size();i++) {
                if (i!= pool.size()-1) std::cout << i << " hop: ";
                else std::cout << "other: ";
                for(auto _id: pool[i]) std::cout << _id << ", ";
                std::cout << "\n";
            }
        }
    }

}




std::string RoadSystem::getRandomExit(std::string start) {
    double randNum = par("randomExitDistribution").doubleValue();
    int hop;
    if (randNum <= 0.4) hop = 0; //0.4 - 0.5
    else if (randNum <= 0.6) hop = 1; // 0.6 - 0.51
    else if (randNum <= 0.75) hop = 2; // 0.75 - 0.99
    else if (randNum <= 0.9) hop = 3; // 0.9 - 0.999
    else hop = 4;
    while(exitPool[start][hop].size() == 0) {
        hop += 1;
        if (hop >= 5) hop = 0;//throw cRuntimeError("RoadSystem::getRandomExit: invalid exit");
    }
    int index = exitPool[start][hop].size() * par("randomExitDistribution").doubleValue();
    return exitPool[start][hop][index];
}



std::string RoadSystem::getRandomExit() {
    std::srand(par("randomExitDistribution"));
    int random_pos = std::rand() % exitId.size();
    return exitId[random_pos];
}

Lane RoadSystem::getRandomNextCloseLane(std::string id, Lane l, double t, Lane ld) {

    std::vector<Lane> results;
    int d = LaneUtil::getDistance(l, ld);
    for(auto &i : getNeighborLane(l,id)) {
        if (LaneUtil::getDistance(i, ld) < d and getRoadOpen(id, i, t) < t) results.push_back(i);
    }
    if (results.size() == 0) return l;
    int i = results.size();
    return results[std::rand() % i];
}




Vector3d RoadSystem::getRF1() {
    return rf1;
}

Vector3d RoadSystem::getRF2() {
    return rf2;
}

double RoadSystem::getDistance(std::string id, Lane l, double t1, double t2) {
    if (t1 > t2) return -9999;
    int size = getLaneCoord(l, id).rows() - 1;
    int n1 = floor(t1 * size);
    int n2 = floor(t2 * size);
    double t = t1;
    double tUp = (double) (n1+1) / (double) size;
    double tDown = (double) n1 / (double) size;
    double d = 0;
    while(n1 != n2) {
        double p = 1 - ((t - tDown) / (tUp - tDown));
        try{
            //std::cout << id << "  " << l.getI() << "  " << l.getJ() << "  " << n1 << "  " << t1 << "   " <<   t2 << endl;
            d = d + laneSLengths[id][l][n1] * p;
        } catch (std::exception e){

        };
        n1++;
        t = (double) n1 / (double) size;
        tUp = (double) (n1+1) / (double) size;
        tDown = t;
    }
    double p = (t2 - t) / (tUp - tDown);
    if (n1 == size) return d;
    d = d + laneSLengths[id][l][n1] * p;
    return d;
}


double RoadSystem::getT(std::string id, Lane l, double dis) {
    int size = getLaneCoord(l, id).rows() - 1;
    int n = 0;
    double left = 0;
    while (n <= size) {
        dis -= laneSLengths[id][l][n];
        if (dis >= 0) {
            n++;
        } else {
            left = (dis + laneSLengths[id][l][n]) / ( laneSLengths[id][l][n] * (double) size);
            break;
        }
    }
    return (double) n / (double) size + left;
}


int RoadSystem::checkConnect(std::string id1, std::string id2, int i1, int i2, int j1, int j2) {
    if (roadConnects.find(id1) != roadConnects.end() and roadConnects[id1].checkPartConnect(id2,i1,i2,j1,j2)) {
        return 1;
    } else if (roadConnects.find(id2) != roadConnects.end() and roadConnects[id2].checkPartConnect(id1,i2,i1,j2,j1)) {
        return 2;
    } else return 0;
}

std::vector<double> RoadSystem::getConnectT(std::string id1, std::string id2) {
    auto ts = roadConnects[id1].getNextT();
    return ts[id2];
}

bool RoadSystem::checkIsTurnLane(std::string id, Lane l) {
    if (roadConnects.find(id) == roadConnects.end()) return false;
    for (auto c : roadConnects[id].getNext()) {
        int i = (c.second)[0];
        int j = (c.second)[1];
        if (Lane(i,j) == l) return true;
    }
    return false;
}






RoadSystem::RoadSystem() {
}

RoadSystem::~RoadSystem() {

    for (auto&i:roadParts){
        delete i.second;
    }
    roadParts.clear();
}

