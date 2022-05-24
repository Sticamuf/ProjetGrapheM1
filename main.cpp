#include <ogdf/basic/GridLayout.h>

#include <ogdf/basic/graph_generators.h>

#include <ogdf/planarity/PlanarizationLayout.h>
#include <ogdf/planarity/SubgraphPlanarizer.h>

#include <ogdf/planarlayout/PlanarStraightLayout.h>

#include <ogdf/planarity/EmbedderMinDepth.h>

#include <ogdf/basic/simple_graph_alg.h>

#include "jsonIO.hpp"
#include "edgeMap.hpp"
#include "dispOpenGL.hpp"
#include "embedder.hpp"

using ogdf::Graph;
using ogdf::GraphAttributes;
using std::cout, std::endl;



int main() {

    Graph G;
    GridLayout GL{ G };

    int gridWidth, gridHeight, maxBends;

    // ----- LECTURE D'UN FICHIER JSON DANS UN Graph -----
    string file = "test4.json";
    readFromJson(file, G, GL, gridWidth, gridHeight, maxBends);
    writeToJson("output.json", G, GL, gridWidth, gridHeight, maxBends);

    node nsrc = G.firstNode();
    SListPure<adjEntry> adj;
    int i = 0;
    while (nsrc != nullptr) {
        nsrc->allAdjEntries(adj);
        nsrc = nsrc->succ();
        std::cout << "Nb Adj du noeud " << i << ": " << adj.size() << std::endl;
        i++;
    }

    std::cout << G.representsCombEmbedding() << std::endl;
    int maxX = 0, maxY = 0, minX = 100000, minY = 100000;

    embedderCarte(G, GL);
    std::cout << "Embeded: " << G.representsCombEmbedding() << std::endl;

    bool planarize = false;
    std::cout << "Connexe: " << isConnected(G) << std::endl;
    std::cout << "Planaire: " << isPlanar(G) << std::endl;
    if (planarize) {
        PlanarStraightLayout PL;
        PL.separation(-19); // pas par defaut = 20 multiplie les coord par 40, -19 donne le pas 1 (distance minimale entre deux points)
        PL.callGridFixEmbed(G, GL);
        node n = G.firstNode();
        while (n != nullptr) {
            if (GL.x(n) > maxX) maxX = GL.x(n);
            if (GL.x(n) < minX) minX = GL.x(n);
            if (GL.y(n) > maxY) maxY = GL.x(n);
            if (GL.y(n) < minY) minY = GL.x(n);
            n = n->succ();
        }
        std::cout << "Planarised" << std::endl;
    }
    //GraphIO::write(GL, "output-ERDiagram2.svg", GraphIO::drawSVG);
    std::cout << "Connexe: " << isConnected(G) << std::endl;
    std::cout << "Planaire: " << isPlanar(G) << std::endl;

    // Affichage des maps
    /*
    std::map<edge, double>::iterator it;
    for (it = mapEdgeLength.begin(); it != mapEdgeLength.end(); it++) {
        std::cout << "MapEdgeLength: " << it->second << std::endl;
    }
    std::map<double, std::set<edge>>::iterator it2;
    for (it2 = mapLengthEdgeSet.begin(); it2 != mapLengthEdgeSet.end(); it2++) {
        std::cout << "mapLengthEdgeSet: " << it2->first << std::endl;
    }
    */

    // OpenGL
    srand(static_cast<unsigned int>(time(NULL)));
    dispOpenGL(G, GL, gridWidth, gridHeight, maxX, maxY);
    return 0;
}