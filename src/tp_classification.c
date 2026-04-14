/*
===============================================================================
FICHEIRO: tp_classification.c

DESCRIÇÃO:
Este ficheiro contém a implementação das regras de classificação dos objetos
detetados, com base nas medidas obtidas anteriormente.

OBJETIVO:
Atribuir a cada laranja uma categoria ou classe, por exemplo calibre comercial,
com base nos critérios definidos no enunciado e/ou no regulamento de referência.

O QUE DEVE INCLUIR:
- Regras de decisão baseadas em medidas geométricas.
- Funções que convertam diâmetros ou escalas em calibres.
- Mapeamento entre valores medidos e categorias finais.
- Eventual validação dos limites definidos para cada classe.

O QUE NÃO DEVE INCLUIR:
- Segmentação da imagem.
- Operações morfológicas.
- Tracking entre frames.
- Lógica de leitura do vídeo.

PORQUE EXISTE:
A classificação é uma fase conceptual diferente da segmentação e da medição.
Ter este módulo separado torna o código mais claro, mais fácil de testar e
permite alterar regras de decisão sem mexer no resto do pipeline.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Usa os resultados produzidos por tp_measurements.c.
- Pode recorrer a constantes ou tabelas definidas a partir do regulamento.
- É chamado a partir do fluxo principal em main.cpp.

NOTAS:
Este módulo deve conter apenas a lógica de decisão e categorização, não a
extração das medidas em si.
===============================================================================
*/