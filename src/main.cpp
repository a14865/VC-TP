#include <iostream>
#include <string>
#include <chrono>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>
#include <math.h>

#define MAX_LARANJAS_NO_FRAME 10

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

    string str;

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

    // OTIMIZAÇÃO DAS FUNÇÕES PARA LIMPEZA DE IMAGEM
    Mat elementClose = getStructuringElement(MORPH_ELLIPSE, Size(17, 17));
    Mat elementOpen = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));
    
    OVC *blobs;
    int nlabels = 0;
    int totalOranges = 0;
    // Linha baseada na metade do diâmetro da maior laranja possível
    // Página 5 do Regulamento (100mm == 509px)
    int linhaAtivacao = 150;
    
    // Array com os centros de massa antigos
    OVC *previousCenters = (OVC *)malloc(sizeof(OVC) * MAX_LARANJAS_NO_FRAME);
    int numberPreviousCenters = 0;

    // Inicio do vídeo
    while (key != 'q')
    {   

        int currentFrameOranges = 0;

        if (!capture.read(frame)) {
            cout << "Fim do vídeo alcançado." << endl;
            break;
        }

        video.nFrame = (int)capture.get(CAP_PROP_POS_FRAMES);
        cvtColor(frame, frameRGB, COLOR_BGR2RGB);
        memcpy(image->data, frameRGB.data, video.height * video.width * 3);
        
        // 1. CONVERSÃO E SEGMENTAÇÃO
        vc_rgb_to_hsv(image, imageHSV);        
        vc_hsv_saturation_and_value_modified(imageHSV, imageHSVMod, 1, 255);
        vc_hsv_segmentation(imageHSVMod, imageSEG, 10, 34, 30, 100, 0, 100);
        
        memcpy(frameSeg.data, imageSEG->data, video.width * video.height);

        // 2. LIMPEZA OTIMIZADA - OpenCV
        //medianBlur(frameSeg, frameSeg, 5);
        morphologyEx(frameSeg, frameSeg, MORPH_CLOSE, elementClose);
        morphologyEx(frameSeg, frameSeg, MORPH_OPEN, elementOpen);

        memcpy(imageOpen->data, frameSeg.data, video.width * video.height);

        // 3. ANÁLISE DE BLOBS E RASTREIO DOS MESMOS
        blobs = vc_binary_blob_labelling(imageOpen, imageLabels, &nlabels);

        // Array dos centros de massa atuais
        OVC currentCenters[MAX_LARANJAS_NO_FRAME];
        int numberCurrentCenters = 0;

        if (blobs != NULL) 
        {

            vc_binary_blob_info(imageLabels, blobs, nlabels);

            for(int i = 0; i < nlabels; i++) {

                // Se houver blobs mais pequenos que 60000px ignora
                if (blobs[i].area < 60000) continue;

                if(numberCurrentCenters < MAX_LARANJAS_NO_FRAME){

                    currentCenters[numberCurrentCenters++] = blobs[i];
                }

                int matchIndex = -1;
                double minDistance = 1000000.0; 

                for(int j = 0; j < numberPreviousCenters; j++){

                    double distanceX = (double)(blobs[i].xc - previousCenters[j].xc);
                    double distanceY = (double)(blobs[i].yc - previousCenters[j].yc);
                    double totalDistance = sqrt((distanceX * distanceX) + (distanceY * distanceY)); // TEOREMA DE PITÁGORAS [H^2 = SOMA(C^2)]

                    if(totalDistance < 100.0 && totalDistance < minDistance){

                        minDistance = totalDistance;
                        matchIndex = j;
                    }
                }

                if(matchIndex != -1){

                    if(previousCenters[matchIndex].yc < linhaAtivacao && blobs[i].yc >= linhaAtivacao){

                        totalOranges++;
                    }
                }

                // DESENHA A BOUNDING BOX E O CENTRO DE MASSA QUANDO PASSA PELA LINHA DE ATIVAÇÃO
                if(blobs[i].yc > linhaAtivacao && blobs[i].yc < video.height - linhaAtivacao){

                    vc_draw_bounding_box_all_blobs(image, &blobs[i], 1, 0, 1, 255, 0, 0);
                    vc_draw_center_mass_all_blobs(image, &blobs[i], 1, 5, 3, 255, 0, 0);

                    currentFrameOranges++;
                }
            }

            for(int k = 0; k < numberCurrentCenters; k++){

                previousCenters[k] = currentCenters[k];
            }

            numberPreviousCenters = numberCurrentCenters;
        }

        // 4. TRADUÇÃO E EXIBIÇÃO DO VÍDEO
        memcpy(frameRGB.data, image->data, video.width * video.height * 3);
        cvtColor(frameRGB, frame, COLOR_RGB2BGR);

        // Memória liberta dentro do ciclo
        free(blobs);

        str = string("Numero total de laranjas: ").append(to_string(totalOranges));
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = string("Numero total de laranjas no frame: ").append(to_string(currentFrameOranges));
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

        // Exibir janelas
        imshow("VC - SEGMENTACAO LIMPA", frameSeg);
        imshow("VC - ORIGINAL", frame);

        key = waitKey(1);
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

    free(previousCenters);

    return 0;
}
