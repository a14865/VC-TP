# Analise do Problema

## Descricao Geral

O trabalho consiste no desenvolvimento de um sistema de Visao por Computador capaz de analisar um video com laranjas em movimento e extrair informacao util sobre cada fruto detetado. O video simula, de forma simplificada, um contexto industrial em que frutos passam sob uma camara e sao analisados automaticamente.

O sistema deve processar o video frame a frame, detetar as laranjas presentes, medir propriedades geometricas, acompanhar os objetos ao longo do tempo, evitar contagens duplicadas e produzir uma classificacao relacionada com o regulamento aplicavel a citrinos.

O problema nao se resume a encontrar objetos cor de laranja. E necessario transformar uma observacao visual instavel num conjunto de decisoes quantitativas: numero de frutos, dimensao aproximada, calibre, homogeneidade, tolerancias e categoria visual. Cada uma dessas decisoes depende da qualidade da segmentacao, da estabilidade das medicoes e da interpretacao correta dos criterios do regulamento.

## Objetivos do Enunciado

O sistema deve cumprir os seguintes objetivos principais:

- Ler e processar o video fornecido.
- Detetar as laranjas presentes em cada frame.
- Contar o numero de laranjas visiveis no frame.
- Manter uma contagem total acumulada ao longo do video.
- Calcular propriedades dos objetos detetados:
  - area;
  - perimetro;
  - bounding box;
  - centro de massa;
  - diametro aproximado.
- Converter medidas em pixeis para milimetros.
- Determinar o calibre das laranjas segundo a escala regulamentar.
- Avaliar homogeneidade do lote.
- Avaliar tolerancias de calibre.
- Atribuir categoria de qualidade visual.
- Apresentar os resultados de forma compreensivel durante a execucao do video.

## Dados e Condicionantes

O video usado no projeto tem resolucao 1280x720 e representa laranjas vistas de cima. A aquisicao nao corresponde a um sistema industrial perfeitamente controlado: o video foi captado com telemovel e com movimento relativo entre a camara e os frutos, simulando um tapete rolante.

Foram identificadas as seguintes condicionantes:

- A iluminacao nao e completamente uniforme ao longo da imagem.
- A cor da laranja varia com sombras, reflexos e orientacao do fruto.
- O fundo pode conter zonas visualmente proximas da cor dos frutos.
- As laranjas podem entrar e sair parcialmente do enquadramento.
- A dimensao aparente do fruto depende da posicao e da qualidade da segmentacao.
- A camara nao garante uma medicao fisica perfeita da seccao equatorial.
- A conversao pixel/mm depende de uma escala assumida no projeto.
- Nem todos os atributos do regulamento sao diretamente observaveis no video.

## Problemas de Segmentacao Identificados

A segmentacao foi uma das partes mais sensiveis do projeto. O objetivo era separar os pixeis pertencentes as laranjas dos restantes pixeis da imagem, mas essa separacao apresentou varios problemas.

Um dos primeiros problemas foi a variacao da cor aparente da laranja. Embora os frutos sejam todos visualmente laranja, a sua representacao na imagem muda com a iluminacao, sombras, reflexos e compressao do video. Um intervalo de cor demasiado restrito perdia partes dos frutos; um intervalo demasiado largo incluia fundo e ruido.

Tambem se verificou que segmentar diretamente em RGB nao era suficientemente robusto. Pequenas alteracoes de luminosidade alteravam muito os valores dos canais, tornando dificil isolar os frutos apenas por limites nos canais vermelho, verde e azul.

Foram analisadas alternativas baseadas em histogramas, filtros e ajustes de cor. Alguns metodos melhoravam casos especificos, mas pioravam outros. Por exemplo, tecnicas mais agressivas de realce ou normalizacao podiam destacar ruido do fundo, alterar a forma dos blobs ou criar falhas no interior das laranjas.

Outro problema foi o equilibrio entre limpeza da mascara e preservacao da forma real. Filtros e operacoes morfologicas ajudam a remover ruido e preencher buracos, mas tambem podem deformar as regioes segmentadas. Quando a mascara fica demasiado suavizada, as medicoes de area, perimetro e bounding box deixam de representar corretamente a laranja.

Houve ainda uma limitacao de desempenho. Algumas abordagens baseadas em processamento mais pesado, como analise detalhada de textura ou filtros aplicados de forma intensiva a toda a imagem, tornavam o video demasiado lento para execucao em tempo real.

## Problemas na Medicao dos Objetos

Depois da segmentacao, os objetos sao representados como blobs. A qualidade dessas medicoes depende diretamente da mascara binaria.

Um problema recorrente foi a variacao da bounding box. Quando a segmentacao falhava numa pequena zona da laranja, a bounding box podia encolher ou deslocar-se. Quando a segmentacao incluia ruido junto ao fruto, a bounding box podia aumentar indevidamente. Isto afetava diretamente o diametro estimado e, por consequencia, o calibre.

