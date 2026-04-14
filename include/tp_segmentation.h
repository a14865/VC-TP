/*
===============================================================================
FICHEIRO: tp_segmentation.h

DESCRIÇÃO:
Este ficheiro declara a interface pública do módulo de segmentação.

OBJETIVO:
Definir os protótipos das funções de segmentação e tornar explícito quais são as
operações disponibilizadas por este módulo ao resto da aplicação.

O QUE DEVE INCLUIR:
- Protótipos das funções implementadas em tp_segmentation.c.
- Eventuais estruturas auxiliares específicas da segmentação.
- Constantes relacionadas com parâmetros de segmentação, se fizer sentido.

O QUE NÃO DEVE INCLUIR:
- Implementação de funções.
- Código de medição, classificação ou tracking.
- Algoritmos genéricos que pertencem a vc.c.

PORQUE EXISTE:
Este ficheiro existe para permitir que o módulo seja usado de forma limpa e
controlada a partir do main ou de outros módulos, sem expor detalhes internos
da implementação.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Está associado a tp_segmentation.c.
- Pode incluir vc.h para aceder à estrutura IVC.

NOTAS:
Sempre que forem criadas novas funções de segmentação reutilizáveis, os seus
protótipos devem ser adicionados aqui.
===============================================================================
*/