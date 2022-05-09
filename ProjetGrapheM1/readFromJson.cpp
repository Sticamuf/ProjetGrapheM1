#include "readFromJson.h"

// ----- CREATION D'UN Graph A PARTIR D'UN FICHIER JSON -----
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
        std::cout << "Edge " << i << ": " << calcEdgeLength(edgeTab[i], GA) << std::endl;
    }

    delete[] nodeTab;
    delete[] edgeTab;
}