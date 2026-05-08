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
#include "../lib/vc.h"
}

int main(void)
{

  string videofile = "video.avi";
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
  Mat frameRGB;
  Mat test;
  Mat frameSeg(video.height, video.width, CV_8UC1);
  IVC *image = vc_image_new(video.width, video.height, 3, 255);
  IVC *imageHSV = vc_image_new(video.width, video.height, 3, 255);
  IVC *imageHSVMod = vc_image_new(video.width, video.height, 3, 255);
  IVC *imageSEG = vc_image_new(video.width, video.height, 1, 255);
  IVC *imageLowpass = vc_image_new(video.width, video.height, 1, 255);
  IVC *imageClose = vc_image_new(video.width, video.height, 1, 255);
  IVC *imageOpen = vc_image_new(video.width, video.height, 1, 255);
  IVC *imageLabels = vc_image_new(video.width, video.height, 1, 255);
  OVC *blobs;
  int nlabels = 0;

  // Inicio do vídeo
  while (key != 'q')
  {

    // Verifica a leitura da frame
    if (!capture.read(frame))
      break;

    video.nFrame = (int)capture.get(CAP_PROP_POS_FRAMES);
    cvtColor(frame, frameRGB, COLOR_BGR2RGB);
    memcpy(image->data, frameRGB.data, video.height * video.width * 3);

    ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////// Nosso código ////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////

    // Conversão e Segmentação
    vc_rgb_to_hsv(image, imageHSV);
    vc_hsv_saturation_and_value_modified(imageHSV, imageHSVMod, 1.2, 0.9);
    vc_hsv_segmentation(imageHSVMod, imageSEG, 15, 40, 60, 100, 30, 100);

    // Limpeza
    vc_gray_lowpass_median_filter(imageSEG, imageLowpass, 7);
    vc_image_close(imageLowpass, imageClose, 11, 11);
    vc_image_open(imageClose, imageOpen, 5, 11);

    // Análise de blobs
    blobs = vc_binary_blob_labelling(imageSEG, imageLabels, &nlabels);

    if (blobs != NULL) // Verifica se foram encontrados objetos
    {

      vc_binary_blob_info(imageLabels, blobs, nlabels);
      vc_draw_bounding_box_all_blobs(image, blobs, nlabels);
      vc_draw_center_mass_all_blobs(image, blobs, nlabels, 13, 255);

      free(blobs);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////// Fim nosso código //////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////

    // Tradução IVC -> Mat
    memcpy(frameSeg.data, imageSEG->data, video.width * video.height);
    memcpy(frameRGB.data, image->data, video.width * video.height * 3);
    cvtColor(frameRGB, frame, COLOR_RGB2BGR);
    // memcpy(frameRGB.data, imageHSVMod->data, video.width * video.height * 3);
    // cvtColor(frameRGB, test, COLOR_RGB2BGR);

    imshow("VC - VIDEO", frameSeg);
    imshow("VC - ORIGINAL", frame);
    // imshow("VC - HSVMod", test);

    key = waitKey(25);
  }

  capture.release();

  vc_image_free(image);
  vc_image_free(imageHSV);
  vc_image_free(imageHSVMod);
  vc_image_free(imageSEG);
  vc_image_free(imageLowpass);
  vc_image_free(imageOpen);
  vc_image_free(imageClose);
  vc_image_free(imageLabels);

  return 0;
}