O perimetro tambem se mostrou sensivel ao ruido. Pequenas irregularidades na fronteira do blob aumentam o perimetro, mesmo quando visualmente a laranja parece bem segmentada. Isto afeta metricas como circularidade e pode influenciar a categoria de qualidade.

A area e mais estavel do que o perimetro, mas tambem depende de buracos internos e de falhas na mascara. Se uma parte da laranja nao for segmentada, a area diminui; se o fundo for incluído, a area aumenta.

Outro problema foi a conversao de pixeis para milimetros. A escala usada no projeto assume uma relacao fixa entre pixeis e milimetros. No entanto, num video real captado com telemovel, pequenas variacoes de distancia, perspectiva e orientacao podem introduzir erro. A conversao e necessaria para aplicar o regulamento, mas nao e uma medicao fisica perfeita.

## Problemas de Calibre

O calibre das laranjas no regulamento e definido por intervalos de diametro em milimetros. Um problema importante foi a interpretacao desses intervalos, porque varios calibres se sobrepoem.

Por exemplo, determinados diametros podem pertencer a mais do que um intervalo regulamentar. Isto cria uma dificuldade: uma laranja isolada pode caber em varios calibres, mas o lote ou embalagem deve ser coerente com uma designacao comercial.

Tambem surgiu o problema da escolha do diametro a usar. O regulamento refere a seccao equatorial, mas no video temos uma vista superior e uma estimativa baseada na largura do blob. Se a laranja estiver ligeiramente deformada na segmentacao, ou se a bounding box nao refletir exatamente a seccao equatorial, o calibre pode variar.

Outro problema foi a tolerancia de calibre. O regulamento apresenta regras diferentes consoante a forma de apresentacao. Foi necessario distinguir entre a abordagem por calibre mencionado e a abordagem de apresentacao a granel com exigencia de calibre minimo. Esta diferenca exigiu cuidado para nao aplicar uma regra regulamentar fora do contexto escolhido.

## Problemas de Homogeneidade

A homogeneidade do calibre exige comparar o menor e o maior fruto de um mesmo lote. No projeto, o video foi tratado como um lote, o que introduziu algumas dificuldades.

O primeiro problema foi definir quando uma laranja deve entrar no lote. Se forem usadas medicoes enquanto a laranja ainda esta parcialmente visivel, a homogeneidade pode ser afetada por diametros incorretos. Por isso, a contagem e o registo das medicoes dependem de uma linha de ativacao.

Outro problema foi escolher o calibre que define o limite de homogeneidade. O regulamento define limites diferentes para grupos de calibres. Como os intervalos de calibre se sobrepoem, a escolha do calibre representativo do lote influencia o limite admissivel.

Tambem se verificou que uma unica medicao incorreta pode alterar bastante a diferenca entre o menor e o maior diametro. Isto torna a homogeneidade sensivel a erros de segmentacao ou a frutos medidos numa posicao menos favoravel.

## Problemas de Tracking e Contagem

A contagem total exige saber se uma laranja ja foi vista antes. Detetar objetos por frame nao e suficiente, porque a mesma laranja aparece durante varios frames consecutivos.

Um problema foi evitar contagem duplicada. Se a mesma laranja for interpretada como novo objeto em frames sucessivos, o total fica incorreto. Isto pode acontecer quando o blob muda de forma, quando ha falhas de segmentacao ou quando a distancia entre centroides varia demasiado.

Outro problema foi definir uma linha de ativacao. Se a linha estiver demasiado cedo, a laranja pode ainda nao estar completamente visivel ou suficientemente estavel para medir. Se estiver demasiado tarde, pode haver menos tempo para avaliar a qualidade do fruto.

Tambem surgiu o problema da linha inferior de desativacao. Com a introducao de uma barra de informacao no fundo do video, tornou-se necessario evitar que a zona de avaliacao coincidisse com a area da interface visual.

## Problemas na Classificacao de Qualidade

A categoria de qualidade foi uma das partes mais dificeis do projeto. O regulamento descreve caracteristicas como qualidade superior, boa qualidade, defeitos ligeiros de forma, defeitos de coloracao, rugosidade, alteracoes da epiderme e estado geral de conservacao. Muitos destes conceitos sao qualitativos e nao correspondem diretamente a uma unica medida computacional.

Um problema foi transformar texto regulamentar em criterios mensuraveis. Termos como "ligeiro defeito", "boa qualidade", "aspecto geral" ou "qualidade comercial" exigem interpretacao. O video nao fornece informacao suficiente para avaliar todos estes aspetos com rigor absoluto.

Outro problema foi a flutuacao da categoria ao longo do video. A mesma laranja podia parecer melhor numa zona da imagem e pior noutra, devido a iluminacao, movimento, sombras ou variacoes da segmentacao. Isto fazia com que a categoria mudasse de Extra para I, II ou III enquanto a laranja se deslocava.

Tambem se verificou que uma abordagem simples baseada apenas em cor tendia a classificar demasiadas laranjas como Extra. Isso era pouco convincente, porque ignorava forma, textura, rugosidade e defeitos superficiais.

