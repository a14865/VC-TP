/*
===============================================================================
FICHEIRO: tp_measurements.h

DESCRICAO:
Interface publica do modulo de medicoes.

OBJETIVO:
Declarar as funcoes que etiquetam blobs e extraem medidas dos objetos.
===============================================================================
*/
#ifndef TP_MEASUREMENTS_H
#define TP_MEASUREMENTS_H

#include "../include/tp_utils.h"

/**
 * @brief Etiqueta componentes conexas numa imagem binaria.
 *
 * @param src Mascara binaria de entrada.
 * @param dst Imagem de etiquetas gerada.
 * @param nlabels Ponteiro onde sera guardado o numero de blobs encontrados.
 * @return OVC* Vetor de blobs alocado dinamicamente, ou NULL se nao houver blobs.
 */
OVC *vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);

/**
 * @brief Calcula area, perimetro, bounding box e centro de massa dos blobs.
 *
 * @param src Imagem de etiquetas.
 * @param blobs Vetor de blobs a preencher.
 * @param nblobs Numero de blobs no vetor.
 * @return int 1 se as medidas forem calculadas, 0 em caso de erro.
 */
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);

#endif
