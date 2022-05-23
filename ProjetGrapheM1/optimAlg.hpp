#ifndef OPTIM_ALG
#define OPTIM_ALG

#include "edgeMap.hpp"
#include "nodeMap.hpp"

double calcMoyenneEdgeLength() {
	int size = mapEdgeLength.size();
	std::map<edge, double>::iterator it = mapEdgeLength.begin();
	double moy = it->second / size;
	it++;
	while (it != mapEdgeLength.end()) {
		moy += it->second / size; //division en cours de route pour ne pas dépasser la taille des double, moins efficace mais plus sécurisé
		it++;
	}
	return moy;
}

double calcEcartTypeCarreEdgeLength() {
	std::map<edge, double>::iterator it = mapEdgeLength.begin();
	double moy = calcMoyenneEdgeLength();
	double sum = (it->second - moy) * (it->second - moy);
	it++;
	while (it != mapEdgeLength.end()) {
		sum += (it->second - moy) * (it->second - moy);
		it++;
	}
	return sum/mapEdgeLength.size();
}

#endif OPTIM_ALG