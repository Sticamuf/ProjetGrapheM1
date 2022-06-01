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

//calcule la variance en fonction de : la somme des carr�s des longueurs divis� par le nombre de edge sommeVar,  et de la moyenne moy
void calcVarianceEdgeLength(double& var, double sommeVar, double moy) {
	var = sommeVar - (moy * moy);
}

//calcule la moyenne des longueurs des edge dans moy
//la somme des carr�s des longueurs divis� par le nombre de edge dans sommeVar (utilis�e pour ne pas recalculer toute la somme a chaque changement)
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



//prends en param�tre la longueur d'un edge avant et apres changement puis recalcule moy et sommeVar
//ATTENTION appeler calcVarianceEdgeLength apr�s cette fonction (n'est pas dans la fonction par souci de temps de calcul)
void calcVarianceChang(double ancienLongEdge, double nouvLongEdge, double& moy, double& sommeVar) {
	moy -= ancienLongEdge / mapEdgeLength.size();
	moy += nouvLongEdge / mapEdgeLength.size();

	sommeVar -= (ancienLongEdge*ancienLongEdge) / mapEdgeLength.size();
	sommeVar += (nouvLongEdge * nouvLongEdge) / mapEdgeLength.size();
}

//renvoie n fois la variance �tant donn�es :
// - la somme des longueurs des edge dans sommeLong
// - la somme des longeurs des edge au carr� dans sommeLong2
double calcNVar2(double sommeLong, double sommeLong2) {
	return mapEdgeLength.size() * sommeLong2 - sommeLong * sommeLong;
}

//calcule la somme des longueurs des edge dans sommeLong
//calcule la somme les longueurs des edge au carre dans sommeLong2
//calcule n fois la variance dans nVar
void prepCalcNVar2(double& sommeLong, double& sommeLong2, double& nVar) {
	sommeLong = 0;
	sommeLong2 = 0;
	for (auto it = mapEdgeLength.begin(); it != mapEdgeLength.end(); it++) {
		sommeLong += it->second;
		sommeLong2 += it->second * it->second;
	}
	nVar = calcNVar2(sommeLong, sommeLong2);
}

//prend en parametre une longueur d'un edge et la soustrait de sommeLong et sommeLong2
void deleteEdgeNVar2(double edgeLength, double& sommeLong, double& sommelong2) {
	sommeLong -= edgeLength;
	sommelong2 -= edgeLength*edgeLength;
}

//prend en param�tre une longueur d'un edge et l'ajoute � sommeLong et someLong2
void addEdgeNVar2(double edgeLength, double& sommeLong, double& sommeLong2) {
	sommeLong += edgeLength;
	sommeLong2 += edgeLength * edgeLength;
}




#endif OPTIM_ALG