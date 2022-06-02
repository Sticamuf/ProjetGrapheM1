﻿#ifndef GEOMETRIE_HPP
#define GEOMETRIE_HPP
#include <algorithm>

class Segment {
public:
	int sourceX, sourceY;
	int targetX, targetY;
	Segment(int srcx, int srcy, int trgx, int trgy) {
		sourceX = srcx;
		sourceY = srcy;
		targetX = trgx;
		targetY = trgy;
	}
};

//renvoie 1,2,3 ou 4 si lpoint t est 1: en haut � droite, 2: en haut � gauche, 3: en bas � gauche, 4: en bas � droite du point s
//on consid�re s != t
int quadrant(int sx, int sy, int tx, int ty) {
	if (tx > sx) {
		if (ty >= sy) {
			return 1;
		}
		return 4;
	}
	if (ty > sy) {
		return 2;
	}
	return 3;
}

//s= source, t=target, c=comp=nouveau noeud � ajouter, renvoie vrai si c est � gauche de (s;t) faux sinon
bool aGauche(int sx, int sy, int tx, int ty, int cx, int cy) {
	return ((static_cast<long long>(tx) - sx) * (static_cast<long long>(cy) - sy) -
		(static_cast<long long>(ty) - sy) * (static_cast<long long>(cx) - sx)) > 0;
}
//si c est � gauche de (s;t) renvoie 1, si c est � droite de (s;t) renvoie -1, si c,s et t sont align�s renvoie 0
int aGaucheInt(int sx, int sy, int tx, int ty, int cx, int cy) {
	long long det = ((static_cast<long long>(tx) - sx) * (static_cast<long long>(cy) - sy) -
		(static_cast<long long>(ty) - sy) * (static_cast<long long>(cx) - sx));
	if (det > 0) return 1;
	if (det < 0) return -1;
	return 0;
}

// Renvoie vrai si c est dans le rectangle form� par st
bool dansRectangle(int sx, int sy, int tx, int ty, int cx, int cy) {
	return (cx <= max(sx, tx) && cx >= min(sx, tx) && cy <= max(sy, ty) && cy >= min(sy, ty));
}

// Renvoie vrai si c est sur le segment st
bool surSegment(int sx, int sy, int tx, int ty, int cx, int cy) {
	return (dansRectangle(sx, sy, tx, ty, cx, cy) && (aGaucheInt(sx, sy, tx, ty, cx, cy) == 0));
}

#endif