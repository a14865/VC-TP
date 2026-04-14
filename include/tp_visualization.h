/*
===============================================================================
FICHEIRO: tp_visualization.h

DESCRIÇÃO:
Este ficheiro declara as funções do módulo de visualização responsáveis por
desenhar sobre as frames do vídeo a informação produzida pelo sistema.

OBJETIVO:
Disponibilizar a interface do módulo de visualização aos restantes ficheiros do
projeto, permitindo a utilização organizada das rotinas de apresentação gráfica.

O QUE DEVE INCLUIR:
- Protótipos das funções de desenho e anotação.
- Inclusões necessárias para definição dos tipos utilizados.
- Estruturas ou constantes relacionadas exclusivamente com a visualização,
  caso sejam necessárias.

O QUE NÃO DEVE INCLUIR:
- Implementação das funções.
- Algoritmos de segmentação, tracking ou classificação.
- Código que não esteja diretamente relacionado com a interface do módulo.

PORQUE EXISTE:
Este ficheiro permite separar a declaração da implementação, promovendo uma
organização modular do código e facilitando a integração do módulo de
visualização com o restante sistema.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Deve ser incluído no ficheiro principal e em qualquer módulo que necessite de
  invocar funções de visualização.
- Está diretamente associado a tp_visualization.c.

NOTAS:
Os protótipos aqui definidos devem representar apenas operações de visualização.
A lógica de cálculo dos dados apresentados deve permanecer nos módulos próprios.
===============================================================================
*/