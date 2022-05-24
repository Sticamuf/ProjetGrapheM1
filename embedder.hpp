#ifndef quad_hpp
#define quad_hpp

#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GridLayout.h>
#include "geometrie.hpp"

using ogdf::node;
using ogdf::Graph;
using ogdf::GridLayout;



void embedNode(Graph& G, GridLayout& GL, node nsrc) {
	SListPure<adjEntry> adj;
	SListPure<adjEntry> newOrder;
	// Itérateur qui itere sur le tableau des adjacent non trié
	SListIterator<adjEntry> it;
	// Itérateur sur le nouveau tableau d'adjacent trié
	SListIterator<adjEntry> it2;
	// Itérateur égal a it2-1 :)
	SListIterator<adjEntry> it3;
	node ntrg;
	// Coord x,y du node source
	int sx, sy;
	// Coor x,y du node target
	int tx, ty;
	// Coor x,y du nouveau node a ajouter
	int nx, ny;
	sx = GL.x(nsrc);
	sy = GL.y(nsrc);
	nsrc->allAdjEntries(adj);
	newOrder.clear();
	it = adj.begin();
	// On insere le premier element et on passe directement au prochain
	newOrder.pushBack((*it));
	it++;
	// On itere sur tout les éléments de la liste non triée
	for (; it.valid(); it++) {
		bool inserted = false;
		edge tmpEdge2;
		int qnewnode;
		//if (newOrder.size() > 0) {
			tmpEdge2 = (*it)->theEdge();
			IPolyline& p2 = GL.bends(tmpEdge2);
			if (p2.size() > 0) {
				if (tmpEdge2->source() == nsrc) {
					nx = p2.front().m_x;
					ny = p2.front().m_y;
				}
				else {
					nx = p2.back().m_x;
					ny = p2.back().m_y;
				}
			}
			else {
				node newnode = (*it)->twinNode();
				nx = GL.x(newnode);
				ny = GL.y(newnode);
			}
			qnewnode = quadrant(sx, sy, nx, ny);
		//}
		// On itere sur le deuxieme tableau tant qu'on est pas a la fin et tant qu'on a pas inseré
		for (it2 = newOrder.begin(), it3 = it2; ((it2.valid()) && (!inserted)); it3 = it2++) {
			edge tmpEdge = (*it2)->theEdge();
			IPolyline& p = GL.bends(tmpEdge);
			// Si l'edge contient des bends
			if (p.size() > 0) {
				// Si le noeud source est le meme, on prend le premier bend
				if (tmpEdge->source() == nsrc) {
					tx = p.front().m_x;
					ty = p.front().m_y;
				}
				// Sinon on prend le dernier bend
				else {
					tx = p.back().m_x;
					ty = p.back().m_y;
				}
			}
			// Si pas de bends on prends les coordonnées du noeud
			else {
				ntrg = (*it2)->twinNode();
				tx = GL.x(ntrg);
				ty = GL.y(ntrg);
			}
			// Quadrant du noeud/premier bend
			int qtrg = quadrant(sx, sy, tx, ty);

			// Si le quadrant du point que l'on veut inserer est inférieur a celui qu'on compare
			if (qnewnode < qtrg) {
				// Si on compare au premier, on insere en premiere place
				if (it2 == newOrder.begin()) {
					newOrder.pushFront((*it));
				}
				// Sinon on insere apres it3 car insertBefore n'existe pas :)
				else {
					newOrder.insertAfter((*it), it3);
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
					// Sinon on insere apres it3 car insertBefore n'existe pas :)
					else {
						newOrder.insertAfter((*it), it3);
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
	G.sort(nsrc, newOrder);
}

void embedderCarte(Graph& G, GridLayout& GL) {
	node nsrc = G.firstNode();
	while (nsrc != nullptr) {
		embedNode(G, GL, nsrc);
		nsrc = nsrc->succ();
	}
}

//on admet les ajdEntries triés dans l'ordre trigonométrique
face getFace(ConstCombinatorialEmbedding& CCE, GridLayout& GL, const node& nsrc, int newX, int newY, face& f2) {
	SListPure<adjEntry> adj;
	nsrc->allAdjEntries(adj);
	int sx = GL.x(nsrc);
	int sy = GL.y(nsrc);
	int tx, ty;
	int qnew = quadrant(sx, sy, newX, newY);
	node ntrg;
	for (auto it = adj.begin(); it.valid(); it++) {
		edge tmpEdge = (*it)->theEdge();
		IPolyline& p = GL.bends(tmpEdge);
		// Si l'edge contient des bends
		if (p.size() > 0) {
			// Si le noeud source est le meme, on prend le premier bend
			if (tmpEdge->source() == nsrc) {
				tx = p.front().m_x;
				ty = p.front().m_y;
			}
			// Sinon on prend le dernier bend
			else {
				tx = p.back().m_x;
				ty = p.back().m_y;
			}
		}
		// Si pas de bends on prends les coordonnées du noeud
		else {
			ntrg = (*it)->twinNode();
			tx = GL.x(ntrg);
			ty = GL.y(ntrg);
		}
		// Quadrant du noeud/premier bend
		int qtrg = quadrant(sx, sy, tx, ty);

		// Si le quadrant du nouveau point est inférieur a celui qu'on compare
		if (qnew < qtrg) {
			//on renvoie la face entre le noeud actuel et le précédent
			return CCE.leftFace(*it);
		}
		// Si le quadrant du nouveau est le même que celui à comparer
		else if (qnew == qtrg) {
			//si le nouveau noeud est à droite de targ on renvoie la face entre targ et son précédent
			int det = aGaucheInt(sx, sy, tx, ty, newX, newY);
			if (det == -1) {
				return CCE.leftFace(*it);
			}
			else if (det == 0) {
				f2 = CCE.rightFace(*it);
				return CCE.leftFace(*it);
			}
		}
		//le quadrant est plus grand ou égal mais point a gauche: on passe au prochain noeud
	}
	//Si l'on arrive ici, il faut renvoyer la face entre le dernier et le premier noeud
	return CCE.leftFace(*adj.begin());
}


#endif