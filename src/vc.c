/*
===============================================================================
FICHEIRO: vc.c

DESCRIÇÃO:
Este ficheiro contém a implementação da biblioteca base de processamento de
imagem usada no projeto. Aqui devem ficar as funções genéricas e reutilizáveis
sobre imagens, independentes da lógica específica do trabalho prático.

OBJETIVO:
Disponibilizar operações fundamentais sobre imagens, como leitura, escrita,
conversões, binarização, morfologia, labeling e análise de blobs, de forma
modular e reutilizável.

O QUE DEVE INCLUIR:
- Gestão de memória da estrutura IVC.
- Leitura e escrita de imagens.
- Funções de conversão de imagem.
- Operações de binarização.
- Operações morfológicas.
- Etiquetagem de componentes conectados.
- Funções auxiliares de análise de blobs.
- Funções genéricas que possam ser reutilizadas noutras fases do projeto.

O QUE NÃO DEVE INCLUIR:
- Regras específicas do trabalho das laranjas.
- Cálculo de calibre comercial.
- Lógica de tracking entre frames.
- Regras de negócio associadas ao enunciado.

PORQUE EXISTE:
Este ficheiro existe para isolar a base técnica de processamento de imagem da
aplicação concreta do TP. Desta forma, a biblioteca pode ser usada por vários
módulos do projeto sem duplicação de código.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- É utilizado por praticamente todos os módulos.
- Deve ser incluído através do cabeçalho vc.h.
- Serve de base ao processamento de segmentação e análise.

NOTAS:
Todas as funções implementadas neste ficheiro devem ser o mais genéricas,
robustas e independentes possível. Sempre que uma função puder ser reutilizada,
deve preferencialmente estar aqui e não num módulo específico do TP.
===============================================================================
*/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2025/2026
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//                    VISÃO POR COMPUTADOR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de fun��es n�o seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "../include/vc.h"
#include <math.h>
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar memória para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *) malloc(sizeof(IVC));

	if(image == NULL) return NULL;
	if((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

	if(image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}

// Libertar memória de uma imagem
IVC *vc_image_free(IVC *image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;
	
	for(;;)
	{
		while(isspace(c = getc(file)));
		if(c != '#') break;
		do c = getc(file);
		while((c != '\n') && (c != EOF));
		if(c == EOF) break;
	}
	
	t = tok;
	
	if(c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));
		
		if(c == '#') ungetc(c, file);
	}
	
	*t = 0;
	
	return tok;
}

long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);
				
				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}

IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;
	
	// Abre o ficheiro
	if((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
			#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
			#endif

			fclose(file);
			return NULL;
		}
		
		if(levels == 1) // PBM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			size = image->width * image->height * image->channels;

			if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}
		
		fclose(file);
	}
	else
	{
		#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
		#endif
	}
	
	return image;
}

int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;
	
	if(image == NULL) return 0;

	if((file = fopen(filename, "wb")) != NULL)
	{
		if(image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;
			
			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);
			
			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);
		
			if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				return 0;
			}
		}
		
		fclose(file);

		return 1;
	}
	
	return 0;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: CONVERSÃO E MANIPULAÇÃO DE ESPAÇOS DE COR
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Função que calcula o negativo de uma imagem Gray
int vc_gray_negative(IVC *srcdst){
	if(srcdst == NULL) return 0;
	if(srcdst->data == NULL) return 0;
	if(srcdst->channels != 1) return 0;

	int x, y;
	int pos;

	for(y=0; y<srcdst->height; y++)
	{
		for(x=0; x<srcdst->width; x++)
		{
			pos = srcdst->bytesperline * y + x * srcdst->channels;
			srcdst->data[pos] = 255 - srcdst->data[pos];
		}
	}

	return 1;
}


//Função que calcule o negativo de uma imagem RGB
int vc_rgb_negative(IVC *srcdst){
	if(srcdst == NULL) return 0;
	if(srcdst->data == NULL) return 0;
	if(srcdst->channels != 3) return 0;


	int x, y;
	int pos;
	for(y=0; y<srcdst->height; y++)
	{
		for(x=0; x<srcdst->width; x++)
		{
			pos = srcdst->bytesperline * y + x * srcdst->channels;
			srcdst->data[pos] = 255 - srcdst->data[pos];			// R
			srcdst->data[pos + 1] = 255 - srcdst->data[pos + 1];	// G
			srcdst->data[pos + 2] = 255 - srcdst->data[pos + 2];	// B
		}
	}

	return 1;
}


//Funções que extraiam as componentes R, G e B de uma imagem RGB, para imagens em tons de cinzento
int vc_rgb_get_red_gray(IVC *srcdst){
	if(srcdst == NULL) return 0;
	if(srcdst->data == NULL) return 0;
	if(srcdst->channels != 3) return 0;

	int x, y;
	int pos;

	for(y=0; y<srcdst->height; y++)
	{
		for(x=0; x<srcdst->width; x++)
		{
			pos = srcdst->bytesperline * y + x * srcdst->channels;
			srcdst->data[pos + 1] = srcdst->data[pos];	// G
			srcdst->data[pos + 2] = srcdst->data[pos];	// B
		}
	}

	return 1;
}

int vc_rgb_get_green_gray(IVC *srcdst){
	if(srcdst == NULL) return 0;
	if(srcdst->data == NULL) return 0;
	if(srcdst->channels != 3) return 0;

	int x, y;
	int pos;

	for(y=0; y<srcdst->height; y++)
	{
		for(x=0; x<srcdst->width; x++)
		{
			pos = srcdst->bytesperline * y + x * srcdst->channels;
			srcdst->data[pos] = srcdst->data[pos + 1];	// R
			srcdst->data[pos + 2] = srcdst->data[pos + 1];	// B
		}
	}

	return 1;
}

int vc_rgb_get_blue_gray(IVC *srcdst){
	if(srcdst == NULL) return 0;
	if(srcdst->data == NULL) return 0;
	if(srcdst->channels != 3) return 0;

	int x, y;
	int pos;

	for(y=0; y<srcdst->height; y++)
	{
		for(x=0; x<srcdst->width; x++)
		{
			pos = srcdst->bytesperline * y + x * srcdst->channels;
			srcdst->data[pos] = srcdst->data[pos + 2];	// R
			srcdst->data[pos + 1] = srcdst->data[pos + 2];	// G
		}
	}

	return 1;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//O de cima passando ppm para pbm
int vc_rgb_get_red_gray2(IVC *src, IVC *dst)
{
    int x, y;
    int pos_src, pos_dst;

    if (src == NULL || dst == NULL) return 0;
    if (src->data == NULL || dst->data == NULL) return 0;

    if (src->channels != 3) return 0;
    if (dst->channels != 1) return 0;

    if (src->width != dst->width || src->height != dst->height) return 0;

    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            pos_src = src->bytesperline * y + x * src->channels; // RGB
            pos_dst = dst->bytesperline * y + x * dst->channels; // Gray (1)

            dst->data[pos_dst] = src->data[pos_src]; // R
        }
    }

    return 1;
}

int vc_rgb_get_green_gray2(IVC *src, IVC *dst)
{
    int x, y;
    int pos_src, pos_dst;

    if (src == NULL || dst == NULL) return 0;
    if (src->data == NULL || dst->data == NULL) return 0;

    if (src->channels != 3) return 0;
    if (dst->channels != 1) return 0;

    if (src->width != dst->width || src->height != dst->height) return 0;

    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            pos_src = src->bytesperline * y + x * src->channels; // RGB
            pos_dst = dst->bytesperline * y + x * dst->channels; // Gray (1)

            dst->data[pos_dst] = src->data[pos_src + 1]; // G
        }
    }

    return 1;
}

int vc_rgb_get_blue_gray2(IVC *src, IVC *dst)
{
    int x, y;
    int pos_src, pos_dst;

    if (src == NULL || dst == NULL) return 0;
    if (src->data == NULL || dst->data == NULL) return 0;

    if (src->channels != 3) return 0;
    if (dst->channels != 1) return 0;

    if (src->width != dst->width || src->height != dst->height) return 0;

    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            pos_src = src->bytesperline * y + x * src->channels; // RGB
            pos_dst = dst->bytesperline * y + x * dst->channels; // Gray (1)

            dst->data[pos_dst] = src->data[pos_src + 2]; // B
        }
    }

    return 1;
}




//Função que converta uma imagem no espaço RGB para uma imagem em tons de cinzento
int vc_rgb_to_gray_ricardo(IVC *src, IVC *dst)
{
    int x, y;
    long int pos_src, pos_dst;
	float rf,gf,bf;

    if (src == NULL || dst == NULL) return 0;
    if (src->data == NULL || dst->data == NULL) return 0;

    if (src->channels != 3) return 0;
    if (dst->channels != 1) return 0;

    if (src->width != dst->width || src->height != dst->height) return 0;

    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            pos_src = src->bytesperline * y + x * src->channels; // RGB
            pos_dst = dst->bytesperline * y + x * dst->channels; // Gray (1)

            rf = (float) src->data[pos_src];
			gf = (float) src->data[pos_src + 1];
			bf = (float) src->data[pos_src + 2];

			dst->data[pos_dst] = (unsigned char) ((rf*0.299)+(gf*0.587)+(bf*0.114));
        }
    }

    return 1;
}

int vc_color_to_gray_raul(IVC *src, IVC *dst, char color) {
    int x, y;
    long int pos;

    for (x = 0; x < src->width; x++)
    {
        for (y = 0; y < src->height; y++)
        {
            pos = (y * src->bytesperline) + (x * src->channels);

            if(color == 'r'){
                dst->data[pos] = src->data[pos]; // Manter o canal R
            } else if(color == 'g'){
                dst->data[pos] = src->data[pos + 1]; // Manter o canal G
            } else if(color == 'b'){
                dst->data[pos] = src->data[pos + 2]; // Manter o canal B
            } else if(color == 'a'){
                int r = src->data[pos];
                int g = src->data[pos + 1];
                int b = src->data[pos + 2];
                int gray = (int)(0.299 * r + 0.587 * g + 0.114 * b);

                dst->data[pos] = gray;
            }

            dst->data[pos + 1] = dst->data[pos]; // Passar o valor do canal R para o canal G
            dst->data[pos + 2] = dst->data[pos]; // Passar o valor do canal R para o canal B
        }
    }

    return 1;
}


//Função de conversão de PPM para PGM
int vc_rgb_to_gray_ze(IVC *src, IVC *dst)
{

	int lenght = src->height * src->width * src->channels;

	for (int i = 0, j = 0; i < lenght; i += 3, j++)
	{

		dst->data[j] = src->data[i] * 0.299 + src->data[i + 1] * 0.587 + src->data[i + 2] * 0.114;
	}
}

