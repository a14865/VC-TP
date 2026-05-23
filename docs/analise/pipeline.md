# Pipeline de Processamento

## Diagrama Global

```mermaid
flowchart TD
    A["Video AVI<br/>data/raw/video.avi"] --> B["Frame OpenCV<br/>cv::Mat BGR"]
    B --> C["cvtColor<br/>BGR -> RGB"]
    C --> D["Copia para IVC<br/>image RGB"]

    D --> E["Conversao de cor<br/>RGB -> HSV<br/>vc_rgb_to_hsv"]
    E --> F["Segmentacao HSV<br/>H: 10-34<br/>S: 30-100<br/>V: 0-100"]
    F --> G["Mascara binaria<br/>imageSEG"]

    G --> H["Filtro espacial<br/>medianBlur 5x5"]
    H --> I["Morfologia<br/>Fecho: preencher falhas"]
    I --> J["Morfologia<br/>Abertura: remover ruido"]
    J --> K["Mascara limpa<br/>frameSeg / imageSEG"]

    K --> L["Etiquetagem<br/>Componentes conexas"]
    L --> M["Imagem de labels<br/>imageLabels"]
    L --> N["Lista de blobs<br/>OVC[]"]

    M --> O["Medicoes dos blobs<br/>area, perimetro,<br/>bounding box, centro"]
    N --> O
    O --> P{"Area >= 60000 px?"}
    P -->|Nao| Q["Ignorar blob"]
    P -->|Sim| R["Laranja candidata"]

    R --> S["Tracking temporal<br/>distancia entre centroides"]
    S --> T{"Cruza linha<br/>de ativacao?"}
    T -->|Sim| U["Incrementa total<br/>regista no lote"]
    T -->|Nao| V["Mantem associacao<br/>com frame anterior"]

    R --> W{"Dentro da<br/>zona util?"}
    W -->|Sim| X["Classificacao visual<br/>forma + cor + textura"]
    W -->|Nao| Y["Mantem categoria<br/>anterior"]

    X --> X1["Histograma de intensidade<br/>P10 / P90 / contraste"]
    X --> X2["Filtro espacial local<br/>mediana 3x3"]
    X --> X3["Arestas locais<br/>gradiente tipo Prewitt/Sobel"]
    X --> X4["Metricas de cor<br/>cor tipica / defeitos"]
    X --> X5["Metricas de forma<br/>circularidade / preenchimento"]

    U --> Z["Conversao px -> mm<br/>diametro, area, perimetro"]
    Z --> AA["Calibre individual"]
    AA --> AB["Estatisticas do lote<br/>calibre dominante,<br/>homogeneidade,<br/>tolerancia minimo"]

    X --> AC["Categoria individual"]
    AC --> AD["Categoria do lote<br/>tolerancia qualidade"]

    AB --> AE["Overlay final"]
    AD --> AE
    O --> AE
    S --> AE

    AE --> AF["Desenho em IVC<br/>bounding box + centro"]
    AF --> AG["Copia para cv::Mat<br/>RGB -> BGR"]
    AG --> AH["Texto com putText<br/>barra inferior + dados por laranja"]
    AH --> AI["imshow<br/>resultado final"]
```

## Diagrama por Tipo de Dados

```mermaid
flowchart LR
    subgraph OpenCV["Camada OpenCV / C++"]
        A["cv::Mat frame<br/>BGR"]
        B["cv::Mat frameRGB<br/>RGB"]
        C["cv::Mat frameSeg<br/>Mascara binaria"]
        D["cv::Mat frame final<br/>Visualizacao"]
    end

    subgraph CData["Camada C / Estruturas do projeto"]
        E["IVC image<br/>RGB"]
        F["IVC imageHSV<br/>HSV"]
        G["IVC imageSEG<br/>Binaria"]
        H["IVC imageLabels<br/>Labels"]
        I["OVC blobs[]<br/>Objetos medidos"]
        J["OrangeBatchStats<br/>Estado do lote"]
        K["OrangeQualityMetrics<br/>Categoria visual"]
    end

    A --> B --> E --> F --> G --> C --> G --> H --> I
    I --> K
    I --> J
    K --> J
    E --> D
    J --> D
    K --> D
```

