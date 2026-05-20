/*
===============================================================================
FICHEIRO: tp_classification.c

DESCRIÇÃO:
Este ficheiro contém a implementação das regras de classificação dos objetos
detetados, com base nas medidas obtidas anteriormente.

OBJETIVO:
Atribuir a cada laranja uma categoria ou classe, por exemplo calibre comercial,
com base nos critérios definidos no enunciado e/ou no regulamento de referência.

O QUE DEVE INCLUIR:
- Regras de decisão baseadas em medidas geométricas.
- Funções que convertam diâmetros ou escalas em calibres.
- Mapeamento entre valores medidos e categorias finais.
- Eventual validação dos limites definidos para cada classe.

O QUE NÃO DEVE INCLUIR:
- Segmentação da imagem.
- Operações morfológicas.
- Tracking entre frames.
- Lógica de leitura do vídeo.

PORQUE EXISTE:
A classificação é uma fase conceptual diferente da segmentação e da medição.
Ter este módulo separado torna o código mais claro, mais fácil de testar e
permite alterar regras de decisão sem mexer no resto do pipeline.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Usa os resultados produzidos por tp_measurements.c.
- Pode recorrer a constantes ou tabelas definidas a partir do regulamento.
- É chamado a partir do fluxo principal em main.cpp.

NOTAS:
Este módulo deve conter apenas a lógica de decisão e categorização, não a
extração das medidas em si.
===============================================================================
*/
#include <stdlib.h>
#include "../include/tp_utils.h"
#include "../include/tp_classification.h"
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

OVC *vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels)
{

	int channels = src->channels;
	int bytesPerLine = src->bytesperline;
	int height = src->height;
	int width = src->width;
	int label = 1;
	OVC *blobs;

	// Fix do chatgpt
	int labelTable[256];
	for (int i = 0; i < 256; i++)
	{
		labelTable[i] = i;
	}

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int pos = y * bytesPerLine + x * channels;

			if (src->data[pos])
			{
				int neighbors[4][2] = {{x - 1, y - 1}, {x, y - 1}, {x + 1, y - 1}, {x - 1, y}};
				int actualLabel = 255;
				int count = 0; // contar vizinhos válidos - fix do chatgpt

				for (int i = 0; i < 4; i++)
				{
					int kx = neighbors[i][0];
					int ky = neighbors[i][1];

					if (kx < 0 || ky < 0 || kx >= width || ky >= height)
						continue;

					int kpos = ky * bytesPerLine + kx * channels;

					if (dst->data[kpos])
					{
						count++;

						if (dst->data[kpos] < actualLabel)
						{
							actualLabel = dst->data[kpos];
						}
					}
				}

				if (actualLabel != 255)
				{
					dst->data[pos] = actualLabel;

					// só unir se houver mais que 1 vizinho - fix do chatgpt
					if (count > 1)
					{
						for (int i = 0; i < 4; i++)
						{
							int kx = neighbors[i][0];
							int ky = neighbors[i][1];

							if (kx < 0 || ky < 0 || kx >= width || ky >= height)
								continue;

							int kpos = ky * bytesPerLine + kx * channels;
							int neighborLabel = dst->data[kpos];

							if (neighborLabel != 0 && neighborLabel != actualLabel)
							{
								int a = neighborLabel;
								int b = actualLabel;

								while (labelTable[a] != a)
									a = labelTable[a];

								while (labelTable[b] != b)
									b = labelTable[b];

								if (a != b)
									labelTable[a] = b;
							}
						}
					}
				}
				else
				{
					dst->data[pos] = label;
					labelTable[label] = label;
					label++;
				}
			}
			else
			{
				dst->data[pos] = 0;
			}
		}
	}

	// segunda passagem
	for (int y = height - 1; y >= 0; y--)
	{
		for (int x = width - 1; x >= 0; x--)
		{
			int pos = y * bytesPerLine + x * channels;

			if (dst->data[pos])
			{
				int i = dst->data[pos];

				while (labelTable[i] != i)
				{
					i = labelTable[i];
				}

				dst->data[pos] = i; // * 5; // para visualização
			}
		}
	}

	for (int a = 1; a < label - 1; a++)
	{
		for (int b = a + 1; b < label; b++)
		{
			if (labelTable[a] == labelTable[b])
				labelTable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (int a = 1; a < label; a++)
	{
		if (labelTable[a] != 0)
		{
			labelTable[*nlabels] = labelTable[a]; // Organiza tabela de etiquetas
			(*nlabels)++;						  // Conta etiquetas
		}
	}

	// Se n�o h� blobs
	if (*nlabels == 0)
		return NULL;

	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC *)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (int a = 0; a < (*nlabels); a++)
			blobs[a].label = labelTable[a];
	}
	else
		return NULL;

	return blobs;
}

