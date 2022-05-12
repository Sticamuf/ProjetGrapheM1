﻿#ifndef DISPOPENGL_HPP
#define DISPOPENGL_HPP

#include <chrono>
#include <thread>

#include <Windows.h>
#undef max
#include <gl/GL.h>

//#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/Layout.h>
#include "edgeMap.hpp"

using namespace ogdf;

bool move_randomly = false;
bool show_grid_size = true;

double calculEdgeLengthRatio() {
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
		case  GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
			// Action deplacer un noeud aleatoirement?
			// TOUCHE Q SUR UN CLAVIER AZERTY!
		case  GLFW_KEY_A:
			move_randomly = !move_randomly;
			break;
		case GLFW_KEY_R:
			show_grid_size = !show_grid_size;
			break;
		}
}

void changeEdgeMapValue(edge e, GraphAttributes& GA) {
	auto it = mapEdgeLength.find(e);
	double length = it->second;
	double newLength = calcEdgeLength(e, GA);
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

void dispOpenGL(const Graph& G, GraphAttributes& GA, const int gridWidth, const int gridHeight, int maxX, int maxY)
{
	//debut ogdf
	//Layout l = Layout{ G };
	node n = G.firstNode();
	//while (n != nullptr) {
	//	l.x(n) = (GA.x(n) / gridWidth * 1.9) - 0.95;
	//	l.y(n) = (GA.y(n) / gridHeight * 1.9) - 0.95;
	//	n = n->succ();
	//}

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
	double tempX, tempY;
	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		//glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		if (show_grid_size) {
			glOrtho(-1, static_cast<float>(gridWidth) + 1, -1, static_cast<float>(gridHeight) + 1, 1.f, -1.f);
		}
		else {
			glOrtho(-1, static_cast<float>(maxX) + 1, -1, static_cast<float>(maxY) + 1, 1.f, -1.f);
		}

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// Deplacer un noeud aléatoirement
		//if (move_randomly) {
		//	node n = G.firstNode();
		//	GA.x(n) += 0.1;
		//	l.x(n) = (GA.x(n) / gridWidth * 1.9) - 0.95;
		//	std::cout << "Layout: " << l.x(n) << std::endl;
		//	std::cout << "GA: " << GA.x(n) << std::endl;
		//	SListPure<edge> edges;
		//	n->adjEdges(edges);
		//	for (SListConstIterator<edge> i = edges.begin(); i.valid(); i++) {
		//		edge e = (*i);
		//		changeEdgeMapValue(e, GA);
		//	}
		//	move_randomly = false;
		//}
		//afficher les edge
		glColor3f(1.0f, 1.0f, 1.0f);
		for (auto e : G.edges)
		{
			glBegin(GL_LINE_STRIP);
			glVertex2d(GA.x(e->source()), GA.y(e->source()));
			DPolyline& bends = GA.bends(e);
			for (ListIterator<DPoint> i = bends.begin(); i.valid(); i++) {
				//tempX = ((*i).m_x / (double)gridWidth * 1.9) - 0.95;
				//tempY = ((*i).m_y / (double)gridHeight * 1.9) - 0.95;
				//glVertex2d(tempX, tempY);
				glVertex2d((*i).m_x, (*i).m_y);
			}
			glVertex2d(GA.x(e->target()), GA.y(e->target()));
			glEnd();
		}
		//afficher les nodes
		glColor3f(1.0f, 0.0f, 0.0f);
		glPointSize(5);
		n = G.firstNode();
		glBegin(GL_POINTS);
		while (n != nullptr) {
			glVertex2d(GA.x(n), GA.y(n));
			n = n->succ();
		}
		glEnd();
		glfwSwapBuffers(window);
		glfwPollEvents();
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}
	glfwDestroyWindow(window);
	glfwTerminate();
}

#endif