## Tecnicas Aplicadas no Pipeline

```mermaid
mindmap
  root((Pipeline))
    Entrada
      Video AVI
      Frame BGR
      Conversao BGR-RGB
    Segmentacao
      RGB para HSV
      Threshold HSV
      Mascara binaria
    Filtros
      Mediana 5x5
      Mediana local 3x3
      Sem filtros de frequencia na versao final
    Morfologia
      Fecho
      Abertura
      Kernel eliptico
    Blobs
      Componentes conexas
      Area
      Perimetro
      Bounding box
      Centro de massa
    Tracking
      Centroides
      Linha de ativacao
      Linha de desativacao
    Classificacao
      Calibre
      Categoria visual
      Histograma intensidade
      Arestas locais
      Cor tipica
      Rugosidade
    Lote
      Homogeneidade
      Tolerancia minimo
      Categoria lote
      Tolerancia qualidade
    Visualizacao
      Bounding box
      Centro
      Barra inferior
      Texto informativo
```

## Subpipeline de Segmentacao

```mermaid
flowchart LR
    A["Frame RGB"] --> B["RGB -> HSV"]
    B --> C["Threshold HSV<br/>laranja"]
    C --> D["Mascara binaria"]
    D --> E["medianBlur<br/>reducao de ruido"]
    E --> F["Fecho morfologico<br/>preencher falhas"]
    F --> G["Abertura morfologica<br/>remover ruido"]
    G --> H["Mascara limpa"]
```

## Subpipeline de Classificacao Visual

```mermaid
flowchart TD
    A["Blob da laranja<br/>+ imagem RGB<br/>+ labels"] --> B["Amostragem na zona do blob"]

    B --> C["Forma"]
    C --> C1["Circularidade"]
    C --> C2["Relacao largura/altura"]
    C --> C3["Preenchimento da bounding box"]

    B --> D["Cor"]
    D --> D1["Pixeis com cor tipica"]
    D --> D2["Pixeis com possivel defeito"]
    D --> D3["Variacao face a media RGB"]

    B --> E["Textura / intensidade"]
    E --> E1["Mediana local 3x3"]
    E --> E2["Rugosidade por diferencas locais"]
    E --> E3["Gradiente tipo Prewitt/Sobel"]
    E --> E4["Densidade de arestas"]

    B --> F["Histograma"]
    F --> F1["Histograma 0-255"]
    F --> F2["Percentil 10"]
    F --> F3["Percentil 90"]
    F --> F4["Contraste P90-P10"]

    C1 --> G["Regras de categoria"]
    C2 --> G
    C3 --> G
    D1 --> G
    D2 --> G
    D3 --> G
    E2 --> G
    E4 --> G
    F4 --> G

    G --> H["Extra / I / II / III / Rejeitada"]
```

## Subpipeline de Lote

```mermaid
flowchart TD
    A["Laranja contada"] --> B["Guardar diametro mm"]
    A --> C["Guardar calibre"]
    A --> D["Guardar categoria estabilizada"]

    B --> E["Minimo e maximo diametro"]
    C --> F["Calibre dominante / melhor calibre"]
    D --> G["Melhor categoria valida"]

    E --> H["Homogeneidade<br/>diferenca max-min"]
    F --> H

    B --> I["Tolerancia calibre<br/>minimo 53 mm<br/>tolerancia ate 50 mm"]

    G --> J["Tolerancia qualidade<br/>Extra 5%<br/>I 10%<br/>II 10%<br/>III 15%"]

    H --> K["Overlay lote"]
    I --> K
    J --> K
```