int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs)
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
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if (channels != 1)
		return 0;

	// Conta �rea de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
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
					if (xmin > x)
						xmin = x;
					if (ymin > y)
						ymin = y;
					if (xmax < x)
						xmax = x;
					if (ymax < y)
						ymax = y;

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
		blobs[i].xc = (xmax - xmin) / 2;
		blobs[i].yc = (ymax - ymin) / 2;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

int vc_draw_bounding_box_all_blobs(IVC* srcdst, OVC* blobs, int nlabels, int padding, int thickness, int colorR, int colorG, int colorB){

	if(!srcdst || !blobs) return 0;

	for(int i = 0; i < nlabels; i++){

		int xMin = blobs[i].x - padding;
		int yMin = blobs[i].y + padding;
		int xMax = blobs[i].x + blobs[i].width + padding;
		int yMax = blobs[i].y + blobs[i].height + padding;

		for(int y = 0; y < thickness; y++){

			int yTop = yMin + y;
			int yBottom = yMax - y;

			for(int x = xMin; x <= xMax; x++){

				if (x >= 0 && x < srcdst->width){

					if(yTop >= 0 && yTop < srcdst->height) {

						int pos = yTop * srcdst->bytesperline + x * srcdst->channels;
						srcdst->data[pos] = colorR;
						srcdst->data[pos + 1] = colorG;
						srcdst->data[pos + 2] = colorB;
					}

					if (yBottom >= 0 && yBottom < srcdst->height) {
                        int pos = yBottom * srcdst->bytesperline + x * srcdst->channels;
                        srcdst->data[pos] = colorR;
                        srcdst->data[pos + 1] = colorG;
                        srcdst->data[pos + 2] = colorB;
                    }
				}				
            }
        }

        for (int x = 0; x < thickness; x++) {

            int xLeft = xMin + x;
            int xRight = xMax - x;

            for (int y = yMin; y <= yMax; y++) {

                if (y >= 0 && y < srcdst->height) {
                
                    if (xLeft >= 0 && xLeft < srcdst->width) {
                        int pos = y * srcdst->bytesperline + xLeft * srcdst->channels;
                        srcdst->data[pos] = colorR;
                        srcdst->data[pos + 1] = colorG;
                        srcdst->data[pos + 2] = colorB;
                    }

                    if (xRight >= 0 && xRight < srcdst->width) {
                        int pos = y * srcdst->bytesperline + xRight * srcdst->channels;
                        srcdst->data[pos] = colorR;
                        srcdst->data[pos + 1] = colorG;
                        srcdst->data[pos + 2] = colorB;
                    }
                }
            }
        }
    }

    return 1;
}

int vc_draw_center_mass_all_blobs(IVC* srcdst, OVC* blobs, int nlabels, int kernel, int thickness, int colorR, int colorG, int colorB){

	if(!srcdst || !blobs) return 0;

	for (int i = 0; i < nlabels; i++){

        int xc = blobs[i].xc;
        int yc = blobs[i].yc;
		int offset = (kernel - 1) / 2;
		int thickOffset = thickness / 2;

        for(int y = yc - offset; y <= yc + offset; y++){

			if(y < 0 || y >= srcdst->height) continue;

            for(int x = xc - offset; x <= xc + offset; x++){

				if(x < 0 || x >= srcdst->width) continue;

                if((x < xc - thickOffset || x > xc + thickOffset) && (y < yc - thickOffset || y > yc + thickOffset)) continue;
                int pos = y * srcdst->bytesperline + x * srcdst->channels;
				
                srcdst->data[pos] = colorR;
				srcdst->data[pos + 1] = colorG;
				srcdst->data[pos + 2] = colorB;
            }
        }
    }

	return 1;
}