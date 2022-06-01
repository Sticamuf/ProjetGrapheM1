#ifndef GRAPHFUNCTIONS_HPP
#define GRAPHFUNCTIONS_HPP

#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GridLayout.h>
#include <ogdf/basic/Layout.h>
#include <ogdf/basic/CombinatorialEmbedding.h>
#include <stdio.h>
#include "geometrie.hpp"
#include "optimAlg.hpp"
#include "intersection.hpp"
#include "calcEdgeLength.hpp"
#include "EdgeMap.hpp"
#include "NodeBend.hpp"
#include <random>

using namespace ogdf;

// Retourne une valeur entiere comprise dans [1,n]
int generateRand(int n) {
	std::random_device rd;  // Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> dis(1, n);
	return dis(gen);
}

// Renvoie un set composé de tout les edges qui composent les faces adjacentes a un edge
std::vector<Segment> getSegmentFromAdjFacesFromAdjEntry(adjEntry& adj, ConstCombinatorialEmbedding& ccem, GridLayout& GL) {
	std::set<edge> setAllEdges;
	std::set<face> setAdjFaces;
	std::vector<Segment> vectorSegment;
	// On recupere la face gauche et droite de l'adjentry
	setAdjFaces.insert(ccem.rightFace(adj));
	setAdjFaces.insert(ccem.leftFace(adj));
	// On ajoute tout les edge dans un set en premier temps pour eviter les doublons
	for (auto it = setAdjFaces.begin(); it != setAdjFaces.end(); it++) {
		adjEntry firstAdj = (*it)->firstAdj();
		adjEntry nextAdj = firstAdj;
		if (firstAdj != nullptr) {
			do {
				if (nextAdj->theEdge() != nullptr) {
					setAllEdges.insert(nextAdj->theEdge());
				}
				nextAdj = (*it)->nextFaceEdge(nextAdj);
			} while ((nextAdj != firstAdj) && (nextAdj != nullptr));
		}
	}
	// On parcour tout les edges et on les transforme en segments.
	int edgeSrcX, edgeSrcY, edgeTrgX, edgeTrgY;
	for (auto it = setAllEdges.begin(); it != setAllEdges.end(); it++) {
		edgeSrcX = GL.x((*it)->source());
		edgeSrcY = GL.y((*it)->source());
		IPolyline& bends = GL.bends((*it));
		for (ListIterator<IPoint> i = bends.begin(); i.valid(); i++) {
			edgeTrgX = (*i).m_x;
			edgeTrgY = (*i).m_y;
			Segment tmpSegmentEdge(edgeSrcX, edgeSrcY, edgeTrgX, edgeTrgY);
			vectorSegment.push_back(tmpSegmentEdge);
			edgeSrcX = edgeTrgX;
			edgeSrcY = edgeTrgY;
		}
		edgeTrgX = GL.x((*it)->target());
		edgeTrgY = GL.y((*it)->target());
		Segment tmpSegmentEdge(edgeSrcX, edgeSrcY, edgeTrgX, edgeTrgY);
		vectorSegment.push_back(tmpSegmentEdge);
	}
	return vectorSegment;
}

// Renvoie un set composé de tout les edges qui composent les faces adjacentes a un NodeBend
std::set<edge> getEdgesFromAdjFacesFromNodeBend(NodeBend& n, ConstCombinatorialEmbedding& ccem) {
	std::set<edge> setAllEdges;
	std::set<face> setAdjFaces;
	if (n.isNode) {
		SListPure<edge> edges;
		n.getNode()->adjEdges(edges);
		for (SListConstIterator<edge> i = edges.begin(); i.valid(); i++) {
			edge e = (*i);
			setAdjFaces.insert(ccem.leftFace(e->adjSource()));
		}
	}
	else {
		setAdjFaces.insert(ccem.rightFace(n.getEdge()->adjSource()));
		setAdjFaces.insert(ccem.leftFace(n.getEdge()->adjSource()));
	}
	for (auto it = setAdjFaces.begin(); it != setAdjFaces.end(); it++) {
		adjEntry firstAdj = (*it)->firstAdj();
		adjEntry nextAdj = firstAdj;
		if (firstAdj != nullptr) {
			do {
				if (nextAdj->theEdge() != nullptr) {
					setAllEdges.insert(nextAdj->theEdge());
				}
				nextAdj = (*it)->nextFaceEdge(nextAdj);
			} while ((nextAdj != firstAdj) && (nextAdj != nullptr));
		}
	}
	return setAllEdges;
}