int vc_rgb_to_gray_ze2(char color){
    int x, y;
    long int pos;

    IVC *image = vc_read_image("./images/Classic/baboon.ppm");

    if (image == NULL)
    {
        printf("ERROR -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    for (x = 0; x < image->width; x++)
    {
        for (y = 0; y < image->height; y++)
        {
            pos = (y * image->bytesperline) + (x * image->channels);

            if(color == 'r'){
                image->data[pos] = image->data[pos]; // Manter o canal R
            } else if(color == 'g'){
                image->data[pos] = image->data[pos + 1]; // Manter o canal G
            } else if(color == 'b'){
                image->data[pos] = image->data[pos + 2]; // Manter o canal B
            } else if(color == 'a'){
                int r = image->data[pos];
                int g = image->data[pos + 1];
                int b = image->data[pos + 2];
                int gray = (int)(0.299 * r + 0.587 * g + 0.114 * b);

                image->data[pos] = gray;
            }

            image->data[pos + 1] = image->data[pos]; // Passar o valor do canal R para o canal G
            image->data[pos + 2] = image->data[pos]; // Passar o valor do canal R para o canal B
        }
    }

    char filename[256];
    char path[512];
    sprintf(filename, "./images/generated/baboon_%c_gray.pgm", color);
    sprintf(path, "cmd /c start FilterGear  ./images/generated/baboon_%c_gray.pgm", color);
    vc_write_image(filename, image);

    system(path); // Show output image

    vc_image_free(image);

    return 0;
}

int vc_ppm_to_pgm_raul(IVC *src, IVC *dst) {
	int x, y;
    long int pos;

    for (x = 0; x < src->width; x++)
    {
        for (y = 0; y < src->height; y++)
        {
            pos = (y * src->bytesperline) + (x * src->channels);

            int r = src->data[pos];
			int g = src->data[pos + 1];
			int b = src->data[pos + 2];
			int gray = (int)(0.299 * r + 0.587 * g + 0.114 * b);

			dst->data[pos] = gray;

            dst->data[pos + 1] = dst->data[pos]; // Passar o valor do canal R para o canal G
            dst->data[pos + 2] = dst->data[pos]; // Passar o valor do canal R para o canal B
        }
    }

    return 1;
}

//Função que converta uma imagem no espaço RGB para uma imagem no espaço HSV
int vc_rgb_to_hsv_ricardo(IVC *src, IVC *dst)
{
    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
    if (src->channels != 3) return 0;

    unsigned char *datasrc = (unsigned char *)src->data;
    unsigned char *datadst = (unsigned char *)dst->data;

    int size = src->width * src->height * src->channels;

    for (int i = 0; i < size; i += src->channels)
    {
        float r = (float)datasrc[i];
        float g = (float)datasrc[i + 1];
        float b = (float)datasrc[i + 2];
        float hue, saturation, value;
        float rgb_max, rgb_min;

        rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
        rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

        value = rgb_max;

        if (value == 0.0)
        {
            hue = 0.0;
            saturation = 0.0;
        }
        else
        {
            saturation = ((rgb_max - rgb_min) / rgb_max) * 255.0f;

            if (saturation == 0.0)
            {
                hue = 0.0;
            }
            else
            {
                r /= 255.0f;
                g /= 255.0f;
                b /= 255.0f;

                rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
                rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

                if ((rgb_max == r) && (g >= b))
                    hue = 60 * (g - b) / (rgb_max - rgb_min);
                else if ((rgb_max == r) && (b > g))
                    hue = 360 + 60 * (g - b) / (rgb_max - rgb_min);
                else if (rgb_max == g)
                    hue = 120 + 60 * (b - r) / (rgb_max - rgb_min);
                else
                    hue = 240 + 60 * (r - g) / (rgb_max - rgb_min);
            }
        }

        datadst[i]     = (unsigned char)(hue / 360.0 * 255.0);
        datadst[i + 1] = (unsigned char)(saturation);
        datadst[i + 2] = (unsigned char)(value);
    }

    return 1;
}

int vc_rgb_to_hsv_raul(IVC *src, IVC *dst){

    for (int i = 0; i < src->bytesperline * src->height; i += src->channels)
    {
        int val = src->data[i];
        if(val < src->data[i + 1]) val = src->data[i + 1];
        if(val < src->data[i + 2]) val = src->data[i + 2];
                    
        if(val > 0){
            int min = src->data[i];
            if(min > src->data[i + 1]) min = src->data[i + 1];
            if(min > src->data[i + 2]) min = src->data[i + 2];

            int sat = (val - min) * 255 / val;
            int hue = 0;
            if (sat != 0)
            {
                int r = src->data[i];
                int g = src->data[i + 1];
                int b = src->data[i + 2];

                if (val == r && g >= b)
                {
                    hue = 60 * (g - b) / (val - min);
                }
                else if (val == r && b > g)
                {
                    hue = 360 + 60 * (g - b) / (val - min);
                }
                else if (val == g)
                {
                    hue = 120 + 60 * (b - r) / (val - min);
                }
                else if (val == b)
                {
                    hue = 240 + 60 * (r - g) / (val - min);
                }
            }

            dst->data[i] = (int)((hue * 255) / 360);
            dst->data[i + 1] = sat;
            dst->data[i + 2] = val;
        }
        else
        {
            dst->data[i] = 0;
            dst->data[i + 1] = 0;
            dst->data[i + 2] = 0;
        }
    }

    return 0;
}

int vc_rbg_to_hsv_ze(IVC *src, IVC *dst)
{
	int i, val, max, min, saturation, hue, r, g, b, convertHue;

	src->data[i] = r;
	src->data[i + 1] = g;
	src->data[i + 2] = b;	

	//Conversão de ângulo para "cor"
	convertHue = (int) (hue * 255) / 360;

	for(i = 0; i < src->bytesperline * src->height; i += src->channels)
	{	
		val = r;
		if(val < g) val = g;
		if(val < b) val = b;

		max = val;

		min = r;
		if(val > g) val = g;
		if(val > b) val = b;

		if(val == 0)
		{
			saturation = 0;
			hue = 0;
		}

		else
		{
			saturation = (max - min) * 255 / val; //Multiplica por 255 para garantir que a saturação está entre 0 e 255

			if(saturation == 0)
			{
				
			}

			if(max == r && g >= b) hue = 60 * (g - b) / (max - min);				
			if(max == r && b > g) hue = 360 + 60 * (g - b) / (max - min);	
			if(max == g) hue = 120 + 60 * (b - r) / (max - min);
			if(max == b) hue = 240 + 60 * (r - g) / (max - min);

			r = convertHue;
			g = saturation;
			b = val;
		}
	}
}

int vc_rgb_to_hsv_ze_2(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	for (i = 0; i<size; i = i + channels)
	{
		r = (float)datasrc[i];
		g = (float)datasrc[i + 1];
		b = (float)datasrc[i + 2];

		// Calcula valores m ximo e m nimo dos canais de cor R, G e B
		rgb_max = (float)(r > g ? (r > b ? r : b) : (g > b ? g : b));
		rgb_min = (float)(r < g ? (r < b ? r : b) : (g < b ? g : b));

		// Value toma valores entre [0,255]
		value = rgb_max;
		if (value == 0.0)
		{
			hue = 0.0;
			saturation = 0.0;
		}
		else
		{
			// Saturation toma valores entre [0,255]
			saturation = ((rgb_max - rgb_min) / rgb_max) * (float) 255.0;

			if (saturation == 0.0)
			{
				hue = 0.0;
			}
			else
			{
				// R, G e B tomam valores entre [0,1]
				r /= 255.0;
				g /= 255.0;
				b /= 255.0;

				// Calcula valores m ximo e m nimo dos canais de cor R, G e B (tomam valores entre [0,1])
				rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
				rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

				// Hue toma valores entre [0,360]
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60 * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360 + 60 * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120 + 60 * (b - r) / (rgb_max - rgb_min);
				}
				else /* rgb_max == b*/
				{
					hue = 240 + 60 * (r - g) / (rgb_max - rgb_min);
				}
			}
		}

		// Atribui valores entre [0,255]
		datadst[i] = (unsigned char)(hue / 360.0 * 255.0);
		datadst[i + 1] = (unsigned char)(saturation);
		datadst[i + 2] = (unsigned char)(value);
	}

	return 1;
}


//Função que receba uma imagem HSV e retorne uma imagem com 1 canal (admitindo valores entre 0 e 255 por pixel).
//Essa função deverá receber ainda os intervalos de valores da Matiz (H), Saturação (S) e Brilho (V) da cor que se pretende segmentar. 
//A imagem de saída deverá apresentar a branco (255) os pixéis que estão dentro desse intervalo, e a preto (0) todos os outros.
int vc_hsv_segmentation_ricardo(IVC *src, IVC *dst,
                        int hmin, int hmax,   // 0..360
                        int smin, int smax,   // 0..100
                        int vmin, int vmax)   // 0..100
{
    unsigned char *src_data, *dst_data;
    int x, y;

    if (!src || !dst) return 0;
    if (!src->data || !dst->data) return 0;
    if (src->width <= 0 || src->height <= 0) return 0;
    if (src->channels != 3) return 0;
    if (dst->channels != 1) return 0;
    if (dst->width != src->width || dst->height != src->height) return 0;
    if (dst->levels != 255) return 0;

    // Clamp dos limites
    if (hmin < 0) hmin = 0; if (hmin > 360) hmin = 360;
    if (hmax < 0) hmax = 0; if (hmax > 360) hmax = 360;
    if (smin < 0) smin = 0; if (smin > 100) smin = 100;
    if (smax < 0) smax = 0; if (smax > 100) smax = 100;
    if (vmin < 0) vmin = 0; if (vmin > 100) vmin = 100;
    if (vmax < 0) vmax = 0; if (vmax > 100) vmax = 100;

    // Converter limites "humanos" -> escala 0..255 (como está na imagem HSV)
    int hmin8 = (hmin * 255) / 360;
    int hmax8 = (hmax * 255) / 360;
    int smin8 = (smin * 255) / 100;
    int smax8 = (smax * 255) / 100;
    int vmin8 = (vmin * 255) / 100;
    int vmax8 = (vmax * 255) / 100;

    src_data = (unsigned char *)src->data;
    dst_data = (unsigned char *)dst->data;

    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            int pos_src = y * src->bytesperline + x * 3;
            int pos_dst = y * dst->bytesperline + x;

            int H = src_data[pos_src + 0]; // 0..255
            int S = src_data[pos_src + 1]; // 0..255
            int V = src_data[pos_src + 2]; // 0..255

            int h_ok;
            if (hmin8 <= hmax8)
                h_ok = (H >= hmin8 && H <= hmax8);
            else
                h_ok = (H >= hmin8 || H <= hmax8); // wrap (ex: 350..20)

            dst_data[pos_dst] = (h_ok &&
                                 (S >= smin8 && S <= smax8) &&
                                 (V >= vmin8 && V <= vmax8)) ? 255 : 0;
        }
    }

    return 1;
}

int vc_hsv_segmentation_raul(IVC *src, IVC *dst,
                        int hmin, int hmax,   // 0..360
                        int smin, int smax,   // 0..100
                        int vmin, int vmax)   // 0..100
{
    unsigned char *src_data, *dst_data;
    int x, y;

    if (!src || !dst) return 0;
    if (!src->data || !dst->data) return 0;
    if (src->width <= 0 || src->height <= 0) return 0;
    if (src->channels != 3) return 0;
    if (dst->channels != 1) return 0;
    if (dst->width != src->width || dst->height != src->height) return 0;
    if (dst->levels != 255) return 0;

    // Clamp dos limites
    if (hmin < 0) hmin = 0; if (hmin > 360) hmin = 360;
    if (hmax < 0) hmax = 0; if (hmax > 360) hmax = 360;
    if (smin < 0) smin = 0; if (smin > 100) smin = 100;
    if (smax < 0) smax = 0; if (smax > 100) smax = 100;
    if (vmin < 0) vmin = 0; if (vmin > 100) vmin = 100;
    if (vmax < 0) vmax = 0; if (vmax > 100) vmax = 100;

    // Converter limites "humanos" -> escala 0..255 (como está na imagem HSV)
    int hmin8 = (hmin * 255) / 360;
    int hmax8 = (hmax * 255) / 360;
    int smin8 = (smin * 255) / 100;
    int smax8 = (smax * 255) / 100;
    int vmin8 = (vmin * 255) / 100;
    int vmax8 = (vmax * 255) / 100;

    src_data = (unsigned char *)src->data;
    dst_data = (unsigned char *)dst->data;

    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            int pos_src = y * src->bytesperline + x * 3;
            int pos_dst = y * dst->bytesperline + x;

            int H = src_data[pos_src + 0]; // 0..255
            int S = src_data[pos_src + 1]; // 0..255
            int V = src_data[pos_src + 2]; // 0..255

            int h_ok;
            if (hmin8 <= hmax8)
                h_ok = (H >= hmin8 && H <= hmax8);
            else
                h_ok = (H >= hmin8 || H <= hmax8); // wrap (ex: 350..20)

            dst_data[pos_dst] = (h_ok &&
                                 (S >= smin8 && S <= smax8) &&
                                 (V >= vmin8 && V <= vmax8)) ? 255 : 0;
        }
    }

    return 1;
}

