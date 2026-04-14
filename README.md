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

O sistema será desenvolvido recorrendo a **C/C++**, podendo integrar bibliotecas como OpenCV.

---

## 🧱 Estrutura do Repositório

A organização do projeto segue uma abordagem modular, separando código, dados e documentação:

```text
.
├── data/
│   ├── raw/          # Dados originais (ex: vídeo de entrada)
│   └── samples/      # Amostras para testes
│
├── docs/
│   ├── enunciado/    # Documentação oficial do trabalho
│   └── notas/        # Notas e decisões técnicas
│
├── examples/         # Código de exemplo/apoio
│
├── include/          # Ficheiros de cabeçalho (.h)
│
├── src/              # Implementação (.c / .cpp)
│
├── results/
│   ├── frames/       # Frames gerados (output)
│   └── videos/       # Vídeos processados (output)
│
├── .github/          # Templates e configuração do GitHub
├── .vscode/          # Configuração local do editor
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
| Ricardo Pinto | XXXXX  | XXXXXXXXXXXXXXXXXXXXXX |
| Nuno Oliveira | XXXXX  | XXXXXXXXXXXXXXXXXXXXXX |
| José Ferreira | XXXXX  | XXXXXXXXXXXXXXXXXXXXXX |
| Raul Ribeiro  | XXXXX  | XXXXXXXXXXXXXXXXXXXXXX |
