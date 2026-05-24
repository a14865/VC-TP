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
						srcdst->data[pos] = colorB;
						srcdst->data[pos + 1] = colorG;
						srcdst->data[pos + 2] = colorR;
					}

					if (yBottom >= 0 && yBottom < srcdst->height) {
                        int pos = yBottom * srcdst->bytesperline + x * srcdst->channels;
                        srcdst->data[pos] = colorB;
                        srcdst->data[pos + 1] = colorG;
                        srcdst->data[pos + 2] = colorR;
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
                        srcdst->data[pos] = colorB;
                        srcdst->data[pos + 1] = colorG;
                        srcdst->data[pos + 2] = colorR;
                    }

                    if (xRight >= 0 && xRight < srcdst->width) {
                        int pos = y * srcdst->bytesperline + xRight * srcdst->channels;
                        srcdst->data[pos] = colorB;
                        srcdst->data[pos + 1] = colorG;
                        srcdst->data[pos + 2] = colorR;
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

                srcdst->data[pos] = colorB;
				srcdst->data[pos + 1] = colorG;
				srcdst->data[pos + 2] = colorR;
            }
        }
    }

	return 1;
}
