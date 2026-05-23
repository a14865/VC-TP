/*
===============================================================================
FICHEIRO: tp_measurements.c

DESCRICAO:
Este ficheiro contem a implementacao das funcoes responsaveis pela extracao de
medidas e caracteristicas dos objetos segmentados.
===============================================================================
*/

#include <stdlib.h>
#include "../include/tp_measurements.h"

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
				int count = 0;

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

				dst->data[pos] = i;
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

	*nlabels = 0;
	for (int a = 1; a < label; a++)
	{
		if (labelTable[a] != 0)
		{
			labelTable[*nlabels] = labelTable[a];
			(*nlabels)++;
		}
	}

	if (*nlabels == 0)
		return NULL;

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

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if (channels != 1)
		return 0;

	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;
		sumx = 0;
		sumy = 0;
		blobs[i].area = 0;
		blobs[i].perimeter = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					blobs[i].area++;
					sumx += x;
					sumy += y;

					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) ||
						(data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}
