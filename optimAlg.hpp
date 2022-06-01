#ifndef OPTIM_ALG
#define OPTIM_ALG

#include <ogdf/basic/GridLayout.h>
#include "edgeMap.hpp"
#include "nodeMap.hpp"

using ogdf::GridLayout;
using ogdf::node;
using ogdf::IPolyline;
using ogdf::IPoint;
using ogdf::ListIterator;

//renvoie n fois la variance étant données :
// - la somme des longueurs des edge dans sommeLong
// - la somme des longeurs des edge au carré dans sommeLong2
double calcNVar(double sommeLong, double sommeLong2) {
	return mapEdgeLength.size() * sommeLong2 - sommeLong * sommeLong;
}

//calcule la somme des longueurs des edge dans sommeLong
//calcule la somme les longueurs des edge au carre dans sommeLong2
//calcule n fois la variance dans nVar
void prepCalcNVar(double& sommeLong, double& sommeLong2, double& nVar) {
	sommeLong = 0;
	sommeLong2 = 0;
	for (auto it = mapEdgeLength.begin(); it != mapEdgeLength.end(); it++) {
		sommeLong += it->second;
		sommeLong2 += it->second * it->second;
	}
	nVar = calcNVar(sommeLong, sommeLong2);
}

//prend en parametre une longueur d'un edge et la soustrait de sommeLong et sommeLong2
void deleteEdgeNVar(double edgeLength, double& sommeLong, double& sommelong2) {
	sommeLong -= edgeLength;
	sommelong2 -= edgeLength*edgeLength;
}

//prend en paramètre une longueur d'un edge et l'ajoute à sommeLong et someLong2
void addEdgeNVar(double edgeLength, double& sommeLong, double& sommeLong2) {
	sommeLong += edgeLength;
	sommeLong2 += edgeLength * edgeLength;
}


#endif OPTIM_ALG