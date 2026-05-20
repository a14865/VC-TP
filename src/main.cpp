#include <iostream>
#include <string>
#include <chrono>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>
#include <math.h>

using namespace cv;
using namespace std;

extern "C"
{
#include "../include/tp_utils.h"
#include "../include/tp_classification.h"
#include "../include/tp_segmentation.h"
}

int main(void)
{

    string videofile = "../data/raw/video.avi";
    char key = 0;

    VideoCapture capture;

    struct
    {
        int width, height;
        int nTotalFrames;
        int fps;
        int nFrame;
    } video;

    capture.open(videofile);

    if (!capture.isOpened())
    {
        cerr << "Erro ao abrir o ficheiro de video!\n";
        return 1;
    }

    video.nTotalFrames = (int)capture.get(CAP_PROP_FRAME_COUNT);
    video.fps = (int)capture.get(CAP_PROP_FPS);
    video.width = (int)capture.get(CAP_PROP_FRAME_WIDTH);
    video.height = (int)capture.get(CAP_PROP_FRAME_HEIGHT);

    Mat frame;
    Mat frameRGB;
    Mat frameSeg(video.height, video.width, CV_8UC1);

    // Alocação de memória para imagens auxiliares
    IVC *image = vc_image_new(video.width, video.height, 3, 255);
    IVC *imageHSV = vc_image_new(video.width, video.height, 3, 255);
    IVC *imageHSVEqualized = vc_image_new(video.width, video.height, 3, 255);
    IVC *imageHSVMod = vc_image_new(video.width, video.height, 3, 255);
    IVC *imageSEG = vc_image_new(video.width, video.height, 1, 255);
    IVC *imageOpen = vc_image_new(video.width, video.height, 1, 255);
    IVC *imageLabels = vc_image_new(video.width, video.height, 1, 255);    
    
    OVC *blobs;
    int nlabels = 0;
    
    // Linha baseada na metade do diâmetro da maior laranja possível (100mm = ~509px)
    int linhaAtivacao = 255; 

    // Linha de detecção baseada na posição onde as laranjas começam a ser visíveis (testes indicam que é por volta dos 100px)
    int linhaDetecao = 100; 

    // Número total de Laranjas
    int totalOranges = 0;
    // última laranja na frame
    int lastOrangeFrame = 0;

    // OTIMIZAÇÃO DAS FUNÇÕES PARA LIMPEZA DE IMAGEM
    Mat elementClose = getStructuringElement(MORPH_ELLIPSE, Size(15, 15));
    Mat elementOpen = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));

    int blobsArea[100];
    int blobsPerimeter[100];

    int prevX = 0, prevY = 0, prevFrame = 0;
    int uniqueBlobLabel = 0;

    // Inicio do vídeo
    while (key != 'q')
    {
        OVC orangeBlobs[20] = {0}; // Array para armazenar os blobs de laranja detectados no frame atual

        // Reset da contagem de laranjas por frame
        int nOrangesPerFrame = 0;

        if (!capture.read(frame)) {
            cout << "Fim do vídeo alcançado." << endl;
            break;
        }

        video.nFrame = (int)capture.get(CAP_PROP_POS_FRAMES);
        cvtColor(frame, frameRGB, COLOR_BGR2RGB);
        memcpy(image->data, frameRGB.data, video.height * video.width * 3);
        
        // 1. CONVERSÃO E SEGMENTAÇÃO
        vc_rgb_to_hsv(image, imageHSV);        
        vc_hsv_saturation_and_value_modified(imageHSV, imageHSVMod, 1.2, 0.8);
        vc_hsv_segmentation(imageHSVMod, imageSEG, 10, 34, 30, 100, 13, 100);
        
        memcpy(frameSeg.data, imageSEG->data, video.width * video.height);

        // 2. LIMPEZA OTIMIZADA - OpenCV
        medianBlur(frameSeg, frameSeg, 5);
        morphologyEx(frameSeg, frameSeg, MORPH_CLOSE, elementClose);
        morphologyEx(frameSeg, frameSeg, MORPH_OPEN, elementOpen);

        memcpy(imageOpen->data, frameSeg.data, video.width * video.height);

        // 3. ANÁLISE DE BLOBS E RASTREIO DOS MESMOS
        blobs = vc_binary_blob_labelling(imageOpen, imageLabels, &nlabels);       

        if (blobs != NULL) 
        {
            vc_binary_blob_info(imageLabels, blobs, nlabels);

            for(int i = 0; i < nlabels; i++) {

                // Se houver blobs mais pequenos que 60000px ignora
                if (blobs[i].area < 60000) continue;

                if(orangeBlobs[nOrangesPerFrame].perimeter < blobs[i].perimeter && blobs[i].perimeter < 1400) {
                    orangeBlobs[nOrangesPerFrame] = blobs[i];
                } else {
                    orangeBlobs[nOrangesPerFrame].xc = blobs[i].xc;
                    orangeBlobs[nOrangesPerFrame].yc = blobs[i].yc;
                    orangeBlobs[nOrangesPerFrame].x = blobs[i].x;
                    orangeBlobs[nOrangesPerFrame].y = blobs[i].y;
                }
                
                // printf("Blob %d: Area = %d, Perimetro = %d\n", blobs[i].label, blobs[i].area, blobs[i].perimeter);
                
                // Contagem de laranjas por frame
                nOrangesPerFrame++;

                vc_draw_bounding_box_all_blobs(image, &blobs[i], 1, 3, 5, 255, 0, 0);

                // ATIVAÇÃO: Só processa e desenha a Bounding Box se passar a linha
                if(blobs[i].yc > linhaAtivacao && blobs[i].yc < video.height - linhaAtivacao) {
                    // Mostrar o centro de massa apenas enquanto a laranja estiver entre as linhas de ativação
                    vc_draw_center_mass_all_blobs(image, &blobs[i], 1, 11, 3, 0, 0, 0);

                // Gravação das imagens das laranjas pós segmentação e marcação do centro de gravidade
                //   char filename[256];
                //   sprintf(filename,"../Images/Video%04d.pbm", video.nFrame);
                //   printf("Saving image to: %s\n", filename);
                //   vc_write_image(filename, imageSEG);
                  }

                  double defPrevX = fabs(prevX - blobs[i].xc);
                  double defPrevY = fabs(prevY - blobs[i].yc);

                  if(defPrevX < 20) {
                    blobs[i].label = uniqueBlobLabel;
                  } else {
                    uniqueBlobLabel++;
                    blobs[i].label = uniqueBlobLabel;
                  }

                  prevX = blobs[i].xc;
                  prevY = blobs[i].yc;

                  if(blobsArea[uniqueBlobLabel] <= 0) blobsArea[uniqueBlobLabel] = blobs[i].area;
                  if(blobsPerimeter[uniqueBlobLabel] <= 0) blobsPerimeter[uniqueBlobLabel] = blobs[i].perimeter;

                  printf("Blob %d - Label: %d, Area: %d, Perimetro: %d\n", i, blobs[i].label, blobsArea[uniqueBlobLabel], blobsPerimeter[uniqueBlobLabel]);

                //   if(blobsArea[blobs[i].label] < blobs[i].area || blobsPerimeter[blobs[i].label] < blobs[i].perimeter) {
                //     blobsArea[blobs[i].label] = blobs[i].area;
                //     blobsPerimeter[blobs[i].label] = blobs[i].perimeter;
                //   } 
                }

                // DETECÇÃO: Verifica se o centro de massa do blob está dentro da linha de detecção (margem de 5px para evitar falhas de detecção)
                if(blobs[i].yc >= (linhaAtivacao - 5) && blobs[i].yc <= (linhaAtivacao + 5)) {
                  // Se a última detecção de laranja foi há mais de 10 frames, conta como uma nova laranja
                  if(lastOrangeFrame == 0 || (video.nFrame - lastOrangeFrame) > 10) {
                    lastOrangeFrame = video.nFrame;
                    totalOranges++;
                  }
                }
            }
        }

        // 4. TRADUÇÃO E EXIBIÇÃO
        memcpy(frameRGB.data, image->data, video.width * video.height * 3);
        cvtColor(frameRGB, frame, COLOR_RGB2BGR);

        printf("Oranges in frame: %d\n", nOrangesPerFrame);

        for(int i = 0; i < nOrangesPerFrame; i++) {
            // printf("nOrangesPerFrame: %d\n", nOrangesPerFrame);
            // printf("Blob %d: Area = %d, Perimetro = %d\n", orangeBlobs[i].label, orangeBlobs[i].area, orangeBlobs[i].perimeter);

            if(orangeBlobs[i].yc > linhaAtivacao) {
                std::string strArea = std::string("Area: ").append(std::to_string(orangeBlobs[i].area)).append("px");
                cv::putText(frame, strArea, cv::Point(orangeBlobs[i].x + (orangeBlobs[i].width / 2), orangeBlobs[i].y - 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
                cv::putText(frame, strArea, cv::Point(orangeBlobs[i].x + (orangeBlobs[i].width / 2), orangeBlobs[i].y - 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);

                std::string strPerimetro = std::string("Perimetro: ").append(std::to_string(orangeBlobs[i].perimeter)).append("px");
                cv::putText(frame, strPerimetro, cv::Point(orangeBlobs[i].x + (orangeBlobs[i].width / 2), orangeBlobs[i].y - 50), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
                cv::putText(frame, strPerimetro, cv::Point(orangeBlobs[i].x + (orangeBlobs[i].width / 2), orangeBlobs[i].y - 50), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);
            }
        }

        // Memória liberta dentro do ciclo (Crucial para não crashar)
        free(blobs);

        std::string str = std::string("Total Laranjas: ").append(std::to_string(totalOranges));
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        std::string str2 = std::string("Laranjas No Frame: ").append(std::to_string(nOrangesPerFrame));
        cv::putText(frame, str2, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str2, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

        // Exibir janelas
        imshow("VC - SEGMENTACAO LIMPA", frameSeg);
        imshow("VC - ORIGINAL", frame);

        key = waitKey(25);
    }

    // Limpeza de recursos
    capture.release();
    vc_image_free(image);
    vc_image_free(imageHSV);
    vc_image_free(imageHSVEqualized);
    vc_image_free(imageHSVMod);
    vc_image_free(imageSEG);
    vc_image_free(imageOpen);
    vc_image_free(imageLabels);

    return 0;
}
