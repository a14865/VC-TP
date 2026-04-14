/*
===============================================================================
FICHEIRO: vc.c

DESCRIÇÃO:
Este ficheiro contém a implementação da biblioteca base de processamento de
imagem usada no projeto. Aqui devem ficar as funções genéricas e reutilizáveis
sobre imagens, independentes da lógica específica do trabalho prático.

OBJETIVO:
Disponibilizar operações fundamentais sobre imagens, como leitura, escrita,
conversões, binarização, morfologia, labeling e análise de blobs, de forma
modular e reutilizável.

O QUE DEVE INCLUIR:
- Gestão de memória da estrutura IVC.
- Leitura e escrita de imagens.
- Funções de conversão de imagem.
- Operações de binarização.
- Operações morfológicas.
- Etiquetagem de componentes conectados.
- Funções auxiliares de análise de blobs.
- Funções genéricas que possam ser reutilizadas noutras fases do projeto.

O QUE NÃO DEVE INCLUIR:
- Regras específicas do trabalho das laranjas.
- Cálculo de calibre comercial.
- Lógica de tracking entre frames.
- Regras de negócio associadas ao enunciado.

PORQUE EXISTE:
Este ficheiro existe para isolar a base técnica de processamento de imagem da
aplicação concreta do TP. Desta forma, a biblioteca pode ser usada por vários
módulos do projeto sem duplicação de código.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- É utilizado por praticamente todos os módulos.
- Deve ser incluído através do cabeçalho vc.h.
- Serve de base ao processamento de segmentação e análise.

NOTAS:
Todas as funções implementadas neste ficheiro devem ser o mais genéricas,
robustas e independentes possível. Sempre que uma função puder ser reutilizada,
deve preferencialmente estar aqui e não num módulo específico do TP.
===============================================================================
*/