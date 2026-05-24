/*
===============================================================================
FICHEIRO: tp_classification.c

DESCRICAO:
Este ficheiro contem a implementacao das regras de classificacao dos objetos
detetados, com base nas medidas obtidas anteriormente.

OBJETIVO:
Atribuir a cada laranja uma categoria ou classe, por exemplo calibre comercial,
com base nos criterios definidos no enunciado e/ou no regulamento de referencia.
===============================================================================
*/

#include "../include/tp_classification.h"

/**
 * @brief Devolve o valor absoluto de um inteiro.
 */
static int absInt(int value)
{
  return value < 0 ? -value : value;
}

/**
 * @brief Calcula a intensidade em escala de cinzento a partir de RGB.
 */
static int rgbIntensity(int r, int g, int b)
{
  return (int)(0.299f * r + 0.587f * g + 0.114f * b);
}

/**
 * @brief Verifica se uma coordenada pertence ao blob da laranja.
 */
static int pixelBelongsToOrange(IVC *labelsImage, OVC *orange, int x, int y)
{
  int labelPos;

  if (labelsImage == 0 || orange == 0) return 0;
  if (x < 0 || y < 0 || x >= labelsImage->width || y >= labelsImage->height) return 0;

  labelPos = y * labelsImage->bytesperline + x * labelsImage->channels;
  return labelsImage->data[labelPos] == orange->label;
}

/**
 * @brief Le um pixel RGB e devolve a sua intensidade em cinzento.
 */
static int grayPixel(IVC *rgbImage, int x, int y)
{
  int rgbPos = y * rgbImage->bytesperline + x * rgbImage->channels;
  return rgbIntensity(rgbImage->data[rgbPos + 2], rgbImage->data[rgbPos + 1], rgbImage->data[rgbPos]);
}

/**
 * @brief Ordena um pequeno vetor de inteiros usado no calculo da mediana.
 */
static void sortSmallArray(int *values, int count)
{
  for (int i = 0; i < count - 1; i++)
  {
    for (int j = i + 1; j < count; j++)
    {
      if (values[j] < values[i])
      {
        int tmp = values[i];
        values[i] = values[j];
        values[j] = tmp;
      }
    }
  }
}

/**
 * @brief Calcula a mediana 3x3 da intensidade, considerando apenas pixeis da laranja.
 */
static int medianGrayInOrange(IVC *rgbImage, IVC *labelsImage, OVC *orange, int x, int y)
{
  int values[9];
  int count = 0;

  for (int ky = -1; ky <= 1; ky++)
  {
    for (int kx = -1; kx <= 1; kx++)
    {
      int nx = x + kx;
      int ny = y + ky;

      if (nx < 0 || ny < 0 || nx >= rgbImage->width || ny >= rgbImage->height) continue;
      if (!pixelBelongsToOrange(labelsImage, orange, nx, ny)) continue;

      values[count++] = grayPixel(rgbImage, nx, ny);
    }
  }

  if (count <= 0) return grayPixel(rgbImage, x, y);

  sortSmallArray(values, count);
  return values[count / 2];
}

/**
 * @brief Define o passo de amostragem para acelerar a avaliacao de qualidade.
 */
static int qualitySampleStep(OVC *orange)
{
  if (orange == 0) return 1;
  if (orange->area > 180000) return 4;
  if (orange->area > 80000) return 3;
  return 2;
}

/**
 * @brief Calcula um percentil a partir de um histograma de intensidade.
 */
static int percentileFromHistogram(int *histogram, int total, int percentile)
{
  int target;
  int acc = 0;

  if (total <= 0) return 0;

  target = (total * percentile) / 100;
  if (target <= 0) target = 1;

  for (int i = 0; i < 256; i++)
  {
    acc += histogram[i];
    if (acc >= target) return i;
  }

  return 255;
}

/**
 * @brief Verifica se um pixel tem cor compativel com a cor tipica da laranja.
 */
static int isTypicalOrangePixel(int r, int g, int b)
{
  int max = r > g ? (r > b ? r : b) : (g > b ? g : b);
  int min = r < g ? (r < b ? r : b) : (g < b ? g : b);
  int saturation = max > 0 ? ((max - min) * 100) / max : 0;

  return r >= 95 && r <= 255 &&
         g >= 45 && g <= 190 &&
         b <= 125 &&
         r > g + 12 &&
         g >= b &&
         saturation >= 35;
}