// Recupere les coordonnées du points target (node ou bend) a partir d'un node source (ici contenu dans le adjEntry)
void getTargetCoord(GridLayout& GL, const adjEntry& adj, int& trgX, int& trgY) {
	edge tmpEdge = adj->theEdge();
	IPolyline& p = GL.bends(tmpEdge);
	// Si l'edge contient des bends
	if (p.size() > 0) {
		// Si le noeud source est le meme, on prend le premier bend
		if (tmpEdge->source() == adj->theNode()) {
			trgX = p.front().m_x;
			trgY = p.front().m_y;
		}
		// Sinon on prend le dernier bend
		else {
			trgX = p.back().m_x;
			trgY = p.back().m_y;
		}
	}
	// Si pas de bends on prends les coordonnées du noeud
	else {
		node tmpNode = adj->twinNode();
		trgX = GL.x(tmpNode);
		trgY = GL.y(tmpNode);
	}
}

// Renvoie vrai si le noeud n ou un de ses noeuds adjacent a un ordre différent apres le déplacement du node n
bool orderNodeAdjChanged(NodeBend n, int newX, int newY) {
	bool orderChanged = false;
	if (n.isNode) {
		// On regarde l'ordre autour du noeud
		List<adjEntry> nodeAdjEntries;
		n.getNode()->allAdjEntries(nodeAdjEntries);
		// a completer
	}
	return orderChanged;
}

