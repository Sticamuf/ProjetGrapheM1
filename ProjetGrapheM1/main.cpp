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

#include "jsonIO.hpp"
#include "edgeMap.hpp"
#include "dispOpenGL.hpp"

using ogdf::Graph;
using ogdf::GraphAttributes;
using std::cout, std::endl;

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
	srand(static_cast<unsigned int>(time(NULL)));

	Graph G;
	GraphAttributes GA{ G/*, GraphAttributes::nodeGraphics | GraphAttributes::nodeType | GraphAttributes::edgeGraphics | GraphAttributes::edgeType |GraphAttributes::edgeStyle*/ };
	int gridWidth, gridHeight, maxBends;

	// ----- LECTURE D'UN FICHIER JSON DANS UN Graph -----
	string file = "test4.json";
	//string file = "auto21-13.json";
	readFromJson(file, G, GA, gridWidth, gridHeight, maxBends);


	// ---- - PLANARISER LE GRAPHE LU---- -
	PlanarStraightLayout PL;
	//PlanarDrawLayout PL;
	//PlanarizationGridLayout PL;
	//MixedModelLayout PL;

	//SchnyderLayout PL;
	//FPPLayout PL;
	//GA.representsCombEmbedding();


	PL.call(GA);



	int nbNodes = 0;
	int maxX = 0, maxY = 0, minX = 100000, minY = 100000;
	node n = G.firstNode();

	int divComm = pgcd(GA, G);
	cout << "pgcd = " << divComm << endl;

	while (n != nullptr) {
		nbNodes++;
		
		GA.x(n) = GA.x(n) / divComm;
		GA.y(n) = GA.y(n) / divComm;

		if (GA.x(n) > maxX) maxX = GA.x(n);
		if (GA.x(n) < minX) minX = GA.x(n);
		if (GA.y(n) > maxY) maxY = GA.x(n);
		if (GA.y(n) < minY) minY = GA.x(n);
		//cout << GA.x(n) << ' ' << GA.y(n) << endl;
		n = n->succ();
	}


	//if (PL.sizeOptimization())
	//	std::cout << "sizeOptimization == true" << std::endl;
	//std::cout << "nb of crossings " << PL.numberOfCrossings() << std::endl;
	std::cout << "nb noeuds " << nbNodes << std::endl;
	std::cout << "minX, minY : " << minX << ' ' << minY << std::endl;
	std::cout << "maxX, maxY : " << maxX << ' ' << maxY << std::endl;
	//std::cout << "devrait etre de (0 ; 0) a (" << 2 * nbNodes - 4 << " ; " << nbNodes - 2 << ") (2*nbNoeuds-4 ; nbNoeuds-2)" << std::endl;

	//GraphIO::write(GA, "output-ERDiagram.gml", GraphIO::writeGML);
	//GraphIO::write(GA, "output-ERDiagram.svg", GraphIO::drawSVG);

	// ----- INITIALISER LES VARIABLES GLOBALES -----
	//std::map<edge, double>::iterator it;
	for (auto it = mapEdgeLength.begin(); it != mapEdgeLength.end(); it++) {
		std::cout << "MapEdgeLength: " << it->second << std::endl;
	}

	//std::map<double, std::set<edge>>::iterator it2;
	for (auto it2 = mapLengthEdgeSet.begin(); it2 != mapLengthEdgeSet.end(); it2++) {
		std::cout << "mapLengthEdgeSet: " << it2->first << std::endl;
	}

	//GA.setAllHeight(0.01);
	//GA.setAllWidth(0.01);
	writeToJson("output.json", G, GA, gridWidth, gridHeight, maxBends);

	// ----- AFFICHAGE OPENGL -----
	dispOpenGL(G, GA, gridWidth, gridHeight,maxX,maxY);

	return 0;
}