# Parametros de categorizacao da qualidade das laranjas

Este documento resume os criterios usados pela funcao `orangeQualityCategory`
para atribuir a categoria de qualidade a cada laranja detetada.

A classificacao e feita por ordem: primeiro tenta classificar como **Extra**,
depois **I**, depois **II**, depois **III**. Se nenhuma regra for cumprida, a
laranja fica como **Rejeitada**.

## Como a categoria e atribuida

A classificacao nao usa uma media nem uma pontuacao final. A laranja tem de
cumprir todos os parametros de uma categoria para receber essa categoria.

O processo e sequencial:

1. Testa se a laranja cumpre todos os criterios da categoria **Extra**.
2. Se falhar pelo menos um criterio de **Extra**, testa os criterios da
   categoria **I**.
3. Se falhar pelo menos um criterio de **I**, testa os criterios da categoria
   **II**.
4. Se falhar pelo menos um criterio de **II**, testa os criterios da categoria
   **III**.
5. Se tambem falhar a categoria **III**, fica como **Rejeitada**.

Isto significa que basta nao cumprir um parametro de uma categoria para nao
ficar nessa categoria. No entanto, isso nao quer dizer que desce
automaticamente so um nivel. Por exemplo, uma laranja pode falhar **Extra** e
tambem falhar **I**, mas ainda cumprir **II**. Tambem pode falhar diretamente
todas as categorias e ser **Rejeitada**.

Exemplo:

- Se uma laranja tem todos os valores de **Extra**, exceto
  `typicalColorPercent = 88`, nao pode ser **Extra**, porque **Extra** exige
  `typicalColorPercent >= 92`.
- Depois e testada em **I**. Como **I** exige apenas
  `typicalColorPercent >= 80`, pode ser classificada como **I**, desde que
  cumpra tambem todos os outros parametros de **I**.

## Metricas usadas

| Metrica | Significado | Melhor quando |
| --- | --- | --- |
| `circularityPercent` | Circularidade aproximada da laranja | Maior |
| `shapeRatioPercent` | Relacao entre a menor e a maior dimensao | Maior |
| `fillRatioPercent` | Percentagem da bounding box ocupada pelo blob | Maior |
| `typicalColorPercent` | Percentagem de pixeis com cor tipica de laranja | Maior |
| `defectColorPercent` | Percentagem de pixeis considerados defeito de cor | Menor |
| `colorVariation` | Variacao media de cor face a media da laranja | Menor |
| `roughness` | Estimativa de rugosidade por diferencas locais de intensidade | Menor |
| `edgeDensityPercent` | Densidade de arestas na superficie da laranja | Menor |
| `intensityContrast` | Diferenca entre os percentis 90 e 10 da intensidade | Menor |

## Porque estes valores foram escolhidos

Os valores foram escolhidos como limiares praticos para separar laranjas com
aspeto visual muito bom, aceitavel ou degradado, usando as caracteristicas que
o algoritmo consegue medir no video.

As categorias seguem uma logica progressiva:

| Categoria | Ideia geral |
| --- | --- |
| **Extra** | Muito exigente: laranja quase circular, bem preenchida, cor muito tipica, poucos defeitos e pouca textura irregular. |
| **I** | Ainda exigente, mas permite pequenas deformacoes, alguma variacao de cor e ligeira rugosidade. |
| **II** | Mais permissiva: aceita forma menos regular, menos cor tipica e mais defeitos visuais. |
| **III** | Categoria minima: usa poucos criterios essenciais para aceitar laranjas que ainda sao reconheciveis como laranjas, mas com qualidade baixa. |
| **Rejeitada** | Nao cumpre sequer os limites minimos da Categoria III. |

Os parametros em que valores altos sao bons, como `circularityPercent`,
`shapeRatioPercent`, `fillRatioPercent` e `typicalColorPercent`, vao diminuindo
de exigencia de **Extra** para **III**.

Os parametros em que valores altos sao maus, como `defectColorPercent`,
`colorVariation`, `roughness`, `edgeDensityPercent` e `intensityContrast`, vao
aumentando de tolerancia de **Extra** para **III**.

Assim, os valores nao representam regras oficiais isoladas para cada metrica.
Funcionam como uma aproximacao computacional baseada na imagem: forma, cor,
defeitos, textura e contraste.

## Categoria Extra

Uma laranja e classificada como **Extra** quando cumpre todos os seguintes
criterios:

