/*
===============================================================================
FICHEIRO: tp_utils.h

DESCRICAO:
Interface publica das estruturas e funcoes utilitarias.

OBJETIVO:
Disponibilizar tipos base, leitura/escrita de imagens e conversoes auxiliares.
===============================================================================
*/
#ifndef TP_UTILS_H
#define TP_UTILS_H

#include <stdio.h>

/**
 * @brief Estrutura base para representar uma imagem no projeto.
 */
typedef struct {
    unsigned char *data; /**< Dados da imagem em memoria. */
    int width;           /**< Largura da imagem em pixeis. */
    int height;          /**< Altura da imagem em pixeis. */
    int channels;        /**< Numero de canais da imagem. */
    int levels;          /**< Nivel maximo de intensidade. */
    int bytesperline;    /**< Numero de bytes por linha. */
} IVC;

/**
 * @brief Estrutura com as propriedades de um blob detetado.
 */
typedef struct {
    int x, y, width, height; /**< Bounding box do objeto. */
    int area;                /**< Area em pixeis. */
    int xc, yc;              /**< Centro de massa. */
    int perimeter;           /**< Perimetro em pixeis. */
    int label;               /**< Etiqueta do objeto. */
} OVC;

/**
 * @brief Aloca uma nova imagem IVC.
 *
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 * @param channels Numero de canais.
 * @param levels Nivel maximo de intensidade.
 * @return IVC* Imagem alocada, ou NULL em caso de erro.
 */
IVC *vc_image_new(int width, int height, int channels, int levels);

/**
 * @brief Liberta uma imagem IVC previamente alocada.
 *
 * @param image Imagem a libertar.
 * @return IVC* NULL apos libertar a imagem.
 */
IVC *vc_image_free(IVC *image);

/**
 * @brief Le o proximo token de um ficheiro NetPBM.
 *
 * @param file Ficheiro aberto.
 * @param tok Buffer onde o token sera guardado.
 * @param len Tamanho do buffer.
 * @return char* Ponteiro para o buffer do token.
 */
char *netpbm_get_token(FILE *file, char *tok, int len);

/**
 * @brief Converte dados binarios de imagem para formato bit a bit.
 *
 * @param datauchar Dados em unsigned char.
 * @param databit Buffer de destino em bits.
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 * @return long int Numero total de bytes gerados.
 */
long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height);

/**
 * @brief Converte dados bit a bit para unsigned char.
 *
 * @param databit Dados compactados em bits.
 * @param datauchar Buffer de destino.
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 */
void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height);

/**
 * @brief Le uma imagem PBM, PGM ou PPM.
 *
 * @param filename Caminho do ficheiro.
 * @return IVC* Imagem lida, ou NULL em caso de erro.
 */
IVC *vc_read_image(char *filename);

/**
 * @brief Escreve uma imagem PBM, PGM ou PPM.
 *
 * @param filename Caminho do ficheiro de destino.
 * @param image Imagem a escrever.
 * @return int 1 se escrever com sucesso, 0 em caso de erro.
 */
int vc_write_image(char *filename, IVC *image);

/**
 * @brief Valida se duas imagens existem.
 *
 * @param img1 Primeira imagem.
 * @param img2 Segunda imagem.
 */
void valImages(IVC *img1, IVC *img2);

/**
 * @brief Converte um valor em pixeis para milimetros.
 *
 * @param pixelValue Valor medido em pixeis.
 * @return int Valor convertido e arredondado em milimetros.
 */
int convertPixToMM(float pixelValue);

/**
 * @brief Converte uma area em pixeis quadrados para milimetros quadrados.
 *
 * @param pixelArea Area medida em pixeis quadrados.
 * @return int Area convertida e arredondada em mm2.
 */
int convertPixAreaToMM2(float pixelArea);

#endif
