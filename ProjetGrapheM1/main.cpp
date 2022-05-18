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

//int gcd(int a, int b) {
//	if (b == 0)
//		return a;
//	return gcd(b, a % b);
//}
//
//void pgcd(const GraphAttributes& GA, const Graph& G, int& pgcdX, int& pgcdY)
//{
//	pgcdX = INT_MAX;
//	node n = G.firstNode();
//	while (n != nullptr && GA.x(n) == 0) {
//		n = n->succ();
//	}
//	if (GA.x(n) == 0) {
//		cout << "ERREUR : tous les X sont à 0" << endl;
//		return;
//	}
//	pgcdX = GA.x(n);
//	int candidatX;
//	while (n != nullptr) {
//		candidatX = gcd(pgcdX, GA.x(n));
//		if (pgcdX > candidatX) {
//			pgcdX = candidatX;
//		}
//		n = n->succ();
//	}
//	if (pgcdX == INT_MAX) {
//		cout << "ERREUR : pgcdX = INT_MAX" << endl;
//	}
//
//	pgcdY = INT_MAX;
//	n = G.firstNode();
//	while (GA.y(n) == 0 && n != nullptr) {
//		n = n->succ();
//	}
//	if (GA.y(n) == 0) {
//		cout << "ERREUR : tous les Y sont à 0" << endl;
//		return;
//	}
//	int candidatY;
//	pgcdY = GA.y(n);
//	while (n != nullptr) {
//		candidatY = gcd(pgcdY, GA.y(n));
//		if (pgcdY > candidatY) {
//			pgcdY = candidatY;
//		}
//		n = n->succ();
//	}
//	if (pgcdY == INT_MAX) {
//		cout << "ERREUR : pgcdY = INT_MAX" << endl;
//	}
//}


int main() {
	srand(static_cast<unsigned int>(time(NULL)));

	Graph G;
	//GraphAttributes GA{ G/*, GraphAttributes::nodeGraphics | GraphAttributes::nodeType | GraphAttributes::edgeGraphics | GraphAttributes::edgeType |GraphAttributes::edgeStyle*/ };
	//GA.directed() = false;
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
	

	//cout << G.representsCombEmbedding() << endl;
	//PL.callFixEmbed(GA);
	cout << "separation "<< PL.separation() << endl;



	//int pgcdX, pgcdY;
	//pgcd(GA, G, pgcdX, pgcdY);
	//cout << "pgcdX = " << pgcdX << " ,pgcdY = " << pgcdY << endl;

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

		//if (pgcdX != 0) {
		//	GA.x(n) = GA.x(n) / pgcdX;
		//}
		//if (pgcdY != 0) {
		//	GA.y(n) = GA.y(n) / pgcdY;
		//}


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

	//GA.setAllHeight(0.01);
	//GA.setAllWidth(0.01);
	writeToJson("output.json", G, GL, gridWidth, gridHeight, maxBends);

	// ----- AFFICHAGE OPENGL -----
	dispOpenGL(G, GL, gridWidth, gridHeight, maxX, maxY);


	return 0;
}