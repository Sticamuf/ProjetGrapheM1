#include <ogdf/basic/GridLayout.h>

#include <ogdf/planarlayout/PlanarStraightLayout.h>
#include <ogdf/planarity/EmbedderMinDepth.h>
#include <ogdf/planarity/SimpleEmbedder.h>
#include <ogdf/basic/simple_graph_alg.h>

#include "jsonIO.hpp"
#include "dispOpenGL.hpp"
#include "EdgeMap.hpp"
#include "NodeMap.hpp"
#include "graphFunctions.hpp"

using ogdf::Graph;
using ogdf::GridLayout;
using std::cout, std::endl;

int main() {

	Graph G;
	GridLayout GL{ G };

	int gridWidth, gridHeight, maxBends;

	// ----- LECTURE D'UN FICHIER JSON DANS UN Graph -----
	string file = "exemples/exempleBonEmbedding.json";
	readFromJson(file, G, GL, gridWidth, gridHeight, maxBends);
	writeToJson("output.json", G, GL, gridWidth, gridHeight, maxBends);

	int maxX = 0, maxY = 0, minX = 100000, minY = 100000;

	bool planarize = true;
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

	// Ajout des node dans le vector
	node n = G.firstNode();
	while (n != nullptr) {
		NodeBend tmpNodeBend(n, GL);
		vectorNodeBends.push_back(tmpNodeBend);
		setNodeBends.insert(tmpNodeBend);
		n = n->succ();
	}

	// Ajout des bend dans le vector
	edge e = G.firstEdge();
	while (e != nullptr) {
		IPolyline& bends = GL.bends(e);
		int k = 0;
		for (ListIterator<IPoint> i = bends.begin(); i.valid(); i++, k++) {
			NodeBend tmpNodeBend((*i), e, k);
			vectorNodeBends.push_back(tmpNodeBend);
			setNodeBends.insert(tmpNodeBend);
		}
		e = e->succ();
	}
	//for (const auto& it : vectorNodeBends) {
	//    cout << "vector : " << *it.a_x << " " << *it.a_y << endl;
	//}
	//for (const auto& it : setNodeBends) {
	//    cout << "set : " << *it.a_x << " " << *it.a_y << endl;
	//    //cout << "set address " << it.a_x << endl;
	//}
	//cout << "vector size : " << vectorNodeBends.size() << endl;
	//cout << "set size : " << setNodeBends.size() << endl;
	//for (int i = 0; i < mapPosNode.size(); i++) {
	//    for (int j = 0; j < mapPosNode[i].size(); j++) {
	//        cout << mapPosNode[i][j] << " ";
	//    }
	//    cout << endl;
	//}
	//cout << gridWidth << " " << gridHeight << endl;

	////test pour vérifier que changer la valeur du set la change dans le vector -> fonctionne || l'autre sens fonctionne aussi
	//int i = 0;
	//NodeBend nb = vectorNodeBends[i];
	//auto itnb = setNodeBends.find(nb);
	//cout << "vetor : " << *vectorNodeBends[i].a_x << " " << *vectorNodeBends[i].a_y << endl;
	//cout << "nb : " << *nb.a_x << " " << *nb.a_y << endl;
	//cout << "itnb : " << *(*itnb).a_x << " " << *(*itnb).a_y << endl;
	//*(nb).a_x += 1;
	//*(nb).a_y -= 1;
	//cout << "vetor : " << *vectorNodeBends[i].a_x << " " << *vectorNodeBends[i].a_y << endl;
	//cout << "nb : " << *nb.a_x << " " << *nb.a_y << endl;
	//cout << "itnb : " << *(*itnb).a_x << " " << *(*itnb).a_y << endl;

	// OpenGL
	srand(static_cast<unsigned int>(time(NULL)));
	dispOpenGL(G, GL, gridWidth, gridHeight, maxX, maxY);
	return 0;
}