/**
 * @brief Verifica se um pixel parece defeito de cor face a media da laranja.
 */
static int isDefectColorPixel(int r, int g, int b, int meanR, int meanG, int meanB)
{
  int colorDistance = absInt(r - meanR) + absInt(g - meanG) + absInt(b - meanB);
  int veryDark = r < 55 && g < 55 && b < 55;
  int greenish = g > r + 20;
  int bluish = b > r;
  int pale = absInt(r - g) < 18 && absInt(g - b) < 18 && r < 180;

  return veryDark || greenish || bluish || pale || colorDistance > 135;
}

// Calibre segundo a escala de laranjas do Regulamento CEE-379-71.
/**
 * @brief Devolve o calibre preferencial para um diametro em milimetros.
 */
int orangeCaliber(int mmWidth)
{
  if (mmWidth < 53) return -1;
  if (mmWidth >= 100) return 0;
  if (mmWidth >= 87) return 1;
  if (mmWidth >= 84) return 2;
  if (mmWidth >= 81) return 3;
  if (mmWidth >= 77) return 4;
  if (mmWidth >= 73) return 5;
  if (mmWidth >= 70) return 6;
  if (mmWidth >= 67) return 7;
  if (mmWidth >= 64) return 8;
  if (mmWidth >= 62) return 9;
  if (mmWidth >= 60) return 10;
  if (mmWidth >= 58) return 11;
  if (mmWidth >= 56) return 12;
  return 13;
}

/**
 * @brief Verifica se um diametro pertence ao intervalo oficial de um calibre.
 */
int orangeFitsCaliber(int diameterMM, int caliber)
{
  static const int minDiameterByCaliber[14] = {100, 87, 84, 81, 77, 73, 70, 67, 64, 62, 60, 58, 56, 53};
  static const int maxDiameterByCaliber[14] = {1000000, 100, 96, 92, 88, 84, 80, 76, 73, 70, 68, 66, 63, 60};

  if (caliber < 0 || caliber > 13) return 0;

  return diameterMM >= minDiameterByCaliber[caliber] && diameterMM <= maxDiameterByCaliber[caliber];
}

/**
 * @brief Calcula a categoria de qualidade de uma laranja.
 *
 * A categoria e estimada por forma, preenchimento, cor tipica, defeitos de cor,
 * rugosidade, densidade de arestas e contraste de intensidade.
 */
