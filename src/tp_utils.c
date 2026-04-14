/*
===============================================================================
FICHEIRO: tp_utils.c

DESCRIÇÃO:
Este ficheiro contém funções auxiliares de uso transversal no projeto que não
pertencem exclusivamente a nenhum dos módulos principais.

OBJETIVO:
Reunir utilitários que simplificam o código e evitam duplicação, mantendo a
aplicação organizada.

O QUE DEVE INCLUIR:
- Funções de apoio genérico.
- Conversões auxiliares entre estruturas.
- Funções simples de desenho ou anotação, se necessário.
- Rotinas de debug, validação ou apoio ao desenvolvimento.
- Pequenas funções reutilizáveis por vários módulos.

O QUE NÃO DEVE INCLUIR:
- Algoritmos principais de segmentação.
- Regras de classificação.
- Tracking complexo.
- Código demasiado específico de um único módulo.

PORQUE EXISTE:
Em projetos médios é comum surgirem funções pequenas mas úteis que não têm um
local natural noutro módulo. Este ficheiro existe para evitar espalhar essas
rotinas por vários ficheiros sem critério.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Pode ser utilizado por vários módulos.
- Deve ser incluído através de tp_utils.h.

NOTAS:
Só devem ser colocadas aqui funções verdadeiramente auxiliares e reutilizáveis.
Este ficheiro não deve tornar-se um depósito desorganizado de código.
===============================================================================
*/