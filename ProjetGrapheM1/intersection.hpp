#ifndef INTERSECTION_HPP
#define INTERSECTION_HPP

#include "geometrie.hpp"
#include <algorithm>

using std::min;
using std::max;

//renvoie vrai si les segments [p,q] et [r,s] se croisent
bool seCroisent(int px, int py, int qx, int qy, int rx, int ry, int sx, int sy) {
	
	//[P;Q] est a l'ouest de [R;S]
	if (max(px, qx) < min(rx, sx))
	{
		return false;
	}
	//[P;Q] est a l'est de [R;S]
	if (min(px, qx) > max(rx, sx))
	{
		return false;
	}
	//[P;Q] est au sud de [R;S]
	if (max(py, qy) < min(ry, sy))
	{
		return false;
	}
	//[P;Q] est au sud de [R;S]
	if (min(py, qy) > max(ry, sy))
	{
		return false;
	}

	//R est a gauche, droite ou align� a [P;Q]
	int ag1 = aGaucheInt(px, py, qx, qy, rx, ry);
	//S est a gauche, droite ou align� a [P;Q]
	int ag2 = aGaucheInt(px, py, qx, qy, sx, sy);
	//P est a gauche, droite ou align� a [R;S]
	int ag3 = aGaucheInt(rx, ry, sx, sy, px, py);
	//Q est a gauche, droite ou align� a [R;S]
	int ag4 = aGaucheInt(rx, ry, sx, sy, qx, qy);

	int p1 = ag1 * ag2;
	int p2 = ag3 * ag4;
	
	//R et S sont du meme cote par rapport a PQ
	//OU P et Q sont du meme cote par rapport a RS
	if (p1 == 1 || p2 == 1)
	{
		return false;
	}
	//R et S sont de cotes opposes par rapport a PQ
	//ET P et Q sont de cotes opposes par rapport a RS
	if (p1 == -1 && p2 == -1)
	{
		return true;
	}
	//les quatre points sont alignes
	if (p1 == 0 && p2 == 0)
	{
		return true;
	}

	//il ne reste plus que les cas ou un produit est � 0 et l'autre � -1
	//trois points sont alignes
	//deux points sont de cotes opposes par rapport a un segment
	//les segments se croisent forcement
	return true;
}

#endif