int orangeQualityCategory(IVC *rgbImage, IVC *labelsImage, OVC *orange, OrangeQualityMetrics *metrics)
{
  int totalPixels = 0;
  int typicalPixels = 0;
  int defectPixels = 0;
  int roughnessSum = 0;
  int roughnessCount = 0;
  int edgePixels = 0;
  int edgeCandidates = 0;
  int histogram[256] = {0};
  long sumR = 0;
  long sumG = 0;
  long sumB = 0;
  long colorVariationSum = 0;
  int meanR = 0;
  int meanG = 0;
  int meanB = 0;
  int maxDimension;
  int minDimension;
  int bboxArea;
  int sampleStep;

  if (rgbImage == 0 || labelsImage == 0 || orange == 0 || metrics == 0)
  {
    return ORANGE_CATEGORY_REJECTED;
  }

  maxDimension = orange->width > orange->height ? orange->width : orange->height;
  minDimension = orange->width < orange->height ? orange->width : orange->height;
  bboxArea = orange->width * orange->height;

  metrics->shapeRatioPercent = maxDimension > 0 ? (minDimension * 100) / maxDimension : 0;
  metrics->fillRatioPercent = bboxArea > 0 ? (orange->area * 100) / bboxArea : 0;
  metrics->circularityPercent = orange->perimeter > 0 ? (int)((1257.0f * (float)orange->area) / ((float)orange->perimeter * (float)orange->perimeter)) : 0;
  if (metrics->circularityPercent > 100) metrics->circularityPercent = 100;
  sampleStep = qualitySampleStep(orange);

  for (int y = orange->y; y < orange->y + orange->height; y += sampleStep)
  {
    if (y < 0 || y >= labelsImage->height || y >= rgbImage->height) continue;

    for (int x = orange->x; x < orange->x + orange->width; x += sampleStep)
    {
      int labelPos;
      int rgbPos;
      int r, g, b;

      if (x < 0 || x >= labelsImage->width || x >= rgbImage->width) continue;

      labelPos = y * labelsImage->bytesperline + x * labelsImage->channels;
      if (labelsImage->data[labelPos] != orange->label) continue;

      rgbPos = y * rgbImage->bytesperline + x * rgbImage->channels;
      b = rgbImage->data[rgbPos];
      g = rgbImage->data[rgbPos + 1];
      r = rgbImage->data[rgbPos + 2];

      totalPixels++;
      histogram[grayPixel(rgbImage, x, y)]++;
      sumR += r;
      sumG += g;
      sumB += b;
    }
  }

  if (totalPixels > 0)
  {
    meanR = (int)(sumR / totalPixels);
    meanG = (int)(sumG / totalPixels);
    meanB = (int)(sumB / totalPixels);
  }

  for (int y = orange->y; y < orange->y + orange->height; y += sampleStep)
  {
    if (y < 0 || y >= labelsImage->height || y >= rgbImage->height) continue;

    for (int x = orange->x; x < orange->x + orange->width; x += sampleStep)
    {
      int labelPos;
      int rgbPos;
      int r, g, b;
      int intensity;

      if (x < 0 || x >= labelsImage->width || x >= rgbImage->width) continue;

      labelPos = y * labelsImage->bytesperline + x * labelsImage->channels;
      if (labelsImage->data[labelPos] != orange->label) continue;

      rgbPos = y * rgbImage->bytesperline + x * rgbImage->channels;
      b = rgbImage->data[rgbPos];
      g = rgbImage->data[rgbPos + 1];
      r = rgbImage->data[rgbPos + 2];
      intensity = medianGrayInOrange(rgbImage, labelsImage, orange, x, y);

      if (isTypicalOrangePixel(r, g, b)) typicalPixels++;
      if (isDefectColorPixel(r, g, b, meanR, meanG, meanB)) defectPixels++;

      colorVariationSum += absInt(r - meanR) + absInt(g - meanG) + absInt(b - meanB);

      if (x + sampleStep < labelsImage->width)
      {
        int rightLabelPos = y * labelsImage->bytesperline + (x + sampleStep) * labelsImage->channels;
        if (labelsImage->data[rightLabelPos] == orange->label)
        {
          int rightIntensity = medianGrayInOrange(rgbImage, labelsImage, orange, x + sampleStep, y);
          roughnessSum += absInt(intensity - rightIntensity);
          roughnessCount++;
        }
      }

      if (y + sampleStep < labelsImage->height)
      {
        int downLabelPos = (y + sampleStep) * labelsImage->bytesperline + x * labelsImage->channels;
        if (labelsImage->data[downLabelPos] == orange->label)
        {
          int downIntensity = medianGrayInOrange(rgbImage, labelsImage, orange, x, y + sampleStep);
          roughnessSum += absInt(intensity - downIntensity);
          roughnessCount++;
        }
      }

      if (pixelBelongsToOrange(labelsImage, orange, x - 1, y - 1) &&
          pixelBelongsToOrange(labelsImage, orange, x, y - 1) &&
          pixelBelongsToOrange(labelsImage, orange, x + 1, y - 1) &&
          pixelBelongsToOrange(labelsImage, orange, x - 1, y) &&
          pixelBelongsToOrange(labelsImage, orange, x + 1, y) &&
          pixelBelongsToOrange(labelsImage, orange, x - 1, y + 1) &&
          pixelBelongsToOrange(labelsImage, orange, x, y + 1) &&
          pixelBelongsToOrange(labelsImage, orange, x + 1, y + 1))
      {
        int g00 = medianGrayInOrange(rgbImage, labelsImage, orange, x - 1, y - 1);
        int g01 = medianGrayInOrange(rgbImage, labelsImage, orange, x, y - 1);
        int g02 = medianGrayInOrange(rgbImage, labelsImage, orange, x + 1, y - 1);
        int g10 = medianGrayInOrange(rgbImage, labelsImage, orange, x - 1, y);
        int g12 = medianGrayInOrange(rgbImage, labelsImage, orange, x + 1, y);
        int g20 = medianGrayInOrange(rgbImage, labelsImage, orange, x - 1, y + 1);
        int g21 = medianGrayInOrange(rgbImage, labelsImage, orange, x, y + 1);
        int g22 = medianGrayInOrange(rgbImage, labelsImage, orange, x + 1, y + 1);
        int gx = -g00 + g02 - g10 + g12 - g20 + g22;
        int gy = g00 + g01 + g02 - g20 - g21 - g22;
        int magnitude = absInt(gx) + absInt(gy);

        edgeCandidates++;
        if (magnitude > 90) edgePixels++;
      }
    }
  }

  metrics->typicalColorPercent = totalPixels > 0 ? (typicalPixels * 100) / totalPixels : 0;
  metrics->defectColorPercent = totalPixels > 0 ? (defectPixels * 100) / totalPixels : 100;
  metrics->colorVariation = totalPixels > 0 ? (int)(colorVariationSum / totalPixels) : 255;
  metrics->roughness = roughnessCount > 0 ? roughnessSum / roughnessCount : 255;
  metrics->edgeDensityPercent = edgeCandidates > 0 ? (edgePixels * 100) / edgeCandidates : 100;
  metrics->intensityP10 = percentileFromHistogram(histogram, totalPixels, 10);
  metrics->intensityP90 = percentileFromHistogram(histogram, totalPixels, 90);
  metrics->intensityContrast = metrics->intensityP90 - metrics->intensityP10;

  if (metrics->circularityPercent >= 82 &&
      metrics->shapeRatioPercent >= 94 &&
      metrics->fillRatioPercent >= 68 &&
      metrics->typicalColorPercent >= 92 &&
      metrics->defectColorPercent <= 2 &&
      metrics->colorVariation <= 28 &&
      metrics->roughness <= 8 &&
      metrics->edgeDensityPercent <= 2 &&
      metrics->intensityContrast <= 45)
  {
    metrics->category = ORANGE_CATEGORY_EXTRA;
  }
  else if (metrics->circularityPercent >= 70 &&
           metrics->shapeRatioPercent >= 86 &&
           metrics->fillRatioPercent >= 60 &&
           metrics->typicalColorPercent >= 80 &&
           metrics->defectColorPercent <= 8 &&
           metrics->colorVariation <= 45 &&
           metrics->roughness <= 14 &&
           metrics->edgeDensityPercent <= 6 &&
           metrics->intensityContrast <= 70)
  {
    metrics->category = ORANGE_CATEGORY_I;
  }
  else if (metrics->circularityPercent >= 55 &&
           metrics->shapeRatioPercent >= 74 &&
           metrics->fillRatioPercent >= 48 &&
           metrics->typicalColorPercent >= 62 &&
           metrics->defectColorPercent <= 22 &&
           metrics->colorVariation <= 70 &&
           metrics->roughness <= 24 &&
           metrics->edgeDensityPercent <= 14 &&
           metrics->intensityContrast <= 100)
  {
    metrics->category = ORANGE_CATEGORY_II;
  }
  else if (metrics->fillRatioPercent >= 38 &&
           metrics->typicalColorPercent >= 42 &&
           metrics->defectColorPercent <= 36 &&
           metrics->roughness <= 34)
  {
    // Categoria III e suplementar no regulamento; o calice nao e fiavel neste video.
    metrics->category = ORANGE_CATEGORY_III;
  }
  else
  {
    metrics->category = ORANGE_CATEGORY_REJECTED;
  }

  return metrics->category;
}

