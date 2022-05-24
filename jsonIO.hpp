#ifndef JSONIO
#define JSONIO

#include <string>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/GraphIO.h>
#include <nlohmann/json.hpp>

#include <ogdf/basic/MinHeap.h>
#include <ogdf/basic/heap/BinaryHeap.h>
#include "calcEdgeLength.hpp"
#include "EdgeMap.hpp"
#include "NodeMap.hpp"

using std::string;
using nlohmann::json;
using namespace ogdf;

// ----- CREATION D'UN Graph A PARTIR D'UN FICHIER JSON -----
void readFromJson(string input, Graph& G, GridLayout& GL, int& gridWidth, int& gridHeight, int& maxBends) {
    std::ifstream i(input);
    json j;
    i >> j;
    gridWidth = j["width"];
    gridHeight = j["height"];
    maxBends = j["bends"];
    // Remplissage de mapPosNode
    for (int i = 0; i <= gridHeight; i++) {
        std::vector<bool> tmpVector;
        for (int j = 0; j <= gridWidth; j++) {
            tmpVector.push_back(false);
        }
        mapPosNode.push_back(tmpVector);
    }
    if (j["nodes"] == nullptr) {
        exit(1);
    }
    int nodeNumber = static_cast<int>(j["nodes"].size());
    node* nodeTab = new node[nodeNumber];
    for (int i = 0; i < nodeNumber; i++) {
        nodeTab[i] = G.newNode();
        GL.x(nodeTab[i]) = j["nodes"][i]["x"];
        GL.y(nodeTab[i]) = j["nodes"][i]["y"];
        std::pair<int, int> coord((int)GL.x(nodeTab[i]), (int)GL.y(nodeTab[i]));
        mapPosNode[(int)GL.y(nodeTab[i])][(int)GL.x(nodeTab[i])] = true;
    }
    int edgeNumber = static_cast<int>(j["edges"].size());
    edge* edgeTab = new edge[edgeNumber];
    for (int i = 0; i < edgeNumber; i++) {

        // DEBUT TEST
        node n1 = nodeTab[j["edges"][i]["source"]];
        node n2 = nodeTab[j["edges"][i]["target"]];
        if ((n1->lastAdj() != nullptr) && (n2->lastAdj() != nullptr)) {
            edgeTab[i] = G.newEdge(n1->lastAdj(), n2->lastAdj());
        }
        else if ((n1->lastAdj() == nullptr) && (n2->lastAdj() == nullptr)) {
            edgeTab[i] = G.newEdge(n1, n2);
        }
        else if (n1->lastAdj() == nullptr) {
            edgeTab[i] = G.newEdge(n1, n2->lastAdj());
        }
        else {
            edgeTab[i] = G.newEdge(n1->lastAdj(), n2);
        }
        // -- FIN TEST
        //edgeTab[i] = G.newEdge(nodeTab[j["edges"][i]["source"]], nodeTab[j["edges"][i]["target"]]);

        if (j["edges"][i]["bends"] != nullptr) {
            IPolyline& p = GL.bends(edgeTab[i]);
            int bendsNumber = static_cast<int>(j["edges"][i]["bends"].size());
            for (int k = 0; k < bendsNumber; k++) {
                p.pushBack(IPoint(j["edges"][i]["bends"][k]["x"], j["edges"][i]["bends"][k]["y"]));
            }
        }
        //recuperer longueur edge
        double length = calcEdgeLength(edgeTab[i], GL);
        mapEdgeLength.insert(std::pair<edge, double>(edgeTab[i], length));
        std::map<double, std::set<edge>>::iterator it2 = mapLengthEdgeSet.begin();
        it2 = mapLengthEdgeSet.find(length);
        // La valeur est deja presente, on ajoute dans le set
        if (it2 != mapLengthEdgeSet.end()) {
            it2->second.insert(edgeTab[i]);
        }
        // La valeur n'est pas presente, on creer un nouveau set.
        else {
            std::set<edge> tmpSet;
            tmpSet.insert(edgeTab[i]);
            mapLengthEdgeSet.insert(std::pair<double, std::set<edge>>(length, tmpSet));
        }

    }

    delete[] nodeTab;
    delete[] edgeTab;
}

// ----- ECRITURE D'UN Graph DANS UN FICHIER JSON -----
void writeToJson(string output, const Graph& G, const GridLayout& GL, int gridWidth, int gridHeight, int maxBends) {
    json j2;
    j2["width"] = gridWidth;
    j2["height"] = gridHeight;
    j2["bends"] = maxBends;

    node n = G.firstNode();
    int m = 0;
    while (n != nullptr) {
        j2["nodes"][m]["id"] = n->index();
        j2["nodes"][m]["x"] = (int)GL.x(n);
        j2["nodes"][m]["y"] = (int)GL.y(n);
        n = n->succ();
        m++;
    }

    edge e = G.firstEdge();
    m = 0;
    while (e != nullptr) {
        j2["edges"][m]["source"] = e->source()->index();
        j2["edges"][m]["target"] = e->target()->index();
        IPolyline bends = GL.bends(e);
        if (bends.size() > 0) {
            int l = 0;
            for (ListIterator<IPoint> i = bends.begin(); i.valid(); i++) {
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