// Renvoie un vecteur de booléen de meme taille que "vectorMoveCoord". Ces booleen indiquent si le déplacement a la meme position dans ce vecteur est valide ou non.
std::vector<bool> getLegalMoves(NodeBend& n, GridLayout& GL, std::vector<std::pair<int, int>> vectorMoveCoord, ConstCombinatorialEmbedding& ccem) {
	int srcX = (*n.a_x);
	int srcY = (*n.a_y);
	int trgX, trgY;
	// Vecteur qui indique si un déplacement est autorisé ou non
	std::vector<bool> vectorMoveAutorised;
	bool intersection;
	SList<adjEntry> nodeAdjEntries;
	std::vector<std::pair<int, int>> vectorTargetBend;
	if (n.isNode) {
		n.getNode()->allAdjEntries(nodeAdjEntries);
	}
	else {
		nodeAdjEntries.pushBack(n.getAdjEntry());
		nodeAdjEntries.pushBack(n.getAdjEntry());
		IPolyline& bends = GL.bends(n.getEdge());
		int bendX, bendY;
		for (ListIterator<IPoint> i = bends.begin(); i != bends.end(); i++) {
			bendX = (*i).m_x;
			bendY = (*i).m_y;
			// On trouve le bend, on récupere les coordonnée du node/bend précédent et suivant
			if ((bendX == srcX) && (bendY == srcY)) {
				std::pair<int, int> tmpPoint;
				if ((*i) == (*bends.begin())) {
					node debut = n.getEdge()->source();
					tmpPoint.first = GL.x(debut);
					tmpPoint.second = GL.y(debut);
				}
				else {
					i--;
					tmpPoint.first = (*i).m_x;
					tmpPoint.second = (*i).m_y;
					i++;
				}
				vectorTargetBend.push_back(tmpPoint);
				i++;
				std::pair<int, int> tmpPoint2;
				if (i != bends.end()) {
					tmpPoint2.first = (*i).m_x;
					tmpPoint2.second = (*i).m_y;
				}
				else {
					node fin = n.getEdge()->target();
					tmpPoint2.first = GL.x(fin);
					tmpPoint2.second = GL.y(fin);
				}
				vectorTargetBend.push_back(tmpPoint2);
				break;
			}
		}
	}
	// Pour chaque déplacement, on regarde si il y a une intersection associé
	int segmentSourceTrgX, segmentSourceTrgY;
	for (int i = 0; i < vectorMoveCoord.size(); i++) {
		intersection = false;
		// On parcour la liste des adjentry du point de départ
		int j = 0;
		for (auto it = nodeAdjEntries.begin(); ((it != nodeAdjEntries.end()) && (!intersection)); it++, j++) {
			// On recupere les coordonnées du target du segment
			if (n.isNode) {
				getTargetCoord(GL, (*it), segmentSourceTrgX, segmentSourceTrgY);
			}
			else {
				segmentSourceTrgX = vectorTargetBend[j].first;
				segmentSourceTrgY = vectorTargetBend[j].second;
			}
			// On recupere les segments des faces adjacentes de l'adjentry
			std::vector<Segment> vectorAdjFaceSegments = getSegmentFromAdjFacesFromAdjEntry((*it), ccem, GL);
			// Et on regarde si une intersection se créer avec la liste des segments non adjacents
			for (int k = 0; (k < vectorAdjFaceSegments.size()) && (!intersection); k++) {
				// Si le segment que l'on teste est adjacent au noeud source:
				if ((vectorAdjFaceSegments[k].sourceX == srcX) && (vectorAdjFaceSegments[k].sourceY == srcY)) {
					// Si le segment est différent de lui meme
					if ((vectorAdjFaceSegments[k].targetX != segmentSourceTrgX) || (vectorAdjFaceSegments[k].targetY != segmentSourceTrgY)) {
						// Alors on teste s'ils sont alignés:
						if (aGaucheInt(vectorMoveCoord[i].first, vectorMoveCoord[i].second, segmentSourceTrgX, segmentSourceTrgY, vectorAdjFaceSegments[k].targetX, vectorAdjFaceSegments[k].targetY) == 0) {
							// On teste si le noeud en commun ne se trouve pas entre les deux autres noeuds, dans ce cas intersection
							if (!dansRectangle(segmentSourceTrgX, segmentSourceTrgY, vectorAdjFaceSegments[k].targetX, vectorAdjFaceSegments[k].targetY, vectorMoveCoord[i].first, vectorMoveCoord[i].second)) {
								intersection = true;
							}
						}
					}
				}
				// Si le segment que l'on teste est adjacent au noeud source dans l'autre sens:
				else if ((vectorAdjFaceSegments[k].targetX == srcX) && (vectorAdjFaceSegments[k].targetY == srcY)) {
					// Si le segment est différent de lui meme
					if ((vectorAdjFaceSegments[k].sourceX != segmentSourceTrgX) || (vectorAdjFaceSegments[k].sourceY != segmentSourceTrgY)) {
						// Alors on teste s'ils sont alignés:
						if (aGaucheInt(vectorMoveCoord[i].first, vectorMoveCoord[i].second, segmentSourceTrgX, segmentSourceTrgY, vectorAdjFaceSegments[k].sourceX, vectorAdjFaceSegments[k].sourceY) == 0) {
							// On teste si le noeud en commun ne se trouve pas entre les deux autres noeuds, dans ce cas intersection
							if (!dansRectangle(segmentSourceTrgX, segmentSourceTrgY, vectorAdjFaceSegments[k].sourceX, vectorAdjFaceSegments[k].sourceY, vectorMoveCoord[i].first, vectorMoveCoord[i].second)) {
								intersection = true;
							}
						}
					}
				}
				// Si les segments sont adjacent entre eux:
				else if ((segmentSourceTrgX == vectorAdjFaceSegments[k].sourceX) && (segmentSourceTrgY == vectorAdjFaceSegments[k].sourceY)) {
					// Alors on teste s'ils sont alignés:
					if (aGaucheInt(vectorMoveCoord[i].first, vectorMoveCoord[i].second, segmentSourceTrgX, segmentSourceTrgY, vectorAdjFaceSegments[k].targetX, vectorAdjFaceSegments[k].targetY) == 0) {
						// On teste si le noeud en commun ne se trouve pas entre les deux autres noeuds, dans ce cas intersection
						if (!dansRectangle(vectorMoveCoord[i].first, vectorMoveCoord[i].second, vectorAdjFaceSegments[k].targetX, vectorAdjFaceSegments[k].targetY, segmentSourceTrgX, segmentSourceTrgY)) {
							intersection = true;
						}
					}
				}
				// Si les segments sont adjacent entre eux dans l'autre sens
				else if ((segmentSourceTrgX == vectorAdjFaceSegments[k].targetX) && (segmentSourceTrgY == vectorAdjFaceSegments[k].targetY)) {
					// Alors on teste s'ils sont alignés:
					if (aGaucheInt(vectorMoveCoord[i].first, vectorMoveCoord[i].second, segmentSourceTrgX, segmentSourceTrgY, vectorAdjFaceSegments[k].sourceX, vectorAdjFaceSegments[k].sourceY) == 0) {
						// On teste si le noeud en commun ne se trouve pas entre les deux autres noeuds, dans ce cas intersection
						if (!dansRectangle(vectorMoveCoord[i].first, vectorMoveCoord[i].second, vectorAdjFaceSegments[k].sourceX, vectorAdjFaceSegments[k].sourceY, segmentSourceTrgX, segmentSourceTrgY)) {
							intersection = true;
						}
					}
				}
				// Sinon:
				else {
					// On regarde s'ils se croisent
					if (seCroisent(vectorMoveCoord[i].first, vectorMoveCoord[i].second, segmentSourceTrgX, segmentSourceTrgY, vectorAdjFaceSegments[k].sourceX, vectorAdjFaceSegments[k].sourceY, vectorAdjFaceSegments[k].targetX, vectorAdjFaceSegments[k].targetY)) {
						intersection = true;
					}
				}
			}
		}
		// On regarde si la face s'inverse ou que l'ordre autour d'un noeud change
		if (n.isNode) {
			// Si le noeud a 3 edge adjacent ou plus, alors on regarde si l'ordre autour de son noeud ou de ses noeud adjacents ont changés
			if (n.getNode()->degree() >= 3) {
				if (orderNodeAdjChanged(n, vectorMoveCoord[i].first, vectorMoveCoord[i].second)) {
					intersection = true;
				}
			}
		}
		else {

		}
		// Intersection = déplacement pas autorisé
		vectorMoveAutorised.push_back(!intersection);
	}
	return vectorMoveAutorised;
}

