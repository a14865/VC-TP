/*
===============================================================================
FICHEIRO: tp_classification.c

DESCRICAO:
Este ficheiro contem a implementacao das regras de classificacao dos objetos
detetados, com base nas medidas obtidas anteriormente.

OBJETIVO:
Atribuir a cada laranja uma categoria ou classe, por exemplo calibre comercial,
com base nos criterios definidos no enunciado e/ou no regulamento de referencia.
===============================================================================
*/

#include "../include/tp_classification.h"

// Calibre segundo a escala de laranjas do Regulamento CEE-379-71.
int orangeCaliber(int mmWidth)
{
  if (mmWidth < 53) return -1;
  if (mmWidth >= 100) return 0;
  if (mmWidth >= 87) return 1;
  if (mmWidth >= 84) return 2;
  if (mmWidth >= 81) return 3;
  if (mmWidth >= 77) return 4;
  if (mmWidth >= 73) return 5;
  if (mmWidth >= 70) return 6;
  if (mmWidth >= 67) return 7;
  if (mmWidth >= 64) return 8;
  if (mmWidth >= 62) return 9;
  if (mmWidth >= 60) return 10;
  if (mmWidth >= 58) return 11;
  if (mmWidth >= 56) return 12;
  return 13;
}
