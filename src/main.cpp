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

#include <iostream>
#include <string>
#include <chrono>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>

using namespace cv;
using namespace std;

extern "C"
{
#include "../include/vc.h"
}

int main(void)
{

  char videofile[20] = "video.avi";

  VideoCapture capture;

  struct
  {
    int width, height;
    int nTotalFrames;
    int fps;
    int nFrame;
  } video;

  // Leitura do vídeo em questão
  capture.open(videofile);

  if (!capture.isOpened())
  {

    cerr << "Erro ao abrir o ficheiro de vídeo!\n";
    return 1;
  }

  // Número total de frames no vídeo
  video.nTotalFrames = (int)capture.get(CAP_PROP_FRAME_COUNT);

  // Frame rate do vídeo
  video.fps = (int)capture.get(CAP_PROP_FPS);

  // Resolução do vídeo
  video.width = (int)capture.get(CAP_PROP_FRAME_WIDTH);
  video.height = (int)capture.get(CAP_PROP_FRAME_HEIGHT);

  Mat frame;

  // Inicio do vídeo
  while (key != 'q')
  {

    // Verifica a leitura da frame
    if (!capture.read(frame))
      break;

    video.nFrame = (int)capture.get(CAP_PROP_POS_FRAMES);

    // Criação da imagem e tradução Mat -> IVC
    IVC *image = vc_image_new(video.width, video.height, 3, 255);
    memcpy(image->data, frame.data, video.width * video.height * 3);


    // Nosso código

    // Fim nosso código


    // Tradução IVC -> Mat
    memcpy(frame.data, image->data, video.width * video.height * 3);
    vc_image_free(image);

    imshow("VC - VIDEO", frame);

    key = waitKey(25);
  }

  capture.release();

  return 0;
}