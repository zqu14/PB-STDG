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

#include "../roadReader/XMLRoadReader.h"

#include "../roadSystem/Road.h"
#include "../roadSystem/Bezier.h"
#include "../roadSystem/Ramp.h"


#include <iostream>
#include <fstream>
#include <string>


#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NodeIterator.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/SAX/InputSource.h>
#include <Poco/DOM/TreeWalker.h>
#include <Poco/SAX/SAXException.h>
#include <Poco/DOM/Element.h>
#include "Poco/DOM/NamedNodeMap.h"



using namespace Poco::XML;


Define_Module(XMLRoadReader);
XMLRoadReader::XMLRoadReader() {
    // TODO Auto-generated constructor stub

}


void XMLRoadReader::initialize() {

    rs = check_and_cast<RoadSystem*>(this->getParentModule()->getSubmodule("roadSystem"));
    filedir = par("fileDir").stdstringValue();
    readFile(filedir);
}

//=============================== test code =========================

void XMLRoadReader::readFile(std::string dir) {

    bool print_info = false;
    if (print_info) std::cout<< " START READ XML ROAD INFO" << endl;
    std::ifstream in(dir);
    InputSource src(in);
    Poco::AutoPtr<Poco::XML::Document> pDoc;
    try {
        Poco::XML::DOMParser parser;
        pDoc = parser.parse(&src);

    } catch (Poco::XML::SAXException e) {
        throw std::runtime_error("XMLRoadReader::readFile -- Cannot found XML file");
    }


    Poco::XML::NodeIterator it(pDoc, Poco::XML::NodeFilter::SHOW_ELEMENT);
    Poco::XML::Node* root = it.root();
    Poco::XML::Node* roadSystem = root->getNodeByPath("/DroneRoadSystem");
    if (!roadSystem || roadSystem->nodeName() != "DroneRoadSystem") throw std::runtime_error("XMLRoadReader::readFile -- Cannot read correct road system");
    if (print_info) std::cout<< " --start reading radius" << endl;
    Poco::XML::Node* radius = root->getNodeByPath("/DroneRoadSystem/radius");
    double r=0;
    if (radius) {
        Poco::AutoPtr<NamedNodeMap> attr = radius->attributes();
        r = stod(attr->item(0)->innerText());
        if (print_info) std::cout << " ----radius is " << r << endl;
    }
    if (print_info) std::cout<< " --end reading radius" << endl;



//-------------------------
    rs->setRadius(r);
//-------------------------


    if (print_info) std::cout<< " --start reading road" << endl;
    for(int i=0; ;i++){
        Poco::XML::Node* road = root->getNodeByPath("/DroneRoadSystem/Road[" + std::to_string(i) +"]");
        if (road) {
            if (print_info) std::cout << " ----reading road [" +  std::to_string(i) + "]" << endl;
            Poco::AutoPtr<NamedNodeMap> attr = road->attributes();
            auto id = attr->item(0)->innerText();
            if (print_info) std::cout << " ------ID: \t" << id << endl;


//--------------------------
            Road* _road = new Road();
            _road->setID(id);
//--------------------------


            for(int j=0; ;j++){
                Poco::XML::Node* cPoints = road->getNodeByPath("/controlPoints[" + std::to_string(j) +"]");
                if (cPoints){
                    if (print_info) std::cout << " ------Curve: \t[" +  std::to_string(j) + "]" << endl;
                    Poco::AutoPtr<NamedNodeMap> attr1 = cPoints->attributes();
                    auto type = attr1->getNamedItem("type")->innerText();
                    auto n = attr1->getNamedItem("n")->innerText();
                    if (print_info) std::cout << " --------Type: \t" << type << endl;
                    if (print_info) std::cout << " --------n: \t" << n << endl;

                    Bezier bezier;

                    for (int k=0;;k++){
                        Poco::XML::Node* point = cPoints->getNodeByPath("/point[" + std::to_string(k) +"]");
                        if (point) {
                            Poco::AutoPtr<NamedNodeMap> attr2 = point->attributes();
                            auto coord = attr2->getNamedItem("coord")->innerText();
                            //----------------------------------------------------------------
                            bezier.addPoint(coord);
                            //----------------------------------------------------------------
                            if (print_info) std::cout << " --------p["+ std::to_string(k) + "]: \t" << coord << endl;
                        } else break;
                    }
                    //----------------------------------------------------------------
                    _road->addCurve(bezier.getCurves(bezier.getPoints(), std::stoi(n)));
                    //----------------------------------------------------------------


                } else break;
            }
            //----------------------------------------------------------------
            //std::cout << _road.getCurve() << endl;
            //std::cout << "==========================================================" << endl;
            //----------------------------------------------------------------
            for(int j=0; ;j++){
                Poco::XML::Node* lane = road->getNodeByPath("/lane[" + std::to_string(j) +"]");
                if (lane){
                    if (print_info) std::cout << " ------Lane: \t[" +  std::to_string(j) + "]" << endl;

                    Poco::AutoPtr<NamedNodeMap> attr3 = lane->attributes();
                    auto _i = attr3->getNamedItem("i")->innerText();
                    auto _j = attr3->getNamedItem("j")->innerText();
                    if (print_info) std::cout << " --------i: \t" << _i << endl;
                    if (print_info) std::cout << " --------j: \t" << _j << endl;

                    for (int k=0;;k++){
                        Poco::XML::Node* interval = lane->getNodeByPath("/openInterval[" + std::to_string(k) +"]");
                        if (interval){
                            Poco::AutoPtr<NamedNodeMap> attr4 = interval->attributes();
                            auto start = attr4->getNamedItem("start")->innerText();
                            auto end = attr4->getNamedItem("close")->innerText();
                            auto inter = attr4->getNamedItem("inter");
                            if (print_info) std::cout << " --------o[" + std::to_string(k) + "]: \t" << start << " - "<< end << endl;
                            //----------------------------------------------------------------
                            if (inter) _road->addLane(std::stoi(_i), std::stoi(_j), std::stod(start), std::stod(end), std::stod(inter->innerText()));
                            else _road->addLane(std::stoi(_i), std::stoi(_j), std::stod(start), std::stod(end));
                            //----------------------------------------------------------------
                        } else break;
                    }
                } else break;
            }

            //----------------------------------------------------------------
            _road->construct();
            rs->addRoadPart(id, _road);
            rs->addEntry(id);
            rs->addExit(id);
            //auto mmm = _road->getLaneStr();
            //std::cout << mmm;
            //std::cout << "=================================" << endl;
            //----------------------------------------------------------------

        } else break;
    }
    if (print_info) std::cout<< " --end reading road" << endl;
    if (print_info) std::cout<< " --start reading ramp" << endl;
    for (int i=0; ;i++) {
        Poco::XML::Node* ramp = root->getNodeByPath("/DroneRoadSystem/Ramp[" + std::to_string(i) +"]");
        if (ramp){
            if (print_info) std::cout << " ----reading ramp [" +  std::to_string(i) + "]" << endl;
            Poco::AutoPtr<NamedNodeMap> attr5 = ramp->attributes();
            auto id = attr5->item(0)->innerText();
            if (print_info) std::cout << " ------ID: \t" << id << endl;
            //--------------------------
                        Ramp * _ramp = new Ramp();
                        _ramp->setID(id);
            //--------------------------
            for(int j=0; ;j++){
                Poco::XML::Node* cPoints = ramp->getNodeByPath("/controlPoints[" + std::to_string(j) +"]");
                if (cPoints){
                    if (print_info) std::cout << " ------Curve: \t[" +  std::to_string(j) + "]" << endl;
                    Poco::AutoPtr<NamedNodeMap> attr6 = cPoints->attributes();
                    auto type = attr6->getNamedItem("type")->innerText();
                    auto n = attr6->getNamedItem("n");
                    std::string _n = "100";
                    if (n) _n = n->innerText();
                    if (print_info) std::cout << " --------Type: \t" << type << endl;
                    if (print_info) std::cout << " --------n: \t" << _n << endl;
                    Bezier bezier;
                    for (int k=0;;k++){
                        Poco::XML::Node* point = cPoints->getNodeByPath("/point[" + std::to_string(k) +"]");
                        if (point) {
                            Poco::AutoPtr<NamedNodeMap> attr7 = point->attributes();
                            auto coord = attr7->getNamedItem("coord")->innerText();
                            if (print_info) std::cout << " --------p["+ std::to_string(k) + "]: \t" << coord << endl;
                            //----------------------------------------------------------------
                            bezier.addPoint(coord);
                            //----------------------------------------------------------------
                        } else break;
                    }
                    //----------------------------------------------------------------
                    _ramp->addCurve(bezier.getCurves(bezier.getPoints(), std::stoi(_n)));
                    //----------------------------------------------------------------
                } else break;
            }

            //----------------------------------------------------------------
            _ramp->construct();
            rs->addRoadPart(id, _ramp);
            //auto mmm = _ramp->getLaneStr();
            //std::cout << mmm;
            //std::cout << "=================================" << endl;
            //----------------------------------------------------------------

            bool off_key=false;
            for (int j=0; ;j++){
                Poco::XML::Node* connect = ramp->getNodeByPath("/connectedPart[" + std::to_string(j) +"]");
                if (connect){
                    Poco::AutoPtr<NamedNodeMap> attr8 = connect->attributes();
                    auto type = attr8->getNamedItem("type")->innerText();
                    if (print_info) std::cout << " ------Connect " << type << endl;

                    Poco::AutoPtr<NamedNodeMap> attr9 = connect->getNodeByPath("/roadID")->attributes();
                    Poco::AutoPtr<NamedNodeMap> attr10 = connect->getNodeByPath("/i")->attributes();
                    Poco::AutoPtr<NamedNodeMap> attr11 = connect->getNodeByPath("/j")->attributes();
                    Poco::AutoPtr<NamedNodeMap> attr12 = connect->getNodeByPath("/t")->attributes();
                    auto roadID = attr9->getNamedItem("value")->innerText();
                    auto _i = attr10->getNamedItem("value")->innerText();
                    auto _j = attr11->getNamedItem("value")->innerText();
                    auto _t = attr12->getNamedItem("value")->innerText();

                    if (print_info) std::cout << " --------roadID: \t" << roadID << endl;
                    if (print_info) std::cout << " --------i: \t" << _i << endl;
                    if (print_info) std::cout << " --------j: \t" << _j << endl;
                    if (print_info) std::cout << " --------t: \t" << _t << endl;

                    if (type == "in") {
                        off_key=true;
                        rs->addConnects(std::stoi(_i), std::stoi(_j), 0, 0, roadID, id, std::stod(_t), 0.0);
                    } else {
                        rs->addConnects(0, 0, std::stoi(_i), std::stoi(_j), id, roadID, 1.0, std::stod(_t));
                    }


                } else {
                    if (j==1) {
                        if (off_key) {
                            rs->addExit(id);
                        } else rs->addEntry(id);
                    }
                    break;
                }
            }
        } else break;
    }
    if (print_info) std::cout<< " --end reading ramp" << endl;
    rs->initExitPool();
    if (print_info) std::cout<< " END READ XML ROAD INFO" << endl;

}


void XMLRoadReader::createRoadSystem(std::vector<RoadPart>) {

}



XMLRoadReader::~XMLRoadReader() {
    // TODO Auto-generated destructor stub
}

