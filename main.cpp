#include <ogdf/basic/GridLayout.h>

#include <ogdf/planarlayout/PlanarStraightLayout.h>
#include <ogdf/planarity/EmbedderMinDepth.h>
#include <ogdf/planarity/SimpleEmbedder.h>
#include <ogdf/basic/simple_graph_alg.h>

#include "jsonIO.hpp"
#include "dispOpenGL.hpp"
#include "EdgeMap.hpp"
#include "NodeMap.hpp"
#include "embedder.hpp"

using ogdf::Graph;
using ogdf::GridLayout;
using std::cout, std::endl;

int main() {
    
    Graph G;
    GridLayout GL{G};

    int gridWidth, gridHeight, maxBends;

    // ----- LECTURE D'UN FICHIER JSON DANS UN Graph -----
    string file = "exemples/test.json";
    readFromJson(file, G, GL, gridWidth, gridHeight, maxBends);
    writeToJson("output.json", G, GL, gridWidth, gridHeight, maxBends);
    
    int maxX = 0, maxY = 0, minX = 100000, minY = 100000;

    bool planarize = false;
    if (planarize) {
        std::cout << "Planarizing..." << std::endl;
        PlanarStraightLayout PL;
        PL.separation(-19);
        PL.callGrid(G, GL);
        node n = G.firstNode();
        while (n != nullptr) {
            if (GL.x(n) > maxX) maxX = GL.x(n);
            if (GL.x(n) < minX) minX = GL.x(n);
            if (GL.y(n) > maxY) maxY = GL.x(n);
            if (GL.y(n) < minY) minY = GL.x(n);
            n = n->succ();
        }
    }

    std::cout << "Embedding..." << std::endl;
    embedderCarte(G, GL);
    std::cout << "Embeded: " << G.representsCombEmbedding() << std::endl;

    //GraphIO::write(GL, "output-ERDiagram2.svg", GraphIO::drawSVG);
    std::cout << "Connexe: " << isConnected(G) << std::endl;
    std::cout << "Planaire: " << isPlanar(G) << std::endl;

    // Affichage des maps
    
    std::map<edge, double>::iterator it;
    for (it = mapEdgeLength.begin(); it != mapEdgeLength.end(); it++) {
        std::cout << "MapEdgeLength: " << it->second << std::endl;
    }

    std::map<double, std::set<edge>>::iterator it2;
    for (it2 = mapLengthEdgeSet.begin(); it2 != mapLengthEdgeSet.end(); it2++) {
        std::cout << "mapLengthEdgeSet: " << it2->first << std::endl;
    }
    

    // OpenGL
    srand(static_cast<unsigned int>(time(NULL)));
    dispOpenGL(G,GL,gridWidth,gridHeight,maxX,maxY);
    return 0;
}