int vc_hsv_segmentation_ze(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int i, size;
	int hmin_deg, hmax_deg, hmin_255, hmax_255;
	int hue_ok, sv_ok;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 3) return 0;

	// Hue entra em graus [0,360], mas no pixel HSV está em [0,255].
	// Normaliza e converte para comparar diretamente com datasrc[i].
	hmin_deg = hmin;
	hmax_deg = hmax;
	if (hmin_deg < 0) hmin_deg = 0;
	if (hmin_deg > 360) hmin_deg = 360;
	if (hmax_deg < 0) hmax_deg = 0;
	if (hmax_deg > 360) hmax_deg = 360;
	hmin_255 = (hmin_deg * 255) / 360;
	hmax_255 = (hmax_deg * 255) / 360;

	size = width * height * channels;

	for (i = 0; i<size; i = i + channels)
	{
		// Se hmax < hmin, o intervalo de Hue atravessa 360->0 (zona do vermelho).
		if (hmin_255 <= hmax_255)
		{
			hue_ok = (datasrc[i] >= hmin_255) && (datasrc[i] <= hmax_255);
		}
		else
		{
			hue_ok = (datasrc[i] >= hmin_255) || (datasrc[i] <= hmax_255);
		}

		sv_ok = (datasrc[i + 1] >= smin) && (datasrc[i + 1] <= smax) &&
				(datasrc[i + 2] >= vmin) && (datasrc[i + 2] <= vmax);

		if (hue_ok && sv_ok)
		{
			datadst[i] = 255; //datasrc[i];
			//datadst[i + 1] = 255; //datasrc[i + 1];
			//datadst[i + 2] = 255; //datasrc[i + 2];
		}
		else
		{
			datadst[i] = 0;
			//datadst[i + 1] = 0;
			// datadst[i + 2] = 0;
		}
	}

	return 1;
}

int vc_hsv_segmentation_diogo(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax){

	int i, hue, sat, val;

	if(hmin < 0 || hmax > 360 || smin < 0 || smax > 100 || vmin < 0 || vmax > 100){
		return 0;
	}

	hmin = (hmin % 360) * 255 / 360;
	hmax = (hmax % 360) * 255 / 360;
	smin = smin * 255 /100;
	smax = smax * 255 /100;
	vmin = vmin * 255 /100;
	vmax = vmax * 255 /100;

	long int size = src->height * src->width * src->channels;

	for(i=0; i<size; i=i+src->channels){
		hue = src->data[i];
		sat = src->data[i+1];
		val = src->data[i+2];

			if(hmin < hmax){
				if((hue >= hmin && hue <= hmax) && sat >= smin && sat <= smax && val >= vmin && val <= vmax){
					dst->data[i] = 255;
					dst->data[i+1] = 255;
					dst->data[i+2] = 255;
				}else{
					dst->data[i] = 0;
					dst->data[i+1] = 0;
					dst->data[i+2] = 0;
				}
			}else{
				if((hue >= hmin || hue <= hmax) && sat >= smin && sat <= smax && val >= vmin && val <= vmax){
					dst->data[i] = 255;
					dst->data[i+1] = 255;
					dst->data[i+2] = 255;
				}else{
					dst->data[i] = 0;
					dst->data[i+1] = 0;
					dst->data[i+2] = 0;
				}
			}
	}
		
}


//Função que converta uma imagem com escala de intensidades em cinzento numa imagem com uma escala em cores do espaço RGB.
int vc_scale_gray_to_rgb_ricardo(IVC *src, IVC *dst)
{
    unsigned char *datasrc, *datadst;
    int x, y;

    if (src == NULL || dst == NULL) return 0;
    if (src->data == NULL || dst->data == NULL) return 0;

    // src: 1 canal (grayscale), dst: 3 canais (RGB)
    if (src->channels != 1) return 0;
    if (dst->channels != 3) return 0;	

    // Mesma dimensão
    if (src->width != dst->width || src->height != dst->height) return 0;

    // 8-bit
    if (src->levels != 255 || dst->levels != 255) return 0;

    datasrc = (unsigned char *)src->data;
    datadst = (unsigned char *)dst->data;

    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            int pos_src = y * src->bytesperline + x;
            int pos_dst = y * dst->bytesperline + x * 3;

            unsigned char gray = datasrc[pos_src]; // 0..255
            unsigned char r, g, b;

            // Colormap tipo "rainbow/jet" em 4 segmentos (0..255)
            // 0..63:    azul -> ciano
            // 64..127:  ciano -> verde
            // 128..191: verde -> amarelo
            // 192..255: amarelo -> vermelho
            if (gray < 64)
            {
                // azul (0,0,255) -> ciano (0,255,255)
                r = 0;
                g = (unsigned char)(gray * 255 / 63);   // 0..252
                b = 255;
            }
            else if (gray < 128)
            {
                // ciano (0,255,255) -> verde (0,255,0)
                r = 0;
                g = 255;
                b = (unsigned char)(255 - (gray - 64) * 255 / 63); // 255..3
            }
            else if (gray < 192)
            {
                // verde (0,255,0) -> amarelo (255,255,0)
                r = (unsigned char)((gray - 128) * 255 / 63); // 0..252
                g = 255;
                b = 0;
            }
            else
            {
                // amarelo (255,255,0) -> vermelho (255,0,0)
                r = 255;
                g = (unsigned char)(255 - (gray - 192) * 4); // 255..3
                b = 0;
            }

            datadst[pos_dst + 0] = r;   // R
            datadst[pos_dst + 1] = g; // G
            datadst[pos_dst + 2] = b;   // B
        }
    }

    return 1;
}

int vc_scale_gray_to_rgb_ricardo_2(IVC *src, IVC *dst) // Mais eficiente a partir de ~350pixeis
{
    unsigned char *datasrc, *datadst;
    int x, y;

    // LUTs (0..255)
    unsigned char red[256], green[256], blue[256];
    int i;
	int size = dst->width * dst->height;

    if (src == NULL || dst == NULL) return 0;
    if (src->data == NULL || dst->data == NULL) return 0;

    // src: 1 canal (grayscale), dst: 3 canais (RGB)
    if (src->channels != 1) return 0;
    if (dst->channels != 3) return 0;

    // Mesma dimensão
    if (src->width != dst->width || src->height != dst->height) return 0;

    // 8-bit
    if (src->levels != 255 || dst->levels != 255) return 0;

    // -----------------------------
    // 1) Construir o colormap (LUT)
    // -----------------------------
    for (i = 0; i < 256; i++)
    {
        if (i < 64)
        {
            // azul (0,0,255) -> ciano (0,255,255)
            red[i]   = 0;
            green[i] = (unsigned char)(i * 255 / 63);   // 0..255
            blue[i]  = 255;
        }
        else if (i < 128)
        {
            // ciano (0,255,255) -> verde (0,255,0)
            red[i]   = 0;
            green[i] = 255;
            blue[i]  = (unsigned char)(255 - (i - 64) * 255 / 63); // 255..0
        }
        else if (i < 192)
        {
            // verde (0,255,0) -> amarelo (255,255,0)
            red[i]   = (unsigned char)((i - 128) * 255 / 63); // 0..255
            green[i] = 255;
            blue[i]  = 0;
        }
        else
        {
            // amarelo (255,255,0) -> vermelho (255,0,0)
            red[i]   = 255;
            green[i] = (unsigned char)(255 - (i - 192) * 255 / 63); // 255..0
            blue[i]  = 0;
        }
    }

    datasrc = (unsigned char *)src->data;
    datadst = (unsigned char *)dst->data;

    // ---------------------------------
    // 2) Aplicar LUT a todos os pixels
    // ---------------------------------
    for (i=0; i<size; i++)
	{
		unsigned char gray = datasrc[i]; // 0..255

		datadst[i * 3 + 0] = red[gray];   // R
		datadst[i * 3 + 1] = green[gray]; // G
		datadst[i * 3 + 2] = blue[gray];  // B
	}

    return 1;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: TÉCNICAS DE SEGMENTAÇÃO DE IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//Função que realize a binarização, por thresholding manual, de uma imagem em tons de cinzento.

int vc_gray_to_binary_raul(IVC *src, IVC *dst, int threshold) {
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;

	for (int i = 0; i < bytesperline * height; i += channels)
    {
		datadst[i] = (datasrc[i] >= threshold) ? 255 : 0;
	}

	return 1;
}

int vc_gray_to_binary_ricardo(IVC *src, IVC *dst, int threshold) {
    unsigned char *datasrc, *datadst;
    int x, y;

    if (src == NULL || dst == NULL) return 0;
    if (src->data == NULL || dst->data == NULL) return 0;

    if (src->channels != 1 || dst->channels != 1) return 0;
    if (src->width != dst->width || src->height != dst->height) return 0;


    datasrc = (unsigned char *)src->data;
    datadst = (unsigned char *)dst->data;

    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            int pos_src = y * src->bytesperline + x;
            int pos_dst = y * dst->bytesperline + x;

            datadst[pos_dst] = (datasrc[pos_src] >= threshold) ? 255 : 0;
        }
    }

    return 1;
}

int vc_gray_to_binary_ze(IVC *src, IVC *dst, int threshold) {
	int size = src->bytesperline * src->height;

	for(int i = 0; i < size; i += src->channels)
	{
		if(src->data[i] >= threshold)
		{
			dst->data[i] = 255;
		} 
		else
		{
			dst->data[i] = 0;	
		}
	}
	return 1;
}

int vc_gray_to_binary_prof(IVC *src, IVC *dst, int threshold){
	unsigned char *datasrc = (unsigned char *)src->data;
	int bytesperline_src = src->bytesperline;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->bytesperline;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((dst->width <= 0) || (dst->height <= 0) || (dst->data == NULL)) return 0;
	if (channels_src != 1 || channels_dst != 1) return 0;

	for (y = 0; y < height; y++){
		for (x = 0; x < width; x++){
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			datadst[pos_dst] = (datasrc[pos_src] >= threshold) ? 255 : 0;
		}
	}
	return 1;
}



//Função que realize a binarização, por thresholding através da média global, de uma imagem em tons de cinzento.

int vc_gray_to_binary_global_mean_ricardo(IVC *src, IVC *dst){
    unsigned char *datasrc, *datadst;
    int x, y;
    long int sum = 0;
    int total_pixels;

    if (src == NULL || dst == NULL) return 0;
    if (src->data == NULL || dst->data == NULL) return 0;

    if (src->channels != 1 || dst->channels != 1) return 0;
    if (src->width != dst->width || src->height != dst->height) return 0;

    datasrc = (unsigned char *)src->data;
    datadst = (unsigned char *)dst->data;

    total_pixels = src->width * src->height;

    // Calcular a média global
    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            int pos_src = y * src->bytesperline + x;
            sum += datasrc[pos_src];
        }
    }

    int mean_threshold = sum / total_pixels;

    // Aplicar o limiar para criar a imagem binária
    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            int pos_src = y * src->bytesperline + x;
            int pos_dst = y * dst->bytesperline + x;

            datadst[pos_dst] = (datasrc[pos_src] >= mean_threshold) ? 255 : 0;
        }
    }

    return 1;
}

int vc_gray_to_binary_global_mean_raul(IVC *src, IVC *dst){
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int size;
	long int sum = 0;
	int mean;

	printf("Channels: %d \n", channels);

	size = width * height * channels;
	// Calcula a média global
	for (int i = 0; i < bytesperline * height; i += channels)
    {
		sum += datasrc[i];
	}
	mean = sum / size;

	// Aplica o limiar usando a média global
	for (int i = 0; i < bytesperline * height; i += channels)
    {
		datadst[i] = (datasrc[i] >= mean) ? 255 : 0;
	}

	return 1;
}

int vc_gray_to_binary_global_mean_ze(IVC* src, IVC *dst)
{
	int size = src->bytesperline * src->height;
	int countPixels = 0;
	int average = 0;

	for(int i = 0; i < size; i++)
	{
		countPixels += src->data[i];
	}

	average = countPixels/size;

	for(int i = 0; i < size; i++)
	{
		if(src->data[i] >= average)
		{
			dst->data[i] = 255;
		}
		else
		{
			dst->data[i] = 0;
		}
	}

	return 1;	
}

int vc_gray_to_binary_global_mean_prof(IVC *src, IVC *dst){
	unsigned char *datasrc = (unsigned char *)src->data;
	int bytesperline_src = src->bytesperline;
	int channels_src = src->channels;
	unsigned char *datadst = (unsigned char *)dst->data;
	int bytesperline_dst = dst->bytesperline;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	long int sum = 0;
	int count = 0;
	int mean;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((dst->width <= 0) || (dst->height <= 0) || (dst->data == NULL)) return 0;
	if (channels_src != 1 || channels_dst != 1) return 0;

	// Calcula a média global dos valores dos píxeis da imagem de origem
	for (y = 0; y < height; y++){
		for (x = 0; x < width; x++){
			pos_src = y * bytesperline_src + x * channels_src;
			sum += datasrc[pos_src];
			count++;
		}
	}
	mean = sum / count;

	// Aplica a limiarização global usando a média calculada
	for (y = 0; y < height; y++){
		for (x = 0; x < width; x++){
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			datadst[pos_dst] = (datasrc[pos_src] >= mean) ? 255 : 0;
		}
	}
	return 1;
}


