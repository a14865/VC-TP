/*
===============================================================================
FICHEIRO: tp_visualization.h

DESCRICAO:
Interface publica do modulo de visualizacao.

OBJETIVO:
Declarar as funcoes que desenham informacao visual sobre imagens IVC.
===============================================================================
*/
#ifndef TP_VISUALIZATION_H
#define TP_VISUALIZATION_H

#include "../include/tp_utils.h"

/**
 * @brief Desenha bounding boxes sobre uma imagem RGB.
 *
 * @param srcdst Imagem onde as caixas serao desenhadas.
 * @param blobs Vetor de blobs a desenhar.
 * @param nlabels Numero de blobs no vetor.
 * @param padding Margem adicional em pixeis.
 * @param thickness Espessura da caixa.
 * @param colorR Componente vermelha da cor.
 * @param colorG Componente verde da cor.
 * @param colorB Componente azul da cor.
 * @return int 1 se o desenho for executado.
 */
int vc_draw_bounding_box_all_blobs(IVC* srcdst, OVC* blobs, int nlabels, int padding, int thickness, int colorR, int colorG, int colorB);

/**
 * @brief Desenha o centro de massa dos blobs sobre uma imagem RGB.
 *
 * @param srcdst Imagem onde os centros serao desenhados.
 * @param blobs Vetor de blobs a desenhar.
 * @param nlabels Numero de blobs no vetor.
 * @param kernel Dimensao da marca desenhada.
 * @param thickness Espessura da marca.
 * @param colorR Componente vermelha da cor.
 * @param colorG Componente verde da cor.
 * @param colorB Componente azul da cor.
 * @return int 1 se o desenho for executado.
 */
int vc_draw_center_mass_all_blobs(IVC* srcdst, OVC* blobs, int nlabels, int kernel, int thickness, int colorR, int colorG, int colorB);

#endif
