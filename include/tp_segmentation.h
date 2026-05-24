/*
===============================================================================
FICHEIRO: tp_segmentation.h

DESCRICAO:
Interface publica do modulo de segmentacao.

OBJETIVO:
Declarar as funcoes usadas para converter cor e criar mascaras binarias.
===============================================================================
*/
#ifndef TP_SEGMENTATION_H
#define TP_SEGMENTATION_H

#include "../include/tp_utils.h"

/**
 * @brief Converte uma imagem RGB para HSV.
 *
 * @param src Imagem de entrada em RGB.
 * @param dst Imagem de saida em HSV.
 * @return int 1 se a conversao for executada.
 */
int vc_bgr_to_hsv(IVC *src, IVC *dst);

/**
 * @brief Segmenta uma imagem HSV usando intervalos de matiz, saturacao e valor.
 *
 * @param src Imagem HSV de entrada.
 * @param dst Mascara binaria de saida.
 * @param hmin Valor minimo de hue em graus.
 * @param hmax Valor maximo de hue em graus.
 * @param smin Saturacao minima em percentagem.
 * @param smax Saturacao maxima em percentagem.
 * @param vmin Valor minimo em percentagem.
 * @param vmax Valor maximo em percentagem.
 * @return int 1 se a segmentacao for executada.
 */
int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);

/**
 * @brief Ajusta os canais de saturacao e valor de uma imagem HSV.
 *
 * @param src Imagem HSV de entrada.
 * @param dst Imagem HSV ajustada.
 * @param satMod Fator multiplicativo para a saturacao.
 * @param valMod Fator multiplicativo para o valor.
 * @return int 1 se o ajuste for executado.
 */
int vc_hsv_saturation_and_value_modified(IVC* srcdst, float satMod, float valMod);

#endif