//Função que faz a binarização, por thresholding através da média global, de uma imagem em tons de cinzento
int vc_gray_to_binary_global_mean_and_manual_threshold_ze(IVC *src, IVC *dst, int threshold, int minThreshold, int maxThreshold)
{
	int size = src->bytesperline * src->height;

	if((threshold < 0 || threshold > 255) && (minThreshold < 0 || minThreshold > 255) || (maxThreshold < 0 || maxThreshold > 255))
	{
		return 0;
	}	

	if(maxThreshold < minThreshold)
	{
		return -1;
	}

	if((threshold >= 0 || threshold <= 255) && (minThreshold >= 0 || minThreshold <= 255) || (maxThreshold >= 0 || maxThreshold <= 255)) return -2;

	if(threshold < 0 || threshold > 255)
	{
		for(int i = 0; i < size; i++)
		{
			if(src->data[i] >= minThreshold && src->data[i] <= maxThreshold)
			{
				dst->data[i] = 255;
			}
			else
			{
				dst->data[i] = 0;
			}
		}
	}
	else
	{
		for(int i = 0; i < size; i++)
		{
		if(src->data[i] >= threshold)
		{
			dst->data[i] = 255;
		} 
		else
		{
			dst->data[i] = 0;	
		}
		}
	}
	return 1;
}




int vc_gray_to_binary_range_raul(IVC *src, IVC *dst, int invert, int thresholdMin, int thresholdMax) {
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;

	if (thresholdMax > 0 && thresholdMax > thresholdMin) {
		if(invert > 0){
			for (int i = 0; i < bytesperline * height; i += channels)
			{
				datadst[i] = (datasrc[i] >= thresholdMin && datasrc[i] <= thresholdMax) ? 255 : 0;
			}
		} else {
			for (int i = 0; i < bytesperline * height; i += channels)
			{
				datadst[i] = (datasrc[i] <= thresholdMin || datasrc[i] >= thresholdMax) ? 255 : 0;
			}
		}
	} else {
		if(invert > 0){
			for (int i = 0; i < bytesperline * height; i += channels)
			{
				datadst[i] = (datasrc[i] <= thresholdMin) ? 255 : 0;
			}
		} else {
			for (int i = 0; i < bytesperline * height; i += channels)
			{
				datadst[i] = (datasrc[i] >= thresholdMin) ? 255 : 0;
			}
		}
	}

	return 1;
}

int vc_gray_to_binary_challenge_ricardo(IVC *src, IVC *dst, int t1, int t2, int mode)
{
    unsigned char *datasrc, *datadst;
    int x, y;

    if (src == NULL || dst == NULL) return 0;
    if (src->data == NULL || dst->data == NULL) return 0;

    if (src->channels != 1 || dst->channels != 1) return 0;
    if (src->width != dst->width || src->height != dst->height) return 0;
    if (src->levels != 255 || dst->levels != 1) return 0;

    datasrc = (unsigned char *)src->data;
    datadst = (unsigned char *)dst->data;

    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            int pos = y * src->width + x;
            unsigned char pixel = datasrc[pos];

            // Caso: UM threshold
            if (t2 < 0 || t1 == t2)
            {
                if (mode == 0) // acima
                    datadst[pos] = (pixel > t1) ? 255 : 0;
                else // abaixo
                    datadst[pos] = (pixel < t1) ? 255 : 0;
            }
            // Caso: DOIS thresholds
            else
            {
                if (mode == 1) // entre
                    datadst[pos] = (pixel >= t1 && pixel <= t2) ? 255 : 0;
                else // fora
                    datadst[pos] = (pixel < t1 || pixel > t2) ? 255 : 0;
            }
        }
    }

    return 1;
}

//Função que realize a binarização, por thresholding, de uma imagem em tons de cinzento para cada um dos 
//métodos (Midpoint,Bernsen,Niblack)

int vc_gray_to_binary_midpoint_ricardo(IVC *src, IVC *dst, int kernel)
{
    unsigned char *datasrc, *datadst;
    int x, y, xx, yy;
    int offset;
    int pos_src, pos_dst;
    int min, max, threshold;

    if (src == NULL || dst == NULL) return 0;
    if (src->data == NULL || dst->data == NULL) return 0;

    if (src->channels != 1 || dst->channels != 1) return 0;
    if (src->width != dst->width || src->height != dst->height) return 0;

    // Kernel tem que ser ímpar e >= 3
    if (kernel < 3 || (kernel % 2) == 0) return 0;

    offset = (kernel - 1) / 2;

    datasrc = (unsigned char *)src->data;
    datadst = (unsigned char *)dst->data;

    for (y = 0; y < src->height; y++)
    {
        for (x = 0; x < src->width; x++)
        {
            min = 255;
            max = 0;

            // Percorrer vizinhança
            for (yy = -offset; yy <= offset; yy++)
            {
                for (xx = -offset; xx <= offset; xx++)
                {
                    int neighbor_x = x + xx;
                    int neighbor_y = y + yy;

                    if (neighbor_x >= 0 && neighbor_x < src->width &&
                        neighbor_y >= 0 && neighbor_y < src->height)
                    {
                        pos_src = neighbor_y * src->bytesperline + neighbor_x;
                    }
                }
            }

            threshold = (min + max) / 2;

            pos_src = y * src->bytesperline + x;
            pos_dst = y * dst->bytesperline + x;

            datadst[pos_dst] = (datasrc[pos_src] >= threshold) ? 255 : 0;
        }
    }

    return 1;
}

int vc_gray_to_binary_midpoint_raul(IVC *src, IVC *dst, int kernel)
{
	int offset = (kernel - 1) / 2;
	int vmax, vmin, pos;	
	
	for(int y = 0; y < src->height; y++)
	{
		for(int x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels; //pos é o pixel central

			vmax = 0;
			vmin = 255;
			
			//Percorrer o Kernel 
			for(int kx = -offset; kx <= offset; kx++)
			{
				for(int ky = -offset; ky <= offset; ky++)
				{				

					if(kx >= src->width || ky >= src->height || kx < 0 || ky < 0) continue;
					
					int posk = ky * src->bytesperline + kx * src->channels;

					vmin = vmin < src->data[posk] ? vmin : src->data[posk]; //"?"->if ; ":"->else
					vmax = vmax > src->data[posk] ? vmax : src->data[posk];
				}
			}

			int threshold = (vmin + vmax) / 2;
			dst->data[pos] = src->data[pos] > threshold ? 255 : 0;

		}	
	}
}

int vc_gray_to_binary_midpoint_ze(IVC *src, IVC *dst, int kernel)
{
	int offset = (kernel - 1) / 2;
	int vmax, vmin, pos;	
	
	for(int y = 0; y < src->height; y++)
	{
		for(int x = 0; x < src->width; x++)
		{
			pos = y * src->bytesperline + x * src->channels; //pos é o pixel central

			vmax = 0;
			vmin = 255;
			
			//Percorrer o Kernel 
			for(int kx = -offset; kx <= offset; kx++)
			{
				for(int ky = -offset; ky <= offset; ky++)
				{				

					if(kx >= src->width || ky >= src->height || kx < 0 || ky < 0) continue;
					
					int posk = ky * src->bytesperline + kx * src->channels;

					vmin = vmin < src->data[posk] ? vmin : src->data[posk]; //"?"->if ; ":"->else
					vmax = vmax > src->data[posk] ? vmax : src->data[posk];
				}
			}

			int threshold = (vmin + vmax) / 2;
			dst->data[pos] = src->data[pos] > threshold ? 255 : 0;

		}	
	}
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: OPERADORES MORFOLÓGICOS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//Funções que realizam a dilatação e a erosão binária.
int vc_binary_dilate_ricardo(IVC *src, IVC *dst, int kernel)
{
    unsigned char *datasrc, *datadst;
    int x, y, xx, yy;
    int offset;
    int pos_src, pos_dst;
    int found;

    if(src==NULL || dst==NULL) return 0;
    if(src->data==NULL || dst->data==NULL) return 0;

    //Verificar dimensões
    if(src->width != dst->width || src->height != dst ->height) return 0;
    if(src->channels != 1 || dst->channels != 1) return 0;

    //Kernel tem que ser ímpar e >= 3
    if(kernel < 3 || (kernel % 2) == 0) return 0;

    datasrc = (unsigned char *)src->data;
    datadst = (unsigned char *)dst->data;

    offset = (kernel - 1) / 2;

    for (y=0; y<src->height; y++)
    {
        for (x=0; x<src->width; x++)
        {
            pos_dst = y * dst->bytesperline + x * dst->channels;
            found = 0;

            //Percorrer vizinhança
            for (yy=-offset; yy<=offset && !found; yy++)
            {
                for (xx=-offset; xx<=offset && !found; xx++)
                {
                    int neighbor_x = x + xx;
                    int neighbor_y = y + yy;

                    //Verificar limites da imagem
                    if (neighbor_x >= 0 && neighbor_x < src->width &&
                        neighbor_y >= 0 && neighbor_y < src->height)
                    {
                        pos_src = neighbor_y * src->bytesperline + neighbor_x * src->channels;

                        if (datasrc[pos_src] == 255)
                        {
                            found = 1;
                            break;
                        }
                    }
                }
            }
            
            datadst[pos_dst] = found ? 255 : 0;
        }
    }
    return 1;
}

int vc_binary_dilate_raul(IVC *src, IVC *dst, int kernel) {
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int width = src->width;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = 0;
	long int pos = 0;
	long int posOffset = 0;

	// Check if the kernel is valid
	if (kernel % 2 == 0 || kernel <= 0) {
		printf("ERROR -> The kernel needs to be a positive odd number!\n");
        getchar();
        return 0;
	}

	offset = (kernel - 1) / 2;

	printf("%d", offset);

	for (int x = 0; x < width; x++) {
		
		for (int y = 0; y < height; y++) {
			pos = y * bytesperline + x * channels;

			dst->data[pos] = src->data[pos];
			
			for (int ix = -offset; ix <= offset; ix++) {
				for (int iy = -offset; iy <= offset; iy++) {
					if((x + ix) >= src->width || (y + iy) >= src->height || (x + ix) < 0 || (y + iy) < 0) continue;
					
					posOffset = (y + iy) * bytesperline + (x + ix) * channels;

					if(src->data[posOffset] == 255) {
						dst->data[pos] = 255;
						break;
					}
				}
				
				if(dst->data[pos] == 255) {
					break;
				}
			}
		}
	}

	return 1;
}

int vc_binary_dilate_ze(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width  = src->width;
	int height = src->height;
	int pos, vmax, posk;
	memset(datadst, 0, height * dst->bytesperline);

	if(kernel <= 0 || kernel % 2 == 0)
	{
		return -1;
	}

	int offset = (kernel - 1) / 2;

	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			pos = y * src->bytesperline + x * src->channels; //pos é o pixel central
						
			//Percorrer o Kernel 
			for(int kx = -offset; kx <= offset; kx++)
			{
				for(int ky = -offset; ky <= offset; ky++)
				{	
					//Condição barreira
					if((x + kx) >= src->width || (y + ky) >= src->height || (x + kx) < 0 || (y + ky) < 0) continue;

					posk = (y + ky) * src->bytesperline + (x + kx) * src->channels;

					if(src->data[posk] == 255)
					{
						dst->data[pos] = 255;
						dst->data[pos + 1] = 255;
						dst->data[pos + 2] = 255;
						break;
					} 

					
				}

				if(dst->data[pos] == 255 && dst->data[pos + 1] == 255 && dst->data[pos + 2] == 255)
				{
					break;
				}
			}			
		}
	}

	return 1;
}



