# 📌 Pipeline de Processamento

## Pipeline Global

1. Aquisição da frame
2. Conversão de cor (RGB → HSV)
3. Segmentação por cor
4. Operações morfológicas
5. Etiquetagem de blobs
6. Filtragem de blobs inválidos
7. Extração de características
8. Conversão px → mm
9. Tracking temporal
10. Classificação
11. Visualização

## Descrição das Etapas

### Segmentação
Baseada em HSV para isolar laranjas.

### Morfologia
- Abertura → remover ruído
- Fecho → preencher falhas

### Blobs
- Identificação de objetos
- Cálculo de métricas

### Tracking
- Associação por centroides
- Evitar duplicação na contagem

### Classificação
- Calibre → baseado em diâmetro
- Categoria → heurística visual

## Observações

Pipeline alinhado com:
- Segmentação
- Morfologia
- Blobs
- Análise de imagem