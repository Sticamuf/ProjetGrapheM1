#include <ogdf/basic/GridLayout.h>

#include <ogdf/planarlayout/PlanarStraightLayout.h>
#include <ogdf/planarity/EmbedderMinDepth.h>
#include <ogdf/planarity/SimpleEmbedder.h>
#include <ogdf/basic/simple_graph_alg.h>

#include "jsonIO.hpp"
#include "dispOpenGL.hpp"
#include "EdgeMap.hpp"
#include "NodeMap.hpp"

using ogdf::Graph;
using ogdf::GridLayout;

const int Q1 = 0;
const int Q2 = 1;
const int Q3 = 2;
const int Q4 = 3;

//renvoie 1,2,3 ou 4 si la node targ est 1: en haut à droite, 2: en haut à gauche, 3: en bas à gauche, 4: en bas à droite de la node src 
//dans le layout GL
//on considère src != targ
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

//on considère dans le meme quadrant
bool aGauche(int sx, int sy, int tx, int ty, int cx, int cy) {
    return ((tx - sx) * (cy - sy) - (ty - sy) * (cx - sx)) > 0;
}

void embedderDeFou(Graph& G, GridLayout& GL) {
    node nsrc = G.firstNode();
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
    while (nsrc != nullptr) {
        sx = GL.x(nsrc);
        sy = GL.y(nsrc);
        nsrc->allAdjEntries(adj);
        newOrder.clear(); 
        it = adj.begin();
        // On insere le premier element et on passe directement au prochain
        newOrder.pushBack((*it));
        it++;
        // On itere sur tout les éléments de la liste non triée
        for (;it.valid();it++) {
            bool inserted = false;
            // On itere sur le deuxieme tableau tant qu'on est pas a la fin et tant qu'on a pas inseré
            for (it2 = newOrder.begin(),it3=it2;((it2.valid())&&(!inserted));it3=it2++) {
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
                int qtrg = quadrant(sx,sy,tx,ty);


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
                int qnewnode = quadrant(sx,sy,nx,ny);
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
                    // Tant que les quadrants sont égaux
                    while ((qnewnode == qtrg) && (!inserted)) {
                        // Si on est a droite on insere apres it3
                        if (!aGauche(sx,sy,tx,ty,nx,ny)) {
                            if (it2 == newOrder.begin()) {
                                newOrder.pushFront((*it));
                            }
                            // Sinon on insere apres it3 car insertBefore n'existe pas :)
                            else {
                                newOrder.insertAfter((*it), it3);
                            }
                            inserted = true;
                        }
                        // Sinon on passe au prochain point
                        else {
                            it3 = it2++;
                            if (it2 != newOrder.end()) {
                                edge tmpEdge3 = (*it2)->theEdge();
                                IPolyline& p3 = GL.bends(tmpEdge3);
                                if (p3.size() > 0) {
                                    if (tmpEdge->source() == nsrc) {
                                        tx = p3.front().m_x;
                                        ty = p3.front().m_y;
                                    }
                                    else {
                                        tx = p3.back().m_x;
                                        ty = p3.back().m_y;
                                    }
                                }
                                else {
                                    ntrg = (*it2)->twinNode();
                                    tx = GL.x(ntrg);
                                    ty = GL.y(ntrg);
                                }
                                qtrg = quadrant(sx, sy, tx, ty);
                            }
                            else {
                                qnewnode = 5;
                                break;
                            }
                        }
                    }
                    // Si on a pas inséré, c'est qu'on est le dernier élément du quadrant, donc on insere apres it3
                    if ((qnewnode != qtrg) && (!inserted)) {
                        newOrder.insertAfter((*it), it3);
                        inserted = true;
                        break;
                    }
                }
            }
            // Si on a toujours pas inseré, c'est qu'on est le dernier element
            if (!inserted) {
                newOrder.pushBack((*it));
            }
        }
        G.sort(nsrc, newOrder);
        nsrc = nsrc->succ();
    }
}

int main() {
    
    Graph G;
    GridLayout GL{G};

    int gridWidth, gridHeight, maxBends;

    // ----- LECTURE D'UN FICHIER JSON DANS UN Graph -----
    string file = "exemples/man3.json";
    readFromJson(file, G, GL, gridWidth, gridHeight, maxBends);
    writeToJson("output.json", G, GL, gridWidth, gridHeight, maxBends);

    node nsrc = G.firstNode();
    SListPure<adjEntry> adj;
    int i = 0;
    while (nsrc != nullptr) {
        nsrc->allAdjEntries(adj);
        nsrc = nsrc->succ();
        std::cout << "Nb Adj du noeud " << i << ": " << adj.size() << std::endl;
        i++;
    }

    
    int maxX = 0, maxY = 0, minX = 100000, minY = 100000;

    bool planarize = false;
    if (planarize) {
        std::cout << "Planarizing..." << std::endl;
        PlanarStraightLayout PL;
        PL.separation(-19);
        PL.callGrid(G, GL);
        node n = G.firstNode();
        while (n != nullptr) {
            if (GL.x(n) > maxX) maxX = GL.x(n);
            if (GL.x(n) < minX) minX = GL.x(n);
            if (GL.y(n) > maxY) maxY = GL.x(n);
            if (GL.y(n) < minY) minY = GL.x(n);
            n = n->succ();
        }
    }

    std::cout << "Embedding..." << std::endl;
    embedderDeFou(G, GL);
    std::cout << "Embeded: " << G.representsCombEmbedding() << std::endl;

    //GraphIO::write(GL, "output-ERDiagram2.svg", GraphIO::drawSVG);
    std::cout << "Connexe: " << isConnected(G) << std::endl;
    std::cout << "Planaire: " << isPlanar(G) << std::endl;

    // Affichage des maps
    /*
    std::map<edge, double>::iterator it;
    for (it = mapEdgeLength.begin(); it != mapEdgeLength.end(); it++) {
        std::cout << "MapEdgeLength: " << it->second << std::endl;
    }

    std::map<double, std::set<edge>>::iterator it2;
    for (it2 = mapLengthEdgeSet.begin(); it2 != mapLengthEdgeSet.end(); it2++) {
        std::cout << "mapLengthEdgeSet: " << it2->first << std::endl;
    }
    */

    // OpenGL
    srand(static_cast<unsigned int>(time(NULL)));
    dispOpenGL(G,GL,gridWidth,gridHeight,maxX,maxY);
    return 0;
}