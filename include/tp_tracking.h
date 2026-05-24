/*
===============================================================================
FICHEIRO: tp_tracking.h

DESCRICAO:
Interface publica do modulo de tracking.

OBJETIVO:
Declarar a funcao que associa laranjas entre frames e controla a contagem.
===============================================================================
*/
#ifndef TP_TRACKING_H
#define TP_TRACKING_H

#include "../include/tp_utils.h"

/**
 * @brief Associa uma laranja atual a uma laranja da frame anterior.
 *
 * @param orange Blob da laranja na frame atual.
 * @param previousOranges Vetor de laranjas da frame anterior.
 * @param previousCount Numero de laranjas na frame anterior.
 * @param activationLine Linha usada para contar novas laranjas.
 * @param maxDistance Distancia maxima entre centroides para aceitar associacao.
 * @param totalOranges Contador acumulado de laranjas.
 * @param orangeWasCounted Recebe 1 quando a laranja e contada nesta frame.
 * @return int Indice da laranja associada na frame anterior, ou -1 se nao houver associacao.
 */
int trackOrange(OVC *orange, OVC *previousOranges, int previousCount, int activationLine, double maxDistance, int *totalOranges, int *orangeWasCounted);

#endif
