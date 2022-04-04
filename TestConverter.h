#pragma once
#include <iostream>
#include "Arena\ArenaApi.h"
#include "Save\SaveApi.h"
#include "Converter.h"
#include <chrono>
#include <sstream>
#include <thread>

// pixel format
#define PIXEL_FORMAT_SAVE BGR8
#define IMAGE_TIMEOUT 2000
#define PIXEL_FORMAT_TRANSFORM "Mono8"
#define INNER_PARAMS_FILE "D:\\TryToGetPic\\C++\\IntrinsicMatrix.txt"

void test(int argc, char** argv) {
    IntrinsicParameters params;
    params.readFromFile(INNER_PARAMS_FILE, true);

    cv::Mat* real_pos = new Mat(1, 3, CV_64F);
    real_pos->at<double>(0, 0) = 100.0;
    real_pos->at<double>(0, 1) = 2380.0;
    real_pos->at<double>(0, 2) = 1000.0;
    cv::Mat* P_mat = params.getMatrixPtr();
    cv::Mat pixel_3d = (*P_mat) * real_pos->t();

    cv::Mat sqr_real_coords = real_pos->mul(*real_pos);
    double sum_ = sum(sqr_real_coords)[0];
    double dist = sqrt(sum_);

    cout << *real_pos << "\n" << pixel_3d << "\n" << dist << "\n\n";

    P_mat->inv();
    cv::Mat pixel_coords = pixel_3d / pixel_3d.at<double>(2, 0);
    /*
    pixel_coords.at<double>(2, 0) = 1;
    pixel_coords.at<double>(0, 0) = 368.831829480000;
    pixel_coords.at<double>(1, 0) = 1481.84269344000;
    */
    cv::Mat from_2d = (*P_mat) * pixel_coords;
    //double dist = 2583.48601699332;

    cv::Mat sqr_from_2d = from_2d.mul(from_2d);
    double sum_from_2d = sum(sqr_from_2d)[0];
    double dist_from_2d = sqrt(sum_from_2d);

    double a = dist / dist_from_2d;

    cv::Mat coord_count = from_2d * a;

    cout << from_2d << "\n" << pixel_coords << "\n" << coord_count << "\n\n";

    //------------------------------------------------------------------выше тест из матлаба, он работает нормально 

    // Нам дано в реальности 
    double real_dist = dist;
    cv::Mat* real_pixel = &pixel_coords;

    //cv::Mat* real_pixel = new Mat(row_real_pixel->t());

    cv::Mat on_matrix_pos = *P_mat * (*real_pixel); //типа фром 2д

    cv::Mat sqr_on_matrix = on_matrix_pos.mul(on_matrix_pos);
    double sum_on_matrix = sum(sqr_on_matrix.t())[0];
    double dist_on_matrix = sqrt(sum_on_matrix);

    cout << on_matrix_pos << "\n" << *real_pixel << "\n";

    cout << on_matrix_pos * (real_dist / dist_on_matrix);
    cout << on_matrix_pos.mul(1 / dist_on_matrix) * real_dist;

    //delete real_pixel;
    //-------------------теперьи это работает. проблема была в прямом присваивании риал пикселю элементов пиксель корда
    //-------------------осталось довести код в конвертере до такого же вида (не такого же говёного, а функционально)
    //-------------------и можно приступать к реальным тестам
}

void final_test() {
    IntrinsicParameters params;
    params.readFromFile(INNER_PARAMS_FILE, true);
    int width = 1, height = 1;
    Mat pic(width, height, CV_8UC1);

    int low = 0;
    int high = 255;
    randu(pic, Scalar(low), Scalar(high));
    Converter conv(params, width, height);
    uint8_t test[] = { 100 };

    conv.prepareKoefs();

    auto begin = std::chrono::steady_clock::now();

    //conv.setDistance(pic.ptr<unsigned char>(0));
    conv.setDistance(test);
    conv.get3D();
    conv.print3D();
    //conv.printKandPicandDist();

    //--------------------------------------------------------------------
    /*
    std::ofstream file("D:\\TryToGetPic\\C++\\To3D_NoCMake\\Distance_OpenCV.txt"); // окрываем файл для чтения
    if (!file.is_open())
    {
        cout << "FILE NOT OPEN\n";
        return;
    }
    uint8_t test_[] = { 100 };
    for (int i = 1; i < 101; i++) {
        test_[0] = i ;
        conv.setDistance(test_);
        cv::Mat pixel_3D = conv.get3D();
        file << pixel_3D.at<double>(0, 0, 0) << "\t"
             << pixel_3D.at<double>(0, 0, 1) << "\t"
             << pixel_3D.at<double>(0, 0, 2) << "\n";
    }
    file.close();
    */
    //--------------------------------------------------------------------

    auto end = std::chrono::steady_clock::now();

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
    cout << "Transformation passed " << elapsed_ms.count() << " ms\n";
}

void count_difference() {
    cv::Mat matlab(100, 3, CV_64F, Scalar(1.0));

    std::ifstream file("D:\\TryToGetPic\\C++\\To3D_NoCMake\\matlab_distance.txt"); // окрываем файл для чтения
    if (file.is_open())
    {
        std::string line;
        for (int i = 0; std::getline(file, line); i++)
        {
            std::istringstream stream(line); //затем из каждой считанной строки
            stream >> matlab.at<double>(i, 0)
                >> matlab.at<double>(i, 1)
                >> matlab.at<double>(i, 2);

        }
        file.close();     // закрываем файл
    }
    //cout << matlab << "\n";
    cv::Mat opencv(100, 3, CV_64F, Scalar(1.0));
    std::ifstream cv_file("D:\\TryToGetPic\\C++\\To3D_NoCMake\\Distance_OpenCV.txt"); // окрываем файл для чтения
    if (cv_file.is_open())
    {
        std::string line;
        for (int i = 0; std::getline(cv_file, line); i++)
        {
            std::istringstream stream(line); //затем из каждой считанной строки
            stream >> opencv.at<double>(i, 0)
                >> opencv.at<double>(i, 1)
                >> opencv.at<double>(i, 2);

        }
        cv_file.close();     // закрываем файл
    }
    //cout << opencv << "\n";
    cv::Mat difference = matlab - opencv;
    cout << difference << "\n";

    std::ofstream difference_file("D:\\TryToGetPic\\C++\\To3D_NoCMake\\Distance_difference.txt"); // окрываем файл для чтения
    if (difference_file.is_open())
    {
        for (int i = 0; i < 100; i++)
        {
            difference_file
                << difference.at<double>(i, 0) << "\t\t"
                << difference.at<double>(i, 1) << "\t\t"
                << difference.at<double>(i, 2) << '\n';

        }
        difference_file.close();     // закрываем файл
    }
}