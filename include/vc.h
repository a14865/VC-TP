/*
===============================================================================
FICHEIRO: vc.h

DESCRIÇÃO:
Este ficheiro define as estruturas de dados, tipos e protótipos da biblioteca
base de processamento de imagem do projeto.

OBJETIVO:
Declarar de forma clara e centralizada a interface pública da biblioteca base,
permitindo que os restantes módulos utilizem as funções implementadas em vc.c.

O QUE DEVE INCLUIR:
- Definição da estrutura IVC.
- Definição de estruturas auxiliares, se necessário.
- Protótipos de funções implementadas em vc.c.
- Constantes e macros relevantes para a biblioteca base.

O QUE NÃO DEVE INCLUIR:
- Implementações de funções.
- Código específico do main.
- Regras específicas de classificação das laranjas.
- Código de interface com OpenCV que não seja estritamente necessário.

PORQUE EXISTE:
Este ficheiro existe para separar a interface da implementação. Isso melhora a
organização, facilita a compilação e torna mais clara a utilização da biblioteca
por outros módulos do projeto.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- É incluído no main e nos restantes módulos que necessitem de aceder à
  estrutura IVC e às funções base da biblioteca.
- Está diretamente associado ao ficheiro vc.c.

NOTAS:
Qualquer nova função genérica adicionada a vc.c deve ter o respetivo protótipo
declarado aqui. Este ficheiro deve ser mantido limpo e coerente.
===============================================================================
*/

#ifndef _VC_H_
#define _VC_H_

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


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             PROTÓTIPOS DAS FUNÇÕES
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS
char *netpbm_get_token(FILE *file, char *tok, int len);
long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height);
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

// FUNÇÕES: CONVERSÃO E MANIPULAÇÃO DE ESPAÇOS DE COR
int vc_gray_negative(IVC *srcdst);
int vc_rgb_negative(IVC *srcdst);
int vc_rgb_get_red_gray(IVC *srcdst);
int vc_rgb_get_green_gray(IVC *srcdst);
int vc_rgb_get_blue_gray(IVC *srcdst);
int vc_rgb_get_red_gray2(IVC *src, IVC *dst);
int vc_rgb_get_green_gray2(IVC *src, IVC *dst);
int vc_rgb_get_blue_gray2(IVC *src, IVC *dst);

int vc_rgb_to_gray_ricardo(IVC *src, IVC *dst);
int vc_color_to_gray_raul(IVC *src, IVC *dst, char color);
int vc_rgb_to_gray_ze(IVC *src, IVC *dst);
int vc_rgb_to_gray_ze2(char color);
int vc_ppm_to_pgm_raul(IVC *src, IVC *dst);

int vc_rgb_to_hsv_ricardo(IVC *src, IVC *dst);
int vc_rgb_to_hsv_raul(IVC *src, IVC *dst);
int vc_rbg_to_hsv_ze(IVC *src, IVC *dst);
int vc_rgb_to_hsv_ze_2(IVC *src, IVC *dst);

