#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>

#define MAX_LARANJAS_NO_FRAME 10
#define SHOW_SEGMENTATION_WINDOW 0

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

/**
 * @brief Executa o pipeline principal de leitura, processamento e visualizacao do video.
 *
 * O ciclo principal le cada frame, segmenta as laranjas, extrai blobs, atualiza
 * tracking, medidas, classificacoes e desenha os resultados no ecra.
 */
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
    IVC *imageSEG = vc_image_new(video.width, video.height, 1, 255);
    IVC *imageLabels = vc_image_new(video.width, video.height, 1, 255);

    Mat elementClose = getStructuringElement(MORPH_ELLIPSE, Size(17, 17));
    Mat elementOpen = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));

    OVC *blobs;
    int nlabels = 0;
    int totalOranges = 0;
    int linhaAtivacao = 150;
    int alturaBarraInfo = 116;
    int linhaDesativacao = video.height - linhaAtivacao - alturaBarraInfo;

    OVC *orangesPreviousFrame = (OVC *)malloc(sizeof(OVC) * MAX_LARANJAS_NO_FRAME);
    OrangeQualityMetrics orangesQualityPreviousFrame[MAX_LARANJAS_NO_FRAME];
    int orangesBatchIndexPreviousFrame[MAX_LARANJAS_NO_FRAME];
    int numberOrangesPreviousFrame = 0;
    OrangeBatchStats batchStats;
    orangeBatchInit(&batchStats);

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
        vc_hsv_segmentation(imageHSV, imageSEG, 10, 34, 30, 100, 0, 100);

        memcpy(frameSeg.data, imageSEG->data, video.width * video.height);

        medianBlur(frameSeg, frameSeg, 5);
        morphologyEx(frameSeg, frameSeg, MORPH_CLOSE, elementClose);
        morphologyEx(frameSeg, frameSeg, MORPH_OPEN, elementOpen);

        memcpy(imageSEG->data, frameSeg.data, video.width * video.height);

        blobs = vc_binary_blob_labelling(imageSEG, imageLabels, &nlabels);

        OVC orangesCurrentFrame[MAX_LARANJAS_NO_FRAME];
        OrangeQualityMetrics orangesQualityCurrentFrame[MAX_LARANJAS_NO_FRAME];
        int orangesBatchIndexCurrentFrame[MAX_LARANJAS_NO_FRAME];
        int numberOrangesCurrentFrame = 0;

        if (blobs != NULL)
        {
            vc_binary_blob_info(imageLabels, blobs, nlabels);

            for (int i = 0; i < nlabels; i++)
            {
                int orangeWasCounted = 0;
                int matchIndex;
                int inUsefulZone;
                int batchIndex = -1;
                OrangeQualityMetrics instantQuality;

                if (blobs[i].area < 60000) continue;

                matchIndex = trackOrange(&blobs[i], orangesPreviousFrame, numberOrangesPreviousFrame, linhaAtivacao, 100.0, &totalOranges, &orangeWasCounted);
                inUsefulZone = blobs[i].yc >= linhaAtivacao && blobs[i].yc < linhaDesativacao;

                if (matchIndex >= 0 && matchIndex < numberOrangesPreviousFrame)
                {
                    batchIndex = orangesBatchIndexPreviousFrame[matchIndex];
                }

                instantQuality.category = ORANGE_CATEGORY_REJECTED;
                instantQuality.shapeRatioPercent = 0;
                instantQuality.fillRatioPercent = 0;
                instantQuality.typicalColorPercent = 0;
                instantQuality.defectColorPercent = 100;
                instantQuality.circularityPercent = 0;
                instantQuality.colorVariation = 255;
                instantQuality.roughness = 255;
                instantQuality.edgeDensityPercent = 100;
                instantQuality.intensityP10 = 0;
                instantQuality.intensityP90 = 0;
                instantQuality.intensityContrast = 255;

                if (inUsefulZone)
                {
                    orangeQualityCategory(image, imageLabels, &blobs[i], &instantQuality);
                }

                if (orangeWasCounted)
                {
                    int diameterMM = convertPixToMM(blobs[i].width);
                    int caliber = orangeCaliber(diameterMM);
                    orangeBatchAdd(&batchStats, diameterMM, caliber);
                    batchIndex = batchStats.count - 1;
                }

                if (numberOrangesCurrentFrame < MAX_LARANJAS_NO_FRAME)
                {
                    orangesCurrentFrame[numberOrangesCurrentFrame] = blobs[i];
                    orangesBatchIndexCurrentFrame[numberOrangesCurrentFrame] = batchIndex;

                    if (matchIndex >= 0 && matchIndex < numberOrangesPreviousFrame)
                    {
                        orangesQualityCurrentFrame[numberOrangesCurrentFrame] = orangesQualityPreviousFrame[matchIndex];

                        if (inUsefulZone && instantQuality.category < orangesQualityCurrentFrame[numberOrangesCurrentFrame].category)
                        {
                            orangesQualityCurrentFrame[numberOrangesCurrentFrame] = instantQuality;
                        }
                    }
                    else
                    {
                        orangesQualityCurrentFrame[numberOrangesCurrentFrame] = instantQuality;
                    }

                    if (batchIndex >= 0)
                    {
                        orangeBatchSetQuality(&batchStats, batchIndex, orangesQualityCurrentFrame[numberOrangesCurrentFrame].category);
                    }

                    numberOrangesCurrentFrame++;
                }
                else if (orangeWasCounted)
                {
                    orangeBatchAddQuality(&batchStats, instantQuality.category);
                }

                if (blobs[i].yc > linhaAtivacao && blobs[i].yc < linhaDesativacao)
                {
                    vc_draw_bounding_box_all_blobs(image, &blobs[i], 1, 0, 1, 255, 0, 0);
                    vc_draw_center_mass_all_blobs(image, &blobs[i], 1, 5, 3, 255, 0, 0);

                    currentFrameOranges++;
                }
            }

            for (int k = 0; k < numberOrangesCurrentFrame; k++)
            {
                orangesPreviousFrame[k] = orangesCurrentFrame[k];
                orangesQualityPreviousFrame[k] = orangesQualityCurrentFrame[k];
                orangesBatchIndexPreviousFrame[k] = orangesBatchIndexCurrentFrame[k];
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

                std::string strCategoria = std::string("Categoria: ").append(orangeQualityCategoryName(orangesQualityCurrentFrame[i].category));
                cv::putText(frame, strCategoria, cv::Point(orangesCurrentFrame[i].x + (orangesCurrentFrame[i].width / 2), orangesCurrentFrame[i].y - 120), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2);
                cv::putText(frame, strCategoria, cv::Point(orangesCurrentFrame[i].x + (orangesCurrentFrame[i].width / 2), orangesCurrentFrame[i].y - 120), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);
            }
        }

        free(blobs);

        str = string("-----------------------------------------------------------------------------------------------------------------------------------").append(to_string(totalOranges));
        cv::putText(frame, str, cv::Point(0, linhaAtivacao), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 1);
        str = string("-----------------------------------------------------------------------------------------------------------------------------------").append(to_string(totalOranges));
        cv::putText(frame, str, cv::Point(0, linhaDesativacao), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 1);

        {
            int barHeight = alturaBarraInfo;
            int barTop = video.height - barHeight;
            int opacityPercent = 78;

            for (int y = barTop; y < video.height; y++)
            {
                for (int x = 0; x < video.width; x++)
                {
                    int pos = y * (int)frame.step + x * frame.channels();

                    frame.data[pos] = (unsigned char)((frame.data[pos] * (100 - opacityPercent)) / 100);
                    frame.data[pos + 1] = (unsigned char)((frame.data[pos + 1] * (100 - opacityPercent)) / 100);
                    frame.data[pos + 2] = (unsigned char)((frame.data[pos + 2] * (100 - opacityPercent)) / 100);
                }
            }

            string strTotal = string("Total: ").append(to_string(totalOranges))
                .append(" | Frame: ").append(to_string(currentFrameOranges));

            string strCaliber = string("Calibre lote: ");
            strCaliber.append(batchStats.dominantCaliber < 0 ? "-" : to_string(batchStats.dominantCaliber));

            string strHomogeneity = string("Homogeneidade: ");
            if (batchStats.count <= 1)
            {
                strHomogeneity.append("-");
            }
            else
            {
                strHomogeneity.append(batchStats.homogeneous ? "OK" : "NOK");
                strHomogeneity.append(" (");
                strHomogeneity.append(to_string(batchStats.maxDiameterMM - batchStats.minDiameterMM));
                strHomogeneity.append("/");
                strHomogeneity.append(to_string(batchStats.maxAllowedDifferenceMM));
                strHomogeneity.append("mm)");
            }

            string strMinTolerance = string("Tol. minimo: ");
            if (batchStats.count <= 1)
            {
                strMinTolerance.append("-");
            }
            else
            {
                strMinTolerance.append(batchStats.toleranceOk ? "OK" : "NOK");
                strMinTolerance.append(" (");
                strMinTolerance.append(to_string(batchStats.toleranceOutsideCount));
                strMinTolerance.append("/");
                strMinTolerance.append(to_string(batchStats.count));
                strMinTolerance.append(" fora)");
            }

            string strCategory = string("Categoria lote: ");
            strCategory.append(batchStats.count <= 0 ? "-" : orangeQualityCategoryName(batchStats.dominantQualityCategory));

            string strQualityTolerance = string("Tol. qualidade: ");
            if (batchStats.count <= 1)
            {
                strQualityTolerance.append("-");
            }
            else
            {
                strQualityTolerance.append(batchStats.qualityToleranceOk ? "OK" : "NOK");
                strQualityTolerance.append(" (");
                strQualityTolerance.append(to_string(batchStats.qualityToleranceOutsideCount));
                strQualityTolerance.append("/");
                strQualityTolerance.append(to_string(batchStats.count));
                strQualityTolerance.append(" fora)");
            }

            cv::putText(frame, strTotal, cv::Point(18, barTop + 24), cv::FONT_HERSHEY_SIMPLEX, 0.46, cv::Scalar(255, 255, 255), 1);
            cv::putText(frame, strCaliber, cv::Point(video.width / 2, barTop + 24), cv::FONT_HERSHEY_SIMPLEX, 0.46, cv::Scalar(255, 255, 255), 1);
            cv::putText(frame, strHomogeneity, cv::Point(18, barTop + 50), cv::FONT_HERSHEY_SIMPLEX, 0.46, cv::Scalar(255, 255, 255), 1);
            cv::putText(frame, strMinTolerance, cv::Point(video.width / 2, barTop + 50), cv::FONT_HERSHEY_SIMPLEX, 0.46, cv::Scalar(255, 255, 255), 1);
            cv::putText(frame, strCategory, cv::Point(18, barTop + 76), cv::FONT_HERSHEY_SIMPLEX, 0.46, cv::Scalar(255, 255, 255), 1);
            cv::putText(frame, strQualityTolerance, cv::Point(video.width / 2, barTop + 76), cv::FONT_HERSHEY_SIMPLEX, 0.46, cv::Scalar(255, 255, 255), 1);
        }

        if (SHOW_SEGMENTATION_WINDOW)
        {
            imshow("VC - SEGMENTACAO LIMPA", frameSeg);
        }

        imshow("VC - ORIGINAL", frame);

        key = waitKey(1);
    }

    capture.release();
    vc_image_free(image);
    vc_image_free(imageHSV);
    vc_image_free(imageSEG);
    vc_image_free(imageLabels);

    free(orangesPreviousFrame);

    return 0;
}
