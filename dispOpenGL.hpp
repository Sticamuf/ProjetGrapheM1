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
#include "graphFunctions.hpp"
#include "NodeBend.hpp"
#include "optimAlg.hpp"
#include "jsonIO.hpp"
#include <random>

using namespace ogdf;

bool moveRecuitSimule = false;
bool autoRecuitSimule = false;
bool moveRouletteRusse = false;
bool autoRouletteRusse = false;
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
		case GLFW_KEY_4:
			autoRouletteRusse = !autoRouletteRusse;
			break;
		case GLFW_KEY_5:
			moveRecuitSimule = true;
			break;
		case GLFW_KEY_6:
			autoRecuitSimule = !autoRecuitSimule;
			break;
		}
}

void dispOpenGL(Graph& G, GridLayout& GL, const int gridWidth, const int gridHeight, int maxX, int maxY, int maxBends) {
	//debut ogdf
	node n = G.firstNode();
	CCE = ConstCombinatorialEmbedding{ G };
	double sommeLong = 0, sommeLong2 = 0, variance = 0;
	prepCalcNVar(sommeLong, sommeLong2, variance);
	double bestVariance = variance;
	std::cout << "sommeLong: " << sommeLong << std::endl;
	std::cout << "sommeLong sommeLong: " << sommeLong2 << std::endl;
	std::cout << "Variance: " << variance << std::endl;

	// Chrono pour le temps d'exec, utilisé pour le stockage de donnée pour la création de graphiques, a supprimer lors de vrai tests
	auto start = std::chrono::system_clock::now();
	auto lastWritten = std::chrono::system_clock::now();

	// Parametre pour le recuit simulé
	double coeff = 1.0;
	// Decallage coeff descendant, on soustrait cette valeur
	double coeffDesc = 0.1;
	// Decallage coeff montant, on ajoute cette valeur
	double coeffMont = 0.5;
	// Max et Min du coeff
	double coeffMax = 5;
	double coeffMin = 0.1;
	// Indique si on est sur la vague montante du recuit simulé
	bool recuitMontant = false;
	// Nombre d'execution du recuit simule
	int nbTour = 0;
	// Nombre d'execution requise pour modifier le coeff
	int nbTourModifCoeff = 100;

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
			std::vector<std::pair<int, std::pair<int, int>>> vectorProba = rouletteRusseNodeMove(vectorNodeBends[selectedNodeBendNum], GL, CCE, sommeLong, sommeLong2, variance,gridHeight,gridWidth);
			show_move_variance = false;
		}
		else if (show_variance) {
			double tmpSommeLong = 0, tmpSommeLong2 = 0, tmpVariance = 0;
			prepCalcNVar(tmpSommeLong, tmpSommeLong2, tmpVariance);
			std::cout << "sommeLong: " << tmpSommeLong << std::endl;
			std::cout << "sommeLong sommeLong: " << tmpSommeLong2 << std::endl;
			std::cout << "Variance: " << tmpVariance << std::endl;
			std::cout << "Best Variance: " << bestVariance << std::endl;
			show_variance = false;
		}
		else if (moveRouletteRusse) {
			selectedNodeBendNum = startRouletteRusse(GL, CCE, sommeLong, sommeLong2, variance, gridHeight, gridWidth);
			moveRouletteRusse = false;
		}
		else if (autoRouletteRusse) {
			selectedNodeBendNum = startRouletteRusse(GL, CCE, sommeLong, sommeLong2, variance, gridHeight, gridWidth);
			if (variance < bestVariance) {
				bestVariance = variance;
				writeToJson("bestResult.json", G, GL, gridWidth, gridHeight, maxBends);
			}
			checkTime(start, lastWritten, 10, variance);
		}
		else if (moveRecuitSimule) {
			selectedNodeBendNum = startRecuitSimule(coeff, GL, CCE, sommeLong, sommeLong2, variance, gridHeight, gridWidth);
			modifCoeffRecuit(coeff,coeffDesc,coeffMont,coeffMax,coeffMin,recuitMontant,nbTour,nbTourModifCoeff);
			moveRecuitSimule = false;
		}
		else if (autoRecuitSimule) {
			selectedNodeBendNum = startRecuitSimule(coeff, GL, CCE, sommeLong, sommeLong2, variance, gridHeight, gridWidth);
			modifCoeffRecuit(coeff, coeffDesc, coeffMont, coeffMax, coeffMin, recuitMontant, nbTour, nbTourModifCoeff);
			//std::cout << "Coeff: " << coeff << " Tour: " << nbTour << " Phase: " << recuitMontant << std::endl;
			if (variance < bestVariance) {
				bestVariance = variance;
				writeToJson("bestResult.json", G, GL, gridWidth, gridHeight, maxBends);
			}
			checkTime(start, lastWritten,10, variance);
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