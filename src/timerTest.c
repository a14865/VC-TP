#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../include/vc.h"

#define ITER 100

int main()
{
    IVC* img = vc_read_image("../Images/testeee.ppm");

    if(img == NULL)
    {
        printf("Erro ao ler imagem\n");
        return 0;
    }

    // Imagens
    IVC* hsv = vc_image_new(img->width, img->height, 3, 255);
    IVC* hsv_copy = vc_image_new(img->width, img->height, 3, 255);

    IVC* dst1 = vc_image_new(img->width, img->height, 1, 255);
    IVC* dst2 = vc_image_new(img->width, img->height, 1, 255);
    IVC* dst3 = vc_image_new(img->width, img->height, 3, 255);
    IVC* dst4 = vc_image_new(img->width, img->height, 1, 255);

    // RGB -> HSV (feito uma vez)
    vc_rgb_to_hsv(img, hsv);

    int nlabels;
    clock_t start, end;

    double time1 = 0, time2 = 0, time3 = 0, time4 = 0;

    printf("Start benchmark HSV segmentation...\n");

    // =========================
    // LOOP PRINCIPAL
    // =========================
    for(int i = 0; i < ITER; i++)
    {
        // -------------------------
        // Algoritmo 1
        // -------------------------
        memcpy(hsv_copy->data, hsv->data, hsv->bytesperline * hsv->height);

        start = clock();
        memset(dst1->data, 0, dst1->bytesperline * dst1->height);
        vc_hsv_segmentation(hsv_copy, dst1, 180, 300, 20, 100, 30, 100);
        end = clock();

        time1 += (double)(end - start);

        // -------------------------
        // Algoritmo 2
        // -------------------------
        memcpy(hsv_copy->data, hsv->data, hsv->bytesperline * hsv->height);

        start = clock();
        memset(dst2->data, 0, dst2->bytesperline * dst2->height);
        vc_hsv_segmentation_ricardo(hsv_copy, dst2, 180, 300, 20, 100, 30, 100);
        end = clock();

        time2 += (double)(end - start);

        // -------------------------
        // Algoritmo 3
        // -------------------------
        memcpy(hsv_copy->data, hsv->data, hsv->bytesperline * hsv->height);

        start = clock();
        memset(dst3->data, 0, dst3->bytesperline * dst3->height);
        vc_hsv_segmentation_ze(hsv_copy, dst3, 180, 300, 20, 100, 30, 100);
        end = clock();

        time3 += (double)(end - start);

        // -------------------------
        // Algoritmo 4
        // -------------------------
        memcpy(hsv_copy->data, hsv->data, hsv->bytesperline * hsv->height);

        start = clock();
        memset(dst4->data, 0, dst4->bytesperline * dst4->height);
        vc_hsv_segmentation_raul(hsv_copy, dst4, 180, 300, 20, 100, 30, 100);
        end = clock();

        time4 += (double)(end - start);
    }

    printf("\nFim do benchmark\n\n");

    // =========================
    // RESULTADOS (tempo médio)
    // =========================
    double avg1 = (time1 / ITER) / CLOCKS_PER_SEC * 1000.0;
    double avg2 = (time2 / ITER) / CLOCKS_PER_SEC * 1000.0;
    double avg3 = (time3 / ITER) / CLOCKS_PER_SEC * 1000.0;
    double avg4 = (time4 / ITER) / CLOCKS_PER_SEC * 1000.0;

    printf("Tempo medio HSV Nuno: %.4f ms\n", avg1);
    printf("Tempo medio HSV Ricardo: %.4f ms\n", avg2);
    printf("Tempo medio HSV Ze: %.4f ms\n", avg3);
    printf("Tempo medio HSV Raul: %.4f ms\n", avg4);

    // Libertar memória
    vc_image_free(img);
    vc_image_free(hsv);
    vc_image_free(hsv_copy);

    vc_image_free(dst1);
    vc_image_free(dst2);
    vc_image_free(dst3);
    vc_image_free(dst4);

    printf("\nFim\n");
    getchar();

    return 0;
}