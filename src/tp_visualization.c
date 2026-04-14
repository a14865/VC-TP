/*
===============================================================================
FICHEIRO: tp_visualization.c

DESCRIÇÃO:
Este ficheiro contém as funções responsáveis pela apresentação visual da
informação produzida pelo sistema sobre cada frame do vídeo processado.

OBJETIVO:
Centralizar toda a lógica de desenho e anotação gráfica, permitindo exibir de
forma clara e organizada os resultados obtidos pelos módulos de segmentação,
medição, tracking e classificação.

O QUE DEVE INCLUIR:
- Funções para desenhar texto informativo sobre a frame.
- Funções para desenhar bounding boxes das laranjas detetadas.
- Funções para assinalar o centro de gravidade dos objetos.
- Funções para apresentar dados como área, perímetro, calibre e categoria.
- Funções para mostrar contadores globais, como número atual e total acumulado.
- Rotinas de visualização dos resultados finais do pipeline.

O QUE NÃO DEVE INCLUIR:
- Algoritmos de segmentação.
- Cálculo de blobs, áreas ou perímetros.
- Lógica de tracking.
- Regras de classificação.
- Código de processamento de imagem que altere a deteção dos objetos.

PORQUE EXISTE:
A visualização é uma responsabilidade distinta da análise. Separar esta parte
num ficheiro próprio melhora a organização do projeto, reduz a complexidade do
ficheiro principal e facilita a manutenção, testes e defesa oral do trabalho.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Recebe informação calculada por tp_segmentation.c, tp_measurements.c,
  tp_tracking.c e tp_classification.c.
- Pode utilizar funções auxiliares de tp_utils.c.
- Deve ser incluído através de tp_visualization.h.
- É normalmente chamado a partir do ficheiro principal, onde é executado o
  ciclo de processamento do vídeo.

NOTAS:
Este ficheiro deve tratar apenas da apresentação visual dos resultados, e não da
sua obtenção. Sempre que possível, as funções devem receber dados já calculados
e limitar-se a desenhá-los na frame de saída.
===============================================================================
*/