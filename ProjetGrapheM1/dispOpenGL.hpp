#ifndef DISPOPENGL_H
#define DISPOPENGL_H

#include <Windows.h>
#include <gl/GL.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <ogdf/basic/GridLayout.h>
#include <ogdf/basic/CombinatorialEmbedding.h>
#include <ogdf/basic/Layout.h>
#include "calcEdgeLength.hpp"
#include "EdgeMap.hpp"
#include "embedder.hpp"
#include "optimAlg.hpp"
#include "intersection.hpp"
#include "NodeBend.hpp"
#include <random>

using namespace ogdf;

bool moveRouletteRusse = false;
bool move_nodebend = false;
bool move_randomly = false;
bool show_move_variance = false;
bool show_variance = false;
int selectedNodeBendNum;
edge selectedEdge;
adjEntry selectedAdj;
bool show_grid_size = true;
std::set<face> setFace;
std::set<edge> setEdge;
bool showAllEdges = false;
ConstCombinatorialEmbedding CCE;

// Incrément de déplacement du selected node
int dx, dy;

// Retourne une valeur entiere comprise dans [1,n]
int generateRand(int n) {
	std::random_device rd;  // Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> dis(1, n);
	return dis(gen);
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
// A METTRE DANS UN AUTRE FICHIER ET APPELER DANS L'EMBEDDER
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

// Renvoie un vecteur de booléen de meme taille que "vectorMoveCoord". Ces booleen indiquent si le déplacement a la meme position dans ce vecteur est valide ou non.
std::vector<bool> getLegalMoves(NodeBend& n, GridLayout& GL, std::vector<std::pair<int, int>> vectorMoveCoord, ConstCombinatorialEmbedding& ccem) {
	std::vector<Segment> vectorSegmentNonAdj;
	int srcX = (*n.a_x);
	int srcY = (*n.a_y);
	// On recupere tout les edges qui composent les faces adjacentes
	std::set<edge> setAllEdges = getEdgesFromAdjFacesFromNodeBend(n, ccem);
	int edgeSrcX, edgeSrcY, edgeTrgX, edgeTrgY;
	// On parcourt ces edges et on les insere les segments NON adjacents dans un set
	for (auto it = setAllEdges.begin(); it != setAllEdges.end(); it++) {
		edgeSrcX = GL.x((*it)->source());
		edgeSrcY = GL.y((*it)->source());
		IPolyline& bends = GL.bends((*it));
		for (ListIterator<IPoint> i = bends.begin(); i.valid(); i++) {
			edgeTrgX = (*i).m_x;
			edgeTrgY = (*i).m_y;
			// Si les coords du noeud de l'edge source et target sont différents des coords du noeud de départ, on est pas adjacent
			if (((edgeSrcX != srcX) || (edgeSrcY != srcY)) && ((edgeTrgX != srcX) || (edgeTrgY != srcY))) {
				Segment tmpSegmentEdge(edgeSrcX, edgeSrcY, edgeTrgX, edgeTrgY);
				vectorSegmentNonAdj.push_back(tmpSegmentEdge);
			}
			edgeSrcX = edgeTrgX;
			edgeSrcY = edgeTrgY;
		}
		edgeTrgX = GL.x((*it)->target());
		edgeTrgY = GL.y((*it)->target());
		if (((edgeSrcX != srcX) || (edgeSrcY != srcY)) && ((edgeTrgX != srcX) || (edgeTrgY != srcY))) {
			Segment tmpSegmentEdge(edgeSrcX, edgeSrcY, edgeTrgX, edgeTrgY);
			vectorSegmentNonAdj.push_back(tmpSegmentEdge);
		}
	}
	int trgX, trgY;
	// Contient les coordonnées des points adjacents (node ou bend) au point de départ
	std::vector<std::pair<int, int>> vectorTargetAdjNode;
	if (n.isNode) {
		SListPure<adjEntry> adjEntries;
		n.getNode()->allAdjEntries(adjEntries);
		for (auto it = adjEntries.begin(); it.valid(); it++) {
			getTargetCoord(GL, (*it), trgX, trgY);
			std::pair<int, int> tmpPoint(trgX, trgY);
			vectorTargetAdjNode.push_back(tmpPoint);
		}
	}
	else {
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
				vectorTargetAdjNode.push_back(tmpPoint);
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
				vectorTargetAdjNode.push_back(tmpPoint2);
				break;
			}
		}
	}
	// Pour chaque déplacement, on regarde si il y a une intersection associé
	std::vector<bool> vectorMoveAutorised;
	bool intersection;
	for (int i = 0; i < vectorMoveCoord.size(); i++) {
		intersection = false;
		// On parcour la liste des points adjacents au point de départ
		for (int j = 0; (j < vectorTargetAdjNode.size()) && (!intersection); j++) {
			// Et on regarde si une intersection se créer avec la liste des segments non adjacents
			for (int k = 0; (k < vectorSegmentNonAdj.size()) && (!intersection); k++) {
				// On regarde si les segments a vérifier ne sont pas adjacents entre eux
				//std::cout << " x: " << vectorMoveCoord[i].first << " y: " << vectorMoveCoord[i].second << " x: " << vectorTargetAdjNode[j].first << " y: " << vectorTargetAdjNode[j].second << " x: " << vectorSegmentNonAdj[k].sourceX << " y: " << vectorSegmentNonAdj[k].sourceY << " x: " << vectorSegmentNonAdj[k].targetX << " y: " << vectorSegmentNonAdj[k].targetY << std::endl;
				if (((vectorTargetAdjNode[j].first != vectorSegmentNonAdj[k].sourceX) || (vectorTargetAdjNode[j].second != vectorSegmentNonAdj[k].sourceY)) && ((vectorTargetAdjNode[j].first != vectorSegmentNonAdj[k].targetX) || (vectorTargetAdjNode[j].second != vectorSegmentNonAdj[k].targetY))) {
					if (seCroisent(vectorMoveCoord[i].first, vectorMoveCoord[i].second, vectorTargetAdjNode[j].first, vectorTargetAdjNode[j].second, vectorSegmentNonAdj[k].sourceX, vectorSegmentNonAdj[k].sourceY, vectorSegmentNonAdj[k].targetX, vectorSegmentNonAdj[k].targetY)) {
						intersection = true;
					}
				}
				// Si ils le sont on regarde si le noeud source ne se trouve pas sur le segment adjacent
				else {
					if (surSegment(vectorSegmentNonAdj[k].sourceX, vectorSegmentNonAdj[k].sourceY, vectorSegmentNonAdj[k].targetX, vectorSegmentNonAdj[k].targetY, vectorMoveCoord[i].first, vectorMoveCoord[i].second)) {
						intersection = true;
					}
				}
			}
		}
		//si le point de départ a deux edge ou moins attachés (ne gere pas un point détaché de tout edge mais ce cas ne devrait ni exister ni influer)
		// Code de l'inversion a revoir
		/*
		if (vectorTargetAdjNode.size() <= 2) {
			int comp, comp2;
			int j = 0;
			//on compare la position du noeud qui bouge à l'edge successeur de chaque edge attaché au noeud pour ne pas changer l'embedding
			for (auto it = adjEntries.begin(); (it.valid())&&(!intersection); it++, j++) {
				getTargetCoord(GL, (*it)->cyclicSucc(), trgX, trgY);
				comp = aGaucheInt(vectorTargetAdjNode[j].first, vectorTargetAdjNode[j].second, trgX, trgY, srcX, srcY);
				comp2 = aGaucheInt(vectorTargetAdjNode[j].first, vectorTargetAdjNode[j].second, trgX, trgY, vectorMoveCoord[i].first, vectorMoveCoord[i].second);
				//si le noeud n'est pas du même côté avant et apres déplacement on rend le déplacement illégal (le cas aligné est traité plus haut)
				if (comp * comp2 == -1) {
					intersection = true;
				}
			}
		}
		*/
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
std::vector<std::pair<int, std::pair<int, int>>> rouletteRusseNodeMove(NodeBend& n, GridLayout& GL, ConstCombinatorialEmbedding& ccem, double& moy, double& sommeVar, double& var) {
	int nx = (*n.a_x);
	int ny = (*n.a_y);
	// On stocke les changements de variances apres un déplacement
	std::map<int, double> mapVarChangeMove;
	// On stocke les coordonnées d'arrivée qu'on aurait apres le déplacement
	std::vector<std::pair<int, int>> vectorMoveCoord;
	vectorMoveCoord.push_back(std::pair<int, int>(nx + 1, ny));
	vectorMoveCoord.push_back(std::pair<int, int>(nx, ny + 1));
	vectorMoveCoord.push_back(std::pair<int, int>(nx - 1, ny));
	vectorMoveCoord.push_back(std::pair<int, int>(nx, ny - 1));
	// On stocke si les déplacements sont autorisés, donc s'il n'y a pas de node ou de bend a ces coordonnées
	std::vector<bool> vectorMoveAutorised = getLegalMoves(n, GL, vectorMoveCoord, ccem);

	double tmpMaxVariance = var;
	double tmpMinVariance = var;
	double tmpVarSomme = sommeVar;
	bool atLeastOneValidMove = false;
	SListPure<edge> edges;
	bool isNode = true;
	if (n.isNode) {
		n.getNode()->adjEdges(edges);
	}
	else {
		edges.pushBack(n.getEdge());
		isNode = false;
	}
	// Boucle sur tout les déplacements possibles
	for (int i = 0; i < vectorMoveAutorised.size(); i++) {
		// On regarde si le déplacement est autorisé (si on ne se déplace par sur une node ou un bend)
		if (vectorMoveAutorised[i]) {
			double tmpMoy = moy;
			double tmpSommeVar = sommeVar;
			double tmpVar = var;
			atLeastOneValidMove = true;
			for (auto it = edges.begin(); it.valid(); it++) {
				auto it2 = mapEdgeLength.find((*it));
				double tmpOldLength = it2->second;
				double tmpNewLength;
				if (isNode) {
					tmpNewLength = calcTmpEdgeLength((*it), vectorMoveCoord[i].first, vectorMoveCoord[i].second, GL);
				}
				else {
					tmpNewLength = calcTmpEdgeLengthBends((*it), n, vectorMoveCoord[i].first, vectorMoveCoord[i].second, GL);
				}
				calcVarianceChang(tmpOldLength, tmpNewLength, tmpMoy, tmpSommeVar);
			}
			calcVarianceEdgeLength(tmpVar, tmpSommeVar, tmpMoy);
			mapVarChangeMove.insert(std::pair<int, double>(i, tmpVar));
			if (tmpVar > tmpMaxVariance) {
				tmpMaxVariance = tmpVar;
			}
			if (tmpVar < tmpMinVariance) {
				tmpMinVariance = tmpVar;
			}
			std::cout << "Variance apres deplacement " << i << ": " << tmpVar << std::endl;
		}
	}
	//pas de mouvement -> déplacement 4
	vectorMoveCoord.push_back(std::pair<int, int>(nx, ny));
	vectorMoveAutorised.push_back(true);
	mapVarChangeMove.insert(std::pair<int, double>(vectorMoveCoord.size(), var));
	std::cout << "Variance apres deplacement " << vectorMoveCoord.size()-1 << ": " << var << std::endl;
	//std::vector<int> vectorProbaMove;
	std::vector<std::pair<int, std::pair<int, int>>> vectorProbaMove2;
	if (atLeastOneValidMove) {
		//vectorProbaMove.reserve(vectorMoveAutorised.size());
		// On soustrait a tout les valeurs la variance maximale
		for (auto it = mapVarChangeMove.begin(); it != mapVarChangeMove.end(); it++) {
			it->second = abs(it->second - tmpMaxVariance) + tmpMinVariance;
			tmpVarSomme += it->second;
		}
		// On transforme les valeurs en proba
		int tmpSommeProba = 0;
		auto it = mapVarChangeMove.begin();
		int size = vectorMoveAutorised.size() - 1;
		for (int i = 0; i < size; i++) {
			if (vectorMoveAutorised[i]) {
				int tmpProba = round((it->second / tmpVarSomme) * 100);
				tmpSommeProba += tmpProba;
				it++;
			}
			//vectorProbaMove.push_back(tmpSommeProba);
			std::pair<int, std::pair<int, int>> tmpPair(tmpSommeProba, vectorMoveCoord[i]);
			vectorProbaMove2.push_back(tmpPair);
		}
		std::pair<int, std::pair<int, int>> tmpPair(100, vectorMoveCoord[size]);
		vectorProbaMove2.push_back(tmpPair);
		//vectorProbaMove.push_back(100);
	}
	//return vectorProbaMove;
	return vectorProbaMove2;
}

// Demarre l'algorithme de roulette russe sur le graphe
void startRouletteRusse(GridLayout& GL, ConstCombinatorialEmbedding& ccem) {
	// On choisis au hasard un NodeBend
	int randomNum = generateRand(vectorNodeBends.size()) - 1;
	NodeBend nb = vectorNodeBends[randomNum];
	double moyenne = 0.0;
	double sommeVariance = 0.0;
	double variance = 0.0;
	prepCalcVariance(moyenne, sommeVariance, variance);
	std::vector<std::pair<int, std::pair<int, int>>> probaDeplacement = rouletteRusseNodeMove(nb, GL, ccem, moyenne, sommeVariance, variance);
	if (probaDeplacement.size() > 0) {
		int randomChoice = generateRand(100);
		bool moved = false;
		for (int i = 0; ((i < probaDeplacement.size()) && (!moved)); i++) {
			if (randomChoice <= probaDeplacement[i].first) {
				(*nb.a_x) = probaDeplacement[i].second.first;
				(*nb.a_y) = probaDeplacement[i].second.second;
				moved = true;
			}
		}
	}
	prepCalcVariance(moyenne, sommeVariance, variance);
}

// Calcul le ratio edge/length. longueur la plus grande divisé par la longueur la plus courte.
double calcEdgeLengthRatio() {
	double ratio = (mapLengthEdgeSet.rbegin()->first / mapLengthEdgeSet.begin()->first);
	return ratio;
}

void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

// Callback pour OpenGL
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS)
		switch (key) {
			// Fermer l'application
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
			// Action deplacer un noeud aleatoirement?
			// TOUCHE Q SUR UN CLAVIER AZERTY!
		// Touche D et F pour changer d'edge dans le graphe (edge coloré)
		case GLFW_KEY_D:
			if (selectedEdge->pred() != nullptr)
				selectedEdge = selectedEdge->pred();
			break;
		case GLFW_KEY_F:
			if (selectedEdge->succ() != nullptr)
				selectedEdge = selectedEdge->succ();
			break;
			// Touche C et V pour changer de noeud dans le graphe (noeud coloré)
		case GLFW_KEY_C:
			if (selectedNodeBendNum > 0)
				selectedNodeBendNum--;
			break;
		case GLFW_KEY_V:
			if (selectedNodeBendNum < vectorNodeBends.size() - 1)
				selectedNodeBendNum++;
			break;
			// G permet de sélectionner l'adjEntry associé au noeud sélectionné
		case GLFW_KEY_G:
			if (vectorNodeBends[selectedNodeBendNum].isNode) {
				selectedAdj = vectorNodeBends[selectedNodeBendNum].getNode()->firstAdj();
				selectedEdge = selectedAdj->theEdge();
			}
			break;
			// Permet de tourner sur les edge adjacent a un point en fonction de l'embedding (sens trigo)
		case GLFW_KEY_J:
			if (selectedAdj != nullptr)
				if (selectedAdj->cyclicSucc() != nullptr) {
					selectedAdj = selectedAdj->cyclicSucc();
					selectedEdge = selectedAdj->theEdge();
				}
			break;
			// Change la taille de l'affichage (utile apres la planarisation)
		case GLFW_KEY_R:
			show_grid_size = !show_grid_size;
			break;
			// Récupere les faces adjacentes et ajoute les edges qui les composent dans setEdge
		case GLFW_KEY_P:
			if (vectorNodeBends[selectedNodeBendNum].isNode) {
				setFace.clear();
				setEdge.clear();
				SListPure<edge> edges;
				vectorNodeBends[selectedNodeBendNum].getNode()->adjEdges(edges);
				for (SListConstIterator<edge> i = edges.begin(); i.valid(); i++) {
					edge e = (*i);
					setFace.insert(CCE.rightFace(e->adjSource()));
					setFace.insert(CCE.leftFace(e->adjSource()));
				}
			}
			else {
				setFace.insert(CCE.rightFace(vectorNodeBends[selectedNodeBendNum].getEdge()->adjSource()));
				setFace.insert(CCE.leftFace(vectorNodeBends[selectedNodeBendNum].getEdge()->adjSource()));
			}
			for (auto it = setFace.begin(); it != setFace.end(); it++) {
				adjEntry firstAdj = (*it)->firstAdj();
				adjEntry nextAdj = firstAdj;
				if (firstAdj != nullptr) {
					do {
						if (nextAdj->theEdge() != nullptr) {
							setEdge.insert(nextAdj->theEdge());
						}
						nextAdj = (*it)->nextFaceEdge(nextAdj);
					} while ((nextAdj != firstAdj) && (nextAdj != nullptr));
				}
			}
			break;
			// Active ou désactive l'affichage des edge en bleu contenus dans setEdge
		case GLFW_KEY_L:
			showAllEdges = !showAllEdges;
			break;
			// GAUCHE,DROITE,HAUT,BAS déplace le point sélectionné de 1 case dans la direction de la fleche
		case GLFW_KEY_LEFT:
			dx = -1;
			move_randomly = true;
			break;
		case GLFW_KEY_RIGHT:
			dx = 1;
			move_randomly = true;
			break;
		case GLFW_KEY_DOWN:
			dy = -1;
			move_randomly = true;
			break;
		case GLFW_KEY_UP:
			dy = 1;
			move_randomly = true;
			break;
		case GLFW_KEY_1:
			show_move_variance = true;
			break;
		case GLFW_KEY_2:
			show_variance = true;
			break;
		case GLFW_KEY_3:
			moveRouletteRusse = true;
			break;
		}
}

