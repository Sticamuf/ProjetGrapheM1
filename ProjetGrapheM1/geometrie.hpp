#ifndef GEOMETRIE_HPP
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

//renvoie 1,2,3 ou 4 si lpoint t est 1: en haut a droite, 2: en haut a gauche, 3: en bas a gauche, 4: en bas a droite du point s
//on considere s != t
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

//s= source, t=target, c=comp=nouveau noeud a ajouter, renvoie vrai si c est a gauche de (s;t) faux sinon
bool aGauche(int sx, int sy, int tx, int ty, int cx, int cy) {
	return ((static_cast<long long>(tx) - sx) * (static_cast<long long>(cy) - sy) -
		(static_cast<long long>(ty) - sy) * (static_cast<long long>(cx) - sx)) > 0;
}
//si c est a gauche de (s;t) renvoie 1, si c est a droite de (s;t) renvoie -1, si c,s et t sont alignes renvoie 0
int aGaucheInt(int sx, int sy, int tx, int ty, int cx, int cy) {
	long long det = ((static_cast<long long>(tx) - sx) * (static_cast<long long>(cy) - sy) -
		(static_cast<long long>(ty) - sy) * (static_cast<long long>(cx) - sx));
	if (det > 0) return 1;
	if (det < 0) return -1;
	return 0;
}

// Renvoie vrai si c est sur le segment st
bool surSegment(int sx, int sy, int tx, int ty, int cx, int cy) {
	return ((cx <= max(sx, tx) && cx >= min(sx, tx) && cy <= max(sy, ty) && cy >= min(sy, ty)) && (aGaucheInt(sx, sy, tx, ty, cx, cy) == 0));
}

#endif