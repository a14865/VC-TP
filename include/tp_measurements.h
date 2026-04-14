/*
===============================================================================
FICHEIRO: tp_measurements.h

DESCRIÇÃO:
Este ficheiro declara a interface pública do módulo de medição e análise de
objetos segmentados.

OBJETIVO:
Disponibilizar os protótipos das funções responsáveis pela extração de medidas
das laranjas detetadas.

O QUE DEVE INCLUIR:
- Protótipos das funções implementadas em tp_measurements.c.
- Estruturas auxiliares para armazenar medidas, se necessário.
- Constantes relevantes para o cálculo de métricas.

O QUE NÃO DEVE INCLUIR:
- Implementação das funções.
- Regras específicas de classificação.
- Lógica de tracking.

PORQUE EXISTE:
Este ficheiro existe para separar de forma clara a interface do módulo de
medições da sua implementação, promovendo legibilidade e reutilização.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Está associado a tp_measurements.c.
- Será utilizado pelo main e pelo módulo de classificação.

NOTAS:
As estruturas e funções aqui definidas devem refletir apenas medidas e
características dos objetos, não decisões finais sobre a sua categoria.
===============================================================================
*/