void changeEdgeMapValue(edge e, GridLayout& GL) {
	auto it = mapEdgeLength.find(e);
	double length = it->second;
	double newLength = calcEdgeLength(e, GL);
	if (length != newLength) {
		it->second = newLength;
		auto it2 = mapLengthEdgeSet.find(newLength);
		if (it2 != mapLengthEdgeSet.end()) {
			it2->second.insert(e);
		}
		else {
			std::set<edge> tmpSet;
			tmpSet.insert(e);
			mapLengthEdgeSet.insert(std::pair<double, std::set<edge>>(newLength, tmpSet));
		}
		auto it3 = mapLengthEdgeSet.find(length);
		it3->second.erase(e);
		if (it3->second.empty()) {
			mapLengthEdgeSet.erase(length);
		}
	}
}

void changeNodeAdjEdgesMapValues(node n, GridLayout& GL) {
	SListPure<edge> edges;
	n->adjEdges(edges);
	for (SListConstIterator<edge> i = edges.begin(); i.valid(); i++) {
		edge e = (*i);
		changeEdgeMapValue(e, GL);
	}
	std::cout << "Ratio: " << calcEdgeLengthRatio() << std::endl;
}

void move(NodeBend n, int dx, int dy) {
	(*n.a_x) += dx;
	(*n.a_y) += dy;
}