/**
 * @brief Converte o codigo numerico da categoria para texto.
 */
const char *orangeQualityCategoryName(int category)
{
  switch (category)
  {
    case ORANGE_CATEGORY_EXTRA: return "Extra";
    case ORANGE_CATEGORY_I: return "I";
    case ORANGE_CATEGORY_II: return "II";
    case ORANGE_CATEGORY_III: return "III";
    default: return "Rejeitada";
  }
}

/**
 * @brief Inicializa todos os campos das estatisticas de lote.
 */
void orangeBatchInit(OrangeBatchStats *stats)
{
  if (stats == 0) return;

  stats->count = 0;
  stats->minDiameterMM = 0;
  stats->maxDiameterMM = 0;
  stats->dominantCaliber = -1;
  stats->dominantQualityCategory = ORANGE_CATEGORY_REJECTED;
  stats->maxAllowedDifferenceMM = 0;
  stats->homogeneous = 1;
  stats->toleranceOk = 1;
  stats->toleranceOutsideCount = 0;
  stats->toleranceOutsidePercent = 0.0f;
  stats->qualityToleranceOk = 1;
  stats->qualityToleranceOutsideCount = 0;
  stats->qualityToleranceOutsidePercent = 0.0f;
}

/**
 * @brief Adiciona ao lote uma laranja contada com diametro e calibre.
 */
