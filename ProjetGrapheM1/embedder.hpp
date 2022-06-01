#ifndef EMBEDDER_HPP
#define EMBEDDER_HPP

#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GridLayout.h>
#include "geometrie.hpp"

using ogdf::node;
using ogdf::Graph;
using ogdf::GridLayout;



void embedNode(Graph& G, GridLayout& GL, node nsrc) {
	SListPure<adjEntry> adj;
	SListPure<adjEntry> newOrder;
	// It�rateur qui itere sur le tableau des adjacent non tri�
	SListIterator<adjEntry> it;
	// It�rateur sur le nouveau tableau d'adjacent tri�
	SListIterator<adjEntry> it2;
	// It�rateur �gal a it2-1 :)
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
	// On itere sur tout les �l�ments de la liste non tri�e
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
		// On itere sur le deuxieme tableau tant qu'on est pas a la fin et tant qu'on a pas inser�
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
			// Si pas de bends on prends les coordonn�es du noeud
			else {
				ntrg = (*it2)->twinNode();
				tx = GL.x(ntrg);
				ty = GL.y(ntrg);
			}
			// Quadrant du noeud/premier bend
			int qtrg = quadrant(sx, sy, tx, ty);

			// Si le quadrant du point que l'on veut inserer est inf�rieur a celui qu'on compare
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
			// Si les quadrants sont �gaux
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
		// Si on a toujours pas inser�, c'est qu'on est le dernier element
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
#endif