int vc_hsv_segmentation_ricardo(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_hsv_segmentation_raul(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_hsv_segmentation_ze(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_hsv_segmentation_diogo(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);

int vc_scale_gray_to_rgb_ricardo(IVC *src, IVC *dst);
int vc_scale_gray_to_rgb_ricardo_2(IVC *src, IVC *dst);

// FUNÇÕES: TÉCNICAS DE SEGMENTAÇÃO DE IMAGEM
int vc_gray_to_binary_raul(IVC *src, IVC *dst, int threshold);
int vc_gray_to_binary_ricardo(IVC *src, IVC *dst, int threshold);
int vc_gray_to_binary_ze(IVC *src, IVC *dst, int threshold);
int vc_gray_to_binary_prof(IVC *src, IVC *dst, int threshold);

int vc_gray_to_binary_global_mean_ricardo(IVC *src, IVC *dst);
int vc_gray_to_binary_global_mean_raul(IVC *src, IVC *dst);
int vc_gray_to_binary_global_mean_ze(IVC* src, IVC *dst);
int vc_gray_to_binary_global_mean_prof(IVC *src, IVC *dst);
int vc_gray_to_binary_global_mean_and_manual_threshold_ze(IVC *src, IVC *dst, int threshold, int minThreshold, int maxThreshold);
int vc_gray_to_binary_range_raul(IVC *src, IVC *dst, int invert, int thresholdMin, int thresholdMax);
int vc_gray_to_binary_challenge_ricardo(IVC *src, IVC *dst, int t1, int t2, int mode);

int vc_gray_to_binary_midpoint_ricardo(IVC *src, IVC *dst, int kernel);
int vc_gray_to_binary_midpoint_raul(IVC *src, IVC *dst, int kernel);
int vc_gray_to_binary_midpoint_ze(IVC *src, IVC *dst, int kernel);

// FUNÇÕES: OPERADORES MORFOLÓGICOS
int vc_binary_dilate_ricardo(IVC *src, IVC *dst, int kernel);
int vc_binary_dilate_raul(IVC *src, IVC *dst, int kernel);
int vc_binary_dilate_ze(IVC *src, IVC *dst, int kernel);
int vc_binary_erode_ricardo(IVC *src, IVC *dst, int kernel);
int vc_binary_erode_raul(IVC *src, IVC *dst, int kernel);
int vc_binary_erode_ze(IVC *src, IVC *dst, int kernel);
int vc_image_open_ricardo(IVC *src, IVC *dst, int kernelErode, int kernelDilate);
int vc_image_close_ricardo(IVC *src, IVC *dst, int kernelDilate, int kernelErode);

// FUNÇÕES: DETEÇÃO DE BLOBS E ETIQUETAGEM DE COMPONENTES
OVC* vc_binary_blob_labelling_ricardo(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_labelling2_ricardo(IVC *src, IVC *dst);
OVC* vc_binary_blob_labelling_raul(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_labelling_ze(IVC *src, IVC *dst);
OVC* vc_binary_blob_labelling2_ze(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info_ricardo(IVC *src, OVC *blobs, int nblobs);
int vc_binary_blob_info_raul(IVC *src, OVC *blobs, int nblobs);
int vc_binary_blob_info_ze(IVC *src, OVC *blobs, int nblobs);

// FUNÇÕES: ANÁLISE DE HISTOGRAMAS DE IMAGEM
int vc_gray_histogram_show_ricardo(IVC *src, IVC *dst);
int vc_gray_histogram_show_ze(IVC * src, IVC *dst);
int vc_gray_histogram_equalization_ricardo(IVC *src, IVC *dst);

// FUNÇÕES: DETEÇÃO DE CONTORNOS
int vc_gray_edge_prewitt_ricardo(IVC *src, IVC *dst, float th);
int vc_gray_edge_prewitt_prof(IVC *src, IVC *dst, float th);

// FUNÇÕES: FILTROS (PASSA-BAIXO / GAUSSIANO / MEDIANA)
int vc_gray_lowpass_mean_filter_ricardo(IVC *src, IVC *dst, int kernelsize);
int vc_gray_lowpass_mean_filter_raul(IVC *src, IVC *dst, int kernelsize);
int vc_gray_lowpass_mean_filter_ze(IVC *src, IVC *dst, int kernelsize);
int vc_gray_lowpass_median_filter_ricardo(IVC *src, IVC *dst, int kernelsize);
int vc_gray_lowpass_median_filter_raul(IVC *src, IVC *dst, int kernelsize);
int vc_gray_lowpass_median_filter_ze(IVC *src, IVC *dst, int kernelsize);
int vc_gray_lowpass_gaussian_filter_ricardo(IVC *src, IVC *dst);
int vc_gray_lowpass_gaussian_filter_raul(IVC *src, IVC *dst);
int vc_gray_lowpass_gaussian_ze(IVC *src, IVC *dst);

// FUNÇÕES: EXTRAS
int segmentar_cerebro_preto(IVC *src, IVC *dst, int *total);
int analisar_pet(IVC *src, IVC *seg, int total_cerebro);
int vc_binary_difference(IVC *src1, IVC *src2, IVC *dst);
int vc_erode_minus_dilate(IVC* srcEroded, IVC* srcDilated, IVC* dst);
int vc_apply_mask(IVC* src, IVC* mask, IVC* dst);
int vc_binary_dilate_gray(IVC *src, IVC *dst, int kernel);

// FUNÇÕES COMPLEMENTARES (Chamadas internamente)
int vc_rgb_to_hsv(IVC *src, IVC *dst);
int vc_binary_erode(IVC *src, IVC *dst, int kernel);
int vc_binary_dilate(IVC *src, IVC *dst, int kernel);

int vc_hsv_saturation_and_value_modified(IVC* src, IVC* dst, float satMod, float valMod);

// FUN��ES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUN��ES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);
int vc_gray_negative(IVC *srcdst);
int vc_rgb_negative(IVC *srcdst);
int vc_rgb_get_red_gray(IVC *srcdst);
int vc_rgb_get_green_gray(IVC *srcdst);
int vc_rgb_get_blue_gray(IVC *srcdst);
int vc_rgb_to_gray(IVC *src, IVC *dst);
int vc_rgb_to_hsv(IVC *src, IVC *dst);
int vc_rgb_to_hsv_2(IVC *src, IVC *dst);
int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_scale_gray_to_rgb(IVC *src, IVC *dst);
int vc_number_total_black_pixels(IVC *src, int *p);
int vc_number_total_white_pixels(IVC *src, int *p);
int vc_verify_images(IVC* src1, IVC* src2, IVC* dst);
int vc_gray_to_binary(IVC *srcdst, int threshold);
int vc_gray_to_binary_global_mean(IVC *srcdst);
int vc_gray_to_binary_2thresholds(IVC *srcdst, int thresholdMin, int thresholdMax, int judge);
int vc_gray_to_binary_midpoint(IVC *src, IVC *dst, int kernel);
int vc_gray_to_binary_bernsen(IVC *src, IVC *dst, int kernel);
int vc_gray_to_binary_niblack(IVC* src, IVC* dst, int kernel, float k);
int vc_image_dilate(IVC *src, IVC *dst, int kernel);
int vc_image_erode(IVC *src, IVC *dst, int kernel);
int vc_image_open(IVC *src, IVC *dst, int kernelErode, int kernelDilate);
int vc_image_close(IVC *src, IVC *dst, int kernelDilate, int kernelErode);
int vc_erode_minus_dilate(IVC* srcEroded, IVC* srcDilated, IVC* dst);
int vc_compare_after_morpho(IVC* srcOriginal, IVC* srcMorpho, IVC* dst);
int vc_binary_blob_labelling_mine(IVC *src, IVC *dst, int kernel, int typeOfKernel);
OVC* vc_binary_blob_labelling_prof(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);
OVC *vc_binary_blob_labelling_buffer(IVC *src, IVC *dst, int *nlabels);
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_draw_center_mass_all_blobs(IVC* srcdst, OVC* blobs, int nlabels, int kernel, int thickness, int colorR, int colorG, int colorB);
int vc_draw_bounding_box_all_blobs(IVC* srcdst, OVC* blobs, int nlabels, int padding, int thickness, int colorR, int colorG, int colorB);
int vc_gray_histogram_show(IVC *src, IVC *dst);
int vc_gray_histogram_equalization(IVC *src, IVC *dst);
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th);
int vc_gray_edge_prewitt_prof(IVC *src, IVC *dst, float th);
int vc_gray_lowpass_mean_filter(IVC *src, IVC *dst, int kernelsize);
int vc_gray_lowpass_median_filter(IVC *src, IVC *dst, int kernelsize);
int vc_gray_lowpass_gaussian_filter(IVC *src, IVC *dst);
int vc_gray_highpass_filter(IVC *src, IVC *dst);
int vc_gray_highpass_filter_enhance(IVC *src, IVC *dst, int gain);
int vc_hsv_saturation_and_value_modified(IVC* src, IVC* dst, float satMod, float valMod);
int vc_hsv_histogram_equalization(IVC *src, IVC *dst);

#endif // _VC_H_