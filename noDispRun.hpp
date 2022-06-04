#ifndef NODISPRUN_H
#define NODISPRUN_H
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
#include <chrono>
#include <cmath>
#include <fstream>

// Fait tourner l'algo sélectionné tant qu'il n'a pas une variance de 1 sans affichage openGL
// i==0 rouletteRusse i==1 recuit simulé
void runAlgo(int i, Graph& G, GridLayout& GL, const int gridWidth, const int gridHeight, int maxX, int maxY, int maxBends) {
	//debut ogdf
	node n = G.firstNode();
	ConstCombinatorialEmbedding CCE = ConstCombinatorialEmbedding{ G };
	double sommeLong = 0, sommeLong2 = 0, variance = 0;
	prepCalcNVar(sommeLong, sommeLong2, variance);
	double bestVariance = variance;

	// Chrono pour le temps d'exec, utilisé pour le stockage de donnée pour la création de graphiques, a supprimer lors de vrai executions
	auto start = std::chrono::system_clock::now();
	auto lastWritten = std::chrono::system_clock::now();
	// NB tour pour le stockage de donnée pour les graphiques, a supprimer lors de vrai executions
	unsigned long long totalTurn = 0;
	unsigned long long lastWrittenTurn = 0;

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

	int width, height;
	while (variance > 1.00005) {
		float ratio;
		if (i == 0) {
			startRouletteRusse(GL, CCE, sommeLong, sommeLong2, variance, gridHeight, gridWidth);
			if (variance < bestVariance) {
				bestVariance = variance;
				writeToJson("bestResult.json", G, GL, gridWidth, gridHeight, maxBends);
			}
			checkTime(start, lastWritten, 10, variance);
			checkTour(totalTurn, lastWrittenTurn,100,variance);
		}
		else if (i == 1) {
			startRecuitSimule(coeff, GL, CCE, sommeLong, sommeLong2, variance, gridHeight, gridWidth);
			modifCoeffRecuit(coeff, coeffDesc, coeffMont, coeffMax, coeffMin, recuitMontant, nbTour, nbTourModifCoeff);
			//std::cout << "Coeff: " << coeff << " Tour: " << nbTour << " Phase: " << recuitMontant << std::endl;
			if (variance < bestVariance) {
				bestVariance = variance;
				writeToJson("bestResult.json", G, GL, gridWidth, gridHeight, maxBends);
			}
			checkTime(start, lastWritten, 10, variance);
			checkTour(totalTurn, lastWrittenTurn, 100, variance);
		}
	}
}

#endif