/*
===============================================================================
FICHEIRO: tp_utils.h

DESCRIÇÃO:
Este ficheiro declara a interface pública das funções utilitárias do projeto.

OBJETIVO:
Disponibilizar de forma organizada os protótipos das funções auxiliares usadas
por vários módulos.

O QUE DEVE INCLUIR:
- Protótipos das funções implementadas em tp_utils.c.
- Estruturas auxiliares pequenas, se necessário.
- Constantes utilitárias de uso transversal.

O QUE NÃO DEVE INCLUIR:
- Implementação de funções.
- Código específico da lógica principal do trabalho.

PORQUE EXISTE:
Este ficheiro existe para permitir o acesso organizado às funções auxiliares sem
misturar a sua implementação com os módulos principais do projeto.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Está associado a tp_utils.c.
- Pode ser incluído em qualquer módulo que necessite dessas funções.

NOTAS:
As funções declaradas aqui devem manter um caráter genérico e transversal.
===============================================================================
*/

#ifndef TP_UTILS_H
#define TP_UTILS_H

#include <stdio.h>

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             ESTRUTURAS DE DADOS
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Estrutura base de imagem
typedef struct {
    unsigned char *data;
    int width;
    int height;
    int channels;
    int levels;
    int bytesperline;
} IVC;

// Estrutura de blobs (Objetos)
typedef struct {
    int x, y, width, height;    // Caixa Delimitadora (Bounding Box)
    int area;                   // Área
    int xc, yc;                 // Centro-de-massa
    int perimeter;              // Perímetro
    int label;                  // Etiqueta
} OVC;

//Protótipos
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);
char *netpbm_get_token(FILE *file, char *tok, int len);
long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height);
void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height);
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

//Funções de Validação
void valImages(IVC *img1, IVC *img2);

//Conversão de Pixeis para Milimetros
int convertPixToMM(float pixelValue, float mmValue);

#endif
