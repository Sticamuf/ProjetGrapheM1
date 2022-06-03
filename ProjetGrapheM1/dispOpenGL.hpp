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
#include "graphFunctions.hpp"
#include "NodeBend.hpp"
#include <random>

#undef max
#undef min

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

void dispOpenGL(Graph& G, GridLayout& GL, const int gridWidth, const int gridHeight, int maxX, int maxY) {
	//debut ogdf
	node n = G.firstNode();
	CCE = ConstCombinatorialEmbedding{ G };
	double sommeLong = 0, sommeLong2 = 0, variance = 0;
	prepCalcNVar(sommeLong, sommeLong2, variance);
	std::cout << "sommeLong: " << sommeLong << std::endl;
	std::cout << "sommeLong sommeLong: " << sommeLong2 << std::endl;
	std::cout << "Variance: " << variance << std::endl;
	//fin ogdf

	//debut mettre le graphe dans la grille
	////récupérer les coordonnées extrémales du graphe
	auto startX = setNodeBendsX.begin();
	auto startY = setNodeBendsY.begin();
	auto endX = setNodeBendsX.end();
	auto endY = setNodeBendsY.end();
	endX--;
	endY--;


	//calculer le centre de gravité
	long long centreX = 0;
	long long centreY = 0;
	for (int i = 0; i < vectorNodeBends.size(); i++) {
		centreX += *vectorNodeBends[i].a_x;
		centreY += *vectorNodeBends[i].a_y;
	}
	centreX /= vectorNodeBends.size();
	centreY /= vectorNodeBends.size();
	
	//tant qu'il y a des nodebend en dehors de la grille
	while (*(*endX).a_x - *(*startX).a_x > gridWidth || *(*endY).a_y - *(*startY).a_y > gridHeight) {
		//choisir un nodebend au hasard
		int randomNum = generateRand(vectorNodeBends.size()) - 1;
		NodeBend nb = vectorNodeBends[randomNum];
		//calculer quels déplacements sont autorisés
		int nx = (*nb.a_x);
		int ny = (*nb.a_y);
		std::vector<std::pair<int, int>> vectorMoveCoord;
		vectorMoveCoord.push_back(std::pair<int, int>(nx + 1, ny));
		vectorMoveCoord.push_back(std::pair<int, int>(nx, ny + 1));
		vectorMoveCoord.push_back(std::pair<int, int>(nx - 1, ny));
		vectorMoveCoord.push_back(std::pair<int, int>(nx, ny - 1));
		vectorMoveCoord.push_back(std::pair<int, int>(nx + 1, ny + 1));
		vectorMoveCoord.push_back(std::pair<int, int>(nx + 1, ny - 1));
		vectorMoveCoord.push_back(std::pair<int, int>(nx - 1, ny + 1));
		vectorMoveCoord.push_back(std::pair<int, int>(nx - 1, ny - 1));
		std::vector<bool> vectorMoveAutorised = getLegalMoves(nb, GL, vectorMoveCoord, CCE);
		//vectorMoveCoord.push_back(std::pair<int, int>(nx, ny));
		//vectorMoveAutorised.push_back(true);

		std::vector<double> vecDistCentre;
		vecDistCentre.reserve(vectorMoveCoord.size());
		//double moyDist = 0;
		double sumDist = 0;
		double minDist = std::numeric_limits<double>::max();
		double maxDist = -1;
		int nbLegalMoves = 0;
		// Boucle sur tout les d�placements possibles
		for (int i = 0; i < vectorMoveAutorised.size(); i++) {
			// On regarde si le d�placement est autoris� (si on ne se d�place par sur une node ou un bend ou qu'on ne crée pas d'intersection ou d'inversion de face)
			if (vectorMoveAutorised[i]) {
				//si le déplacement est autorisé on calcule sa distance au centre de la grille;
				//vecDistCentre.push_back(sqrt(pow(centreX - vectorMoveCoord[i].first, 2) + pow(centreY - vectorMoveCoord[i].second, 2)));
				vecDistCentre.push_back(pow(centreX - vectorMoveCoord[i].first, 2) + pow(centreY - vectorMoveCoord[i].second, 2));
				sumDist += vecDistCentre[i];
				if (minDist > vecDistCentre[i]) {
					minDist = vecDistCentre[i];
				}
				if (maxDist < vecDistCentre[i]) {
					maxDist = vecDistCentre[i];
				}
				nbLegalMoves++;
			}
			else {
				vecDistCentre.push_back(-1);
			}
		}
		//moyDist = sumDist/nbLegalMoves;

		if (nbLegalMoves != 0) {
			////pour chaque déplacement autorisé lui affecter une probabilité d'apparaitre
			//std::vector<double> vecSumProba;
			//vecSumProba.reserve(vectorMoveCoord.size());
			////atteindre la première proba non nulle
			//int i = 0;
			//while (i < vectorMoveAutorised.size() && !vectorMoveAutorised[i]) {
			//	vecSumProba.push_back(0);
			//	i++;
			//}
			////donner un pourcentage au premier déplacement légal
			//vecSumProba.push_back((abs(vecDistCentre[i]-maxDist)+minDist)/sumDist * 100);
			//i++;
			////pour tous les déplacements suivants
			//while (i < vectorMoveAutorised.size()-1) {
			//	//initialiser à la proba précédente
			//	vecSumProba.push_back(vecSumProba[i - 1]);
			//	if (vectorMoveAutorised[i]) {
			//		vecSumProba[i] += (abs(vecDistCentre[i] - maxDist) + minDist)/sumDist * 100;
			//	}
			//	i++;
			//}
			////impose le dernier déplacement à 100 -> faisable car le dernier déplacement sera de ne pas bouger et donc toujours légal
			//vecSumProba.push_back(100);
			////choisir un nombre aléatoire
			//int randomChoice = generateRand(100);
			////choisir une direction de déplacement en fonction du nombre aléatoire
			//int choix = 0;
			//while (randomChoice > vecSumProba[choix]) {
			//	choix++;
			//}

			//déscente -> la distance la plus faible est choisie
			int choix = -1;
			int dist = std::numeric_limits<int>::max();
			for (int i = 0; i < vecDistCentre.size(); i++) {
				if (vectorMoveAutorised[i]) {
					if (vecDistCentre[i] < dist) {
						choix = i;
						dist = vecDistCentre[i];
					}
				}
			}

			//changer les coordonnées du noeud
			//setNodeBendsX.erase(nb);
			//setNodeBendsY.erase(nb);
			*(nb).a_x = vectorMoveCoord[choix].first;
			*(nb).a_y = vectorMoveCoord[choix].second;
			setNodeBendsX.insert(nb);
			setNodeBendsY.insert(nb);
			startX = setNodeBendsX.begin();
			endX = setNodeBendsX.end();
			endX--;
			startY = setNodeBendsY.begin();
			endY = setNodeBendsY.end();
			endY--;
			//std::cout << "min : " << *(*start).a_x << " " << *(*start).a_y << std::endl;
			//std::cout << "max : " << *(*end).a_x << " " << *(*end).a_y << std::endl;

			//for (int j = 0; j < vecSumProba.size(); j++) {
			//	std::cout << "proba " << j << " : " << vecSumProba[j] << std::endl;
			//}

			//std::cout << "bouger noeud " << randomNum << " vers x: " << *(nb).a_x << " y: " << *(nb).a_y << std::endl;
		}
	}
	int mx = *(*startX).a_x , my = *(*startY).a_y;
	for (int i = 0; i < vectorNodeBends.size(); i++) {
		*vectorNodeBends[i].a_x -= mx;
		*vectorNodeBends[i].a_y -= my;
		//std::cout << "x;y : " << *vectorNodeBends[i].a_x << " " << *vectorNodeBends[i].a_y << std::endl;
	}

	std::cout << "min x : " << *(*startX).a_x << std::endl;
	std::cout << "min y : " << *(*startY).a_y << std::endl;
	std::cout << "max x : " << *(*endX).a_x << std::endl;
	std::cout << "max y : " << *(*endY).a_y << std::endl;

	//fin mettre le graphe dans la grille

	//for (const auto& it : setNodeBends) {
	//	std::cout << "set : " << *it.a_x << " " << *it.a_y << std::endl;
	//	//cout << "set address " << it.a_x << endl;
	//}
	//std::cout << "set size " << setNodeBendsX.size() << std::endl;
	//std::cout << "vec size " << vectorNodeBends.size() << std::endl;

	//debut openGL
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
			glOrtho(-20, static_cast<float>(maxX) + 20, -20, static_cast<float>(maxY) + 20, 1.f, -1.f);
		}
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if (move_nodebend) {
			move_nodebend = false;
		}
		// Deplacer un noeud aléatoirement
		if (move_randomly) {
			move(vectorNodeBends[selectedNodeBendNum], GL, dx, dy, sommeLong, sommeLong2, variance);
			dx = 0;
			dy = 0;
			move_randomly = false;
		}
		else if (show_move_variance) {
			std::vector<std::pair<int, std::pair<int, int>>> vectorProba = rouletteRusseNodeMove(vectorNodeBends[selectedNodeBendNum], GL, CCE, sommeLong, sommeLong2, variance);
			show_move_variance = false;
		}
		else if (show_variance) {
			double tmpSommeLong = 0, tmpSommeLong2 = 0, tmpVariance = 0;
			prepCalcNVar(tmpSommeLong, tmpSommeLong2, tmpVariance);
			std::cout << "sommeLong: " << tmpSommeLong << std::endl;
			std::cout << "sommeLong sommeLong: " << tmpSommeLong2 << std::endl;
			std::cout << "Variance: " << tmpVariance << std::endl;
			show_variance = false;
		}
		else if (moveRouletteRusse) {
			selectedNodeBendNum = startRouletteRusse(GL, CCE, sommeLong, sommeLong2, variance);
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