/*
===============================================================================
FICHEIRO: vc.h

DESCRIÇÃO:
Este ficheiro define as estruturas de dados, tipos e protótipos da biblioteca
base de processamento de imagem do projeto.

OBJETIVO:
Declarar de forma clara e centralizada a interface pública da biblioteca base,
permitindo que os restantes módulos utilizem as funções implementadas em vc.c.

O QUE DEVE INCLUIR:
- Definição da estrutura IVC.
- Definição de estruturas auxiliares, se necessário.
- Protótipos de funções implementadas em vc.c.
- Constantes e macros relevantes para a biblioteca base.

O QUE NÃO DEVE INCLUIR:
- Implementações de funções.
- Código específico do main.
- Regras específicas de classificação das laranjas.
- Código de interface com OpenCV que não seja estritamente necessário.

PORQUE EXISTE:
Este ficheiro existe para separar a interface da implementação. Isso melhora a
organização, facilita a compilação e torna mais clara a utilização da biblioteca
por outros módulos do projeto.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- É incluído no main e nos restantes módulos que necessitem de aceder à
  estrutura IVC e às funções base da biblioteca.
- Está diretamente associado ao ficheiro vc.c.

NOTAS:
Qualquer nova função genérica adicionada a vc.c deve ter o respetivo protótipo
declarado aqui. Este ficheiro deve ser mantido limpo e coerente.
===============================================================================
*/