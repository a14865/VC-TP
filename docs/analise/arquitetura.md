# Arquitetura do Sistema

## Visao Geral

A arquitetura do projeto foi organizada de forma modular, separando as principais responsabilidades do sistema em ficheiros especificos. O objetivo desta organizacao e tornar o codigo mais facil de compreender, testar, alterar e defender.

O projeto usa maioritariamente modulos em C para o processamento e analise, mantendo o ficheiro `main.cpp` como ponto de entrada e camada de integracao com OpenCV. Esta separacao permite respeitar a restricao de uso limitado de OpenCV, concentrando a parte C++/OpenCV nas tarefas que dependem diretamente dessa biblioteca, como leitura do video, apresentacao das janelas e escrita de texto sobre `cv::Mat`.

## Organizacao em Modulos

| Responsabilidade | Ficheiros principais |
|---|---|
| Entrada, ciclo principal e integracao OpenCV | `src/main.cpp` |
| Segmentacao das laranjas | `src/tp_segmentation.c`, `include/tp_segmentation.h` |
| Medicao de blobs | `src/tp_measurements.c`, `include/tp_measurements.h` |
| Tracking temporal e contagem | `src/tp_tracking.c`, `include/tp_tracking.h` |
| Classificacao e regras de lote | `src/tp_classification.c`, `include/tp_classification.h` |
| Visualizacao em imagem IVC | `src/tp_visualization.c`, `include/tp_visualization.h` |
| Estruturas, memoria, leitura/escrita e conversoes | `src/tp_utils.c`, `include/tp_utils.h` |
| Biblioteca base de apoio | `src/vc.c`, `include/vc.h` |

## Papel do `main.cpp`

O ficheiro `main.cpp` funciona como orquestrador da aplicacao. A sua responsabilidade principal e coordenar os modulos, gerir o video e manter o estado necessario entre frames.

Neste ficheiro ficam as tarefas que dependem diretamente de OpenCV/C++:

- abertura e leitura do video;
- criacao e manipulacao de `cv::Mat`;
- conversao entre `cv::Mat` e a estrutura `IVC`;
- aplicacao das operacoes OpenCV ja usadas no projeto;
- apresentacao das janelas;
- escrita de texto com `cv::putText`;
- controlo do ciclo principal de execucao.

Sempre que a logica nao depende diretamente de OpenCV, ela foi movida para modulos em C. Assim, o `main.cpp` evita concentrar algoritmos principais de segmentacao, medicao, tracking ou classificacao.

## Modulo de Segmentacao

O modulo `tp_segmentation` contem funcoes associadas a conversao de cor e criacao da mascara binaria. A responsabilidade deste modulo e transformar informacao de cor numa representacao binaria que indique quais os pixeis candidatos a pertencer as laranjas.

Este modulo nao calcula medidas finais, nao classifica frutos e nao faz tracking. A sua saida e usada pelos modulos seguintes para identificar objetos.

Funcoes principais:

- `vc_rgb_to_hsv`;
- `vc_hsv_segmentation`;
- `vc_hsv_saturation_and_value_modified`.

Embora a funcao de modificacao de saturacao/valor exista na interface, o pipeline atual nao depende dela no fluxo principal. Ela permanece disponivel como apoio a testes e ajustes.

## Modulo de Medicoes

O modulo `tp_measurements` e responsavel por transformar uma mascara binaria em objetos mediveis. Para isso, faz etiquetagem de componentes conexas e extrai propriedades geometricas dos blobs encontrados.

As medidas calculadas incluem:

- area;
- perimetro;
- bounding box;
- centro de massa;
- etiqueta do objeto.

Este modulo nao decide se um blob e uma laranja valida, nao faz classificacao comercial e nao gere contagem temporal. Ele apenas calcula informacao geometrica a partir da imagem de etiquetas.

## Modulo de Tracking

O modulo `tp_tracking` trata da associacao de objetos entre frames. A sua finalidade e evitar que a mesma laranja seja contada varias vezes enquanto permanece visivel no video.

O tracking usa a posicao dos centroides e compara a deteccao atual com deteccoes anteriores. Quando uma laranja cruza a linha de ativacao, o contador total e atualizado.

Este modulo foi mantido independente da segmentacao e da classificacao. Ele recebe blobs ja medidos e devolve informacao temporal sobre a associacao e contagem.

## Modulo de Classificacao

O modulo `tp_classification` concentra as regras associadas a calibre, categoria de qualidade e estatisticas de lote.

Este modulo inclui:

- determinacao do calibre preferencial;
- verificacao dos intervalos oficiais de calibre;
- avaliacao de qualidade individual;
- acumulacao de estatisticas do lote;
- homogeneidade do calibre;
- tolerancia de calibre;
- categoria de qualidade do lote;
- tolerancia de qualidade do lote.

Esta organizacao evita espalhar regras regulamentares pelo `main.cpp`. Tambem permite manter juntas as estruturas relacionadas com classificacao individual e classificacao por lote.

