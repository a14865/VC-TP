/*
===============================================================================
FICHEIRO: tp_tracking.h

DESCRIÇÃO:
Este ficheiro declara a interface pública do módulo de tracking temporal dos
objetos ao longo do vídeo.

OBJETIVO:
Definir os protótipos e estruturas necessárias para gerir a associação entre
objetos detetados em diferentes frames.

O QUE DEVE INCLUIR:
- Protótipos das funções de tracking.
- Estruturas para representar objetos acompanhados ao longo do tempo.
- Constantes de controlo relacionadas com critérios de associação.

O QUE NÃO DEVE INCLUIR:
- Implementação das funções.
- Regras de classificação.
- Algoritmos base de processamento de imagem.

PORQUE EXISTE:
Este ficheiro existe para separar claramente a interface do módulo temporal da
sua implementação, facilitando a utilização do tracking no resto do projeto.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Está associado a tp_tracking.c.
- Pode depender de estruturas produzidas por tp_measurements.c.

NOTAS:
Deve manter uma interface clara e estável, porque o tracking tende a sofrer
ajustes ao longo do desenvolvimento.
===============================================================================
*/