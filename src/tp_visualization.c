/*
===============================================================================
FICHEIRO: tp_visualization.c

DESCRIÇÃO:
Este ficheiro contém as funções responsáveis pela apresentação visual da
informação produzida pelo sistema sobre cada frame do vídeo processado.

OBJETIVO:
Centralizar toda a lógica de desenho e anotação gráfica, permitindo exibir de
forma clara e organizada os resultados obtidos pelos módulos de segmentação,
medição, tracking e classificação.

O QUE DEVE INCLUIR:
- Funções para desenhar texto informativo sobre a frame.
- Funções para desenhar bounding boxes das laranjas detetadas.
- Funções para assinalar o centro de gravidade dos objetos.
- Funções para apresentar dados como área, perímetro, calibre e categoria.
- Funções para mostrar contadores globais, como número atual e total acumulado.
- Rotinas de visualização dos resultados finais do pipeline.

O QUE NÃO DEVE INCLUIR:
- Algoritmos de segmentação.
- Cálculo de blobs, áreas ou perímetros.
- Lógica de tracking.
- Regras de classificação.
- Código de processamento de imagem que altere a deteção dos objetos.

PORQUE EXISTE:
A visualização é uma responsabilidade distinta da análise. Separar esta parte
num ficheiro próprio melhora a organização do projeto, reduz a complexidade do
ficheiro principal e facilita a manutenção, testes e defesa oral do trabalho.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Recebe informação calculada por tp_segmentation.c, tp_measurements.c,
  tp_tracking.c e tp_classification.c.
- Pode utilizar funções auxiliares de tp_utils.c.
- Deve ser incluído através de tp_visualization.h.
- É normalmente chamado a partir do ficheiro principal, onde é executado o
  ciclo de processamento do vídeo.

NOTAS:
Este ficheiro deve tratar apenas da apresentação visual dos resultados, e não da
sua obtenção. Sempre que possível, as funções devem receber dados já calculados
e limitar-se a desenhá-los na frame de saída.
===============================================================================
*/

#include "../include/tp_visualization.h"

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
