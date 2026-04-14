# 📌 Decisões Técnicas

## Segmentação

- Método: HSV
- Justificação: robusto a variações de iluminação

## Morfologia

- Fecho: Dilatação + Erosão
- Objetivo: remover ruído e melhorar regiões

## Blobs

- Uso de componentes conexas
- Permite medir objetos individualmente

## Tracking

- Método: distância entre centroides
- Simples e suficiente para o problema

## Calibre

- Baseado no diâmetro em mm
- Conversão px → mm

## Categoria

- Baseada em heurísticas:
  - forma
  - cor
  - defeitos

## Arquitetura

- Modular por ficheiros
- Separação clara de responsabilidades

## Decisão Importante

Optou-se por soluções simples, robustas e explicáveis em vez de métodos complexos.