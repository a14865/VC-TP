/*
===============================================================================
FICHEIRO: tp_tracking.c

DESCRICAO:
Este ficheiro contem a logica de acompanhamento das laranjas entre frames.

OBJETIVO:
Associar deteccoes consecutivas e evitar que a mesma laranja seja contada mais
do que uma vez.
===============================================================================
*/
#include <math.h>
#include "../include/tp_tracking.h"

/**
 * @brief Associa uma laranja atual a uma laranja da frame anterior.
 *
 * A associacao e feita pela menor distancia entre centroides. Quando a laranja
 * cruza a linha de ativacao pela primeira vez, incrementa o contador total.
 */
int trackOrange(OVC *orange, OVC *previousOranges, int previousCount, int activationLine, double maxDistance, int *totalOranges, int *orangeWasCounted)
{
    int matchIndex = -1;
    double minDistance = 1000000.0;

    if (orangeWasCounted != 0)
    {
        *orangeWasCounted = 0;
    }

    for (int i = 0; i < previousCount; i++)
    {
        double distanceX = (double)(orange->xc - previousOranges[i].xc);
        double distanceY = (double)(orange->yc - previousOranges[i].yc);
        double totalDistance = sqrt((distanceX * distanceX) + (distanceY * distanceY));

        if (totalDistance < maxDistance && totalDistance < minDistance)
        {
            minDistance = totalDistance;
            matchIndex = i;
        }
    }

    if (matchIndex != -1)
    {
        if (previousOranges[matchIndex].yc < activationLine && orange->yc >= activationLine)
        {
            (*totalOranges)++;

            if (orangeWasCounted != 0)
            {
                *orangeWasCounted = 1;
            }
        }
        else if (previousOranges[matchIndex].yc >= activationLine)
        {
            orange->area = previousOranges[matchIndex].area;
            orange->perimeter = previousOranges[matchIndex].perimeter;
        }
    }

    return matchIndex;
}
