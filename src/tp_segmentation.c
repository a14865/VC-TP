/*
===============================================================================
FICHEIRO: tp_segmentation.c

DESCRIÇÃO:
Este ficheiro contém a implementação das funções responsáveis pela segmentação
das laranjas nas frames do vídeo.

OBJETIVO:
Isolar a lógica de deteção visual dos objetos de interesse, transformando a
imagem original em regiões segmentadas que possam posteriormente ser medidas e
classificadas.

O QUE DEVE INCLUIR:
- Conversões necessárias para facilitar a segmentação.
- Aplicação de thresholds ou critérios de cor/intensidade.
- Filtragem e limpeza da máscara binária.
- Operações morfológicas específicas da fase de segmentação.
- Estratégias para reduzir ruído e melhorar a separação dos objetos.

O QUE NÃO DEVE INCLUIR:
- Cálculo de medidas finais dos objetos.
- Regras de classificação comercial.
- Gestão de contagens acumuladas entre frames.
- Código de visualização principal.

PORQUE EXISTE:
A segmentação é uma etapa crítica e com bastante detalhe técnico. Mantê-la num
ficheiro próprio melhora a clareza do projeto, facilita testes e permite afinar
os parâmetros sem afetar os outros módulos.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Pode utilizar funções base de vc.c.
- Produz resultados que serão usados em tp_measurements.c.
- É chamado a partir de main.cpp.

NOTAS:
Este módulo deve ser pensado como a etapa que responde à pergunta:
"Que píxeis pertencem às laranjas e quais pertencem ao fundo?"
===============================================================================
*/
#include "../include/tp_segmentation.h"

int vc_rgb_to_hsv(IVC *src, IVC *dst)
{
	//Validação
	valImages(src,dst);

	int lenght = src->height * src->width * src->channels;
	int channels = src->channels;

	for (int i = 0; i < lenght; i += channels)
	{

		float r = src->data[i] / 255.0f;
		float g = src->data[i + 1] / 255.0f;
		float b = src->data[i + 2] / 255.0f;

		float hue = 0;
		float sat = 0;
		float val = 0;

		// Find Value
		float max = r > g ? (r > b ? r : b) : (g > b ? g : b);

		val = max;
		if (val == 0)
		{

			dst->data[i] = 0;
			dst->data[i + 1] = 0;
			dst->data[i + 2] = 0;
			continue;
		}

		// Find Saturation
		float min = r < g ? (r < b ? r : b) : (g < b ? g : b);

		if (max == min)
		{

			dst->data[i] = 0;
			dst->data[i + 1] = 0;
			dst->data[i + 2] = (int)(val * 255);
			continue;
		}

		sat = (max - min) / max;

		// Find Hue
		if (max == r && g >= b)
		{
			hue = 60 * (g - b) / (max - min);
		}
		else if (max == r && g < b)
		{
			hue = 360 + 60 * (g - b) / (max - min);
		}
		else if (max == g)
		{
			hue = 120 + 60 * (b - r) / (max - min);
		}
		else if (max == b)
		{
			hue = 240 + 60 * (r - g) / (max - min);
		}

		dst->data[i] = (int)((hue / 360.0f) * 255.0f);
		dst->data[i + 1] = (int)(sat * 255.0f);
		dst->data[i + 2] = (int)(val * 255.0f);
	}

	return 1;
}

int vc_hsv_segmentation(IVC *src, IVC *dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{

	hmin = ((((hmin % 360) + 360) % 360) * 255) / 360;
	hmax = ((((hmax % 360) + 360) % 360) * 255) / 360;
	smin = (smin * 255) / 100;
	smax = (smax * 255) / 100;
	vmin = (vmin * 255) / 100;
	vmax = (vmax * 255) / 100;

	int length = src->channels * src->width * src->height;

	for (int i = 0, j = 0; i < length; i += src->channels, j++)
	{

		if (hmax >= hmin && src->data[i] >= hmin && src->data[i] <= hmax && src->data[i + 1] >= smin && src->data[i + 1] <= smax && src->data[i + 2] >= vmin && src->data[i + 2] <= vmax)
		{

			dst->data[j] = 255;
		}

		else if (hmax < hmin && (src->data[i] >= hmin || src->data[i] <= hmax) && src->data[i + 1] >= smin && src->data[i + 1] <= smax && src->data[i + 2] >= vmin && src->data[i + 2] <= vmax)
		{

			dst->data[j] = 255;
		}

		else
		{

			dst->data[j] = 0;
		}
	}

	return 1;
}

int vc_hsv_saturation_and_value_modified(IVC* src, IVC* dst, float satMod, float valMod)
{

	int length = src->width * src->height * src->channels;

	for(int i = 0; i < length; i += src->channels){

		int s = (int)(src->data[i+1] * satMod);
		int v = (int)(src->data[i+2] * valMod);

		dst->data[i] = src->data[i];
		dst->data[i+1] = s > 255 ? 255 : s < 0 ? 0 : s;
		dst->data[i+2] = v > 255 ? 255 : v < 0 ? 0 : v;
	}

	return 1;
}