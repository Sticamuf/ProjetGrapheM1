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

//calcule la moyenne des longueurs des edge dans moy
//la somme des carrés des longueurs divisé par le nombre de edge dans sommeVar (utilisée pour ne pas recalculer toute la somme a chaque changement)
//la variance dans var
void prepCalcVariance(double& moy, double& sommeVar, double& var) {
	auto it = mapLengthEdgeSet.begin();
	double temp = it->first * it->second.size();
	moy = temp / mapEdgeLength.size(); //division en cours de route pour ne pas dépasser la taille des double
	sommeVar = (it->first * temp) / mapEdgeLength.size(); //division en cours de route pour ne pas dépasser la taille des double
	it++;
	while (it != mapLengthEdgeSet.end()) {
		temp = it->first * it->second.size();
		moy += temp / mapEdgeLength.size();
		sommeVar += (it->first * temp) / mapEdgeLength.size();
		it++;
	}
	calcVarianceEdgeLength(var, sommeVar, moy);
}

//calcule la variance en fonction de : la somme des carrés des longueurs divisé par le nombre de edge sommeVar,  et de la moyenne moy
void calcVarianceEdgeLength(double& var, double sommeVar, double moy) {
	var = sommeVar - (moy * moy);
}

//prends en paramètre la longueur d'un edge avant et apres changement puis recalcule moy et sommeVar
//ATTENTION appeler calcVarianceEdgeLength après cette fonction (n'est pas dans la fonction par souci de temps de calcul)
void calcVarianceChang(double ancienLongEdge, double nouvLongEdge, double& moy, double& sommeVar) {
	moy -= ancienLongEdge / mapEdgeLength.size();
	moy += nouvLongEdge / mapEdgeLength.size();

	sommeVar -= (ancienLongEdge*ancienLongEdge) / mapEdgeLength.size();
	sommeVar += (nouvLongEdge * nouvLongEdge) / mapEdgeLength.size();
}

unsigned long long calcTmpEdgeLengthSquared(const edge& e, int srcX, int srcY, const GridLayout& GL) {
    node target = e->target();
    unsigned long long length = 0.0;
    int sourceX = srcX;
    int sourceY = srcY;
    int targetX, targetY;
    IPolyline bends = GL.bends(e);
    if (bends.size() > 0) {
        for (ListIterator<IPoint> i = bends.begin(); i.valid(); i++) {
            targetX = (*i).m_x;
            targetY = (*i).m_y;
            length += pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2);
            sourceX = targetX;
            sourceY = targetY;
        }
    }
    targetX = GL.x(target);
    targetY = GL.y(target);
    length += pow((targetX - sourceX), 2) + pow((targetY - sourceY), 2);
    return length;
}

#endif OPTIM_ALG