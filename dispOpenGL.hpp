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
			move(vectorNodeBends[selectedNodeBendNum], GL, dx, dy, sommeLong,sommeLong2,variance);
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