## Modulo de Visualizacao

O modulo `tp_visualization` contem funcoes de desenho diretamente sobre imagens `IVC`, como bounding boxes e centros de massa.

Este modulo nao usa `cv::Mat` nem `cv::putText`, porque esta parte depende de C++/OpenCV e permanece no `main.cpp`. Assim, `tp_visualization.c` continua a ser C e mantem apenas desenho manual em memoria de imagem.

Esta separacao tambem ajuda a respeitar a restricao de nao aumentar o uso de funcoes OpenCV desnecessarias.

## Modulo de Utilitarios

O modulo `tp_utils` contem estruturas e funcoes reutilizaveis por varios modulos.

As estruturas principais sao:

- `IVC`, usada para representar imagens;
- `OVC`, usada para representar objetos/blobs.

As funcoes principais incluem:

- alocacao e libertacao de imagens;
- leitura e escrita de imagens NetPBM;
- validacao simples de imagens;
- conversao de pixeis para milimetros;
- conversao de area em pixeis quadrados para milimetros quadrados.

Este modulo deve conter apenas funcionalidades transversais. Regras especificas de segmentacao, tracking ou classificacao ficam nos modulos correspondentes.

## Separacao entre C e C++/OpenCV

Uma decisao importante da arquitetura foi manter os modulos do trabalho em C sempre que possivel. O `main.cpp` existe porque a leitura e apresentacao de video com OpenCV usam tipos e funcoes C++.

A fronteira entre os dois mundos e feita principalmente atraves das estruturas `IVC` e `OVC`:

- `cv::Mat` e usado no `main.cpp` para entrada, saida e algumas operacoes OpenCV permitidas;
- os dados relevantes sao copiados para `IVC`;
- os modulos em C processam `IVC` e `OVC`;
- os resultados voltam ao `main.cpp` para visualizacao.

Esta separacao permite que a logica do projeto esteja organizada em C, enquanto o OpenCV fica concentrado na camada de integracao.

## Gestao de Estado

O estado acumulado do sistema e mantido no `main.cpp`, porque depende do ciclo temporal do video.

O estado principal inclui:

- total de laranjas contadas;
- laranjas da frame anterior;
- categorias estabilizadas das laranjas acompanhadas;
- indice de cada laranja no lote;
- estatisticas acumuladas do lote.

As estruturas de lote sao definidas no modulo de classificacao, mas a decisao de quando atualizar essas estruturas acontece no ciclo principal, porque depende do tracking e do momento em que uma laranja e contada.

## Dependencias entre Modulos

As dependencias foram mantidas simples:

- `tp_segmentation` depende de `tp_utils` para usar `IVC`;
- `tp_measurements` depende de `tp_utils` para usar `IVC` e `OVC`;
- `tp_tracking` depende de `tp_utils` para usar `OVC`;
- `tp_classification` depende de `tp_utils` para analisar imagens e blobs;
- `tp_visualization` depende de `tp_utils` para desenhar sobre `IVC`;
- `main.cpp` inclui todos os modulos e coordena a execucao.

Os modulos evitam depender uns dos outros quando nao e necessario. Por exemplo, o tracking nao conhece as regras de calibre, e a segmentacao nao conhece as regras de qualidade.

## Beneficios da Arquitetura

A arquitetura modular traz varios beneficios:

- facilita explicar o projeto na defesa;
- reduz a complexidade do `main.cpp`;
- permite alterar parametros de segmentacao sem mexer no tracking;
- permite alterar regras de classificacao sem mexer nas medicoes;
- separa regras regulamentares da logica de video;
- torna mais claro onde cada problema deve ser resolvido;
- ajuda a controlar o uso de OpenCV.

## Limitacoes Arquiteturais

Apesar da modularizacao, o `main.cpp` ainda concentra alguma logica de apresentacao e de gestao temporal. Isto acontece porque essas partes dependem diretamente de `cv::Mat`, de `cv::putText` e do ciclo de leitura do video.

Outra limitacao e que algumas decisoes dependem simultaneamente de tracking, medicao e classificacao. Por exemplo, atualizar a categoria de uma laranja no lote exige saber se ela ja foi contada e qual o indice correspondente no lote. Por isso, esta coordenacao permanece no `main.cpp`.

Tambem foi evitada a criacao de novos modulos C++ apenas para overlay textual, para nao aumentar a complexidade da compilacao e manter o projeto maioritariamente em C.

## Conclusao

A arquitetura implementada organiza o projeto em modulos com responsabilidades bem definidas. O `main.cpp` coordena o fluxo temporal e a integracao com OpenCV, enquanto os modulos em C concentram a segmentacao, medicao, tracking, classificacao, visualizacao em `IVC` e funcoes auxiliares.

Esta organizacao procura equilibrar clareza, desempenho, restricao de uso de OpenCV e facilidade de defesa do projeto.