Foram considerados metodos baseados em textura, histogramas, filtros espaciais e deteccao de arestas. O problema foi equilibrar detalhe e desempenho. Analises muito detalhadas em todos os pixeis de todos os frames tornavam a execucao lenta. Por outro lado, analises demasiado simples nao captavam defeitos relevantes.

Outro problema foi a definicao de limiares. O regulamento nao fornece valores numericos para percentagem de cor tipica, rugosidade, densidade de arestas ou circularidade. Esses limites tiveram de ser tratados como aproximacoes praticas.

## Problemas Relacionados com o Calice

O regulamento menciona tolerancias associadas a frutos desprovidos de calice. No entanto, o video nao permite detetar este atributo de forma fiavel.

O calice pode nao estar visivel se a laranja estiver rodada. Mesmo quando esta visivel, pode aparecer como uma pequena regiao escura, confundivel com sombra, textura natural ou defeito superficial. A resolucao e o movimento do video dificultam uma identificacao robusta.

Assim, a presenca ou ausencia de calice foi identificada como um problema regulamentar relevante, mas dificil de observar com os dados disponiveis.

## Problemas de Desempenho

O video deve correr de forma suficientemente fluida para permitir observacao em tempo real. Algumas implementacoes iniciais tornaram o processamento demasiado lento.

Os principais fatores de custo identificados foram:

- calculo de qualidade em todos os frames;
- analise pixel a pixel de textura e arestas;
- aplicacao de filtros pesados sobre imagens completas;
- processamento repetido da imagem para cada blob;
- apresentacao simultanea de janelas de debug;
- uso de imagens temporarias desnecessarias.

Tambem foi necessario ter cuidado com a quantidade de imagens auxiliares usadas no `main.cpp`. Criar buffers para etapas que nao acrescentavam informacao aumentava consumo de memoria e complexidade do pipeline.

Outro problema foi o equilibrio entre visualizacao e desempenho. Mostrar muita informacao textual sobre o video ajuda na interpretacao, mas tambem pode tornar a imagem mais pesada e dificil de ler durante a execucao.

## Problemas de Visualizacao

A informacao apresentada no ecra tinha de ser legivel sem tapar demasiado o video. Inicialmente, os textos globais no canto superior esquerdo ocupavam uma zona relevante da imagem e podiam interferir visualmente com a observacao das laranjas.

Tambem foi necessario evitar introduzir novas dependencias ou aumentar o uso de funcoes OpenCV para a visualizacao. A apresentacao visual teve de respeitar a restricao de uso limitado de OpenCV, mantendo a maior parte do processamento implementada manualmente ou em C.

Outro problema foi a sobreposicao entre a zona de avaliacao e a barra de informacao no fundo. A linha inferior de desativacao teve de considerar a area ocupada pela interface, para que as laranjas nao fossem avaliadas dentro dessa zona.

## Problemas de Modularizacao

Durante o desenvolvimento, parte da logica estava concentrada no `main.cpp`. Isso dificultava a leitura e a defesa do projeto, porque misturava aquisicao de video, segmentacao, medicoes, tracking, classificacao e visualizacao.

Foi necessario separar responsabilidades por modulos:

- segmentacao;
- medicoes;
- tracking;
- classificacao;
- utilitarios;
- visualizacao.

Mesmo com esta separacao, alguns elementos permanecem no `main.cpp` por dependerem diretamente de OpenCV/C++, como leitura de video, conversao de frames, exibicao e escrita de texto sobre `cv::Mat`.

## Problemas de Interpretacao do Regulamento

O regulamento e escrito para classificacao comercial de citrinos, nao especificamente para um sistema automatico baseado num video simples. Isso criou varios problemas de interpretacao.

Alguns criterios sao objetivos, como diametros minimos e escalas de calibre. Outros sao qualitativos, como defeitos ligeiros, aspeto geral, conservacao ou qualidade comercial.

Tambem existem regras diferentes para calibre, homogeneidade e tolerancias, dependentes da forma de apresentacao do produto. Foi necessario perceber que nem todas as regras se aplicam da mesma forma ao contexto simulado pelo video.

Outro problema foi que o regulamento permite tolerancias em numero ou em peso, mas o projeto trabalha com contagem de frutos, nao com peso. Assim, a analise teve de se focar na tolerancia em numero.

## Conclusao

O problema do enunciado exigiu a construcao de um pipeline completo de Visao por Computador, mas tambem exigiu lidar com incertezas praticas: iluminacao, segmentacao imperfeita, ruido, desempenho, interpretacao regulamentar e limitacoes do proprio video.

Os principais problemas encontrados nao estiveram apenas na implementacao tecnica, mas na transformacao de criterios comerciais e visuais em metricas computacionais suficientemente estaveis para funcionarem frame a frame.

Esta analise identifica os desafios encontrados durante o desenvolvimento e serve de base para explicar, na defesa, porque o pipeline final teve de equilibrar robustez, simplicidade, desempenho e interpretacao do regulamento.