int orangeBatchAdd(OrangeBatchStats *stats, int diameterMM, int caliber)
{
  if (stats == 0) return 0;
  if (stats->count >= ORANGE_BATCH_MAX) return 0;

  stats->diametersMM[stats->count] = diameterMM;
  stats->calibers[stats->count] = caliber;
  stats->qualityCategories[stats->count] = ORANGE_CATEGORY_REJECTED;

  if (stats->count == 0)
  {
    stats->minDiameterMM = diameterMM;
    stats->maxDiameterMM = diameterMM;
  }
  else
  {
    if (diameterMM < stats->minDiameterMM) stats->minDiameterMM = diameterMM;
    if (diameterMM > stats->maxDiameterMM) stats->maxDiameterMM = diameterMM;
  }

  stats->count++;
  orangeBatchUpdate(stats);

  return 1;
}

/**
 * @brief Define a categoria de qualidade da ultima laranja adicionada ao lote.
 */
int orangeBatchAddQuality(OrangeBatchStats *stats, int qualityCategory)
{
  if (stats == 0) return 0;
  if (stats->count <= 0) return 0;

  return orangeBatchSetQuality(stats, stats->count - 1, qualityCategory);
}

/**
 * @brief Atualiza a categoria de qualidade de uma laranja ja registada no lote.
 */
int orangeBatchSetQuality(OrangeBatchStats *stats, int index, int qualityCategory)
{
  if (stats == 0) return 0;
  if (index < 0 || index >= stats->count) return 0;

  if (qualityCategory < ORANGE_CATEGORY_EXTRA || qualityCategory > ORANGE_CATEGORY_REJECTED)
  {
    qualityCategory = ORANGE_CATEGORY_REJECTED;
  }

  stats->qualityCategories[index] = qualityCategory;
  orangeBatchUpdate(stats);

  return 1;
}

/**
 * @brief Devolve o calibre escolhido para representar o lote.
 */
int orangeBatchDominantCaliber(const OrangeBatchStats *stats)
{
  return orangeBatchBestCaliber(stats);
}

/**
 * @brief Escolhe o calibre que melhor enquadra os diametros do lote.
 */
int orangeBatchBestCaliber(const OrangeBatchStats *stats)
{
  int bestCaliber = -1;
  int bestFits = -1;
  int bestOutside = ORANGE_BATCH_MAX + 1;

  if (stats == 0 || stats->count <= 0) return -1;

  for (int caliber = 0; caliber <= 13; caliber++)
  {
    int fits = 0;
    int outsideAdjacent = 0;
    int invalidOutside = 0;

    for (int i = 0; i < stats->count; i++)
    {
      int diameterMM = stats->diametersMM[i];

      if (orangeFitsCaliber(diameterMM, caliber))
      {
        fits++;
      }
      else if (orangeFitsCaliber(diameterMM, caliber - 1) || orangeFitsCaliber(diameterMM, caliber + 1))
      {
        outsideAdjacent++;
      }
      else
      {
        invalidOutside++;
      }
    }

    if (fits > bestFits ||
        (fits == bestFits && invalidOutside == 0 && bestOutside > outsideAdjacent) ||
        (fits == bestFits && outsideAdjacent < bestOutside))
    {
      bestFits = fits;
      bestOutside = outsideAdjacent;
      bestCaliber = caliber;
    }
  }

  return bestCaliber;
}

/**
 * @brief Verifica se uma categoria candidata cumpre a tolerancia de qualidade.
 */
