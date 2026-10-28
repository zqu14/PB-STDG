/*
 * Bezier.cpp
 *
 *  Created on: Mar 28, 2024
 *      Author: zqu14
 */

#include "Bezier.h"
#include <iostream>
#include <string>


using namespace Eigen;

Bezier::Bezier() {
    // TODO Auto-generated constructor stub
    points.resize(1, 3);

}

Bezier::~Bezier() {
    // TODO Auto-generated destructor stub
}

MatrixXd Bezier::getKnots(const MatrixXd &V, double t)
  {
    int nV = V.rows();         // number of points in the control polygon
    int degree = V.rows() - 1; // degree of the curve
    //completed (ex 1)
    int n = nV-1;
    MatrixXd VCrawl = V;

    for(int r=1;r<=n;r++){
        for(int i=0;i<=n-r;i++){
            VCrawl(i,0) = (1-t)*VCrawl(i,0) + t*VCrawl(i+1,0);
            VCrawl(i,1) = (1-t)*VCrawl(i,1) + t*VCrawl(i+1,1);
            VCrawl(i,2) = (1-t)*VCrawl(i,2) + t*VCrawl(i+1,2);
        }
    }

    MatrixXd ans(1,3);
    ans(0,0) = VCrawl(0,0);
    ans(0,1) = VCrawl(0,1);
    ans(0,2) = VCrawl(0,2);

    return ans;
  }


int Bezier::NCR(int n, int r)
{
    if (r == 0) return 1;

    /*
     Extra computation saving for large R,
     using property:
     N choose R = N choose (N-R)
    */
    if (r > n / 2) return NCR(n, n - r);

    long res = 1;

    for (int k = 1; k <= r; ++k)
    {
        res *= n - k + 1;
        res /= k;
    }

    return res;
}


VectorXd Bezier::bernsteinPoly(int i, int n, VectorXd &t)
{
    int comb = NCR(n, i);
    VectorXd temp = 1-t.array();
    VectorXd first_term = t.array().pow(n-i);
    VectorXd second_term = temp.array().pow(i);
    VectorXd result = comb * first_term.array() * second_term.array(); // @suppress("Invalid arguments")
    return result;
}


MatrixXd Bezier::getCurves(MatrixXd V, int nTimes)
{
    int nV = V.rows();
    VectorXd X = V.col(0);
    VectorXd Y = V.col(1);
    VectorXd Z = V.col(2);
    VectorXd t = VectorXd::LinSpaced(nTimes, 0, 1);

    MatrixXd p(nV, nTimes);
    for (int i=0; i<nV; i++) {
        p.row(i) = bernsteinPoly(i, nV-1, t);
    }
    X = X.transpose() * p;
    Y = Y.transpose() * p;
    Z = Z.transpose() * p;

    MatrixXd out(nTimes, 3);
    out.col(0) = X.reverse();
    out.col(1) = Y.reverse();
    out.col(2) = Z.reverse();
    return out;
}



void Bezier::addPoint(std::string point)
{
    int length = point.size();
    char delim = ',';
    std::vector<std::string> pointStr = split(point.substr(1,length-2), delim);
    double x = std::stod(pointStr[0]);
    double y = std::stod(pointStr[1]);
    double z = std::stod(pointStr[2]);

    if (first_point){
        points << x, y, z;
        first_point = false;
    } else {
        points.conservativeResize(points.rows()+1, points.cols());
        Vector3d vec;
        vec << x, y, z;
        points.row(points.rows()-1) = vec;
    }
}

std::vector<std::string> Bezier::split(const std::string& str, char delim) {
    std::vector<std::string> strings;
    size_t start;
    size_t end = 0;
    while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
        end = str.find(delim, start);
        strings.push_back(str.substr(start, end - start));
    }
    return strings;
}


MatrixXd Bezier::getPoints(){
    return points;
}