// Renvoie un vecteur qui attribue une probabilité a un déplacement
// Pour les déplacements: 0=droite(x+1) 1=haut(y+1) 2=gauche(x-1) 3=bas(y-1)
// Cette fonction doit etre appelée avant un déplacement
// Les poids assignés aux déplacements sont attribués en fonction de leur amélioration de l'écart-type
//std::vector<int> rouletteRusseNodeMove(NodeBend& n, GridLayout& GL, ConstCombinatorialEmbedding& ccem, double& moy, double& sommeVar, double& var) {
std::vector<std::pair<int, std::pair<int, int>>> rouletteRusseNodeMove(NodeBend& n, GridLayout& GL, ConstCombinatorialEmbedding& ccem, double& sommeLong, double& sommeLong2, double& variance) {
	std::cout << "Variance avant roulette: " << variance << std::endl;
	int nx = (*n.a_x);
	int ny = (*n.a_y);
	// On stocke les changements de variances apres un déplacement
	std::vector<double> vectorVarChangeMove;
	// On stocke les coordonnées d'arrivée qu'on aurait apres le déplacement
	std::vector<std::pair<int, int>> vectorMoveCoord;
	vectorMoveCoord.push_back(std::pair<int, int>(nx + 1, ny));
	vectorMoveCoord.push_back(std::pair<int, int>(nx, ny + 1));
	vectorMoveCoord.push_back(std::pair<int, int>(nx - 1, ny));
	vectorMoveCoord.push_back(std::pair<int, int>(nx, ny - 1));
	// On stocke si les déplacements sont autorisés, donc s'il n'y a pas de node ou de bend a ces coordonnées
	std::vector<bool> vectorMoveAutorised = getLegalMoves(n, GL, vectorMoveCoord, ccem);
	SListPure<adjEntry> adjEntries;
	bool isNode = true;
	if (n.isNode) {
		n.getNode()->allAdjEntries(adjEntries);
	}
	else {
		adjEntries.pushBack(n.getAdjEntry());
		isNode = false;
	}
	// Minimum et Maximum des variances des différents déplacements pour calcul de probabilité plus tard
	double tmpMaxVariance = variance;
	double tmpMinVariance = variance;
	// Boucle sur tout les déplacements possibles
	for (int i = 0; i < vectorMoveAutorised.size(); i++) {
		// On regarde si le déplacement est autorisé (si on ne se déplace par sur une node ou un bend)
		if (vectorMoveAutorised[i]) {
			double tmpSommeLong = sommeLong;
			double tmpSommeLong2 = sommeLong2;
			double tmpVariance = variance;
			for (auto it = adjEntries.begin(); it.valid(); it++) {
				auto it2 = mapEdgeLength.find((*it)->theEdge());
				double tmpOldLength = it2->second;
				double tmpNewLength;
				if (isNode) {
					tmpNewLength = calcTmpEdgeLength((*it), vectorMoveCoord[i].first, vectorMoveCoord[i].second, GL);
				}
				else {
					tmpNewLength = calcTmpEdgeLengthBends((*it)->theEdge(), n, vectorMoveCoord[i].first, vectorMoveCoord[i].second, GL);
				}
				deleteEdgeNVar(tmpOldLength, tmpSommeLong, tmpSommeLong2);
				addEdgeNVar(tmpNewLength, tmpSommeLong, tmpSommeLong2);
			}
			tmpVariance = calcNVar(tmpSommeLong, tmpSommeLong2);
			vectorVarChangeMove.push_back(tmpVariance);
			if (tmpVariance > tmpMaxVariance) {
				tmpMaxVariance = tmpVariance;
			}
			if (tmpVariance < tmpMinVariance) {
				tmpMinVariance = tmpVariance;
			}

		}
		else {
			vectorVarChangeMove.push_back(-1);
		}
	}
	//pas de mouvement -> déplacement 4
	vectorMoveCoord.push_back(std::pair<int, int>(nx, ny));
	vectorMoveAutorised.push_back(true);
	vectorVarChangeMove.push_back(variance);
	std::vector<std::pair<int, std::pair<int, int>>> vectorProbaMove;
	double tmpVarSomme = 0;
	// On soustrait a tout les valeurs la variance maximale
	for (int i = 0; i < vectorVarChangeMove.size(); i++) {
		if (vectorMoveAutorised[i]) {
			std::cout << "Variance deplacement " << i << ": " << vectorVarChangeMove[i] << std::endl;
			vectorVarChangeMove[i] = abs(vectorVarChangeMove[i] - tmpMaxVariance) + tmpMinVariance;
			tmpVarSomme += vectorVarChangeMove[i];
		}
	}
	// On transforme les valeurs en proba
	int tmpSommeProba = 0;
	int size = vectorMoveAutorised.size() - 1;
	for (int i = 0; i < size; i++) {
		if (vectorMoveAutorised[i]) {
			int tmpProba = round((vectorVarChangeMove[i] / tmpVarSomme) * 100);
			tmpSommeProba += tmpProba;
			std::cout << "Deplacement " << i << " Proba: " << tmpProba << " SommeProba: " << tmpSommeProba << std::endl;
		}
		std::pair<int, std::pair<int, int>> tmpPair(tmpSommeProba, vectorMoveCoord[i]);
		vectorProbaMove.push_back(tmpPair);
	}
	std::cout << "Deplacement " << size << " Proba: " << 100 - tmpSommeProba << " SommeProba: " << 100 << std::endl;
	std::pair<int, std::pair<int, int>> tmpPair(100, vectorMoveCoord[size]);
	vectorProbaMove.push_back(tmpPair);
	return vectorProbaMove;
}

