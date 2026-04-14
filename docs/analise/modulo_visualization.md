# 📌 Módulo de Visualização

## Objetivo

Apresentar visualmente os resultados do sistema sobre o vídeo.

## Funcionalidades

- Desenhar bounding boxes
- Mostrar centro de gravidade
- Exibir:
  - área
  - perímetro
  - calibre
  - categoria
- Mostrar contadores:
  - frame atual
  - total acumulado

## Inputs

- Frame original
- Lista de blobs
- Dados de classificação
- Informação de tracking

## Outputs

- Frame anotada com informação

## Princípio Fundamental

Este módulo NÃO calcula dados — apenas os apresenta.

## Benefícios

- Código mais organizado
- Separação clara de responsabilidades
- Facilita debug e apresentação