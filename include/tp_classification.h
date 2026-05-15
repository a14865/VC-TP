/*
===============================================================================
FICHEIRO: tp_classification.h

DESCRIÇÃO:
Este ficheiro declara a interface pública do módulo de classificação.

OBJETIVO:
Definir os protótipos e estruturas necessárias para que os restantes módulos
possam utilizar as funções de classificação implementadas em
tp_classification.c.

O QUE DEVE INCLUIR:
- Protótipos das funções de classificação.
- Estruturas, enums ou constantes associadas às categorias ou calibres.
- Eventuais tabelas de apoio, se fizer sentido declarar no cabeçalho.

O QUE NÃO DEVE INCLUIR:
- Implementação de funções.
- Código de segmentação ou medição.
- Lógica de tracking.

PORQUE EXISTE:
Este ficheiro existe para expor de forma clara a interface do módulo de
classificação e permitir a sua utilização noutros pontos do projeto.

RELAÇÃO COM OS RESTANTES FICHEIROS:
- Está associado a tp_classification.c.
- Será usado pelo main e possivelmente pelo módulo de tracking ou de overlay.

NOTAS:
É recomendável que as categorias sejam representadas de forma consistente,
por exemplo através de enums ou identificadores claros.
===============================================================================
*/
#include <stdio.h>

OVC *vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);
int vc_draw_bounding_box_all_blobs(IVC* srcdst, OVC* blobs, int nlabels, int padding, int thickness, int colorR, int colorG, int colorB);
int vc_draw_center_mass_all_blobs(IVC* srcdst, OVC* blobs, int nlabels, int kernel, int thickness, int colorR, int colorG, int colorB);