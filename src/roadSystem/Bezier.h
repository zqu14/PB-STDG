/*
 * Bezier.h
 *
 *  Created on: Mar 28, 2024
 *      Author: zqu14
 */

#ifndef ROADSYSTEM_BEZIER_H_
#define ROADSYSTEM_BEZIER_H_

#include<string>
#include<vector>
#include<Eigen/Dense>
using namespace Eigen;

class Bezier {
public:
    Bezier();
    virtual ~Bezier();
    MatrixXd getKnots(const MatrixXd &V, double t);
    int NCR(int n, int r);
    VectorXd bernsteinPoly(int i, int n, VectorXd &t);
    MatrixXd getCurves(MatrixXd V, int nTimes=100);
    void addPoint(std::string point);
    std::vector<std::string> split(const std::string& str, char delim);
    MatrixXd getPoints();
private:

    MatrixXd points;
    bool first_point = true;
};

#endif /* ROADSYSTEM_BEZIER_H_ */