void dispOpenGL(Graph& G, GridLayout& GL, const int gridWidth, const int gridHeight, int maxX, int maxY) {

	for (auto ti : vectorNodeBends) {
		std::cout << "IsNode: " << ti.isNode << " x: " << (*ti.a_x) << " y: " << (*ti.a_y) << std::endl;
	}


	//debut ogdf
	node n = G.firstNode();
	CCE = ConstCombinatorialEmbedding{ G };
	double moyenne = 0, sommeVariance = 0, variance = 0;
	prepCalcVariance(moyenne, sommeVariance, variance);
	std::cout << "Moyenne: " << moyenne << std::endl;
	std::cout << "Somme Variance: " << sommeVariance << std::endl;
	std::cout << "Variance: " << variance << std::endl;

	//fin ogdf
	if (!glfwInit())
		exit(EXIT_FAILURE);
	GLFWwindow* window = glfwCreateWindow(640, 480, "Fenetre OpenGL", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	int width, height;
	selectedEdge = G.firstEdge();
	selectedNodeBendNum = 0;
	glLineWidth(3);
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		if (show_grid_size) {
			glOrtho(-1, static_cast<float>(gridWidth) + 1, -1, static_cast<float>(gridHeight) + 1, 1.f, -1.f);
		}
		else {
			glOrtho(-1, static_cast<float>(maxX) + 1, -1, static_cast<float>(maxY) + 1, 1.f, -1.f);
		}
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if (move_nodebend) {
			move_nodebend = false;
		}
		// Deplacer un noeud aléatoirement
		if (move_randomly) {
			move(vectorNodeBends[selectedNodeBendNum], dx, dy);
			dx = 0;
			dy = 0;
			move_randomly = false;
		}
		else if (show_move_variance) {
			moyenne = 0, sommeVariance = 0, variance = 0;
			prepCalcVariance(moyenne, sommeVariance, variance);
			std::vector<std::pair<int, std::pair<int, int>>> vectorProba = rouletteRusseNodeMove(vectorNodeBends[selectedNodeBendNum], GL, CCE, moyenne, sommeVariance, variance);
			for (int i = 0; i < vectorProba.size(); i++) {
				std::cout << "Probabilite deplacement " << i << ": " << vectorProba[i].first << std::endl;
			}
			show_move_variance = false;
		}
		else if (show_variance) {
			moyenne = 0, sommeVariance = 0, variance = 0;
			prepCalcVariance(moyenne, sommeVariance, variance);
			std::cout << "Moyenne: " << moyenne << std::endl;
			std::cout << "Somme Variance: " << sommeVariance << std::endl;
			std::cout << "Variance: " << variance << std::endl;
			show_variance = false;
		}
		else if (moveRouletteRusse) {
			startRouletteRusse(GL, CCE);
			moveRouletteRusse = false;
		}
		//afficher les edge
		glColor3f(1.0f, 1.0f, 1.0f);
		for (auto e : G.edges)
		{
			if ((showAllEdges) && (setEdge.find(e) != setEdge.end())) {
				glColor3f(0.0f, 0.0f, 1.0f);
			}
			else if (e == selectedEdge) {
				glColor3f(0.0f, 1.0f, 0.0f);
			}
			else {
				glColor3f(1.0f, 1.0f, 1.0f);
			}
			glBegin(GL_LINE_STRIP);
			glVertex2d(GL.x(e->source()), GL.y(e->source()));
			IPolyline& bends = GL.bends(e);
			for (ListIterator<IPoint> i = bends.begin(); i.valid(); i++) {
				glVertex2d((*i).m_x, (*i).m_y);
			}
			glVertex2d(GL.x(e->target()), GL.y(e->target()));
			glEnd();
		}
		//afficher les nodes
		glPointSize(7);
		glBegin(GL_POINTS);
		for (int i = 0; i < vectorNodeBends.size(); i++) {
			if (!vectorNodeBends[i].isNode) {
				glColor3f(1.0f, 1.0f, 1.0f);
			}
			else {
				glColor3f(1.0f, 0.0f, 0.0f);
			}
			if (i == selectedNodeBendNum) {
				glColor3f(0.0f, 0.0f, 1.0f);
			}
			glVertex2d(*vectorNodeBends[i].a_x, *vectorNodeBends[i].a_y);
		}
		glEnd();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
}

#endif