# 🧠 Projeto de Visão por Computador

## 📌 Descrição

Este repositório contém o desenvolvimento de um projeto no âmbito da unidade curricular de **Visão por Computador**, inserida no 2º ano da Licenciatura em Engenharia de Sistemas Informáticos.

O objetivo principal consiste na implementação de um sistema baseado em técnicas de visão computacional para análise de imagens e/ou vídeo, incluindo etapas como segmentação, extração de características, reconhecimento de padrões e análise de objetos.

---

## 🎯 Objetivo do Projeto

Desenvolver um pipeline de processamento de imagem/vídeo capaz de:

* Adquirir dados visuais (imagem ou vídeo)
* Aplicar técnicas de pré-processamento
* Segmentar objetos de interesse
* Extrair características relevantes
* Realizar análise e/ou classificação dos objetos

O sistema será desenvolvido recorrendo a **C/C++**, podendo integrar funcionalidades limitadas da biblioteca OpenCV, conforme definido no enunciado.

---

## 🧱 Estrutura do Repositório

A organização do projeto segue uma abordagem modular, separando código, dados, documentação e ferramentas:

```text
.
├── data/
│   ├── raw/              # Dados originais (ex: vídeo de entrada)
│   └── samples/          # Amostras para testes
│
├── docs/
│   ├── enunciado/        # Documentação oficial do trabalho
│   └── notas/            # Notas e decisões técnicas
│
├── examples/             # Código de exemplo/apoio
│
├── include/              # Ficheiros de cabeçalho (.h)
│
├── src/                  # Implementação (.c / .cpp)
│
├── results/
│   ├── frames/           # Frames gerados (output)
│   └── videos/           # Vídeos processados (output)
│
├── tools/                # Ferramentas externas de apoio
│   └── filtergear/
│       └── FilterGear.exe
│
├── .github/              # Templates e configuração do GitHub
├── .vscode/              # Configuração local do editor
│
├── .gitignore
└── README.md
```

---

## ⚙️ Organização do Código

O código está dividido em módulos funcionais:

* `tp_segmentation` → Segmentação de imagem
* `tp_measurements` → Medição de propriedades (área, perímetro, etc.)
* `tp_classification` → Classificação de objetos
* `tp_tracking` → Acompanhamento temporal
* `tp_utils` → Funções auxiliares
* `vc` → Biblioteca base de processamento de imagem

Esta estrutura permite uma abordagem modular e escalável, alinhada com sistemas reais de visão por computador.

---

## 🔍 Visualização e Debug

Para validação das diferentes etapas do pipeline (segmentação, morfologia, blobs), é utilizada a ferramenta externa **FilterGear**.

### 📌 Localização

```text
tools/filtergear/FilterGear.exe
```

### ▶️ Exemplo de utilização

```c
system("cmd /c start tools\\filtergear\\FilterGear.exe results\\frames\\output.pgm");
```

### 🎯 Objetivo

O FilterGear permite:

* Visualizar imagens intermédias (PGM/PPM)
* Validar resultados de segmentação
* Analisar efeitos de operações morfológicas
* Confirmar a integridade dos blobs

A sua utilização é essencial para debugging em pipelines de processamento de imagem, onde a validação visual é crítica.

---

## ▶️ Compilação e Execução

*(A completar nas próximas fases do projeto)*

Exemplo genérico:

```bash
g++ src/main.cpp src/*.c -o main.exe
./main.exe
```

---

## 📊 Dados

* `data/raw/` contém os dados originais (ex: vídeo de entrada)
* `data/samples/` contém subconjuntos para testes rápidos

Os resultados gerados são armazenados em `results/` e não fazem parte do controlo de versões.

---

## 📝 Notas

Este README corresponde à versão inicial do projeto e será expandido ao longo do desenvolvimento, incluindo:

* descrição detalhada do pipeline
* resultados experimentais
* parâmetros utilizados
* análise de desempenho

---

## 👥 Grupo de Trabalho

| Nome          | Número | Função no Projeto      |
| ------------- | ------ | ---------------------- |
| Ricardo Pinto | 14865  | XXXXXXXXXXXXXXXXXXXXXX |
| Raul Ribeiro  | 22552  | XXXXXXXXXXXXXXXXXXXXXX |
| Nuno Oliveira | 31550  | XXXXXXXXXXXXXXXXXXXXXX |
| José Ferreira | 31556  | XXXXXXXXXXXXXXXXXXXXXX |

---

## 📚 Referências

Gonzalez, R. C., & Woods, R. E. (2002). *Digital Image Processing*. Prentice Hall.

Ribeiro, N. (2025). *Apontamentos da Unidade Curricular de Visão por Computador*.

Kernighan, B., & Ritchie, D. (1988). *The C Programming Language*. Prentice Hall.

---
