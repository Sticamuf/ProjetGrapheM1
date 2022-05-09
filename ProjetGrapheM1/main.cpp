#include <ogdf/basic/GraphAttributes.h>
#include "jsonIO.hpp"
#include "dispOpenGL.hpp"

#include <ogdf/basic/graph_generators.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/orthogonal/OrthoLayout.h>
#include <ogdf/planarity/EmbedderMinDepthMaxFaceLayers.h>
#include <ogdf/planarity/PlanarizationLayout.h>
#include <ogdf/planarity/SubgraphPlanarizer.h>
#include <ogdf/planarity/VariableEmbeddingInserter.h>
//#include <ogdf/planarity/PlanarSubgraphFast.h>

using ogdf::Graph;
using ogdf::GraphAttributes;

int main() {

    Graph G;
    
    int gridWidth, gridHeight, maxBends;
    int n = 100, m = 150;
    gridWidth = n, gridHeight = 2*n, maxBends = 0;
    randomSimpleGraph(G, n, m);

    GraphAttributes GA{ G,  GraphAttributes::nodeGraphics | GraphAttributes::nodeType |
      GraphAttributes::edgeGraphics | GraphAttributes::edgeType
    };

    // ----- LECTURE D'UN FICHIER JSON DANS UN Graph -----
    string file = "test.json";
    
    //readFromJson(file, G, GA, gridWidth, gridHeight, maxBends);
    writeToJson("output.json", G, GA, gridWidth, gridHeight, maxBends);

    PlanarizationLayout pl;

    SubgraphPlanarizer* crossMin = new SubgraphPlanarizer;
    //PlanarSubgraphFast<int>* ps = new PlanarSubgraphFast<int>;
    //ps->runs(100);
    VariableEmbeddingInserter* ves = new VariableEmbeddingInserter;
    ves->removeReinsert(RemoveReinsertType::All);

    //crossMin->setSubgraph(ps);
    crossMin->setInserter(ves);
    pl.setCrossMin(crossMin);

    EmbedderMinDepthMaxFaceLayers* emb = new EmbedderMinDepthMaxFaceLayers;
    pl.setEmbedder(emb);

    OrthoLayout* ol = new OrthoLayout;
    ol->separation(20.0);
    ol->cOverhang(0.4);
    pl.setPlanarLayouter(ol);

    pl.call(GA);

    GraphIO::write(GA, "output-ERDiagram.gml", GraphIO::writeGML);
    GraphIO::write(GA, "output-ERDiagram.svg", GraphIO::drawSVG);

    // Affichage openGL
    srand(static_cast<unsigned int>(time(NULL)));

    //debut opengl
    dispOpenGL(G,GA,gridWidth,gridHeight);
    //fin opengl
    return 0;
}