| Metrica | Condicao |
| --- | --- |
| `circularityPercent` | >= 82 |
| `shapeRatioPercent` | >= 94 |
| `fillRatioPercent` | >= 68 |
| `typicalColorPercent` | >= 92 |
| `defectColorPercent` | <= 2 |
| `colorVariation` | <= 28 |
| `roughness` | <= 8 |
| `edgeDensityPercent` | <= 2 |
| `intensityContrast` | <= 45 |

### Justificacao dos valores da Categoria Extra

Estes valores representam uma laranja de qualidade visual muito elevada. Nao
sao valores oficiais absolutos do regulamento; sao limiares praticos definidos
para o algoritmo distinguir, no video, uma laranja com forma regular, cor
uniforme e poucos defeitos visiveis.

| Metrica | Porque este limite e exigente |
| --- | --- |
| `circularityPercent >= 82` | Exige que o contorno seja bastante proximo de uma circunferencia. Uma laranja Extra deve ter forma regular, sem grandes deformacoes. |
| `shapeRatioPercent >= 94` | Obriga a que a largura e a altura sejam muito semelhantes. Se a menor dimensao for pelo menos 94% da maior, o fruto e quase redondo. |
| `fillRatioPercent >= 68` | Garante que o blob ocupa bem a sua bounding box. Valores baixos indicam recortes, falhas de segmentacao, occlusoes ou forma pouco compacta. |
| `typicalColorPercent >= 92` | Exige que quase todos os pixeis tenham cor compativel com laranja madura. Para Extra, a cor deve ser muito uniforme e caracteristica. |
| `defectColorPercent <= 2` | Permite apenas uma quantidade residual de manchas, zonas escuras, esverdeadas, azuladas ou palidas. |
| `colorVariation <= 28` | Limita a diferenca media da cor dos pixeis face a cor media da laranja. Quanto menor, mais uniforme e a superficie. |
| `roughness <= 8` | Exige pouca variacao local de intensidade, ou seja, uma superficie visualmente suave, sem textura irregular forte. |
| `edgeDensityPercent <= 2` | Permite poucas arestas internas. Muitas arestas sugerem manchas, rugosidade, defeitos ou ruido na imagem. |
| `intensityContrast <= 45` | Limita a diferenca entre zonas claras e escuras. Para Extra, a iluminacao e a aparencia da casca devem ser relativamente homogeneas. |

Em conjunto, estes valores tornam a categoria **Extra** dificil de atingir.
Uma laranja pode parecer boa em forma, mas se tiver demasiadas manchas, muita
variacao de cor ou textura irregular, deixa de cumprir **Extra** e passa a ser
testada na categoria **I**.

## Categoria I

Uma laranja e classificada como **I** quando nao cumpre os criterios de
**Extra**, mas cumpre todos os seguintes criterios:

| Metrica | Condicao |
| --- | --- |
| `circularityPercent` | >= 70 |
| `shapeRatioPercent` | >= 86 |
| `fillRatioPercent` | >= 60 |
| `typicalColorPercent` | >= 80 |
| `defectColorPercent` | <= 8 |
| `colorVariation` | <= 45 |
| `roughness` | <= 14 |
| `edgeDensityPercent` | <= 6 |
| `intensityContrast` | <= 70 |

## Categoria II

Uma laranja e classificada como **II** quando nao cumpre os criterios de
**Extra** nem de **I**, mas cumpre todos os seguintes criterios:

| Metrica | Condicao |
| --- | --- |
| `circularityPercent` | >= 55 |
| `shapeRatioPercent` | >= 74 |
| `fillRatioPercent` | >= 48 |
| `typicalColorPercent` | >= 62 |
| `defectColorPercent` | <= 22 |
| `colorVariation` | <= 70 |
| `roughness` | <= 24 |
| `edgeDensityPercent` | <= 14 |
| `intensityContrast` | <= 100 |

## Categoria III

Uma laranja e classificada como **III** quando nao cumpre os criterios das
categorias anteriores, mas cumpre todos os seguintes criterios:

| Metrica | Condicao |
| --- | --- |
| `fillRatioPercent` | >= 38 |
| `typicalColorPercent` | >= 42 |
| `defectColorPercent` | <= 36 |
| `roughness` | <= 34 |

Nota: no codigo, a Categoria III e tratada como suplementar. O calice nao e
usado porque nao e considerado fiavel no video.

## Rejeitada

A laranja fica como **Rejeitada** quando nao cumpre os criterios minimos da
Categoria III.

Tambem e usada como valor por defeito antes da qualidade ser calculada, para
evitar que uma laranja sem classificacao valida seja considerada boa por engano.
