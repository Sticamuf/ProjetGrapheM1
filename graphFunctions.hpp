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

// Renvoie un vecteur compos� de tout les segments qui composent les faces adjacentes a un adjentry
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

// Renvoie un set compos� de tout les edges qui composent les faces adjacentes a un NodeBend
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

// Recupere les coordonn�es du points target (node ou bend) a partir d'un node source (ici contenu dans le adjEntry)
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
	// Si pas de bends on prends les coordonn�es du noeud
	else {
		node tmpNode = adj->twinNode();
		trgX = GL.x(tmpNode);
		trgY = GL.y(tmpNode);
	}
}

// Renvoie l'ordre des adjentry autour d'un noeud apres d�placement d'un noeud adjacent
ListPure<adjEntry> orderAroundNodeAfterAdjNodeMove(node nsrc, GridLayout& GL, ListPure<adjEntry> adj, adjEntry moved, int newX, int newY) {
	ListPure<adjEntry> newOrder;
	// Coordonn�es: s = source, t = target, n = node a ajouter
	int sx, sy, tx, ty, nx, ny;
	sx = GL.x(nsrc);
	sy = GL.y(nsrc);
	// It�rateur qui itere sur le tableau des adjacent non tri�
	auto it = adj.begin();
	// On insere le premier element et on passe directement au prochain
	newOrder.pushBack((*it));
	it++;
	// On itere sur tout les �l�ments de la liste non tri�e
	for (; it.valid(); it++) {
		bool inserted = false;
		edge tmpEdge2;
		int qnewnode;
		if ((*it) == moved) {
			nx = newX;
			ny = newY;
		}
		else {
			getTargetCoord(GL, (*it), nx, ny);
		}
		qnewnode = quadrant(sx, sy, nx, ny);
		// On itere sur le deuxieme tableau tant qu'on est pas a la fin et tant qu'on a pas inser�
		for (auto it2 = newOrder.begin(); ((it2.valid()) && (!inserted)); it2++) {
			if ((*it2) == moved) {
				tx = newX;
				ty = newY;
			}
			else {
				getTargetCoord(GL, (*it2), tx, ty);
			}
			// Quadrant du noeud/premier bend
			int qtrg = quadrant(sx, sy, tx, ty);
			// Si le quadrant du point que l'on veut inserer est inf�rieur a celui qu'on compare
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
			// Si les quadrants sont �gaux
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
		// Si on a toujours pas inser�, c'est qu'on est le dernier element
		if (!inserted) {
			newOrder.pushBack((*it));
		}
	}
	return newOrder;
}

// Renvoie l'ordre des adjentry autour d'un noeud apres d�placement de ce noeud
ListPure<adjEntry> orderAroundNodeAfterMove(node nsrc, GridLayout& GL, ListPure<adjEntry> adj, int newX, int newY) {
	ListPure<adjEntry> newOrder;
	// Coordonn�es: s = source, t = target, n = node a ajouter
	int sx, sy, tx, ty, nx, ny;
	sx = newX;
	sy = newY;
	// It�rateur qui itere sur le tableau des adjacent non tri�
	auto it = adj.begin();
	// On insere le premier element et on passe directement au prochain
	newOrder.pushBack((*it));
	it++;
	// On itere sur tout les �l�ments de la liste non tri�e
	for (; it.valid(); it++) {
		bool inserted = false;
		edge tmpEdge2;
		int qnewnode;
		getTargetCoord(GL, (*it), nx, ny);
		qnewnode = quadrant(sx, sy, nx, ny);
		// On itere sur le deuxieme tableau tant qu'on est pas a la fin et tant qu'on a pas inser�
		for (auto it2 = newOrder.begin(); ((it2.valid()) && (!inserted)); it2++) {
			getTargetCoord(GL, (*it2), tx, ty);
			// Quadrant du noeud/premier bend
			int qtrg = quadrant(sx, sy, tx, ty);
			// Si le quadrant du point que l'on veut inserer est inf�rieur a celui qu'on compare
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
			// Si les quadrants sont �gaux
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
		// Si on a toujours pas inser�, c'est qu'on est le dernier element
		if (!inserted) {
			newOrder.pushBack((*it));
		}
	}
	return newOrder;
}

// Renvoie l'ordre des adjentry autour d'un noeud
ListPure<adjEntry> orderAroundNode(node nsrc, GridLayout& GL, ListPure<adjEntry> adj) {
	ListPure<adjEntry> newOrder;
	// Coordonn�es: s = source, t = target, n = node a ajouter
	int sx, sy, tx, ty, nx, ny;
	sx = GL.x(nsrc);
	sy = GL.y(nsrc);
	// It�rateur qui itere sur le tableau des adjacent non tri�
	auto it = adj.begin();
	// On insere le premier element et on passe directement au prochain
	newOrder.pushBack((*it));
	it++;
	// On itere sur tout les �l�ments de la liste non tri�e
	for (; it.valid(); it++) {
		bool inserted = false;
		edge tmpEdge2;
		int qnewnode;
		getTargetCoord(GL, (*it), nx, ny);
		qnewnode = quadrant(sx, sy, nx, ny);
		// On itere sur le deuxieme tableau tant qu'on est pas a la fin et tant qu'on a pas inser�
		for (auto it2 = newOrder.begin(); ((it2.valid()) && (!inserted)); it2++) {
			getTargetCoord(GL, (*it2), tx, ty);
			// Quadrant du noeud/premier bend
			int qtrg = quadrant(sx, sy, tx, ty);
			// Si le quadrant du point que l'on veut inserer est inf�rieur a celui qu'on compare
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
			// Si les quadrants sont �gaux
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
		// Si on a toujours pas inser�, c'est qu'on est le dernier element
		if (!inserted) {
			newOrder.pushBack((*it));
		}
	}
	return newOrder;
}

// Renvoie vrai si les deux listes ont les memes suivants/precedent de leur contenu (ABC == BCA)
// Fonction utilis�e pour des listes ayant les memes adjentry en contenu mais pas forc�ment dans le meme ordre
bool sameOrderList(ListPure<adjEntry> l1, ListPure<adjEntry> l2) {
	ListIterator<adjEntry> it = l1.begin();
	ListIterator<adjEntry> it2 = l2.begin();
	for (; (*it2) != (*it); it2++);
	while (l1.cyclicSucc(it) != l1.begin()) {
		it = l1.cyclicSucc(it);
		it2 = l2.cyclicSucc(it2);
		if ((*it) != (*it2)) {
			return false;
		}
	}
	return true;
}

bool hasBends(adjEntry adj, GridLayout& GL) {
	edge tmpEdge = adj->theEdge();
	IPolyline& p = GL.bends(tmpEdge);
	// Si l'edge contient des bends
	return (p.size() > 0);
}

// Renvoie vrai si le noeud n ou un de ses noeuds adjacent a un ordre diff�rent apres le d�placement du node n
bool orderNodeAdjChanged(NodeBend nb, GridLayout& GL, int newX, int newY) {
	if (nb.isNode) {
		int degre = nb.getNode()->degree();
		node n = nb.getNode();
		// On regarde l'ordre autour du noeud
		if (degre >= 3) {
			ListPure<adjEntry> nodeAdjEntries;
			n->allAdjEntries(nodeAdjEntries);
			ListPure<adjEntry> newAdjEntriesOrder;
			n->allAdjEntries(newAdjEntriesOrder);
			newAdjEntriesOrder = orderAroundNodeAfterMove(n, GL, newAdjEntriesOrder, newX, newY);
			if (!sameOrderList(nodeAdjEntries, newAdjEntriesOrder)) {
				return true;
			}
		}
		ListPure<adjEntry> nodeAdjEntries;
		n->allAdjEntries(nodeAdjEntries);
		ListPure<adjEntry> adjNodeAdjEntries;
		ListPure<adjEntry> adjNodeNewAdjEntriesOrder;
		// On regarde l'ordre de tout les noeuds adjacents
		for (auto it = nodeAdjEntries.begin(); it.valid(); it++) {
			node na = (*it)->twinNode();
			// Uniquement si le degree >=3 l'ordre peut changer et que l'edge ne contient pas de bend
			if ((!hasBends((*it), GL))&&(na->degree() >= 3)) {
				adjNodeAdjEntries.clear();
				adjNodeNewAdjEntriesOrder.clear();
				adjEntry oppose = (*it)->twin();
				adjNodeAdjEntries.pushBack(oppose->cyclicPred());
				adjNodeAdjEntries.pushBack(oppose);
				adjNodeAdjEntries.pushBack(oppose->cyclicSucc());
				adjNodeNewAdjEntriesOrder.pushBack(oppose->cyclicPred());
				adjNodeNewAdjEntriesOrder.pushBack(oppose);
				adjNodeNewAdjEntriesOrder.pushBack(oppose->cyclicSucc());
				adjNodeNewAdjEntriesOrder = orderAroundNodeAfterAdjNodeMove(na,GL, adjNodeNewAdjEntriesOrder, oppose,newX,newY);
				if (!sameOrderList(adjNodeAdjEntries, adjNodeNewAdjEntriesOrder)) {
					return true;
				}
			}
		}
	}
	// Si on est un bend on regarde si un noeud est directement adjacent ou non
	else {
		adjEntry adj = nb.getAdjEntry();
		edge e = nb.getEdge();
		IPolyline& bends = GL.bends(e);
		std::vector<node> adjNodes;
		// Numero 0 = on est adjacent au noeud source de l'edge
		if (nb.numero == 0) {
			adjNodes.push_back(e->source());
		}
		// Numero = bends.size() - 1 = on est adjacent au noeud target de l'edge
		if (nb.numero == bends.size() - 1) {
			adjNodes.push_back(e->target());
		}
		for (int i = 0; i < adjNodes.size(); i++) {
			// Pas de changement d'ordre possible si degre < 3
			if (adjNodes[i]->degree() >= 3) {
				// On recupere l'adjentry du bon cot�
				if (adj->theNode() != adjNodes[i]) {
					adj = adj->twin();
				}
				ListPure<adjEntry> adjNodeAdjEntries;
				ListPure<adjEntry> adjNodeNewAdjEntriesOrder;
				adjNodeAdjEntries.clear();
				adjNodeNewAdjEntriesOrder.clear();
				adjNodeAdjEntries.pushBack(adj->cyclicPred());
				adjNodeAdjEntries.pushBack(adj);
				adjNodeAdjEntries.pushBack(adj->cyclicSucc());
				adjNodeNewAdjEntriesOrder.pushBack(adj->cyclicPred());
				adjNodeNewAdjEntriesOrder.pushBack(adj);
				adjNodeNewAdjEntriesOrder.pushBack(adj->cyclicSucc());
				adjNodeNewAdjEntriesOrder = orderAroundNodeAfterAdjNodeMove(adjNodes[i], GL, adjNodeNewAdjEntriesOrder, adj, newX, newY);
				if (!sameOrderList(adjNodeAdjEntries, adjNodeNewAdjEntriesOrder)) {
					return true;
				}
			}
		}
	}
	return false;
}

// Renvoie un vecteur de bool�en de meme taille que "vectorMoveCoord". Ces booleen indiquent si le d�placement a la meme position dans ce vecteur est valide ou non.
std::vector<bool> getLegalMoves(NodeBend& n, GridLayout& GL, std::vector<std::pair<int, int>> vectorMoveCoord, ConstCombinatorialEmbedding& ccem) {
	int srcX = (*n.a_x);
	int srcY = (*n.a_y);
	int trgX, trgY;
	// Vecteur qui indique si un d�placement est autoris� ou non
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
			// On trouve le bend, on r�cupere les coordonn�e du node/bend pr�c�dent et suivant
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
	// Pour chaque d�placement, on regarde si il y a une intersection associ�
	int segmentSourceTrgX, segmentSourceTrgY;
	for (int i = 0; i < vectorMoveCoord.size(); i++) {
		std::cout << "Check Deplacement " << i << std::endl;
		intersection = false;
		// On parcour la liste des adjentry du point de d�part
		int j = 0;
		for (auto it = nodeAdjEntries.begin(); ((it != nodeAdjEntries.end()) && (!intersection)); it++, j++) {
			// On recupere les coordonn�es du target du segment
			if (n.isNode) {
				getTargetCoord(GL, (*it), segmentSourceTrgX, segmentSourceTrgY);
			}
			else {
				segmentSourceTrgX = vectorTargetBend[j].first;
				segmentSourceTrgY = vectorTargetBend[j].second;
			}
			// On recupere les segments des faces adjacentes de l'adjentry
			std::vector<Segment> vectorAdjFaceSegments = getSegmentFromAdjFacesFromAdjEntry((*it), ccem, GL);
			// Et on regarde si une intersection se cr�er avec la liste des segments non adjacents
			for (int k = 0; (k < vectorAdjFaceSegments.size()) && (!intersection); k++) {
				// Si le segment que l'on teste est adjacent au noeud source:
				if ((vectorAdjFaceSegments[k].sourceX == srcX) && (vectorAdjFaceSegments[k].sourceY == srcY)) {
					// Si le segment est diff�rent de lui meme
					if ((vectorAdjFaceSegments[k].targetX != segmentSourceTrgX) || (vectorAdjFaceSegments[k].targetY != segmentSourceTrgY)) {
						// Alors on teste s'ils sont align�s:
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
					// Si le segment est diff�rent de lui meme
					if ((vectorAdjFaceSegments[k].sourceX != segmentSourceTrgX) || (vectorAdjFaceSegments[k].sourceY != segmentSourceTrgY)) {
						// Alors on teste s'ils sont align�s:
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
					// Alors on teste s'ils sont align�s:
					if (aGaucheInt(vectorMoveCoord[i].first, vectorMoveCoord[i].second, segmentSourceTrgX, segmentSourceTrgY, vectorAdjFaceSegments[k].targetX, vectorAdjFaceSegments[k].targetY) == 0) {
						// On teste si le noeud en commun ne se trouve pas entre les deux autres noeuds, dans ce cas intersection
						if (!dansRectangle(vectorMoveCoord[i].first, vectorMoveCoord[i].second, vectorAdjFaceSegments[k].targetX, vectorAdjFaceSegments[k].targetY, segmentSourceTrgX, segmentSourceTrgY)) {
							intersection = true;
						}
					}
				}
				// Si les segments sont adjacent entre eux dans l'autre sens
				else if ((segmentSourceTrgX == vectorAdjFaceSegments[k].targetX) && (segmentSourceTrgY == vectorAdjFaceSegments[k].targetY)) {
					// Alors on teste s'ils sont align�s:
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
		if (!intersection) {
			// On regarde si les ordre du noeud source ou des adjacents ont chang�
			if (orderNodeAdjChanged(n,GL, vectorMoveCoord[i].first, vectorMoveCoord[i].second)) {
				intersection = true;
			}
		}
		// Intersection = d�placement pas autoris�
		vectorMoveAutorised.push_back(!intersection);
	}
	return vectorMoveAutorised;
}

// Renvoie un vecteur qui attribue une probabilit� a un d�placement
// Pour les d�placements: 0=droite(x+1) 1=haut(y+1) 2=gauche(x-1) 3=bas(y-1)
// Cette fonction doit etre appel�e avant un d�placement
// Les poids assign�s aux d�placements sont attribu�s en fonction de leur am�lioration de l'�cart-type
//std::vector<int> rouletteRusseNodeMove(NodeBend& n, GridLayout& GL, ConstCombinatorialEmbedding& ccem, double& moy, double& sommeVar, double& var) {
std::vector<std::pair<int, std::pair<int, int>>> rouletteRusseNodeMove(NodeBend& n, GridLayout& GL, ConstCombinatorialEmbedding& ccem, double& sommeLong, double& sommeLong2, double& variance) {
	std::cout << "Variance avant roulette: " << variance << std::endl;
	int nx = (*n.a_x);
	int ny = (*n.a_y);
	// On stocke les changements de variances apres un d�placement
	std::vector<double> vectorVarChangeMove;
	// On stocke les coordonn�es d'arriv�e qu'on aurait apres le d�placement
	std::vector<std::pair<int, int>> vectorMoveCoord;
	vectorMoveCoord.push_back(std::pair<int, int>(nx + 1, ny));
	vectorMoveCoord.push_back(std::pair<int, int>(nx, ny + 1));
	vectorMoveCoord.push_back(std::pair<int, int>(nx - 1, ny));
	vectorMoveCoord.push_back(std::pair<int, int>(nx, ny - 1));
	//vectorMoveCoord.push_back(std::pair<int, int>(nx + 1, ny+1));
	//vectorMoveCoord.push_back(std::pair<int, int>(nx + 1, ny-1));
	//vectorMoveCoord.push_back(std::pair<int, int>(nx - 1, ny+1));
	//vectorMoveCoord.push_back(std::pair<int, int>(nx - 1, ny-1));
	//vectorMoveCoord.push_back(std::pair<int, int>(nx+2, ny)); // POUR TEST UNIQUEMENT A SUPPRIMER PLUS TARD
	// On stocke si les d�placements sont autoris�s, donc s'il n'y a pas de node ou de bend a ces coordonn�es
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
	// Minimum et Maximum des variances des diff�rents d�placements pour calcul de probabilit� plus tard
	double tmpMaxVariance = variance;
	double tmpMinVariance = variance;
	// Boucle sur tout les d�placements possibles
	for (int i = 0; i < vectorMoveAutorised.size(); i++) {
		// On regarde si le d�placement est autoris� (si on ne se d�place par sur une node ou un bend)
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
	//pas de mouvement -> d�placement 4
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

// A appel� avant le d�placement avec les nouvelles coordonn�es du nodebend
// Change la variance du graphe en fonction du d�placement choisi
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

// Calcul le ratio edge/length. longueur la plus grande divis� par la longueur la plus courte.
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

// Renvoie l'ordre trigonom�trique de tout les adjEntry autour d'un noeud
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