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

//calcule la variance en fonction de : la somme des carrés des longueurs divisé par le nombre de edge sommeVar,  et de la moyenne moy
void calcVarianceEdgeLength(double& var, double sommeVar, double moy) {
	var = sommeVar - (moy * moy);
}

//calcule la moyenne des longueurs des edge dans moy
//la somme des carrés des longueurs divisé par le nombre de edge dans sommeVar (utilisée pour ne pas recalculer toute la somme a chaque changement)
//la variance dans var
void prepCalcVariance(double& moy, double& sommeVar, double& var) {
    double temp;
    moy = 0; 
	sommeVar = 0;
    for (auto it = mapLengthEdgeSet.begin(); it != mapLengthEdgeSet.end(); it++) {
		temp = it->first * it->second.size();
		moy += temp;
		sommeVar += (it->first * temp);
	}
	moy /= mapEdgeLength.size();
	sommeVar /= mapEdgeLength.size();
	calcVarianceEdgeLength(var, sommeVar, moy);
}

//prends en paramètre la longueur d'un edge avant et apres changement puis recalcule moy et sommeVar
//ATTENTION appeler calcVarianceEdgeLength après cette fonction (n'est pas dans la fonction par souci de temps de calcul)
void calcVarianceChang(double ancienLongEdge, double nouvLongEdge, double& moy, double& sommeVar) {
	moy -= ancienLongEdge / mapEdgeLength.size();
	moy += nouvLongEdge / mapEdgeLength.size();

	sommeVar -= (ancienLongEdge*ancienLongEdge) / mapEdgeLength.size();
	sommeVar += (nouvLongEdge * nouvLongEdge) / mapEdgeLength.size();
}

#endif OPTIM_ALG