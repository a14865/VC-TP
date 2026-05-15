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

    // OTIMIZAÇÃO DAS FUNÇÕES PARA LIMPEZA DE IMAGEM
    Mat elementClose = getStructuringElement(MORPH_ELLIPSE, Size(15, 15));
    Mat elementOpen = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));

    // Inicio do vídeo
    while (key != 'q')
    {

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

                vc_draw_bounding_box_all_blobs(image, &blobs[i], 1, 3, 5, 255, 0, 0);

                // ATIVAÇÃO: Só processa e desenha a Bounding Box se passar a linha
                if(blobs[i].yc > linhaAtivacao && blobs[i]. yc < video.height - linhaAtivacao) {

                    // Passamos apenas o blob atual (&blobs[i]) e nlabels=1
                    vc_draw_center_mass_all_blobs(image, &blobs[i], 1, 11, 3, 0, 0, 0);
                }
            }

            // Memória liberta dentro do ciclo (Crucial para não crashar)
            free(blobs);
        }

        // 4. TRADUÇÃO E EXIBIÇÃO
        memcpy(frameRGB.data, image->data, video.width * video.height * 3);
        cvtColor(frameRGB, frame, COLOR_RGB2BGR);

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