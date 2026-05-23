#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>

#define MAX_LARANJAS_NO_FRAME 10

using namespace cv;
using namespace std;

extern "C"
{
#include "../include/tp_utils.h"
#include "../include/tp_segmentation.h"
#include "../include/tp_measurements.h"
#include "../include/tp_tracking.h"
#include "../include/tp_classification.h"
#include "../include/tp_visualization.h"
}

int main(void)
{
    string videofile = "data/raw/video.avi";
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
        cerr << "Erro ao abrir o ficheiro de video: " << videofile << "\n";
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

    IVC *image = vc_image_new(video.width, video.height, 3, 255);
    IVC *imageHSV = vc_image_new(video.width, video.height, 3, 255);
    IVC *imageHSVEqualized = vc_image_new(video.width, video.height, 3, 255);
    IVC *imageHSVMod = vc_image_new(video.width, video.height, 3, 255);
    IVC *imageSEG = vc_image_new(video.width, video.height, 1, 255);
    IVC *imageOpen = vc_image_new(video.width, video.height, 1, 255);
    IVC *imageLabels = vc_image_new(video.width, video.height, 1, 255);

    Mat elementClose = getStructuringElement(MORPH_ELLIPSE, Size(17, 17));
    Mat elementOpen = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));

    OVC *blobs;
    int nlabels = 0;
    int totalOranges = 0;
    int linhaAtivacao = 150;

    OVC *orangesPreviousFrame = (OVC *)malloc(sizeof(OVC) * MAX_LARANJAS_NO_FRAME);
    int numberOrangesPreviousFrame = 0;

    while (key != 'q')
    {
        int currentFrameOranges = 0;

        if (!capture.read(frame))
        {
            cout << "Fim do video alcancado." << endl;
            break;
        }

        video.nFrame = (int)capture.get(CAP_PROP_POS_FRAMES);
        cvtColor(frame, frameRGB, COLOR_BGR2RGB);
        memcpy(image->data, frameRGB.data, video.height * video.width * 3);

        vc_rgb_to_hsv(image, imageHSV);
        vc_hsv_saturation_and_value_modified(imageHSV, imageHSVMod, 1, 255);
        vc_hsv_segmentation(imageHSVMod, imageSEG, 10, 34, 30, 100, 0, 100);

        memcpy(frameSeg.data, imageSEG->data, video.width * video.height);

        medianBlur(frameSeg, frameSeg, 5);
        morphologyEx(frameSeg, frameSeg, MORPH_CLOSE, elementClose);
        morphologyEx(frameSeg, frameSeg, MORPH_OPEN, elementOpen);

        memcpy(imageOpen->data, frameSeg.data, video.width * video.height);

        blobs = vc_binary_blob_labelling(imageOpen, imageLabels, &nlabels);

        OVC orangesCurrentFrame[MAX_LARANJAS_NO_FRAME];
        int numberOrangesCurrentFrame = 0;

        if (blobs != NULL)
        {
            vc_binary_blob_info(imageLabels, blobs, nlabels);

            for (int i = 0; i < nlabels; i++)
            {
                if (blobs[i].area < 60000) continue;

                trackOrange(&blobs[i], orangesPreviousFrame, numberOrangesPreviousFrame, linhaAtivacao, 100.0, &totalOranges);

                if (numberOrangesCurrentFrame < MAX_LARANJAS_NO_FRAME)
                {
                    orangesCurrentFrame[numberOrangesCurrentFrame++] = blobs[i];
                }

                if (blobs[i].yc > linhaAtivacao && blobs[i].yc < video.height - linhaAtivacao)
                {
                    vc_draw_bounding_box_all_blobs(image, &blobs[i], 1, 0, 1, 255, 0, 0);
                    vc_draw_center_mass_all_blobs(image, &blobs[i], 1, 5, 3, 255, 0, 0);

                    currentFrameOranges++;
                }
            }

            for (int k = 0; k < numberOrangesCurrentFrame; k++)
            {
                orangesPreviousFrame[k] = orangesCurrentFrame[k];
            }

            numberOrangesPreviousFrame = numberOrangesCurrentFrame;
        }

        memcpy(frameRGB.data, image->data, video.width * video.height * 3);
        cvtColor(frameRGB, frame, COLOR_RGB2BGR);

        for (int i = 0; i < numberOrangesCurrentFrame; i++)
        {
            if (orangesCurrentFrame[i].yc > linhaAtivacao)
            {
                int diameterMM = convertPixToMM(orangesCurrentFrame[i].width);
                int caliber = orangeCaliber(diameterMM);

                std::string strArea = std::string("Area: ").append(std::to_string(convertPixAreaToMM2(orangesCurrentFrame[i].area))).append("mm2");
                cv::putText(frame, strArea, cv::Point(orangesCurrentFrame[i].x + (orangesCurrentFrame[i].width / 2), orangesCurrentFrame[i].y - 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
                cv::putText(frame, strArea, cv::Point(orangesCurrentFrame[i].x + (orangesCurrentFrame[i].width / 2), orangesCurrentFrame[i].y - 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);

                std::string strPerimetro = std::string("Perimetro: ").append(std::to_string(convertPixToMM(orangesCurrentFrame[i].perimeter))).append("mm");
                cv::putText(frame, strPerimetro, cv::Point(orangesCurrentFrame[i].x + (orangesCurrentFrame[i].width / 2), orangesCurrentFrame[i].y - 45), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
                cv::putText(frame, strPerimetro, cv::Point(orangesCurrentFrame[i].x + (orangesCurrentFrame[i].width / 2), orangesCurrentFrame[i].y - 45), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);

                std::string strDiametro = std::string("Diametro: ").append(std::to_string(diameterMM)).append("mm");
                cv::putText(frame, strDiametro, cv::Point(orangesCurrentFrame[i].x + (orangesCurrentFrame[i].width / 2), orangesCurrentFrame[i].y - 70), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
                cv::putText(frame, strDiametro, cv::Point(orangesCurrentFrame[i].x + (orangesCurrentFrame[i].width / 2), orangesCurrentFrame[i].y - 70), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);

                std::string strCalibre = (caliber < 0) ? std::string("Calibre: excluido") : std::string("Calibre: ").append(std::to_string(caliber));
                cv::putText(frame, strCalibre, cv::Point(orangesCurrentFrame[i].x + (orangesCurrentFrame[i].width / 2), orangesCurrentFrame[i].y - 95), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
                cv::putText(frame, strCalibre, cv::Point(orangesCurrentFrame[i].x + (orangesCurrentFrame[i].width / 2), orangesCurrentFrame[i].y - 95), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);
            }
        }

        free(blobs);

        str = string("-----------------------------------------------------------------------------------------------------------------------------------").append(to_string(totalOranges));
        cv::putText(frame, str, cv::Point(0, linhaAtivacao), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 1);
        str = string("-----------------------------------------------------------------------------------------------------------------------------------").append(to_string(totalOranges));
        cv::putText(frame, str, cv::Point(0, video.height - linhaAtivacao), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 1);

        str = string("Numero total de laranjas: ").append(to_string(totalOranges));
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

        str = string("Numero total de laranjas no frame: ").append(to_string(currentFrameOranges));
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

        imshow("VC - SEGMENTACAO LIMPA", frameSeg);
        imshow("VC - ORIGINAL", frame);

        key = waitKey(1);
    }

    capture.release();
    vc_image_free(image);
    vc_image_free(imageHSV);
    vc_image_free(imageHSVEqualized);
    vc_image_free(imageHSVMod);
    vc_image_free(imageSEG);
    vc_image_free(imageOpen);
    vc_image_free(imageLabels);

    free(orangesPreviousFrame);

    return 0;
}
