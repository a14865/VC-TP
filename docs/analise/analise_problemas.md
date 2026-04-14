# 📌 Análise do Problema

## Descrição Geral

O objetivo do trabalho consiste no desenvolvimento de um sistema de Visão por Computador em C/C++ capaz de processar o vídeo `video.avi` e extrair informação relevante sobre laranjas presentes em cada frame.

## Objetivos do Sistema

O sistema deve, para cada frame:

- Detetar laranjas
- Contar laranjas na frame atual
- Manter contagem total ao longo do vídeo
- Calcular propriedades geométricas:
  - Área
  - Perímetro
  - Bounding box
  - Centro de gravidade
- Classificar:
  - Calibre (com base em dimensão física)
  - Categoria (qualidade visual)

## Dados do Problema

- Resolução: 1280x720
- Frame rate: 25 fps
- Escala: 280 px = 55 mm

## Desafios Identificados

- Segmentação robusta (variações de iluminação)
- Separação de objetos sobrepostos
- Estimativa de dimensão real a partir de píxeis
- Tracking para evitar contagem duplicada
- Classificação baseada em critérios visuais imperfeitos

## Conclusão

O problema requer um pipeline completo de Visão por Computador, integrando:
- processamento de imagem
- análise de blobs
- tracking temporal
- classificação baseada em regras