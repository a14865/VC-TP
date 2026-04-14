/*
===============================================================================
FICHEIRO: tp_measurements.c

DESCRIÇÃO:
Este ficheiro contém a implementação das funções responsáveis pela extração de
medidas e características dos objetos segmentados.

OBJETIVO:
Calcular propriedades relevantes das laranjas detetadas, como área, perímetro,
centro de massa, bounding box, dimensão aparente e outras medidas necessárias
para posterior classificação.

O QUE DEVE INCLUIR:
- Cálculo de propriedades geométricas dos blobs.
- Extração de métricas úteis para o enunciado.
- Funções que convertam resultados visuais em medidas quantitativas.
- Apoio ao cálculo do calibre, quando essa medida depender de parâmetros
  geométricos obtidos da segmentação.

O QUE NÃO DEVE INCLUIR:
- Regras finais de decisão comercial.
- Gestão de identidade dos objetos entre frames.
- Leitura de vídeo ou interação com o utilizador.

PORQUE EXISTE:
Separar a etapa de medição da etapa de segmentação melhora a organização e
permite alterar a forma de medir os objetos sem mexer diretamente no processo
de deteção visual.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Recebe como entrada os resultados da segmentação.
- Fornece dados ao módulo de classificação.
- Pode usar estruturas e funções base declaradas em vc.h.

NOTAS:
Este módulo responde à pergunta:
"Depois de encontrar os objetos, que medidas relevantes conseguimos obter?"
===============================================================================
*/