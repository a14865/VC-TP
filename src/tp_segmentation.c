/*
===============================================================================
FICHEIRO: tp_segmentation.c

DESCRIÇÃO:
Este ficheiro contém a implementação das funções responsáveis pela segmentação
das laranjas nas frames do vídeo.

OBJETIVO:
Isolar a lógica de deteção visual dos objetos de interesse, transformando a
imagem original em regiões segmentadas que possam posteriormente ser medidas e
classificadas.

O QUE DEVE INCLUIR:
- Conversões necessárias para facilitar a segmentação.
- Aplicação de thresholds ou critérios de cor/intensidade.
- Filtragem e limpeza da máscara binária.
- Operações morfológicas específicas da fase de segmentação.
- Estratégias para reduzir ruído e melhorar a separação dos objetos.

O QUE NÃO DEVE INCLUIR:
- Cálculo de medidas finais dos objetos.
- Regras de classificação comercial.
- Gestão de contagens acumuladas entre frames.
- Código de visualização principal.

PORQUE EXISTE:
A segmentação é uma etapa crítica e com bastante detalhe técnico. Mantê-la num
ficheiro próprio melhora a clareza do projeto, facilita testes e permite afinar
os parâmetros sem afetar os outros módulos.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Pode utilizar funções base de vc.c.
- Produz resultados que serão usados em tp_measurements.c.
- É chamado a partir de main.cpp.

NOTAS:
Este módulo deve ser pensado como a etapa que responde à pergunta:
"Que píxeis pertencem às laranjas e quais pertencem ao fundo?"
===============================================================================
*/