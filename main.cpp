#include <ogdf/basic/GraphAttributes.h>

#include <ogdf/planarlayout/PlanarStraightLayout.h>
#include <ogdf/planarity/EmbedderMinDepth.h>

#include "jsonIO.hpp"
#include "dispOpenGL.hpp"
#include "EdgeMap.hpp"
#include "NodeMap.hpp"

using ogdf::Graph;
using ogdf::GraphAttributes;

int gcd(int a, int b) {
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

int pgcd(const GraphAttributes& GA, const Graph& G)
{
    int pgcd = INT_MAX;
    node n = G.firstNode();
    while (n != nullptr)
    {
        int candidat = gcd(GA.x(n), GA.y(n));
        if (pgcd > candidat)
        {
            pgcd = candidat;
        }

        n = n->succ();
    }
    return pgcd;
}

int main() {
    
    Graph G;
    GraphAttributes GA{G};
    GA.directed() = false;

    int gridWidth, gridHeight, maxBends;

    // ----- LECTURE D'UN FICHIER JSON DANS UN Graph -----
    string file = "test4.json";
    readFromJson(file, G, GA, gridWidth, gridHeight, maxBends);
    writeToJson("output.json", G, GA, gridWidth, gridHeight, maxBends);

    std::cout << G.representsCombEmbedding() << std::endl;

    int maxX = 0, maxY = 0, minX = 100000, minY = 100000;

    bool planarize = true;

    if (planarize) {
        PlanarStraightLayout PL;
        PL.call(GA);
        int divComm = pgcd(GA, G);
        int nbNodes = 0;

        node n = G.firstNode();

        while (n != nullptr) {
            nbNodes++;

            GA.x(n) = GA.x(n) / divComm;
            GA.y(n) = GA.y(n) / divComm;

            if (GA.x(n) > maxX) maxX = GA.x(n);
            if (GA.x(n) < minX) minX = GA.x(n);
            if (GA.y(n) > maxY) maxY = GA.x(n);
            if (GA.y(n) < minY) minY = GA.x(n);
            n = n->succ();
        }
    }

    EmbedderMinDepth Emf;
    adjEntry tmpAdj = G.firstNode()->firstAdj();
    Emf.doCall(G, tmpAdj);

    std::cout << G.representsCombEmbedding() << std::endl;

    std::map<edge, double>::iterator it;
    for (it = mapEdgeLength.begin(); it != mapEdgeLength.end(); it++) {
        std::cout << "MapEdgeLength: " << it->second << std::endl;
    }

    std::map<double, std::set<edge>>::iterator it2;
    for (it2 = mapLengthEdgeSet.begin(); it2 != mapLengthEdgeSet.end(); it2++) {
        std::cout << "mapLengthEdgeSet: " << it2->first << std::endl;
    }

    // Affichage openGL
    srand(static_cast<unsigned int>(time(NULL)));

    //debut opengl
    dispOpenGL(G,GA,gridWidth,gridHeight,maxX,maxY);
    //fin opengl
    return 0;
}