/*
 * LaneUtil.h
 */

#ifndef BEACONING_PROCESSOR_LANEUTIL_H_
#define BEACONING_PROCESSOR_LANEUTIL_H_

#include <omnetpp.h>
#include <inet/common/geometry/common/Coord.h>



using namespace inet;

/*
 * Structure of the lane
 */

class Lane {

    protected:
        int i;
        int j;
        int index;

    public:
        int getI() const{return i;};
        int getJ() const {return j;};
        Lane(){i=-1;j=-1;index=-10001;};
        Lane(int inputi, int inputj){i=inputi;j=inputj;index=10000*i+j;};
        void setI(int inputi){i=inputi;index=10000*i+j;};
        void setJ(int inputj){j=inputj;index=10000*i+j;};
        int getIndex() const{return index;};

        bool operator<(const Lane& other) const
            {
                return index < other.index;
            }

        bool operator==(const Lane& other) const
            {
                return i == other.i and j == other.j;
            }
        bool operator!=(const Lane& other) const
            {
                return i !=other.i or j != other.j;
            }

};


/*
 * Include many calculation function used in drone collision avoidance algorithm
 */

class LaneUtil {
    public:
        static Lane coordToLane(Coord c, double r);
        static Coord LaneToCoord(Lane l, double r);
        static double estimateCollisionTime(Coord myv, Coord targetv, Coord myp, Coord targetp, double minsafedis, double r);
        static double _estimateCollisionTime(double myv, double targetv, double myy, double targety, double minsafedis);
        static bool isSameLane(Lane l1, Lane l2, double r);
        static bool isSameLane(Coord c1, Coord c2, double r);
        static std::vector<Coord> getNeighborPosi(Coord currentPosi, double r);
        static int getTier(Lane l);
        static std::vector<Lane> getNeighborLane(Lane l);
        static int getDistance(Lane l1, Lane l2);
};


#endif /* BEACONING_PROCESSOR_LANEUTIL_H_ */
