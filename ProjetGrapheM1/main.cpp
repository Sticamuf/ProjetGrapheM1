#include <ogdf/basic/GraphAttributes.h>

#include <ogdf/basic/graph_generators.h>

#include <ogdf/planarity/PlanarizationLayout.h>
#include <ogdf/planarity/SubgraphPlanarizer.h>
//#include <ogdf/planarity/MaximumPlanarSubgraph.h>
#include <ogdf/planarity/MaximalPlanarSubgraphSimple.h>
#include <ogdf/planarity/SubgraphPlanarizer.h>
#include <ogdf/planarity/VariableEmbeddingInserter.h>
#include <ogdf/orthogonal/OrthoLayout.h>
#include <ogdf/planarity/EmbedderMinDepthMaxFaceLayers.h>

#include <ogdf/planarity/PlanarizationGridLayout.h>
#include <ogdf/planarlayout/MixedModelLayout.h>

#include <ogdf/planarity/PlanarSubgraphFast.h>

#include "jsonIO.hpp"
#include "edgeMap.hpp"
#include "dispOpenGL.hpp"

using ogdf::Graph;
using ogdf::GraphAttributes;



int main() {
    srand(static_cast<unsigned int>(time(NULL)));

    Graph G;

    int gridWidth, gridHeight, maxBends;
    int n = 10, m = 15;
    gridWidth = n, gridHeight = 2 * n, maxBends = 0;
    randomSimpleGraph(G, n, m);

    GraphAttributes GA{ G,
      GraphAttributes::nodeGraphics | GraphAttributes::nodeType |
      GraphAttributes::edgeGraphics | GraphAttributes::edgeType };

    //PlanarizationLayout pl;
    PlanarizationGridLayout pl;
    //pl.call(GA, G);
    pl.call(GA);
    std::cout << "nb of crossings : " << pl.numberOfCrossings() << std::endl;
    SubgraphPlanarizer* crossMin = new SubgraphPlanarizer;
    MaximalPlanarSubgraphSimple<int>* ps = new MaximalPlanarSubgraphSimple<int>;

    VariableEmbeddingInserter* ves = new VariableEmbeddingInserter;
    ves->removeReinsert(RemoveReinsertType::All);

    crossMin->setSubgraph(ps);
    crossMin->setInserter(ves);
    pl.setCrossMin(crossMin);

    EmbedderMinDepthMaxFaceLayers* emb = new EmbedderMinDepthMaxFaceLayers;
    //pl.setEmbedder(emb);

    OrthoLayout* ol = new OrthoLayout;
    ol->separation(20.0);
    ol->cOverhang(0.4);
    //pl.setPlanarLayouter(ol);
    MixedModelLayout* mlp = new MixedModelLayout;
    mlp->separation(20.0);
    pl.setPlanarLayouter(mlp);

    pl.call(GA);




    GraphIO::write(GA, "output-ERDiagram.gml", GraphIO::writeGML);
    GraphIO::write(GA, "output-ERDiagram.svg", GraphIO::drawSVG);

    // ----- LECTURE D'UN FICHIER JSON DANS UN Graph -----
    string file = "auto21-13.json";
    readFromJson(file, G, GA, gridWidth, gridHeight, maxBends);
    writeToJson("output.json", G, GA, gridWidth, gridHeight, maxBends);

    std::map<edge, double>::iterator it;
    for (it = mapEdgeLength.begin(); it != mapEdgeLength.end(); it++) {
        std::cout << "MapEdgeLength: " << it->second << std::endl;
    }

    std::map<double, std::set<edge>>::iterator it2;
    for (it2 = mapLengthEdgeSet.begin(); it2 != mapLengthEdgeSet.end(); it2++) {
        std::cout << "mapLengthEdgeSet: " << it2->first << std::endl;
    }
    
    //debut opengl
    dispOpenGL(G,GA,gridWidth,gridHeight);
    //fin opengl
    return 0;
}