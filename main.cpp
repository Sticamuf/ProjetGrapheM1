#include <ogdf/basic/GraphAttributes.h>

#include <ogdf/planarlayout/PlanarStraightLayout.h>
#include <ogdf/planarity/EmbedderMinDepth.h>
#include <ogdf/planarity/SimpleEmbedder.h>
#include <ogdf/basic/simple_graph_alg.h>

#include "jsonIO.hpp"
#include "dispOpenGL.hpp"
#include "EdgeMap.hpp"
#include "NodeMap.hpp"

using ogdf::Graph;
using ogdf::GraphAttributes;

const int Q1 = 0;
const int Q2 = 1;
const int Q3 = 2;
const int Q4 = 3;

//renvoie 1,2,3 ou 4 si la node targ est 1: en haut à droite, 2: en haut à gauche, 3: en bas à gauche, 4: en bas à droite de la node src 
//dans le layout GL
//on considère src != targ
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

//on considère dans le meme quadrant
bool aGauche(const GridLayout& GL, const node& src, const node& targ, const node& comp) {
    return ((GL.x(targ) - GL.x(src)) * (GL.y(comp) - GL.y(src)) - (GL.y(targ) - GL.y(src)) * (GL.x(comp) - GL.x(src))) > 0;
}

void embedder(Graph& G, GridLayout& GL) {
    node nsrc = G.firstNode();
    SListPure<adjEntry> adj;
    SListPure<adjEntry> newOrder;
    while (nsrc != nullptr) {
        nsrc->allAdjEntries(adj);
        // Itérateur qui itere sur le tableau des adjacent non trié
        SListIterator<adjEntry> it = adj.begin();
        // Itérateur sur le nouveau tableau d'adjacent trié
        SListIterator<adjEntry> it2;
        // Itérateur égal a it2-1 :)
        SListIterator<adjEntry> it3;
        // On insere le premier element et on passe directement au prochain
        newOrder.pushBack((*it));
        it++;
        // On itere sur tout les éléments de la liste non triée
        for (;it.valid();it++) {
            bool inserted = false;
            // On itere sur le deuxieme tableau tant qu'on est pas a la fin et tant qu'on a pas inseré
            for (it2 = newOrder.begin(), it3=it2;(it2 != newOrder.end()&&!inserted); it3=it2++) {
                node ntrg = (*it2)->twinNode();
                int qtrg = quadrant(GL, nsrc, ntrg);
                node newnode = (*it)->twinNode();
                int qnewnode = quadrant(GL, nsrc, newnode);
                // Si le quadrant du point que l'on veut inserer est inférieur a celui qu'on compare
                if (qnewnode < qtrg) {
                    // Si on compare au premier, on insere en premiere place
                    if (it3 == newOrder.begin()) {
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
                        if (!aGauche(GL, nsrc, ntrg, newnode)) {
                            newOrder.insertAfter((*it), it3);
                            inserted = true;
                        }
                        // Sinon on passe au prochain point
                        else {
                            it3 = it2++;
                            ntrg = (*it2)->twinNode();
                            qtrg = quadrant(GL, nsrc, ntrg);
                        }
                    }
                    // Si on a pas inséré, c'est qu'on est le dernier élément du quadrant, donc on insere apres it3
                    if ((qnewnode != qtrg) && (!inserted)) {
                        newOrder.insertAfter((*it), it3);
                        inserted = true;
                    }
                }
            }
            // Si on a toujours pas inseré, c'est qu'on est le dernier element
            if (!inserted) {
                newOrder.pushBack((*it));
            }
        }
        nsrc = nsrc->succ();
    }
}

int main() {
    
    Graph G;
    GraphAttributes GA{G};
    GA.directed() = false;

    int gridWidth, gridHeight, maxBends;

    // ----- LECTURE D'UN FICHIER JSON DANS UN Graph -----
    string file = "exemple7.json";
    readFromJson(file, G, GA, gridWidth, gridHeight, maxBends);
    writeToJson("output.json", G, GA, gridWidth, gridHeight, maxBends);

    std::cout << G.representsCombEmbedding() << std::endl;
    int maxX = 0, maxY = 0, minX = 100000, minY = 100000;

    bool planarize = false;
    std::cout << "Connexe: " << isConnected(G) << std::endl;
    std::cout << "Planaire: " << isPlanar(G) << std::endl;
    if (planarize) {
        PlanarStraightLayout PL;
        PL.call(GA);
        node n = G.firstNode();
        while (n != nullptr) {
            if (GA.x(n) > maxX) maxX = GA.x(n);
            if (GA.x(n) < minX) minX = GA.x(n);
            if (GA.y(n) > maxY) maxY = GA.x(n);
            if (GA.y(n) < minY) minY = GA.x(n);
            n = n->succ();
        }
        std::cout << "Planarised" << std::endl;
    }
    //GraphIO::write(GA, "output-ERDiagram2.svg", GraphIO::drawSVG);
    std::cout << "Connexe: " << isConnected(G) << std::endl;
    std::cout << "Planaire: " << isPlanar(G) << std::endl;
    SimpleEmbedder Emf;
    //EmbedderMinDepth Emf;
    adjEntry tmpAdj;
    Emf.doCall(G, tmpAdj);
    std::cout << "Embeded: " << G.representsCombEmbedding() << std::endl;

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
    dispOpenGL(G,GA,gridWidth,gridHeight,maxX,maxY);
    return 0;
}