int vc_binary_erode_ricardo(IVC *src, IVC *dst, int kernel)
{
    unsigned char *datasrc, *datadst;
    int x, y, xx, yy;
    int offset;
    int pos_src, pos_dst;
    int found;

    if(src==NULL || dst==NULL) return 0;
    if(src->data==NULL || dst->data==NULL) return 0;

    //Verificar dimensões
    if(src->width != dst->width || src->height != dst ->height) return 0;
    if(src->channels != 1 || dst->channels != 1) return 0;

    //Kernel tem que ser ímpar e >= 3
    if(kernel < 3 || (kernel % 2) == 0) return 0;

    datasrc = (unsigned char *)src->data;
    datadst = (unsigned char *)dst->data;

    offset = (kernel - 1) / 2;

    for (y=0; y<src->height; y++)
    {
        for (x=0; x<src->width; x++)
        {
            pos_dst = y * dst->bytesperline + x * dst->channels;
            found = 0;

            //Percorrer vizinhança
            for (yy=-offset; yy<=offset && !found; yy++)
            {
                for (xx=-offset; xx<=offset && !found; xx++)
                {
                    int neighbor_x = x + xx;
                    int neighbor_y = y + yy;

                    //Verificar limites da imagem
                    if (neighbor_x >= 0 && neighbor_x < src->width &&
                        neighbor_y >= 0 && neighbor_y < src->height)
                    {
                        pos_src = neighbor_y * src->bytesperline + neighbor_x * src->channels;

                        if (datasrc[pos_src] == 0)
                        {
                            found = 1;
                            break;
                        }
                    }
                }
            }
            
            datadst[pos_dst] = found ? 0 : 255;
        }
    }
    return 1;
}

int vc_binary_erode_raul(IVC *src, IVC *dst, int kernel) {
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int width = src->width;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = 0;
	long int pos = 0;
	long int posOffset = 0;

	// Check if the kernel is valid
	if (kernel % 2 == 0 || kernel <= 0) {
		printf("ERROR -> The kernel needs to be a positive odd number!\n");
        getchar();
        return 0;
	}

	offset = (kernel - 1) / 2;

	printf("%d", offset);

	for (int x = 0; x < width; x++) {
		
		for (int y = 0; y < height; y++) {
			pos = y * bytesperline + x * channels;

			dst->data[pos] = src->data[pos];
			
			for (int ix = -offset; ix <= offset; ix++) {
				for (int iy = -offset; iy <= offset; iy++) {
					if((x + ix) >= src->width || (y + iy) >= src->height || (x + ix) < 0 || (y + iy) < 0) continue;
					
					posOffset = (y + iy) * bytesperline + (x + ix) * channels;

					if(src->data[posOffset] == 0) {
						dst->data[pos] = 0;
						break;
					}
				}
				
				if(dst->data[pos] == 0) {
					break;
				}
			}
		}
	}

	return 1;
}

int vc_binary_erode_ze(IVC *src, IVC *dst, int kernel)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width  = src->width;
	int height = src->height;
	int pos, vmax, posk;

	if(kernel <= 0 || kernel % 2 == 0)
	{
		return -1;
	}

	int offset = (kernel - 1) / 2;

	for(int x = 0; x < height; x++)
	{
		for(int y = 0; y < width; y++)
		{
			pos = y * src->bytesperline + x * src->channels; //pos é o pixel central

			//Atribuição inicial à imagem de destino
			datadst[pos] = datasrc[pos];
			
			//Percorrer o Kernel 
			for(int kx = -offset; kx <= offset; kx++)
			{
				for(int ky = -offset; ky <= offset; ky++)
				{	
					//Condição barreira
					if((x + kx) >= src->width || (y + ky) >= src->height || (x + kx) < 0 || (y + ky) < 0) continue;

					posk = (y + ky) * src->bytesperline + (x + kx) * src->channels;

					if(src->data[posk] == 0)
					{
						dst->data[pos] = 0;
						break;
					} 					
				}

				if(dst->data[pos] == 0)
				{
					break;
				}
			}			
		}
	}

	return 1;
}



//Funções que realizam a abertura e o fecho binário.

int vc_image_open_ricardo(IVC *src, IVC *dst, int kernelErode, int kernelDilate){

	if(src == NULL || dst == NULL || kernelErode < 1 || kernelDilate < 1) return 0;

	IVC* img = vc_image_new(src->width, src->height, src->channels, src->levels);
	vc_binary_erode(src, img, kernelErode);
	vc_binary_dilate(img, dst, kernelDilate);

	vc_image_free(img);

	return 1;
}

int vc_image_close_ricardo(IVC *src, IVC *dst, int kernelDilate, int kernelErode){

	if(src == NULL || dst == NULL || kernelErode < 1 || kernelDilate < 1) return 0;

	IVC* img = vc_image_new(src->width, src->height, src->channels, src->levels);
	vc_binary_dilate(src, img, kernelDilate);
	vc_binary_erode(img, dst, kernelErode);

	vc_image_free(img);

	return 1;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: DETEÇÃO DE BLOBS E ETIQUETAGEM DE COMPONENTES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



//Função que realiza a etiquetagem de imagens binárias

// Etiquetagem de blobs
// src		: Imagem bin�ria de entrada
// dst		: Imagem grayscale (ir� conter as etiquetas)
// nlabels	: Endere�o de mem�ria de uma vari�vel, onde ser� armazenado o n�mero de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. � necess�rio libertar posteriormente esta mem�ria.
OVC* vc_binary_blob_labelling_ricardo(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que ser� retornado desta fun��o.

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem bin�ria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pix�is de plano de fundo devem obrigat�riamente ter valor 0
	// Todos os pix�is de primeiro plano devem obrigat�riamente ter valor 255
	// Ser�o atribu�das etiquetas no intervalo [1,254]
	// Este algoritmo est� assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem bin�ria
	for (y = 0; y<height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x<width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A est� marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do n�mero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se n�o h� blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}

int vc_binary_blob_labelling2_ricardo(IVC *src, IVC *dst){
    unsigned char *datasrc, *datadst;
    int x,y;
    long int pos_src, pos_dst;
    int label = 1;

    if(src == NULL || dst == NULL) return 0;
    if(src->data == NULL || dst->data == NULL) return 0;
    if(src->width != dst->width || src->height != dst ->height) return 0;
    if(src->channels != 1 || dst->channels != 1) return 0;

    datasrc = (unsigned char *)src->data;
    datadst = (unsigned char *)dst->data;
    

    for (y=0; y<src->height; y++)
    {
        for (x=0; x<src->width; x++)
        {
            int A=0, B=0, C=0, D=0;
            int minlabel=255;

            pos_src = y * src->bytesperline + x * src->channels;
            pos_dst = y * dst->bytesperline + x * dst->channels;

            if (datasrc[pos_src] == 0)
            {
                datadst[pos_dst] = 0;
            }
            else
            {
                // A = cima-esquerda
                if ((x > 0) && (y > 0))
                    A = datadst[(y - 1) * dst->bytesperline + (x - 1) * dst->channels];

                // B = cima
                if (y > 0)
                    B = datadst[(y - 1) * dst->bytesperline + x * dst->channels];

                // C = cima-direita
                if ((x < src->width - 1) && (y > 0))
                    C = datadst[(y - 1) * dst->bytesperline + (x + 1) * dst->channels];

                // D = esquerda
                if (x > 0)
                    D = datadst[y * dst->bytesperline + (x - 1) * dst->channels];

                if (A != 0 && A < minlabel) minlabel = A;
                if (B != 0 && B < minlabel) minlabel = B;
                if (C != 0 && C < minlabel) minlabel = C;
                if (D != 0 && D < minlabel) minlabel = D;

                if (minlabel == 255)
                {
                    datadst[pos_dst] = (unsigned char)label;
                    label++;
                    if (label > 254) return label - 1;
                }
                else
                {
                    datadst[pos_dst] = (unsigned char)minlabel;
                }
            }
        }
    }

    return label - 1;
}

OVC* vc_binary_blob_labelling_raul(IVC *src, IVC *dst, int *nlabels)
	{
		unsigned char *datasrc = (unsigned char *)src->data;
		unsigned char *datadst = (unsigned char *)dst->data;
		int width = src->width;
		int height = src->height;
		int bytesperline = src->bytesperline;
		int channels = src->channels;
		int x, y, a, b;
		long int i, size;
		long int posX, posA, posB, posC, posD;
		int labeltable[256] = { 0 };
		int labelarea[256] = { 0 };
		int label = 1; // Etiqueta inicial.
		int num, tmplabel;
		OVC *blobs; // Apontador para array de blobs (objectos) que ser� retornado desta fun��o.

		// Verifica��o de erros
		if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
		if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
		if (channels != 1) return NULL;

		// Copia dados da imagem bin�ria para imagem grayscale
		memcpy(datadst, datasrc, bytesperline * height);

		// Todos os pix�is de plano de fundo devem obrigat�riamente ter valor 0
		// Todos os pix�is de primeiro plano devem obrigat�riamente ter valor 255
		// Ser�o atribu�das etiquetas no intervalo [1,254]
		// Este algoritmo est� assim limitado a 254 labels
		for (i = 0, size = bytesperline * height; i<size; i++)
		{
			if (datadst[i] != 0) datadst[i] = 255;
		}

		// Limpa os rebordos da imagem bin�ria
		for (y = 0; y<height; y++)
		{
			datadst[y * bytesperline + 0 * channels] = 0;
			datadst[y * bytesperline + (width - 1) * channels] = 0;
		}
		for (x = 0; x<width; x++)
		{
			datadst[0 * bytesperline + x * channels] = 0;
			datadst[(height - 1) * bytesperline + x * channels] = 0;
		}

		// Efectua a etiquetagem
		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				// Kernel:
				// A B C
				// D X

				posA = (y - 1) * bytesperline + (x - 1) * channels; // A
				posB = (y - 1) * bytesperline + x * channels; // B
				posC = (y - 1) * bytesperline + (x + 1) * channels; // C
				posD = y * bytesperline + (x - 1) * channels; // D
				posX = y * bytesperline + x * channels; // X

				// Se o pixel foi marcado
				if (datadst[posX] != 0)
				{
					if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
					{
						datadst[posX] = label;
						labeltable[label] = label;
						label++;
					}
					else
					{
						num = 255;

						// Se A est� marcado
						if (datadst[posA] != 0) num = labeltable[datadst[posA]];
						// Se B est� marcado, e � menor que a etiqueta "num"
						if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
						// Se C est� marcado, e � menor que a etiqueta "num"
						if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
						// Se D est� marcado, e � menor que a etiqueta "num"
						if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

						// Atribui a etiqueta ao pixel
						datadst[posX] = num;
						labeltable[num] = num;

						// Actualiza a tabela de etiquetas
						if (datadst[posA] != 0)
						{
							if (labeltable[datadst[posA]] != num)
							{
								for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
								{
									if (labeltable[a] == tmplabel)
									{
										labeltable[a] = num;
									}
								}
							}
						}
						if (datadst[posB] != 0)
						{
							if (labeltable[datadst[posB]] != num)
							{
								for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
								{
									if (labeltable[a] == tmplabel)
									{
										labeltable[a] = num;
									}
								}
							}
						}
						if (datadst[posC] != 0)
						{
							if (labeltable[datadst[posC]] != num)
							{
								for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
								{
									if (labeltable[a] == tmplabel)
									{
										labeltable[a] = num;
									}
								}
							}
						}
						if (datadst[posD] != 0)
						{
							if (labeltable[datadst[posD]] != num)
							{
								for (tmplabel = labeltable[datadst[posD]], a = 1; a<label; a++)
								{
									if (labeltable[a] == tmplabel)
									{
										labeltable[a] = num;
									}
								}
							}
						}
					}
				}
			}
		}

		// Volta a etiquetar a imagem
		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				posX = y * bytesperline + x * channels; // X

				if (datadst[posX] != 0)
				{
					datadst[posX] = labeltable[datadst[posX]];
				}
			}
		}

		//printf("\nMax Label = %d\n", label);

		// Contagem do n�mero de blobs
		// Passo 1: Eliminar, da tabela, etiquetas repetidas
		for (a = 1; a<label - 1; a++)
		{
			for (b = a + 1; b<label; b++)
			{
				if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
			}
		}
		// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
		*nlabels = 0;
		for (a = 1; a<label; a++)
		{
			if (labeltable[a] != 0)
			{
				labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
				(*nlabels)++; // Conta etiquetas
			}
		}

		// Se n�o h� blobs
		if (*nlabels == 0) return NULL;

		// Cria lista de blobs (objectos) e preenche a etiqueta
		blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
		if (blobs != NULL)
		{
			for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
		}
		else return NULL;

		return blobs;
	}

