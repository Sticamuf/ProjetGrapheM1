﻿#ifndef DISPOPENGL_H
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
#include <random>

using namespace ogdf;

bool move_randomly = false;
bool show_move_variance = false;
bool show_variance = false;
edge selectedEdge;
node selectedNode;
adjEntry selectedAdj;
bool show_grid_size = true;
std::set<face> setFace;
std::set<edge> setEdge;
bool showAllEdges = false;
ConstCombinatorialEmbedding CCE;

// Incrément de déplacement du selected node
int dx, dy;

// Retourne une valeur entiere comprise dans [0,99]
int generateRand() {
	double r;
	std::random_device rd;  // Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<> dis(0.0, 1.0);
	// Use dis to transform the random unsigned int generated by gen into a 
	// double in [0, 1). Each call to dis(gen) generates a new random double
	r = dis(gen);
	int result = floor(r * 100);
	return result;
}

// Renvoie un set composé de tout les edges qui composent les faces adjacentes a un noeud
std::set<edge> getEdgesFromAdjFacesFromNode(const node& n, ConstCombinatorialEmbedding& ccem) {
	std::set<edge> setAllEdges;
	std::set<face> setAdjFaces;
	SListPure<edge> edges;
	n->adjEdges(edges);
	for (SListConstIterator<edge> i = edges.begin(); i.valid(); i++) {
		edge e = (*i);
		//setAdjFaces.insert(CCE.rightFace(e->adjSource()));
		setAdjFaces.insert(ccem.leftFace(e->adjSource()));
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
std::vector<bool> checkAdjIntersection(const node& n, GridLayout& GL, std::vector<std::pair<int, int>> vectorMoveCoord, ConstCombinatorialEmbedding& ccem) {
	SListPure<adjEntry> adjEntries;
	n->allAdjEntries(adjEntries);
	std::vector<Segment> vectorSegmentNonAdj;
	int srcX = GL.x(n);
	int srcY = GL.y(n);
	// On recupere tout les edges qui composent les faces adjacentes
	std::set<edge> setAllEdges = getEdgesFromAdjFacesFromNode(n, ccem);
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
	for (auto it = adjEntries.begin(); it.valid(); it++) {
		getTargetCoord(GL, (*it), trgX, trgY);
		std::pair<int, int> tmpPoint(trgX, trgY);
		vectorTargetAdjNode.push_back(tmpPoint);
	}
	// Pour chaque déplacement, on regarde si il y a une intersection associé
	std::vector<bool> vectorMoveAutorised;
	bool intersection;
	for (int i = 0; i < vectorMoveCoord.size(); i++) {
		intersection = false;
		// On parcour la liste des points adjacents au point de départ
		for (int j = 0; j < vectorTargetAdjNode.size(); j++) {
			// Et on regarde si une intersection se créer avec la liste des segments non adjacents
			for (int k = 0; k < vectorSegmentNonAdj.size(); k++) {
				std::cout << "px: " << vectorMoveCoord[i].first << " py: " << vectorMoveCoord[i].second << " qx: " << vectorTargetAdjNode[j].first << " qy: " << vectorTargetAdjNode[j].second << " rx: " << vectorSegmentNonAdj[k].sourceX << " ry: " << vectorSegmentNonAdj[k].sourceY << " sx: " << vectorSegmentNonAdj[k].targetX << " sy: " << vectorSegmentNonAdj[k].targetY << std::endl;
				// On regarde si les segments a vérifier ne sont pas adjacents entre eux
				if (((vectorTargetAdjNode[j].first != vectorSegmentNonAdj[k].sourceX) || (vectorTargetAdjNode[j].second != vectorSegmentNonAdj[k].sourceY))&& ((vectorTargetAdjNode[j].first != vectorSegmentNonAdj[k].targetX) || (vectorTargetAdjNode[j].second != vectorSegmentNonAdj[k].targetY))) {
					if (seCroisent(vectorMoveCoord[i].first, vectorMoveCoord[i].second, vectorTargetAdjNode[j].first, vectorTargetAdjNode[j].second, vectorSegmentNonAdj[k].sourceX, vectorSegmentNonAdj[k].sourceY, vectorSegmentNonAdj[k].targetX, vectorSegmentNonAdj[k].targetY)) {
						intersection = true;
						std::cout << intersection << std::endl;
						break;
					}
					std::cout << intersection << std::endl;
				}
				// Si ils le sont on regarde si le noeud source ne se trouve pas sur le segment adjacent
				else {
					if (surSegment(vectorSegmentNonAdj[k].sourceX, vectorSegmentNonAdj[k].sourceY, vectorSegmentNonAdj[k].targetX, vectorSegmentNonAdj[k].targetY, vectorMoveCoord[i].first, vectorMoveCoord[i].second)) {
						intersection = true;
						std::cout << intersection << std::endl;
						break;
					}
					std::cout << intersection << std::endl;
				}
			}
			if (intersection) {
				break;
			}
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
std::vector<int> rouletteRusseNodeMove(const node& n, GridLayout& GL, ConstCombinatorialEmbedding& ccem, double& moy, double& sommeVar, double& var) {
	int nx = GL.x(n);
	int ny = GL.y(n);
	SListPure<edge> edges;
	n->adjEdges(edges);
	// On stocke les changements de variances apres un déplacement
	std::map<int, double> mapVarChangeMove;
	// On stocke les coordonnées d'arrivée qu'on aurait apres le déplacement
	std::vector<std::pair<int, int>> vectorMoveCoord;
	vectorMoveCoord.push_back(std::pair<int, int>(nx + 1, ny));
	vectorMoveCoord.push_back(std::pair<int, int>(nx, ny + 1));
	vectorMoveCoord.push_back(std::pair<int, int>(nx - 1, ny));
	vectorMoveCoord.push_back(std::pair<int, int>(nx, ny - 1));
	// On stocke si les déplacements sont autorisés, donc s'il n'y a pas de node ou de bend a ces coordonnées
	std::vector<bool> vectorMoveAutorised = checkAdjIntersection(n, GL, vectorMoveCoord, ccem);
	double tmpMaxVariance = 0;
	double tmpMinVariance = INT_MAX;
	double tmpVarSomme = 0;
	bool atLeastOneValidMove = false;
	// Boucle sur tout les déplacements possibles
	for (int i = 0; i < vectorMoveAutorised.size(); i++) {
		double tmpMoy = moy;
		double tmpSommeVar = sommeVar;
		double tmpVar = var;
		// On regarde si le déplacement est autorisé (si on ne se déplace par sur une node ou un bend)
		if (vectorMoveAutorised[i]) {
			atLeastOneValidMove = true;
			for (auto it = edges.begin(); it.valid(); it++) {
				auto it2 = mapEdgeLength.find((*it));
				double tmpOldLength = it2->second;
				double tmpNewLength = calcTmpEdgeLength((*it), vectorMoveCoord[i].first, vectorMoveCoord[i].second, GL);
				calcVarianceChang(tmpOldLength, tmpNewLength, tmpMoy, tmpSommeVar);
			}
			calcVarianceEdgeLength(tmpVar, tmpSommeVar, tmpMoy);
			mapVarChangeMove.insert(std::pair<int, double>(i, tmpVar));
			tmpMaxVariance = max(tmpMaxVariance, tmpVar);
			tmpMinVariance = min(tmpMinVariance, tmpVar);
			std::cout << "Variance apres deplacement " << i << ": " << tmpVar << std::endl;
		}
	}
	std::vector<int> vectorProbaMove;
	if (atLeastOneValidMove) {
		vectorProbaMove.reserve(vectorMoveAutorised.size());
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
				int tmpProba = floor((it->second / tmpVarSomme) * 100);
				tmpSommeProba += tmpProba;
				it++;
			}
			vectorProbaMove.push_back(tmpSommeProba);
		}
		vectorProbaMove.push_back(100);
	}
	return vectorProbaMove;
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
			if (selectedNode->pred() != nullptr)
				selectedNode = selectedNode->pred();
			break;
		case GLFW_KEY_V:
			if (selectedNode->succ() != nullptr)
				selectedNode = selectedNode->succ();
			break;
			// G permet de sélectionner l'adjEntry associé au noeud sélectionné
		case GLFW_KEY_G:
			selectedAdj = selectedNode->firstAdj();
			selectedEdge = selectedAdj->theEdge();
			selectedNode = selectedAdj->theNode();
			break;
			// T permet de sélectionner l'adjEntry opposée a l'adjEntry actuelle
		case GLFW_KEY_T:
			if (selectedAdj != nullptr) {
				selectedAdj = selectedAdj->twin();
				selectedEdge = selectedAdj->theEdge();
				selectedNode = selectedAdj->theNode();
			}
			break;
			// Permet de tourner sur les edge adjacent a un point en fonction de l'embedding (sens trigo)
		case GLFW_KEY_J:
			if (selectedAdj != nullptr)
				if (selectedAdj->cyclicSucc() != nullptr) {
					selectedAdj = selectedAdj->cyclicSucc();
					selectedEdge = selectedAdj->theEdge();
					selectedNode = selectedAdj->theNode();
				}
			break;
			// Change la taille de l'affichage (utile apres la planarisation)
		case GLFW_KEY_R:
			show_grid_size = !show_grid_size;
			break;
			// Récupere les faces adjacentes et ajoute les edges qui les composent dans setEdge
		case GLFW_KEY_P:
			if (selectedNode != nullptr) {
				setFace.clear();
				setEdge.clear();
				SListPure<edge> edges;
				selectedNode->adjEdges(edges);
				for (SListConstIterator<edge> i = edges.begin(); i.valid(); i++) {
					edge e = (*i);
					setFace.insert(CCE.rightFace(e->adjSource()));
					setFace.insert(CCE.leftFace(e->adjSource()));
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

void move(Graph& G, GridLayout& GL, node n, int dx, int dy) {

	face f2 = nullptr;
	face selectedFace = getFace(CCE, GL, n, GL.x(n) + dx, GL.y(n) + dy, f2);
	GL.x(n) += dx;
	GL.y(n) += dy;
	setEdge.clear();

	embedNode(G, GL, n);
	changeNodeAdjEdgesMapValues(n, GL);

	if (f2 != nullptr) {
		adjEntry firstAdj = f2->firstAdj();
		adjEntry nextAdj = firstAdj;
		if (firstAdj != nullptr) {
			do {
				if (nextAdj->theEdge() != nullptr) {
					setEdge.insert(nextAdj->theEdge());
				}
				nextAdj = f2->nextFaceEdge(nextAdj);
			} while ((nextAdj != firstAdj) && (nextAdj != nullptr));
		}
	}

	adjEntry firstAdj = selectedFace->firstAdj();
	adjEntry nextAdj = firstAdj;
	if (firstAdj != nullptr) {
		do {
			if (nextAdj->theEdge() != nullptr) {
				setEdge.insert(nextAdj->theEdge());
			}
			nextAdj = selectedFace->nextFaceEdge(nextAdj);
		} while ((nextAdj != firstAdj) && (nextAdj != nullptr));
	}
}

void dispOpenGL(Graph& G, GridLayout& GL, const int gridWidth, const int gridHeight, int maxX, int maxY) {
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
	GLFWwindow* window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
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
	selectedNode = G.firstNode();
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
		// Deplacer un noeud aléatoirement
		if (move_randomly) {
			move(G, GL, selectedNode, dx, dy);
			dx = 0;
			dy = 0;
			move_randomly = false;
		}
		if (show_move_variance) {
			moyenne = 0, sommeVariance = 0, variance = 0;
			prepCalcVariance(moyenne, sommeVariance, variance);
			std::vector<int> vectorProba = rouletteRusseNodeMove(selectedNode, GL, CCE, moyenne, sommeVariance, variance);
			for (int i = 0; i < vectorProba.size(); i++) {
				std::cout << "Variance deplacement " << i << ": " << vectorProba[i] << std::endl;
			}
			show_move_variance = false;
		}
		if (show_variance) {
			moyenne = 0, sommeVariance = 0, variance = 0;
			prepCalcVariance(moyenne, sommeVariance, variance);
			std::cout << "Moyenne: " << moyenne << std::endl;
			std::cout << "Somme Variance: " << sommeVariance << std::endl;
			std::cout << "Variance: " << variance << std::endl;
			show_variance = false;
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
		glColor3f(1.0f, 0.0f, 0.0f);
		glPointSize(5);
		n = G.firstNode();
		glBegin(GL_POINTS);
		while (n != nullptr) {
			if (n == selectedNode) {
				glColor3f(0.0f, 0.0f, 1.0f);
			}
			else {
				glColor3f(1.0f, 0.0f, 0.0f);
			}
			glVertex2d(GL.x(n), GL.y(n));
			n = n->succ();
		}
		glEnd();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
}

#endif