int orangeBatchQualityToleranceOkForCategory(const OrangeBatchStats *stats, int lotCategory, int *outsideCount, float *outsidePercent)
{
  int outside = 0;
  int invalid = 0;
  float maxTolerancePercent = 0.0f;

  if (outsideCount != 0) *outsideCount = 0;
  if (outsidePercent != 0) *outsidePercent = 0.0f;

  if (stats == 0) return 0;
  if (stats->count <= 0) return 1;

  switch (lotCategory)
  {
    case ORANGE_CATEGORY_EXTRA:
      maxTolerancePercent = 5.0f;
      break;
    case ORANGE_CATEGORY_I:
      maxTolerancePercent = 10.0f;
      break;
    case ORANGE_CATEGORY_II:
      maxTolerancePercent = 10.0f;
      break;
    case ORANGE_CATEGORY_III:
      maxTolerancePercent = 15.0f;
      break;
    default:
      return 0;
  }

  for (int i = 0; i < stats->count; i++)
  {
    int category = stats->qualityCategories[i];

    if (category < ORANGE_CATEGORY_EXTRA || category > ORANGE_CATEGORY_REJECTED)
    {
      category = ORANGE_CATEGORY_REJECTED;
    }

    if (category <= lotCategory) continue;

    outside++;

    if (lotCategory == ORANGE_CATEGORY_EXTRA && category > ORANGE_CATEGORY_I)
    {
      invalid = 1;
    }
    else if (lotCategory == ORANGE_CATEGORY_I && category > ORANGE_CATEGORY_II)
    {
      invalid = 1;
    }
    else if (lotCategory == ORANGE_CATEGORY_II && category > ORANGE_CATEGORY_REJECTED)
    {
      invalid = 1;
    }
    else if (lotCategory == ORANGE_CATEGORY_III && category > ORANGE_CATEGORY_REJECTED)
    {
      invalid = 1;
    }
  }

  if (outsideCount != 0) *outsideCount = outside;
  if (outsidePercent != 0) *outsidePercent = ((float)outside * 100.0f) / (float)stats->count;

  if (invalid) return 0;

  return (((float)outside * 100.0f) / (float)stats->count) <= maxTolerancePercent;
}

/**
 * @brief Escolhe a melhor categoria de qualidade que cumpre o regulamento.
 */
int orangeBatchBestQualityCategory(OrangeBatchStats *stats)
{
  int outsideCount = 0;
  float outsidePercent = 0.0f;

  if (stats == 0 || stats->count <= 0) return ORANGE_CATEGORY_REJECTED;

  for (int category = ORANGE_CATEGORY_EXTRA; category <= ORANGE_CATEGORY_III; category++)
  {
    if (orangeBatchQualityToleranceOkForCategory(stats, category, &outsideCount, &outsidePercent))
    {
      stats->qualityToleranceOutsideCount = outsideCount;
      stats->qualityToleranceOutsidePercent = outsidePercent;
      return category;
    }
  }

  stats->qualityToleranceOutsideCount = stats->count;
  stats->qualityToleranceOutsidePercent = 100.0f;
  return ORANGE_CATEGORY_REJECTED;
}

/**
 * @brief Calcula se a categoria de qualidade do lote respeita a tolerancia.
 */
int orangeBatchQualityToleranceOk(OrangeBatchStats *stats)
{
  int outsideCount = 0;
  float outsidePercent = 0.0f;

  if (stats == 0) return 0;
  if (stats->count <= 0) return 1;

  stats->dominantQualityCategory = orangeBatchBestQualityCategory(stats);

  if (stats->dominantQualityCategory == ORANGE_CATEGORY_REJECTED)
  {
    stats->qualityToleranceOutsideCount = stats->count;
    stats->qualityToleranceOutsidePercent = 100.0f;
    return 0;
  }

  if (!orangeBatchQualityToleranceOkForCategory(stats, stats->dominantQualityCategory, &outsideCount, &outsidePercent))
  {
    stats->qualityToleranceOutsideCount = outsideCount;
    stats->qualityToleranceOutsidePercent = outsidePercent;
    return 0;
  }

  stats->qualityToleranceOutsideCount = outsideCount;
  stats->qualityToleranceOutsidePercent = outsidePercent;
  return 1;
}

