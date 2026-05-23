/*
===============================================================================
FICHEIRO: tp_visualization.c

DESCRICAO:
Este ficheiro contem funcoes de desenho usadas na visualizacao dos resultados.

OBJETIVO:
Desenhar elementos como bounding boxes e centros de massa sobre a imagem.
===============================================================================
*/
#include "../include/tp_visualization.h"

/**
 * @brief Desenha caixas delimitadoras dos blobs diretamente sobre a imagem.
 *
 * A funcao usa centro, largura e altura do blob para desenhar os limites em RGB.
 */
int vc_draw_bounding_box_all_blobs(IVC* srcdst, OVC* blobs, int nlabels, int padding, int thickness, int colorR, int colorG, int colorB)
{
	for(int i = 0; i < nlabels; i++)
	{
		int height = blobs[i].height;
		int width = blobs[i].width;
		int xc = blobs[i].xc;
		int yc = blobs[i].yc;

		for(int y = yc - (height / 2) - padding; y <= yc + (height / 2) + padding; y++)
		{
			for(int x = xc - (width / 2) - padding; x <= xc + (width / 2) + padding; x++)
			{
				int pos = y * srcdst->bytesperline + x * srcdst->channels;

				if (x - padding < 0 || x + padding >= srcdst->width || y - padding < 0 || y + padding >= srcdst->height) continue;

				if(y == yc - (height / 2) - padding || y == yc + (height/2) + padding)
				{
					for(int j = 0; j < thickness; j++)
					{
						srcdst->data[pos + (j * srcdst->bytesperline)] = colorR;
						srcdst->data[pos + 1 + (j * srcdst->bytesperline)] = colorG;
						srcdst->data[pos + 2 + (j * srcdst->bytesperline)] = colorB;
					}
				}

				if(x == xc - (width / 2) - padding || x == xc + (width / 2) + padding)
				{
					for(int j = 0, k = 0; j < thickness; j++, k += 3)
					{
						srcdst->data[pos + k] = colorR;
						srcdst->data[pos + 1 + k] = colorG;
						srcdst->data[pos + 2 + k] = colorB;
					}
				}
			}
		}
	}

	return 1;
}

/**
 * @brief Desenha uma marca no centro de massa de cada blob.
 *
 * A marca e desenhada manualmente nos pixeis da imagem, usando a cor recebida.
 */
int vc_draw_center_mass_all_blobs(IVC* srcdst, OVC* blobs, int nlabels, int kernel, int thickness, int colorR, int colorG, int colorB)
{
	for (int i = 0; i < nlabels; i++)
    {
        int xc = blobs[i].xc;
        int yc = blobs[i].yc;
		int offset = (kernel - 1) / 2;
		int thickOffset = thickness / 2;

        for(int y = yc - offset; y <= yc + offset; y++)
		{
			if(y < 0 || y >= srcdst->height) continue;

            for(int x = xc - offset; x <= xc + offset; x++)
			{
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
