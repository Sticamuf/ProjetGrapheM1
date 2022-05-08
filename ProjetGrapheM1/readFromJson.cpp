#include <string>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/fileformats/GraphIO.h>
#include <nlohmann/json.hpp>

using std::string;
using nlohmann::json;
using namespace ogdf;

// ----- CREATION D'UN Graph A PARTIR D'UN FICHIER JSON -----
void readFromJson(string input, Graph& G, GraphAttributes& GA, int gridWidth, int gridHeight, int maxBends) {
    std::ifstream i(input);
    json j;
    i >> j;
    gridWidth = j["width"];
    gridHeight = j["height"];
    maxBends = j["bends"];
    if (j["nodes"] == nullptr) {
        exit(1);
    }
    int nodeNumber = j["nodes"].size();
    node* nodeTab = new node[nodeNumber];
    for (int i = 0; i < nodeNumber; i++) {
        nodeTab[i] = G.newNode();
        GA.x(nodeTab[i]) = j["nodes"][i]["x"];
        GA.y(nodeTab[i]) = j["nodes"][i]["y"];
    }
    int edgeNumber = j["edges"].size();
    edge* edgeTab = new edge[edgeNumber];
    for (int i = 0; i < edgeNumber; i++) {
        edgeTab[i] = G.newEdge(nodeTab[j["edges"][i]["source"]], nodeTab[j["edges"][i]["target"]]);
        if (j["edges"][i]["bends"] != nullptr) {
            DPolyline& p = GA.bends(edgeTab[i]);
            int bendsNumber = j["edges"][i]["bends"].size();
            for (int k = 0; k < bendsNumber; k++) {
                p.pushBack(DPoint(j["edges"][i]["bends"][k]["x"], j["edges"][i]["bends"][k]["y"]));
            }
        }
        //recuperer longueur edge
        std::cout << "Edge " << i << ": " << calcEdgeLength(edgeTab[i], GA) << std::endl;
    }

    delete[] nodeTab;
    delete[] edgeTab;
}