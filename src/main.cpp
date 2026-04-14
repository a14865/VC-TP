/*
===============================================================================
FICHEIRO: main.cpp

DESCRIÇÃO:
Este ficheiro contém o ponto de entrada principal da aplicação.
A sua função é coordenar o fluxo global do programa, desde a abertura do vídeo
até ao processamento frame a frame e apresentação dos resultados ao utilizador.

OBJETIVO:
Centralizar a execução do pipeline do trabalho prático sem concentrar aqui a
lógica detalhada de processamento de imagem. O ficheiro deve funcionar como
orquestrador dos vários módulos do projeto.

O QUE DEVE INCLUIR:
- Inicialização da aplicação.
- Abertura do ficheiro de vídeo.
- Leitura sequencial de cada frame.
- Conversão da frame para a estrutura interna usada no projeto.
- Chamada às funções de segmentação, medição, classificação e tracking.
- Apresentação gráfica dos resultados na frame.
- Gestão da contagem acumulada ao longo do vídeo.
- Libertação de memória e encerramento do programa.

O QUE NÃO DEVE INCLUIR:
- Implementação detalhada de algoritmos de binarização.
- Implementação de morfologia, labeling ou análise de blobs.
- Cálculos complexos de classificação.
- Funções auxiliares extensas que devem estar noutros módulos.

PORQUE EXISTE:
Este ficheiro existe para separar a lógica de controlo da aplicação da lógica
algorítmica. Isso torna o programa mais organizado, mais fácil de manter,
testar, explicar e dividir entre os elementos do grupo.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Usa a biblioteca base de imagem definida em vc.h / vc.c.
- Usa os módulos específicos do TP: segmentação, medições, classificação e
  tracking.
- Não deve duplicar código existente noutros ficheiros.

NOTAS:
A estrutura deste ficheiro deve ser simples e legível, permitindo perceber
rapidamente o pipeline completo de processamento de cada frame.
===============================================================================
*/