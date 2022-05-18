#include <ogdf/basic/GraphAttributes.h>

#include <ogdf/basic/graph_generators.h>

#include <ogdf/planarity/PlanarizationLayout.h>
#include <ogdf/planarity/SubgraphPlanarizer.h>

#include <ogdf/planarlayout/PlanarStraightLayout.h>
#include <ogdf/planarlayout/PlanarDrawLayout.h>
#include <ogdf/planarlayout/SchnyderLayout.h>
#include <ogdf/planarity/PlanarizationGridLayout.h>
#include <ogdf/planarlayout/FPPLayout.h>
#include <ogdf/planarlayout/MixedModelLayout.h>

#include <ogdf/planarity/EmbedderMinDepth.h>

#include <ogdf/basic/simple_graph_alg.h>

#include "jsonIO.hpp"
#include "edgeMap.hpp"
#include "dispOpenGL.hpp"

using ogdf::Graph;
using ogdf::GraphAttributes;
using std::cout, std::endl;




int main() {
	srand(static_cast<unsigned int>(time(NULL)));

	Graph G;
	GridLayout GL{ G };
	int gridWidth, gridHeight, maxBends;

	// ----- LECTURE D'UN FICHIER JSON DANS UN Graph -----
	//string file = "test4.json";
	string file = "auto21-5.json";
	//string file = "auto21-13.json";
	//string file = "testConnex.json";
	readFromJson(file, G, GL, gridWidth, gridHeight, maxBends);

	cout << "is connected " << ogdf::isConnected(G) << endl;

	// ---- - PLANARISER LE GRAPHE LU---- -
	PlanarStraightLayout PL;
	//PlanarDrawLayout PL;
	//PlanarizationGridLayout PL;
	//MixedModelLayout PL;

	//SchnyderLayout PL;
	//FPPLayout PL;

	PL.separation(-19);

	cout << "separation "<< PL.separation() << endl;

	EmbedderMinDepth* Emf = new EmbedderMinDepth;
	PL.setEmbedder(Emf);
	adjEntry tmpAdj;
	
	//PL.call(GA);
	Emf->doCall(G, tmpAdj);
	PL.callGridFixEmbed(G,GL);
	cout << "G represents comb embedding : " << G.representsCombEmbedding() << endl;
	

	int nbNodes = 0;
	int maxX = 0, maxY = 0, minX = 100000, minY = 100000;
	node n = G.firstNode();
	
	while (n != nullptr) {
		nbNodes++;

		//cout << "(x;y) (" << GA.x(n) << ";" << GA.y(n) << ")" << endl;

		if (GL.x(n) > maxX) maxX = GL.x(n);
		if (GL.x(n) < minX) minX = GL.x(n);
		if (GL.y(n) > maxY) maxY = GL.y(n);
		if (GL.y(n) < minY) minY = GL.y(n);
		//cout << GA.x(n) << ' ' << GA.y(n) << endl;
		n = n->succ();
	}

	std::cout << "nb noeuds " << nbNodes << std::endl;
	std::cout << "maxX, maxY : " << maxX << ' ' << maxY << std::endl;

	//GraphIO::write(GA, "output-ERDiagram.gml", GraphIO::writeGML);
	//GraphIO::write(GA, "output-ERDiagram.svg", GraphIO::drawSVG);


	//// ----- INITIALISER LES VARIABLES GLOBALES -----
	////std::map<edge, double>::iterator it;
	//for (auto it = mapEdgeLength.begin(); it != mapEdgeLength.end(); it++) {
	//	std::cout << "MapEdgeLength: " << it->second << std::endl;
	//}

	////std::map<double, std::set<edge>>::iterator it2;
	//for (auto it2 = mapLengthEdgeSet.begin(); it2 != mapLengthEdgeSet.end(); it2++) {
	//	std::cout << "mapLengthEdgeSet: " << it2->first << std::endl;
	//}

	writeToJson("output.json", G, GL, gridWidth, gridHeight, maxBends);

	// ----- AFFICHAGE OPENGL -----
	dispOpenGL(G, GL, gridWidth, gridHeight, maxX, maxY);


	return 0;
}