// Change les valeurs dans mapEdgeLength et mapLengthEdgeSet
void changeEdgeLengthInMaps(edge e, double oldEdgeLength, double newEdgeLength) {
	if (oldEdgeLength != newEdgeLength) {
		auto it = mapEdgeLength.find(e);
		it->second = newEdgeLength;
		auto it2 = mapLengthEdgeSet.find(newEdgeLength);
		if (it2 != mapLengthEdgeSet.end()) {
			it2->second.insert(e);
		}
		else {
			std::set<edge> tmpSet;
			tmpSet.insert(e);
			mapLengthEdgeSet.insert(std::pair<double, std::set<edge>>(newEdgeLength, tmpSet));
		}
		auto it3 = mapLengthEdgeSet.find(oldEdgeLength);
		it3->second.erase(e);
		if (it3->second.empty()) {
			mapLengthEdgeSet.erase(oldEdgeLength);
		}
	}
}

// A appelé avant le déplacement avec les nouvelles coordonnées du nodebend
// Change la variance du graphe en fonction du déplacement choisi
// Modifie les maps utilisant les longueurs des egdes
void changeVariance(NodeBend n, GridLayout& GL, int newSrcX, int newSrcY, double& sommeLong, double& sommeLong2, double& variance) {
	if (n.isNode) {
		List<adjEntry> nodeAdjEntries;
		n.getNode()->allAdjEntries(nodeAdjEntries);
		double oldEdgeLength, newEdgeLength;
		for (auto it = nodeAdjEntries.begin(); it != nodeAdjEntries.end(); it++) {
			edge e = (*it)->theEdge();
			auto it2 = mapEdgeLength.find(e);
			double oldEdgeLength = it2->second;
			double newEdgeLength = calcTmpEdgeLength((*it), newSrcX, newSrcY, GL);
			deleteEdgeNVar(oldEdgeLength, sommeLong, sommeLong2);
			addEdgeNVar(newEdgeLength, sommeLong, sommeLong2);
			changeEdgeLengthInMaps(e, oldEdgeLength, newEdgeLength);
		}
	}
	else {
		edge e = n.getEdge();
		auto it = mapEdgeLength.find(e);
		double oldEdgeLength = it->second;
		double newEdgeLength = calcTmpEdgeLengthBends(e, n, newSrcX, newSrcY, GL);
		deleteEdgeNVar(oldEdgeLength, sommeLong, sommeLong2);
		addEdgeNVar(newEdgeLength, sommeLong, sommeLong2);
		changeEdgeLengthInMaps(e, oldEdgeLength, newEdgeLength);
	}
	variance = calcNVar(sommeLong, sommeLong2);
}