/**
 * @brief Devolve a diferenca maxima de diametro permitida para o calibre.
 */
int orangeBatchMaxAllowedDifference(int caliber)
{
  if (caliber >= 0 && caliber <= 2) return 11;
  if (caliber >= 3 && caliber <= 6) return 9;
  if (caliber >= 7 && caliber <= 13) return 7;

  return 0;
}

/**
 * @brief Verifica a homogeneidade do lote pelo menor e maior diametro.
 */
int orangeBatchIsHomogeneous(OrangeBatchStats *stats)
{
  int diameterDifference;

  if (stats == 0) return 0;
  if (stats->count <= 1) return 1;

  stats->dominantCaliber = orangeBatchBestCaliber(stats);
  stats->maxAllowedDifferenceMM = orangeBatchMaxAllowedDifference(stats->dominantCaliber);

  if (stats->maxAllowedDifferenceMM <= 0) return 0;

  diameterDifference = stats->maxDiameterMM - stats->minDiameterMM;

  return diameterDifference <= stats->maxAllowedDifferenceMM;
}

/**
 * @brief Verifica a tolerancia de calibre atualmente adotada para o lote.
 */
int orangeBatchToleranceOk(OrangeBatchStats *stats)
{
  /*
  Abordagem geral por calibre mencionado, deixada para referencia:

  int outsideCount = 0;

  if (stats == 0) return 0;
  if (stats->count <= 0) return 1;

  stats->dominantCaliber = orangeBatchBestCaliber(stats);

  if (stats->dominantCaliber < 0)
  {
    stats->toleranceOutsideCount = stats->count;
    stats->toleranceOutsidePercent = 100.0f;
    return 0;
  }

  for (int i = 0; i < stats->count; i++)
  {
    int diameterMM = stats->diametersMM[i];
    int fitsDominant = orangeFitsCaliber(diameterMM, stats->dominantCaliber);
    int fitsAdjacent = orangeFitsCaliber(diameterMM, stats->dominantCaliber - 1) ||
                       orangeFitsCaliber(diameterMM, stats->dominantCaliber + 1);

    if (!fitsDominant)
    {
      outsideCount++;

      if (!fitsAdjacent)
      {
        stats->toleranceOutsideCount = outsideCount;
        stats->toleranceOutsidePercent = ((float)outsideCount * 100.0f) / (float)stats->count;
        return 0;
      }
    }
  }

  stats->toleranceOutsideCount = outsideCount;
  stats->toleranceOutsidePercent = ((float)outsideCount * 100.0f) / (float)stats->count;

  return stats->toleranceOutsidePercent <= 10.0f;
  */

  return orangeBatchMinimumToleranceOk(stats);
}

/**
 * @brief Aplica a tolerancia de calibre pelo minimo absoluto de 50 mm.
 */
int orangeBatchMinimumToleranceOk(OrangeBatchStats *stats)
{
  int outsideCount = 0;
  int belowAbsoluteMinimum = 0;

  if (stats == 0) return 0;
  if (stats->count <= 0) return 1;

  for (int i = 0; i < stats->count; i++)
  {
    if (stats->diametersMM[i] < 53)
    {
      outsideCount++;

      if (stats->diametersMM[i] < 50)
      {
        belowAbsoluteMinimum = 1;
      }
    }
  }

  stats->toleranceOutsideCount = outsideCount;
  stats->toleranceOutsidePercent = ((float)outsideCount * 100.0f) / (float)stats->count;

  return !belowAbsoluteMinimum && stats->toleranceOutsidePercent <= 10.0f;
}

/**
 * @brief Recalcula calibre dominante, homogeneidade e tolerancias do lote.
 */
void orangeBatchUpdate(OrangeBatchStats *stats)
{
  if (stats == 0) return;

  stats->dominantCaliber = orangeBatchBestCaliber(stats);
  stats->dominantQualityCategory = orangeBatchBestQualityCategory(stats);
  stats->maxAllowedDifferenceMM = orangeBatchMaxAllowedDifference(stats->dominantCaliber);
  stats->homogeneous = orangeBatchIsHomogeneous(stats);
  stats->toleranceOk = orangeBatchToleranceOk(stats);
  stats->qualityToleranceOk = orangeBatchQualityToleranceOk(stats);
}
