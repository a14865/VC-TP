# 📌 Arquitetura do Sistema

## Estrutura Modular

| Módulo | Ficheiro |
|--------|---------|
| Entrada / controlo | main.c |
| Segmentação | tp_segmentation.c |
| Medições | tp_measurements.c |
| Tracking | tp_tracking.c |
| Classificação | tp_classification.c |
| Visualização | tp_visualization.c |
| Utilitários | tp_utils.c |
| Biblioteca base | vc.c |

## Descrição dos Módulos

### Segmentação
Responsável por gerar máscara binária das laranjas.

### Medições
Extrai propriedades geométricas dos blobs.

### Tracking
Associa objetos entre frames.

### Classificação
Determina calibre e categoria.

### Visualização
Desenha informação na frame.

### Utils
Funções auxiliares reutilizáveis.

## Fluxo de Dados

Frame → Segmentação → Blobs → Medições → Tracking → Classificação → Visualização