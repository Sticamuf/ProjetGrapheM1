#ifndef quad_hpp
#define quad_hpp

#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GridLayout.h>
using namespace ogdf;

using ogdf::node;
using ogdf::Graph;
using ogdf::GridLayout;

const int Q1 = 0;
const int Q2 = 1;
const int Q3 = 2;
const int Q4 = 3;

//renvoie 1,2,3 ou 4 si la node targ est 1: en haut � droite, 2: en haut � gauche, 3: en bas � gauche, 4: en bas � droite de la node src 
//dans le layout GL
//on consid�re src != targ
int quadrant(const GridLayout& GL, const node& src, const node& targ) {
	if (GL.x(targ) > GL.x(src)) {
		if (GL.y(targ) >= GL.y(src)) {
			return Q1;
		}
		return Q4;
	}
	if (GL.y(targ) > GL.y(src)) {
		return Q2;
	}
	return Q3;
}

int quadrant(int sx, int sy, int tx, int ty) {
	if (tx > sx) {
		if (ty >= sy) {
			return Q1;
		}
		return Q4;
	}
	if (ty > sy) {
		return Q2;
	}
	return Q3;
}

//on consid�re dans le meme quadrant
bool aGauche(const GridLayout& GL, const node& src, const node& targ, const node& comp) {
	return ((GL.x(targ) - GL.x(src)) * (GL.y(comp) - GL.y(src)) - (GL.y(targ) - GL.y(src)) * (GL.x(comp) - GL.x(src))) > 0;
}
//s= source, t=target, c=comp=nouveau noeud � ajouter
bool aGauche(int sx, int sy, int tx, int ty, int cx, int cy) {
	return ((tx - sx) * (cy - sy) - (ty - sy) * (cx - sx)) > 0;
}

int aGaucheInt(int sx, int sy, int tx, int ty, int cx, int cy) {
	int det = ((tx - sx) * (cy - sy) - (ty - sy) * (cx - sx));
	if (det > 0) return 1;
	if (det < 0) return -1;
	return 0;
}

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


			edge tmpEdge2 = (*it)->theEdge();
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
			int qnewnode = quadrant(sx, sy, nx, ny);
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

//on admet les ajdEntries tri�s dans l'ordre trigonom�trique
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
		// Si pas de bends on prends les coordonn�es du noeud
		else {
			ntrg = (*it)->twinNode();
			tx = GL.x(ntrg);
			ty = GL.y(ntrg);
		}
		// Quadrant du noeud/premier bend
		int qtrg = quadrant(sx, sy, tx, ty);

		// Si le quadrant du nouveau point est inf�rieur a celui qu'on compare
		if (qnew < qtrg) {
			//on renvoie la face entre le noeud actuel et le pr�c�dent
			return CCE.leftFace(*it);
		}
		// Si le quadrant du nouveau est le m�me que celui � comparer
		else if (qnew == qtrg) {
			//si le nouveau noeud est � droite de targ on renvoie la face entre targ et son pr�c�dent
			int det = aGaucheInt(sx, sy, tx, ty, newX, newY);
			if (det == -1) {
				return CCE.leftFace(*it);
			}
			else if (det == 0) {
				f2 = CCE.rightFace(*it);
				return CCE.leftFace(*it);
			}
		}
		//le quadrant est plus grand ou �gal mais point a gauche: on passe au prochain noeud
	}
	//Si l'on arrive ici, il faut renvoyer la face entre le dernier et le premier noeud
	return CCE.leftFace(*adj.begin());
}

#endif