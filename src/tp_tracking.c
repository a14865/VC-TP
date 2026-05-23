/*
===============================================================================
FICHEIRO: tp_tracking.c

DESCRIÇÃO:
Este ficheiro contém a implementação da lógica de acompanhamento dos objetos ao
longo de múltiplas frames do vídeo.

OBJETIVO:
Evitar que a mesma laranja seja contada várias vezes em frames consecutivas,
mantendo uma associação entre deteções atuais e deteções anteriores.

O QUE DEVE INCLUIR:
- Estratégias para identificar objetos ao longo do tempo.
- Associação entre blobs de frames consecutivas.
- Atualização da contagem acumulada.
- Critérios para considerar que um objeto é novo, repetido ou desapareceu.
- Funções auxiliares de gestão de estado temporal.

O QUE NÃO DEVE INCLUIR:
- Segmentação detalhada da imagem.
- Operações básicas de processamento de imagem.
- Regras de classificação de calibre.
- Código principal de interface ou visualização.

PORQUE EXISTE:
O enunciado exige contagem acumulada desde a primeira frame. Para isso não
basta detetar objetos em cada frame isoladamente; é necessário acompanhar a sua
evolução no tempo. Este módulo existe para tratar exclusivamente essa dimensão
temporal do problema.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Usa resultados da segmentação e/ou medições.
- Pode usar classificações previamente calculadas.
- É chamado pelo fluxo principal em main.cpp.

NOTAS:
Este módulo responde à pergunta:
"Este objeto já foi contado antes ou é uma nova laranja?"
===============================================================================
*/

#include <math.h>
#include "../include/tp_tracking.h"

int trackOrange(OVC *orange, OVC *previousOranges, int previousCount, int activationLine, double maxDistance, int *totalOranges)
{
    int matchIndex = -1;
    double minDistance = 1000000.0;

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
        }
        else if (previousOranges[matchIndex].yc >= activationLine)
        {
            orange->area = previousOranges[matchIndex].area;
            orange->perimeter = previousOranges[matchIndex].perimeter;
        }
    }

    return matchIndex;
}
