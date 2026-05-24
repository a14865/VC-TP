/*
===============================================================================
FICHEIRO: tp_classification.h

DESCRICAO:
Interface publica do modulo de classificacao.

OBJETIVO:
Declarar estruturas e funcoes para calibre, categoria e estatisticas do lote.
===============================================================================
*/
#ifndef TP_CLASSIFICATION_H
#define TP_CLASSIFICATION_H

#include "../include/tp_utils.h"

#define ORANGE_BATCH_MAX 500

#define ORANGE_CATEGORY_EXTRA 0
#define ORANGE_CATEGORY_I 1
#define ORANGE_CATEGORY_II 2
#define ORANGE_CATEGORY_III 3
#define ORANGE_CATEGORY_REJECTED 4

/**
 * @brief Metricas visuais usadas para atribuir a categoria de qualidade.
 */
typedef struct {
    int category;              /**< Categoria final da laranja. */
    int shapeRatioPercent;     /**< Relacao entre menor e maior dimensao. */
    int fillRatioPercent;      /**< Percentagem da bounding box ocupada pelo blob. */
    int typicalColorPercent;   /**< Percentagem de pixeis com cor tipica de laranja. */
    int defectColorPercent;    /**< Percentagem de pixeis considerados defeito de cor. */
    int circularityPercent;    /**< Circularidade aproximada em percentagem. */
    int colorVariation;        /**< Variacao media de cor face a media da laranja. */
    int roughness;             /**< Estimativa de rugosidade por diferencas locais. */
    int edgeDensityPercent;    /**< Densidade de arestas na epiderme. */
    int intensityP10;          /**< Percentil 10 do histograma de intensidade. */
    int intensityP90;          /**< Percentil 90 do histograma de intensidade. */
    int intensityContrast;     /**< Contraste entre os percentis 90 e 10. */
} OrangeQualityMetrics;

/**
 * @brief Estatisticas acumuladas das laranjas contadas no lote.
 */
typedef struct {
    int diametersMM[ORANGE_BATCH_MAX];      /**< Diametros das laranjas contadas. */
    int calibers[ORANGE_BATCH_MAX];         /**< Calibres preferenciais calculados. */
    int qualityCategories[ORANGE_BATCH_MAX];/**< Categorias de qualidade individuais. */
    int count;                              /**< Numero de laranjas no lote. */
    int minDiameterMM;                      /**< Menor diametro do lote. */
    int maxDiameterMM;                      /**< Maior diametro do lote. */
    int dominantCaliber;                    /**< Calibre escolhido para representar o lote. */
    int dominantQualityCategory;            /**< Melhor categoria de qualidade valida para o lote. */
    int maxAllowedDifferenceMM;             /**< Diferenca maxima permitida pela homogeneidade. */
    int homogeneous;                        /**< Indica se o lote cumpre homogeneidade. */
    int toleranceOk;                        /**< Indica se a tolerancia de calibre esta OK. */
    int toleranceOutsideCount;              /**< Numero de frutos fora da regra de calibre. */
    float toleranceOutsidePercent;          /**< Percentagem fora da regra de calibre. */
    int qualityToleranceOk;                 /**< Indica se a tolerancia de qualidade esta OK. */
    int qualityToleranceOutsideCount;       /**< Numero de frutos fora da categoria do lote. */
    float qualityToleranceOutsidePercent;   /**< Percentagem fora da categoria do lote. */
} OrangeBatchStats;

/** @brief Devolve o calibre preferencial para um diametro em milimetros. */
int orangeCaliber(int mmWidth);

/** @brief Verifica se um diametro pertence ao intervalo oficial de um calibre. */
int orangeFitsCaliber(int diameterMM, int caliber);

/**
 * @brief Calcula a categoria de qualidade de uma laranja.
 *
 * @param rgbImage Imagem RGB original.
 * @param labelsImage Imagem de etiquetas dos blobs.
 * @param orange Blob correspondente a laranja.
 * @param metrics Estrutura onde serao guardadas as metricas calculadas.
 * @return int Categoria atribuida.
 */
int orangeQualityCategory(IVC *rgbImage, IVC *labelsImage, OVC *orange, OrangeQualityMetrics *metrics);

/** @brief Converte o codigo de categoria para texto. */
const char *orangeQualityCategoryName(int category);

/** @brief Inicializa as estatisticas de lote. */
void orangeBatchInit(OrangeBatchStats *stats);

/** @brief Adiciona uma laranja ao lote com diametro e calibre. */
int orangeBatchAdd(OrangeBatchStats *stats, int diameterMM, int caliber);

/** @brief Define a categoria da ultima laranja adicionada ao lote. */
int orangeBatchAddQuality(OrangeBatchStats *stats, int qualityCategory);

/** @brief Atualiza a categoria de qualidade de uma laranja ja existente no lote. */
int orangeBatchSetQuality(OrangeBatchStats *stats, int index, int qualityCategory);

/** @brief Devolve o calibre dominante do lote. */
int orangeBatchDominantCaliber(const OrangeBatchStats *stats);

/** @brief Escolhe o melhor calibre para representar o lote. */
int orangeBatchBestCaliber(const OrangeBatchStats *stats);

/** @brief Escolhe a melhor categoria de qualidade que cumpre a tolerancia do lote. */
int orangeBatchBestQualityCategory(OrangeBatchStats *stats);

/** @brief Verifica a tolerancia de qualidade para uma categoria candidata. */
int orangeBatchQualityToleranceOkForCategory(const OrangeBatchStats *stats, int lotCategory, int *outsideCount, float *outsidePercent);

/** @brief Verifica a tolerancia de qualidade do lote. */
int orangeBatchQualityToleranceOk(OrangeBatchStats *stats);

/** @brief Devolve a diferenca maxima de diametro permitida para um calibre. */
int orangeBatchMaxAllowedDifference(int caliber);

/** @brief Verifica se o lote e homogeneo no calibre. */
int orangeBatchIsHomogeneous(OrangeBatchStats *stats);

/** @brief Verifica a tolerancia de calibre ativa para o lote. */
int orangeBatchToleranceOk(OrangeBatchStats *stats);

/** @brief Verifica a tolerancia de calibre pela regra do minimo de 50 mm. */
int orangeBatchMinimumToleranceOk(OrangeBatchStats *stats);

/** @brief Recalcula todos os indicadores acumulados do lote. */
void orangeBatchUpdate(OrangeBatchStats *stats);

#endif