// Demarre l'algorithme de roulette russe sur le graphe
// retourne le numero du nodebend choisi, uniquement utile pour l'affichage opengl
int startRouletteRusse(GridLayout& GL, ConstCombinatorialEmbedding& ccem, double& sommeLong, double& sommeLong2, double& variance) {
	// On choisis au hasard un NodeBend
	int randomNum = generateRand(vectorNodeBends.size()) - 1;
	std::cout << "Numero selectionne: " << randomNum << std::endl;
	NodeBend nb = vectorNodeBends[randomNum];
	std::vector<std::pair<int, std::pair<int, int>>> probaDeplacement = rouletteRusseNodeMove(nb, GL, ccem, sommeLong, sommeLong2, variance);
	if (probaDeplacement.size() > 0) {
		int randomChoice = generateRand(100);
		bool moved = false;
		for (int i = 0; ((i < probaDeplacement.size()) && (!moved)); i++) {
			if (randomChoice <= probaDeplacement[i].first) {
				std::cout << "Nombre aleatoire: " << randomChoice << " Deplacement choisi : " << i << std::endl;
				changeVariance(nb, GL, probaDeplacement[i].second.first, probaDeplacement[i].second.second, sommeLong, sommeLong2, variance);
				(*nb.a_x) = probaDeplacement[i].second.first;
				(*nb.a_y) = probaDeplacement[i].second.second;
				moved = true;
			}
		}
	}
	return randomNum;
	std::cout << "Nouvelle variance " << variance << std::endl;
}

