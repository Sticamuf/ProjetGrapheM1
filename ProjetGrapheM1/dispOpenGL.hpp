﻿#ifndef DISPOPENGL_H
#define DISPOPENGL_H

#include <Windows.h>
#include <gl/GL.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/Layout.h>

using namespace ogdf;

bool move_randomly = false;

void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

// Callback pour OpenGL
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS)
        switch (key) {
            // Fermer l'application
        case  GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
            // Action deplacer un noeud al�atoirement?
            // TOUCHE Q SUR UN CLAVIER AZERTY!
        case  GLFW_KEY_A:
            move_randomly = !move_randomly;
            break;
        }
}

void dispOpenGL(const Graph& G, /*const*/ GraphAttributes& GA, const int gridWidth, const int gridHeight)
{
    //debut ogdf
    Layout l = Layout{ G };
    node n = G.firstNode();
    while (n != nullptr) {
        l.x(n) = (GA.x(n) / gridWidth * 1.9) - 0.95;
        l.y(n) = (GA.y(n) / gridHeight * 1.9) - 0.95;
        n = n->succ();
    }

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
        glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // Deplacer un noeud aléatoirement
        if (move_randomly) {
            node n = G.firstNode();
            GA.x(n) += 0.1;
            l.x(n) = (GA.x(n) / gridWidth * 1.9) - 0.95;
            SListPure<edge> edges;
            n->adjEdges(edges);
            for (SListConstIterator<edge> i = edges.begin(); i.valid(); i++) {
                edge e = (*i);
                std::cout << "New length: " << calcEdgeLength(e, GA) << std::endl;
            }
            move_randomly = false;
        }
        //afficher les edge
        glColor3f(1.0f, 1.0f, 1.0f);
        for (auto e : G.edges)
        {
            glBegin(GL_LINE_STRIP);
            glVertex2d(l.x(e->source()), l.y(e->source()));
            DPolyline& bends = GA.bends(e);
            for (ListIterator<DPoint> i = bends.begin(); i.valid(); i++) {
                tempX = ((*i).m_x / (double)gridWidth * 1.9) - 0.95;
                tempY = ((*i).m_y / (double)gridHeight * 1.9) - 0.95;
                glVertex2d(tempX, tempY);
            }
            glVertex2d(l.x(e->target()), l.y(e->target()));
            glEnd();
        }
        //afficher les nodes
        glColor3f(1.0f, 0.0f, 0.0f);
        glPointSize(5);
        n = G.firstNode();
        glBegin(GL_POINTS);
        while (n != nullptr) {
            glVertex2d(l.x(n), l.y(n));
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