int vc_binary_blob_labelling_ze(IVC *src, IVC *dst)
{
	int label = 1;	
	int kernel = 3;
	int offset = (kernel - 1) / 2;

	for(int x = 0; x < src->height; x++)
	{
		for(int y = 0; y < src->width; y++)
		{
			int pos = x * src->bytesperline + y * src->channels; //pos é o atual
			
			if(src->data[pos] == 255)
			{
			//Percorrer o Kernel 
			for(int kx = -offset; kx <= offset; kx++)
			{
				for(int ky = -offset; ky <= offset; ky++)
				{	
					//Condição barreira
					if((x + kx) >= src->height || (y + ky) >= src->width || (x + kx) < 0 || (y + ky) < 0) continue;

					int posk = (x + kx) * src->bytesperline + (y + ky) * src->channels;

					int min = 255;

					if(dst->data[posk] < min && dst->data[posk] != 0) min = dst->data[posk];

					if(dst->data[posk] == 0)
					{						
						dst->data[pos] = label;
						label++;
					}
					else
					{
						if(dst->data[posk] > min && min != 0) dst->data[pos] = min;
					}
				}
			}
			}
			else
			{
				dst->data[pos] = 0;
			}		
			
		}
	}
	return 1;
}

// Etiquetagem de blobs
// src		: Imagem bin�ria de entrada
// dst		: Imagem grayscale (ir� conter as etiquetas)
// nlabels	: Endere�o de mem�ria de uma vari�vel, onde ser� armazenado o n�mero de etiquetas encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas. � necess�rio libertar posteriormente esta mem�ria.
OVC* vc_binary_blob_labelling2_ze(IVC *src, IVC *dst, int *nlabels)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC *blobs; // Apontador para array de blobs (objectos) que ser� retornado desta fun��o.

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem bin�ria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pix�is de plano de fundo devem obrigat�riamente ter valor 0
	// Todos os pix�is de primeiro plano devem obrigat�riamente ter valor 255
	// Ser�o atribu�das etiquetas no intervalo [1,254]
	// Este algoritmo est� assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i<size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem bin�ria
	for (y = 0; y<height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x<width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A est� marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a<label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	//printf("\nMax Label = %d\n", label);

	// Contagem do n�mero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a<label - 1; a++)
	{
		for (b = a + 1; b<label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a<label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se n�o h� blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a<(*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}




int vc_binary_blob_info_ricardo(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta �rea de cada blob
	for (i = 0; i<nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

// #region BLOBS
int vc_binary_blob_info_raul(IVC *src, OVC *blobs, int nblobs)
	{
		unsigned char *data = (unsigned char *)src->data;
		int width = src->width;
		int height = src->height;
		int bytesperline = src->bytesperline;
		int channels = src->channels;
		int x, y, i;
		long int pos;
		int xmin, ymin, xmax, ymax;
		long int sumx, sumy;

		// Verifica��o de erros
		if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
		if (channels != 1) return 0;

		// Conta �rea de cada blob
		for (i = 0; i<nblobs; i++)
		{
			xmin = width - 1;
			ymin = height - 1;
			xmax = 0;
			ymax = 0;

			sumx = 0;
			sumy = 0;

			blobs[i].area = 0;

			for (y = 1; y<height - 1; y++)
			{
				for (x = 1; x<width - 1; x++)
				{
					pos = y * bytesperline + x * channels;

					if (data[pos] == blobs[i].label)
					{
						// �rea
						blobs[i].area++;

						// Centro de Gravidade
						sumx += x;
						sumy += y;

						// Bounding Box
						if (xmin > x) xmin = x;
						if (ymin > y) ymin = y;
						if (xmax < x) xmax = x;
						if (ymax < y) ymax = y;

						// Per�metro
						// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
						if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
						{
							blobs[i].perimeter++;
						}
					}
				}
			}

			// Bounding Box
			blobs[i].x = xmin;
			blobs[i].y = ymin;
			blobs[i].width = (xmax - xmin) + 1;
			blobs[i].height = (ymax - ymin) + 1;

			// Centro de Gravidade
			//blobs[i].xc = (xmax - xmin) / 2;
			//blobs[i].yc = (ymax - ymin) / 2;
			blobs[i].xc = sumx / MAX(blobs[i].area, 1);
			blobs[i].yc = sumy / MAX(blobs[i].area, 1);
		}

		return 1;
	}
// #endregion

int vc_binary_blob_info_ze(IVC *src, OVC *blobs, int nblobs)
{
	unsigned char *data = (unsigned char *)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta �rea de cada blob
	for (i = 0; i<nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y<height - 1; y++)
		{
			for (x = 1; x<width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// �rea
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Per�metro
					// Se pelo menos um dos quatro vizinhos n�o pertence ao mesmo label, ent�o � um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		//blobs[i].xc = (xmax - xmin) / 2;
		//blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: ANÁLISE DE HISTORGRAMAS DE IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Função que exiba o histograma de uma imagem em tons de cinzento.
int vc_gray_histogram_show_ricardo(IVC *src, IVC *dst){
	unsigned char *datasrc, *datadst;
	int x, y;
	long int pos_src, pos_dst;
	int histogram[256] = {0};
	int max_histogram = 0;

	if(src == NULL || dst == NULL) return 0;
	if(src->data == NULL || dst->data == NULL) return 0;
	if(src->width != dst->width || src->height != dst ->height) return 0;
	if(src->channels != 1 || dst->channels != 1) return 0;

	datasrc = (unsigned char *)src->data;
	datadst = (unsigned char *)dst->data;

	// Calcular o histograma
	for (y=0; y<src->height; y++)
	{
		for (x=0; x<src->width; x++)
		{
			pos_src = y * src->bytesperline + x * src->channels;
			histogram[datasrc[pos_src]]++;
		}
	}

	// Encontrar o valor máximo do histograma para normalização
	for (int i=0; i<256; i++)
	{
		if (histogram[i] > max_histogram)
		{
			max_histogram = histogram[i];
		}
	}

	// Exibir o histograma na imagem de destino
	for (y=0; y<dst->height; y++)
	{
		for (x=0; x<dst->width; x++)
		{
			pos_dst = y * dst->bytesperline + x * dst->channels;

			// Normalizar o valor do histograma para a altura da imagem
			int normalized_value = (histogram[x] * dst->height) / max_histogram;

			// Inverter a cor para exibir o histograma
			datadst[pos_dst] = (y < dst->height - normalized_value) ? 0 : 255;
		}
	}

	return 1;
}

int vc_gray_histogram_show_ze(IVC * src, IVC *dst)
{
	#define length 256
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int width = src->width;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int hist[length] = {0};
	int pos;
	int max = 0;
	int size = dst->height * dst->width * dst->channels;
	int numPixeis;

	//Preencher a imagem de destino com todos os pixeis a zero
	for(int i = 0; i < size; i++)
	{
		dst->data[i] = 0;
	}

	//Guardar número de pixeis no array
	for(int x = 0; x < width; x++)
	{
		for(int y = 0; y < height; y++)
		{
			pos = (y * bytesperline) + (x * channels);
			
			hist[datasrc[pos]]++;
		}
	}

	//Debug
	// for(int z = 0; z < length; z++)
	// {
	// 	printf("%d\n", hist[z]);
	// }

	//Ciclo para encontrar máximo
	for(int m = 0; m < length; m++)
	{		
		if(hist[m] > max) max = hist[m]; 		
	}

	// printf("%d", max);

	//Ciclo para escrever os pixeis brancos na imagem de destino
	//Percorrer o array hist[]
	for(int posArr = 0; posArr <= length - 1; posArr++)
	{
		//Validar se a posição do array é diferente de 0
		if(hist[posArr] != 0)
		{
			//Transformar o numero de pixeis na escala de 256
			numPixeis = (hist[posArr] * dst->height) / max;

			int xDst = posArr;
			for(int yDst = dst->height - 1; yDst >= dst->height - numPixeis; yDst--)
			{
				int posDst = yDst * bytesperline + xDst * channels;
				dst->data[posDst] = 255;
			}
		}		
	}	

	return 1;
}


//Função que realize a equalização de imagens em tons de cinzento.

int vc_gray_histogram_equalization_ricardo(IVC *src, IVC *dst){
	unsigned char *datasrc, *datadst;
	int x, y;
	long int pos_src, pos_dst;
	int histogram[256] = {0};
	int cdf[256] = {0};
	int total_pixels = src->width * src->height;

	if(src == NULL || dst == NULL) return 0;
	if(src->data == NULL || dst->data == NULL) return 0;
	if(src->width != dst->width || src->height != dst ->height) return 0;
	if(src->channels != 1 || dst->channels != 1) return 0;

	datasrc = (unsigned char *)src->data;
	datadst = (unsigned char *)dst->data;

	// Calcular o histograma
	for (y=0; y<src->height; y++)
	{
		for (x=0; x<src->width; x++)
		{
			pos_src = y * src->bytesperline + x * src->channels;
			histogram[datasrc[pos_src]]++;
		}
	}

	// Calcular a função de distribuição acumulada (CDF) normalizada para 0..1
	float cdf_norm[256] = {0.0f};
	cdf_norm[0] = histogram[0] / (float) total_pixels;
	for (int i=1; i<256; i++)
	{
		cdf_norm[i] = cdf_norm[i-1] + histogram[i] / (float) total_pixels;
	}

	// Equalizar a imagem usando a CDF normalizada
	for (y=0; y<src->height; y++)
	{
		for (x=0; x<src->width; x++)
		{
			pos_src = y * src->bytesperline + x * src->channels;
			pos_dst = y * dst->bytesperline + x * dst->channels;
			
			datadst[pos_dst] = (unsigned char)((cdf_norm[datasrc[pos_src]] - cdf_norm[0]) * 255.0f / (1.0f - cdf_norm[0]));
		}
	}

	return 1;
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: DETEÇÃO DE CONTORNOS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//Função que calcule os contornos de uma imagem em tons de cinzento, utilizando os operadores de Prewitt.
int vc_gray_edge_prewitt_ricardo(IVC *src, IVC *dst, float th){
	unsigned char *datasrc, *datadst;
	int x, y;
	long int pos_src, pos_dst;

	if(src == NULL || dst == NULL) return 0;
	if(src->data == NULL || dst->data == NULL) return 0;
	if(src->width != dst->width || src->height != dst->height) return 0;
	if(src->channels != 1 || dst->channels != 1) return 0;

	datasrc = (unsigned char *)src->data;
	datadst = (unsigned char *)dst->data;

	// Operadores de Prewitt
	int Gx[3][3] = {
		{-1, 0, 1},
		{-1, 0, 1},
		{-1, 0, 1}
	};

	int Gy[3][3] = {
		{-1, -1, -1},
		{0, 0, 0},
		{1, 1, 1}
	};

	// Valor máximo teórico da magnitude
	float max_magnitude = sqrtf(255.0f * 255.0f + 255.0f * 255.0f);

	// Converter threshold de [0,1] para valor real
	float threshold = th * max_magnitude;

	for (y = 1; y < src->height - 1; y++)
	{
		for (x = 1; x < src->width - 1; x++)
		{
			float sumX = 0.0f;
			float sumY = 0.0f;

			for (int j = -1; j <= 1; j++)
			{
				for (int i = -1; i <= 1; i++)
				{
					pos_src = (y + j) * src->bytesperline + (x + i) * src->channels;
					sumX += datasrc[pos_src] * Gx[j + 1][i + 1];
					sumY += datasrc[pos_src] * Gy[j + 1][i + 1];
				}
			}

			sumX = sumX / 3.0f; // Normalização para manter a magnitude dentro do intervalo [0, 255]
			sumY = sumY / 3.0f; // Normalização para manter a magnitude dentro do intervalo [0, 255]

			float magnitude = sqrtf(sumX * sumX + sumY * sumY)/sqrtf(2.0f); // Normalização para manter a magnitude dentro do intervalo [0, 255]

			pos_dst = y * dst->bytesperline + x * dst->channels;

			datadst[pos_dst] = (magnitude >= threshold) ? 255 : 0;
		}
	}

	return 1;
}

int vc_gray_edge_prewitt_prof(IVC *src, IVC *dst, float th) // th = [0.001, 1.000]
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y;
	long int posX, posA, posB, posC, posD, posE, posF, posG, posH;
	int i, size;
	float histmax;
	int histthreshold;
	int sumx, sumy;
	float hist[256] = { 0.0f };

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return 0;
	if (channels != 1) return 0;

	size = width * height;

	for (y = 1; y<height - 1; y++)
	{
		for (x = 1; x<width - 1; x++)
		{
			// PosA PosB PosC
			// PosD PosX PosE
			// PosF PosG PosH

			posA = (y - 1) * bytesperline + (x - 1) * channels;
			posB = (y - 1) * bytesperline + x * channels;
			posC = (y - 1) * bytesperline + (x + 1) * channels;
			posD = y * bytesperline + (x - 1) * channels;
			posX = y * bytesperline + x * channels;
			posE = y * bytesperline + (x + 1) * channels;
			posF = (y + 1) * bytesperline + (x - 1) * channels;
			posG = (y + 1) * bytesperline + x * channels;
			posH = (y + 1) * bytesperline + (x + 1) * channels;

			// PosA*(-1) PosB*0 PosC*(1)
			// PosD*(-1) PosX*0 PosE*(1)
			// PosF*(-1) PosG*0 PosH*(1)

			sumx = datasrc[posA] * -1;
			sumx += datasrc[posD] * -1;
			sumx += datasrc[posF] * -1;

			sumx += datasrc[posC] * +1;
			sumx += datasrc[posE] * +1;
			sumx += datasrc[posH] * +1;
			sumx = sumx / 3; // 3 = 1 + 1 + 1

			// PosA*(-1) PosB*(-1) PosC*(-1)
			// PosD*0    PosX*0    PosE*0
			// PosF*(1)  PosG*(1)  PosH*(1)

			sumy = datasrc[posA] * -1;
			sumy += datasrc[posB] * -1;
			sumy += datasrc[posC] * -1;

			sumy += datasrc[posF] * +1;
			sumy += datasrc[posG] * +1;
			sumy += datasrc[posH] * +1;
			sumy = sumy / 3; // 3 = 1 + 1 + 1

			//datadst[posX] = (unsigned char)sqrt((double)(sumx*sumx + sumy*sumy));
			datadst[posX] = (unsigned char) (sqrt((double) (sumx*sumx + sumy*sumy)) / sqrt(2.0));
			// Explicação:
			// Queremos que no caso do pior cenário, em que sumx = sumy = 255, o resultado
			// da operação se mantenha no intervalo de valores admitido, isto é, entre [0, 255].
			// Se se considerar que:
			// max = 255
			// Então,
			// sqrt(pow(max,2) + pow(max,2)) * k = max <=> sqrt(2*pow(max,2)) * k = max <=> k = max / (sqrt(2) * max) <=> 
			// k = 1 / sqrt(2)
		}
	}

	// Calcular o histograma com o valor das magnitudes
	for (i = 0; i < size; i++)
	{
		hist[datadst[i]]++;
	}

	// Definir o threshold.
	// O threshold é definido pelo nível de intensidade (das magnitudes)
	// quando se atinge uma determinada percentagem de pixeis, definida pelo utilizador.
	// Por exemplo, se o parâmetro 'th' tiver valor 0.8, significa the o threshold será o 
	// nível de magnitude, abaixo do qual estão pelo menos 80% dos pixeis.
	histmax = 0.0f;
	for (i = 0; i <= 255; i++)
	{
		histmax += hist[i];

		// th = Prewitt Threshold
		if (histmax >= (((float)size) * th)) break;
	}
	histthreshold = i == 0 ? 1 : i;

	// Aplicada o threshold
	for (i = 0; i < size; i++)
	{
		if (datadst[i] >= (unsigned char) histthreshold) datadst[i] = 255;
		else datadst[i] = 0;
	}

	return 1;
}



//Função que calcule os contornos de uma imagem em tons de cinzento, utilizando os operadores de Sobel.





// Exercícios:
// • Construa a seguinte função:
// int vc_gray_highpass_filter(IVC *src, IVC *dst);
// • O filtro Laplaciano pode ser utilizado para melhorar a qualidade de uma
// imagem. Construa uma nova função que implementa o filtro Laplaciano e
// soma o resultado à imagem original. Ao realizar esta operação tenha em
// conta os limites de valores que um pixel pode tomar, isto é, o intervalo
// [0, 255]. Faça com que seja aplicado um ganho (um valor inteiro) que
// será aplicado (multiplicado) ao resultado do filtro, de modo a controlar o
// efeito pretendido.
// int vc_gray_highpass_filter_enhance(IVC *src, IVC *dst, int gain);






//VC10 - Gray Low Pass Mean Filter
int vc_gray_lowpass_mean_filter_ricardo(IVC *src, IVC *dst, int kernelsize)
{
    unsigned char *datasrc;
    unsigned char *datadst;
    int bytesperline_src = src->bytesperline;
    int bytesperline_dst = dst->bytesperline;
    int channels_src = src->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;

    int x, y, kx, ky;
    long int pos_src, pos_dst;
    long int sum;
    int offset;
    int area;

    // Validação básica
    if ((src == NULL) || (dst == NULL)) return 0;
    if ((src->data == NULL) || (dst->data == NULL)) return 0;
    if ((width <= 0) || (height <= 0)) return 0;
    if ((dst->width != width) || (dst->height != height)) return 0;
    if (channels_src != 1 || channels_dst != 1) return 0;
    if (kernelsize < 1 || (kernelsize % 2) == 0) return 0;

	datasrc = (unsigned char *)src->data;
	datadst = (unsigned char *)dst->data;

    offset = kernelsize / 2;
    area = kernelsize * kernelsize;

    // Opcional: inicializar dst a 0
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos_dst = y * bytesperline_dst + x * channels_dst;
            datadst[pos_dst] = 0;
        }
    }

    // Aplicação do filtro apenas na região válida
    for (y = offset; y < (height - offset); y++)
    {
        for (x = offset; x < (width - offset); x++)
        {
            sum = 0;

            for (ky = -offset; ky <= offset; ky++)
            {
                for (kx = -offset; kx <= offset; kx++)
                {
                    pos_src = (y + ky) * bytesperline_src + (x + kx) * channels_src;
                    sum += datasrc[pos_src];
                }
            }

            pos_dst = y * bytesperline_dst + x * channels_dst;
            datadst[pos_dst] = (unsigned char)(sum / area);
        }
    }

    return 1;
}

int vc_gray_lowpass_mean_filter_raul(IVC *src, IVC *dst, int kernelsize) {
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int width = src->width;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (kernelsize - 1) / 2;
	long int pos = 0;
	long int posOffset = 0;

	for (int x = 0; x < width; x++) {
		
		for (int y = 0; y < height; y++) {
			pos = y * bytesperline + x * channels;

			int sum = 0;
			int count = 0;

			for(int ix = -offset; ix <= offset; ix++){
				for(int iy = -offset; iy <= offset; iy++){
					if((x + ix) >= src->width || (y + iy) >= src->height || (x + ix) < 0 || (y + iy) < 0) continue;

					posOffset = ((y + iy) * bytesperline) + ((x + ix) * channels);
					sum += datasrc[posOffset];
					count++;
				}
			}

			datadst[pos] = sum / count;
		}
	}
	return 1;
}

int vc_gray_lowpass_mean_filter_ze(IVC *src, IVC *dst, int kernelsize) {
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int width = src->width;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (kernelsize - 1) / 2;
	long int pos = 0;
	long int posOffset = 0;

	for (int x = 0; x < width; x++) {
		
		for (int y = 0; y < height; y++) {
			pos = y * bytesperline + x * channels;

			int sum = 0;
			int count = 0;

			for(int ix = -offset; ix <= offset; ix++){
				for(int iy = -offset; iy <= offset; iy++){
					if((x + ix) >= src->width || (y + iy) >= src->height || (x + ix) < 0 || (y + iy) < 0) continue;

					posOffset = ((y + iy) * bytesperline) + ((x + ix) * channels);
					sum += datasrc[posOffset];
					count++;
				}
			}

			datadst[pos] = sum / count;
		}
	}
}


//VC10 - Gray Low Pass Median Filter
int vc_gray_lowpass_median_filter_ricardo(IVC *src, IVC *dst, int kernelsize)
{
    unsigned char *datasrc = (unsigned char *)src->data;
    unsigned char *datadst = (unsigned char *)dst->data;
    int bytesperline_src = src->bytesperline;
    int bytesperline_dst = dst->bytesperline;
    int channels_src = src->channels;
    int channels_dst = dst->channels;
    int width = src->width;
    int height = src->height;

    int x, y, kx, ky, i, j;
    long int pos_src, pos_dst;
    int offset;
    int area;
    unsigned char *window;
    unsigned char temp;

    // Validação básica
    if ((src == NULL) || (dst == NULL)) return 0;
    if ((src->data == NULL) || (dst->data == NULL)) return 0;
    if ((width <= 0) || (height <= 0)) return 0;
    if ((dst->width != width) || (dst->height != height)) return 0;
    if (channels_src != 1 || channels_dst != 1) return 0;
    if (kernelsize < 1 || (kernelsize % 2) == 0) return 0;

    offset = kernelsize / 2;
    area = kernelsize * kernelsize;

    // Aloca memória para armazenar os valores da vizinhança
    window = (unsigned char *)malloc(sizeof(unsigned char) * area);
    if (window == NULL) return 0;

    // Inicializar dst a 0
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pos_dst = y * bytesperline_dst + x * channels_dst;
            datadst[pos_dst] = 0;
        }
    }

    // Aplicar filtro de mediana na região válida
    for (y = offset; y < (height - offset); y++)
    {
        for (x = offset; x < (width - offset); x++)
        {
            i = 0;

            // Recolher os valores da vizinhança
            for (ky = -offset; ky <= offset; ky++)
            {
                for (kx = -offset; kx <= offset; kx++)
                {
                    pos_src = (y + ky) * bytesperline_src + (x + kx) * channels_src;
                    window[i++] = datasrc[pos_src];
                }
            }

            // Ordenação simples (bubble sort / exchange sort)
            for (i = 0; i < area - 1; i++)
            {
                for (j = i + 1; j < area; j++)
                {
                    if (window[j] < window[i])
                    {
                        temp = window[i];
                        window[i] = window[j];
                        window[j] = temp;
                    }
                }
            }

            // Mediana = elemento central do vetor ordenado
            pos_dst = y * bytesperline_dst + x * channels_dst;
            datadst[pos_dst] = window[area / 2];
        }
    }

    free(window);
    return 1;
}

int vc_gray_lowpass_median_filter_raul(IVC *src, IVC *dst, int kernelsize) {
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int width = src->width;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (kernelsize - 1) / 2;
	long int pos = 0;
	long int posOffset = 0;

	for (int x = 0; x < width; x++) {
		
		for (int y = 0; y < height; y++) {
			pos = y * bytesperline + x * channels;

			int values[kernelsize * kernelsize];
			int count = 0;

			for(int ix = -offset; ix <= offset; ix++){
				for(int iy = -offset; iy <= offset; iy++){
					if((x + ix) >= src->width || (y + iy) >= src->height || (x + ix) < 0 || (y + iy) < 0) continue;

					posOffset = ((y + iy) * bytesperline) + ((x + ix) * channels);
					values[count] = datasrc[posOffset];
					count++;
				}
			}

			// Ordenar os valores para encontrar a mediana
			for(int i = 0; i < count - 1; i++){
				for(int j = 0; j < count - i - 1; j++){
					if(values[j] > values[j + 1]){
						int temp = values[j];
						values[j] = values[j + 1];
						values[j + 1] = temp;
					}
				}
			}

			datadst[pos] = values[(int)floorf(count / 2)]; // Mediana
		}
	}
	return 1;
}

int vc_gray_lowpass_median_filter_ze(IVC *src, IVC *dst, int kernelsize) {
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int width = src->width;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (kernelsize - 1) / 2;
	long int pos = 0;
	long int posOffset = 0;

	for (int x = 0; x < width; x++) {
		
		for (int y = 0; y < height; y++) {
			pos = y * bytesperline + x * channels;

			int values[kernelsize * kernelsize];
			int count = 0;

			for(int ix = -offset; ix <= offset; ix++){
				for(int iy = -offset; iy <= offset; iy++){
					if((x + ix) >= src->width || (y + iy) >= src->height || (x + ix) < 0 || (y + iy) < 0) continue;

					posOffset = ((y + iy) * bytesperline) + ((x + ix) * channels);
					values[count] = datasrc[posOffset];
					count++;
				}
			}

			// Ordenar os valores para encontrar a mediana
			for(int i = 0; i < count - 1; i++){
				for(int j = 0; j < count - i - 1; j++){
					if(values[j] > values[j + 1]){
						int temp = values[j];
						values[j] = values[j + 1];
						values[j + 1] = temp;
					}
				}
			}

			datadst[pos] = values[(int)(count / 2)]; // Mediana
		}
	}
}

//VC 10 - Filtro passa baixo Gaussiano
int vc_gray_lowpass_gaussian_filter_ricardo(IVC *src, IVC *dst){
	unsigned char *datasrc;
	unsigned char *datadst;

    int x, y,kx,ky;
    long int pos,posX;
    float sum;
    int pixel;
    int offset = 2;

    float kernel[5] = {0.054f, 0.242f, 0.399f, 0.242f, 0.054f};

    // Verificação de erros
    if ((src == NULL) || (dst == NULL)) return 0;
    if ((src->data == NULL) || (dst->data == NULL)) return 0;
    if ((src->width <= 0) || (src->height <= 0)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;

	datadst = (unsigned char *)dst->data;
	datasrc = (unsigned char *)src->data;

	for(y=0; y<src->height; y++){
		for(x=0; x<src->height; x++){
			sum = 0.0f;
			for(ky=-offset;ky<=offset;ky++){
				for(kx=-offset;kx<=offset;kx++){
					pos = (y + ky)*src->bytesperline + (x + kx)*src->channels;
					if((x + kx) >= src->width || (y + ky) >= src->height || (x + kx) < 0 || (y + ky) < 0) continue;
					sum += ((float)datasrc[pos]) * kernel[kx + offset] * kernel[ky + offset];
				}
			}
			posX= y*dst->bytesperline + x*dst->channels;
			datadst[posX] = (unsigned char)sum;
		}
	}
}

int vc_gray_lowpass_gaussian_filter_raul(IVC *src, IVC *dst){
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int width = src->width;
	int bytesperline = src->bytesperline;
	int channels = src->channels;

	// Kernel Gaussiano 3x3
	int kernel[3][3] = {
		{1, 2, 1},
		{2, 4, 2},
		{1, 2, 1}
	};

	for (int x = 0; x < width; x++) {
		
		for (int y = 0; y < height; y++) {
			long int pos = y * bytesperline + x * channels;

			int sum = 0;
			int weightSum = 0;

			for(int ix = -1; ix <= 1; ix++){
				for(int iy = -1; iy <= 1; iy++){
					if((x + ix) >= src->width || (y + iy) >= src->height || (x + ix) < 0 || (y + iy) < 0) continue;

					long int posOffset = ((y + iy) * bytesperline) + ((x + ix) * channels);
					int weight = kernel[iy + 1][ix + 1];
					sum += datasrc[posOffset] * weight;
					weightSum += weight;
				}
			}

			datadst[pos] = sum / weightSum;
		}
	}
	return 1;

}

int vc_gray_lowpass_gaussian_ze(IVC *src, IVC *dst)
{
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int width = src->width;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = (5 - 1) / 2;
	int y, x, kx, ky;
	float gaussum;
	float gaussArr[5] = {(float) 0.054, (float) 0.242, (float) 0.399, (float) 0.242, (float) 0.054}; 	
	long int posOffset = 0;	
	long int pos = 0;

	for(y = offset; y < height - offset; y++)
	{
		for(x = offset; x < width - offset; x++)
		{			
			gaussum = 0.0;

			for(kx = -offset; kx <= offset; kx++){
				for(ky = -offset; ky <= offset; ky++){
					posOffset = ((y - ky) * bytesperline) + ((x - kx) * channels);

					gaussum += ((float)datasrc[posOffset] * gaussArr[kx + offset] * gaussArr[ky + offset]);
				}
			}

			pos = y * dst->bytesperline + x * dst->channels;
			datadst[pos] = (unsigned char) gaussum;
		}
	}
}





//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: EXTRAS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Ricardo-----------------------------------------------------------------------------
int segmentar_cerebro_preto(IVC *src, IVC *dst, int *total)
{
    IVC *hsv = NULL;
    int x, y;
    long int pos_src, pos_dst, pos_hsv;
    int S, V;
    int count = 0;

    if(src == NULL || dst == NULL || total == NULL) return 0;
    if(src->width != dst->width || src->height != dst->height) return 0;
    if(src->channels != 3 || dst->channels != 3) return 0;

    hsv = vc_image_new(src->width, src->height, 3, 255);
    if(hsv == NULL) return 0;

    if(vc_rgb_to_hsv(src, hsv) == 0)
    {
        vc_image_free(hsv);
        return 0;
    }

    for(y = 0; y < src->height; y++)
    {
        for(x = 0; x < src->width; x++)
        {
            pos_src = y * src->bytesperline + x * src->channels;
            pos_dst = y * dst->bytesperline + x * dst->channels;
            pos_hsv = y * hsv->bytesperline + x * hsv->channels;

            S = hsv->data[pos_hsv + 1];
            V = hsv->data[pos_hsv + 2];

            /* Fundo preto: S entre 0 e 50% e V entre 0 e 30% */
            if(S <= 127 && V <= 76)
            {
                dst->data[pos_dst]     = 255;
                dst->data[pos_dst + 1] = 255;
                dst->data[pos_dst + 2] = 0;
            }
            else
            {
                /* Cérebro */
                dst->data[pos_dst]     = 0;
                dst->data[pos_dst + 1] = 0;
                dst->data[pos_dst + 2] = 0;
                count++;
            }
        }
    }

    *total = count;

    vc_image_free(hsv);
    return 1;
}

int analisar_pet(IVC *src, IVC *seg, int total_cerebro)
{
    IVC *hsv = NULL;
    int x, y;
    long int pos_seg, pos_hsv;
    int vermelho = 0, amarelo = 0, verde = 0, azul = 0;
    int H, S, V;
    int rs, gs, bs;

    if(src == NULL || seg == NULL) return 0;
    if(src->channels != 3 || seg->channels != 3) return 0;
    if(src->width != seg->width || src->height != seg->height) return 0;
    if(total_cerebro <= 0) return 0;

    hsv = vc_image_new(src->width, src->height, 3, 255);
    if(hsv == NULL) return 0;

    if(vc_rgb_to_hsv(src, hsv) == 0)
    {
        vc_image_free(hsv);
        return 0;
    }

    for(y = 0; y < src->height; y++)
    {
        for(x = 0; x < src->width; x++)
        {
            pos_seg = y * seg->bytesperline + x * seg->channels;
            pos_hsv = y * hsv->bytesperline + x * hsv->channels;

            rs = seg->data[pos_seg];
            gs = seg->data[pos_seg + 1];
            bs = seg->data[pos_seg + 2];

            /* Só contar píxeis do cérebro */
            if(!(rs == 0 && gs == 0 && bs == 0))
                continue;

            H = hsv->data[pos_hsv];
            S = hsv->data[pos_hsv + 1];
            V = hsv->data[pos_hsv + 2];

            /* Só analisar pixels com cor forte */
            if(S < 127 || V < 127)
                continue;

            /* Vermelho: 0-45 ou 291-360 */
            if((H >= 0 && H <= 32) || (H >= 206 && H <= 255))
            {
                vermelho++;
            }
            /* Amarelo: 46-70 */
            else if(H >= 33 && H <= 50)
            {
                amarelo++;
            }
            /* Verde: 71-160 */
            else if(H >= 51 && H <= 113)
            {
                verde++;
            }
        }
    }

    /* Azul = resto */
    azul = total_cerebro - vermelho - amarelo - verde;

    printf("Numero de pixeis vermelhos: %d\n", vermelho);
    printf("Numero de pixeis amarelos: %d\n", amarelo);
    printf("Numero de pixeis verdes: %d\n", verde);
    printf("Numero de pixeis azuis: %d\n", azul);
    printf("Numero de pixeis do cerebro: %d\n", total_cerebro);

    printf("0 a 25%%   : %.2f%%\n", (azul * 100.0) / total_cerebro);
    printf("26 a 50%%  : %.2f%%\n", (verde * 100.0) / total_cerebro);
    printf("51 a 75%%  : %.2f%%\n", (amarelo * 100.0) / total_cerebro);
    printf("76 a 100%% : %.2f%%\n", (vermelho * 100.0) / total_cerebro);

    vc_image_free(hsv);
    return 1;
}

//Função para calcular a diferença entre duas imagens binárias (usada para operações morfológicas)
int vc_binary_difference(IVC *src1, IVC *src2, IVC *dst)
{
    unsigned char *data1, *data2, *datadst;
    int i;
    int pos1, pos2, pos_dst;

    if(src1 == NULL || src2 == NULL || dst == NULL) return 0;
    if(src1->data == NULL || src2->data == NULL || dst->data == NULL) return 0;
    if(src1->width != src2->width || src1->height != src2->height ||
       src1->width != dst->width || src1->height != dst->height) return 0;
    if(src1->channels != 1 || src2->channels != 1 || dst->channels != 1) return 0;

    data1 = (unsigned char *)src1->data;
    data2 = (unsigned char *)src2->data;
    datadst = (unsigned char *)dst->data;

    int length = dst->height * dst->width * dst->channels;

    for (i = 0; i < length; i++)
    {
        datadst[i] = (data1[i] == 255 && data2[i] == 0) ? 255 : 0;
    }
    return 1;
}

int vc_erode_minus_dilate(IVC* srcEroded, IVC* srcDilated, IVC* dst){

	int length = dst->height * dst->width * dst->channels;

	for(int i = 0; i < length; i++){

		dst->data[i] = srcEroded->data[i] - srcDilated->data[i];
	}

	return 1;
}

//Função que verifica da mask for branco, mantém o pixel original, caso contrário, torna preto (usada para operações morfológicas)
int vc_apply_mask(IVC* src, IVC* mask, IVC* dst){

    unsigned char* data_src = (unsigned char*)src->data;
    unsigned char* data_mask = (unsigned char*)mask->data;
    unsigned char* data_dst = (unsigned char*)dst->data;

    int length = dst->height * dst->width * dst->channels;

    for(int i = 0; i < length; i++){
        dst->data[i]=0;
        data_dst[i] = (data_mask[i]==255) ? data_src[i] : 0;
    }

    return 1;
}



//Raul--------------------------------------------------------------------------------
// Não completo
int vc_binary_dilate_gray(IVC *src, IVC *dst, int kernel) {
	unsigned char *datasrc = (unsigned char *)src->data;
	unsigned char *datadst = (unsigned char *)dst->data;
	int height = src->height;
	int width = src->width;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int offset = 0;
	long int pos = 0;
	long int posOffset = 0;

	// Check if the kernel is valid
	if (kernel % 2 == 0 || kernel <= 0) {
		printf("ERROR -> The kernel needs to be a positive odd number!\n");
        getchar();
        return 0;
	}

	offset = (kernel - 1) / 2;

	printf("%d", offset);

	for (int x = 0; x < width; x++) {
		
		for (int y = 0; y < height; y++) {
			pos = y * bytesperline + x * channels;

			dst->data[pos] = src->data[pos];
			
			for (int ix = -offset; ix <= offset; ix++) {
				for (int iy = -offset; iy <= offset; iy++) {
					if((x + ix) >= src->width || (y + iy) >= src->height || (x + ix) < 0 || (y + iy) < 0) continue;
					
					posOffset = (y + iy) * bytesperline + (x + ix) * channels;

					if(src->data[posOffset] == 255) {
						dst->data[pos] = 255;
						dst->data[pos + 1] = 255;
						dst->data[pos + 2] = 255;
						break;
					}
				}
				
				if(dst->data[pos] == 255) {
					break;
				}
			}
		}
	}

	return 1;
}














