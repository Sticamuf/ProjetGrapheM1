#ifndef JSONIO
#define JSONIO

#include <string>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/GraphIO.h>
#include <nlohmann/json.hpp>

#include <ogdf/basic/MinHeap.h>
#include <ogdf/basic/heap/BinaryHeap.h>
#include "calcEdgeLength.h"

using std::string;
using nlohmann::json;
using namespace ogdf;

// ----- LECTURE D'UN Graph DANS UN FICHIER JSON -----
void readFromJson(string input, Graph& G, GraphAttributes& GA, int& gridWidth, int& gridHeight, int& maxBends) {
    BinaryHeap<double> BH;

    std::ifstream i(input);
    json j;
    i >> j;
    gridWidth = j["width"];
    gridHeight = j["height"];
    maxBends = j["bends"];
    if (j["nodes"] == nullptr) {
        exit(1);
    }
    int nodeNumber = static_cast<int>(j["nodes"].size());
    node* nodeTab = new node[nodeNumber];
    for (int i = 0; i < nodeNumber; i++) {
        nodeTab[i] = G.newNode();
        GA.x(nodeTab[i]) = j["nodes"][i]["x"];
        GA.y(nodeTab[i]) = j["nodes"][i]["y"];
    }
    int edgeNumber = static_cast<int>(j["edges"].size());

    BinaryHeapSimple<double> bhs{ edgeNumber };

    edge* edgeTab = new edge[edgeNumber];
    for (int i = 0; i < edgeNumber; i++) {
        edgeTab[i] = G.newEdge(nodeTab[j["edges"][i]["source"]], nodeTab[j["edges"][i]["target"]]);
        if (j["edges"][i]["bends"] != nullptr) {
            DPolyline& p = GA.bends(edgeTab[i]);
            int bendsNumber = static_cast<int>(j["edges"][i]["bends"].size());
            for (int k = 0; k < bendsNumber; k++) {
                p.pushBack(DPoint(j["edges"][i]["bends"][k]["x"], j["edges"][i]["bends"][k]["y"]));
            }
        }
        //recuperer longueur edge
        BH.push(calcEdgeLength(edgeTab[i], GA));
        double tmp = calcEdgeLength(edgeTab[i], GA);
        bhs.insert(tmp);
    }

    for (int i = 1; i <= edgeNumber; i++)
    {
        std::cout << "BHS " << i - 1 << ": " << bhs[i - 1] << std::endl;
        std::cout << "Edge " << i << ": " << BH.value(&i) << std::endl;
    }

    delete[] nodeTab;
    delete[] edgeTab;
}

// ----- ECRITURE D'UN Graph DANS UN FICHIER JSON -----
void writeToJson(string output, const Graph& G, const GraphAttributes& GA, int gridWidth, int gridHeight, int maxBends) {
    json j2;
    j2["width"] = gridWidth;
    j2["height"] = gridHeight;
    j2["bends"] = maxBends;

    node n = G.firstNode();
    int m = 0;
    while (n != nullptr) {
        j2["nodes"][m]["id"] = n->index();
        j2["nodes"][m]["x"] = (int)GA.x(n);
        j2["nodes"][m]["y"] = (int)GA.y(n);
        n = n->succ();
        m++;
    }

    edge e = G.firstEdge();
    m = 0;
    while (e != nullptr) {
        j2["edges"][m]["source"] = e->source()->index();
        j2["edges"][m]["target"] = e->target()->index();
        DPolyline bends = GA.bends(e);
        if (bends.size() > 0) {
            int l = 0;
            for (ListIterator<DPoint> i = bends.begin(); i.valid(); i++) {
                j2["edges"][m]["bends"][l]["x"] = (int)(*i).m_x;
                j2["edges"][m]["bends"][l]["y"] = (int)(*i).m_y;
                l++;
            }
        }
        e = e->succ();
        m++;
    }

    std::ofstream o(output);
    o << std::setw(4) << j2 << std::endl;
}

#endif