// Calcul le ratio edge/length. longueur la plus grande divisé par la longueur la plus courte.
double calcEdgeLengthRatio() {
	double ratio = (mapLengthEdgeSet.rbegin()->first / mapLengthEdgeSet.begin()->first);
	return ratio;
}

void move(NodeBend n, GridLayout& GL, int dx, int dy, double& sommeLong, double& sommeLong2, double& variance) {
	int newX = (*n.a_x) + dx;
	int newY = (*n.a_y) + dy;
	changeVariance(n, GL, newX, newY, sommeLong, sommeLong2, variance);
	(*n.a_x) = newX;
	(*n.a_y) = newY;
}

ListPure<adjEntry> orderAroundNode(node nsrc, GridLayout& GL, ListPure<adjEntry> adj) {
	ListPure<adjEntry> newOrder;
	// Coordonnées: s = source, t = target, n = node a ajouter
	int sx, sy, tx, ty, nx, ny;
	sx = GL.x(nsrc);
	sy = GL.y(nsrc);
	// Itérateur qui itere sur le tableau des adjacent non trié
	auto it = adj.begin();
	// On insere le premier element et on passe directement au prochain
	newOrder.pushBack((*it));
	it++;
	// On itere sur tout les éléments de la liste non triée
	for (; it.valid(); it++) {
		bool inserted = false;
		edge tmpEdge2;
		int qnewnode;
		getTargetCoord(GL,(*it),nx,ny);
		qnewnode = quadrant(sx, sy, nx, ny);
		// On itere sur le deuxieme tableau tant qu'on est pas a la fin et tant qu'on a pas inseré
		for (auto it2 = newOrder.begin(); ((it2.valid()) && (!inserted)); it2++) {
			getTargetCoord(GL, (*it2), tx, ty);
			// Quadrant du noeud/premier bend
			int qtrg = quadrant(sx, sy, tx, ty);
			// Si le quadrant du point que l'on veut inserer est inférieur a celui qu'on compare
			if (qnewnode < qtrg) {
				// Si on compare au premier, on insere en premiere place
				if (it2 == newOrder.begin()) {
					newOrder.pushFront((*it));
				}
				else {
					it2--;
					newOrder.insertAfter((*it), it2);
					it2++;
				}
				inserted = true;
			}
			// Si les quadrants sont égaux
			else if (qnewnode == qtrg) {
				// Si on est a droite on insere apres it3
				if (!aGauche(sx, sy, tx, ty, nx, ny)) {
					if (it2 == newOrder.begin()) {
						newOrder.pushFront((*it));
					}
					else {
						it2--;
						newOrder.insertAfter((*it), it2);
						it2++;
					}
					inserted = true;
				}
			}
		}
		// Si on a toujours pas inseré, c'est qu'on est le dernier element
		if (!inserted) {
			newOrder.pushBack((*it));
		}
	}
	return newOrder;
}

// Renvoie l'ordre trigonométrique de tout les adjEntry autour d'un noeud
ListPure<adjEntry> embedNode(Graph& G, GridLayout& GL, node nsrc) {
	ListPure<adjEntry> adj;
	nsrc->allAdjEntries(adj);
	return orderAroundNode(nsrc, GL, adj);
}

void embedderCarte(Graph& G, GridLayout& GL) {
	ListPure<adjEntry> newOrder;
	node nsrc = G.firstNode();
	while (nsrc != nullptr) {
		newOrder = embedNode(G, GL, nsrc);
		G.sort(nsrc, newOrder);
		nsrc = nsrc->succ();
	}
}

#endif