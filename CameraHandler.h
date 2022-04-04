#pragma once
#include <iostream>
#include "Arena\ArenaApi.h"
#include "Save\SaveApi.h"
#include "Converter.h"
#include <chrono>
#include <sstream>
#include <thread>

using namespace std;

#define TAB1 "  "
#define IMAGE_TIMEOUT 2000
#define PIXEL_FORMAT_TRANSFORM "Mono8"


class CameraHandler {
public:
    CameraHandler(string intrinsicMatrixFile = "D:\\TryToGetPic\\C++\\IntrinsicMatrix.txt") :
    IntrinsicMatrixFile(intrinsicMatrixFile)
    {}

    auto getConverterTo3D(int width = 1, int height = 1) {
        IntrinsicParameters params;
        params.readFromFile(IntrinsicMatrixFile, true);

        Converter* conv = new Converter(params, width, height);
        converter = conv;

        conv->prepareKoefs();

        auto func_converter = [conv](const uint8_t* row_image) {
            conv->setDistance(row_image);
            return conv->get3D();
        };
        return func_converter;
    }

    void AcquireImage(bool& work)
    {
        std::cout << TAB1 << "Enumerate device\n";
        Arena::ISystem* pSystem = Arena::OpenSystem();
        pSystem->UpdateDevices(100);

        std::vector<Arena::DeviceInfo> deviceInfos = pSystem->GetDevices();

        if (deviceInfos.size() > 0)
        {
            Arena::IDevice* pDevice = pSystem->CreateDevice(deviceInfos[0]);
            PrepareDevice(pDevice);
            //double focalX = Arena::GetNodeValue<double>(pDevice->GetNodeMap(), "FocalLengthX");

            std::cout << TAB1 << "Acquire image\n";

            int64_t widthInitial = Arena::GetNodeValue<int64_t>(pDevice->GetNodeMap(), "Width");
            int64_t heightInitial = Arena::GetNodeValue<int64_t>(pDevice->GetNodeMap(), "Height");

            auto converter = getConverterTo3D(widthInitial, heightInitial);

            pDevice->StartStream();
            ofstream log;
            bool logging = false;

            if (logging) {
                log = ofstream("D:\\TryToGetPic\\C++\\To3D_NoCMake\\log.txt");
            }
            if (!log.is_open()) {
                logging = false;
            }

            while (work)
            {
                auto begin = std::chrono::steady_clock::now();

                Arena::IImage* pImage = pDevice->GetImage(IMAGE_TIMEOUT);
                cv::Mat image = converter(pImage->GetData());

                
                pDevice->RequeueBuffer(pImage);
                auto end = std::chrono::steady_clock::now();
                auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
                
                if (logging)
                {
                    log << "\t<-" << image.size[0] << "-" << image.size[1] << "-" << image.size[2] << "->\t";
                    log << "Transformation passed " << elapsed_ms.count() << " ms\n";
                }
                else
                {
                    cout << "\t<-" << image.size[0] << "-" << image.size[1] << "-" << image.size[2] << "->\t";
                    cout << "Transformation passed " << elapsed_ms.count() << " ms\n";
                }
                
            }
            log.close();
            std::cout << TAB1 << "Clean up Arena\n";

            pDevice->StopStream();
            pSystem->DestroyDevice(pDevice);
        }
        Arena::CloseSystem(pSystem);
    }

    void PrepareDevice(Arena::IDevice* pDevice) {
        int64_t widthInitial = Arena::GetNodeValue<int64_t>(pDevice->GetNodeMap(), "Width");
        int64_t heightInitial = Arena::GetNodeValue<int64_t>(pDevice->GetNodeMap(), "Height");

        // enable stream auto negotiate packet size ?
        Arena::SetNodeValue<bool>(pDevice->GetTLStreamNodeMap(), "StreamAutoNegotiatePacketSize", true);
        // enable stream packet resend ?
        Arena::SetNodeValue<bool>(pDevice->GetTLStreamNodeMap(), "StreamPacketResendEnable", true);

        Arena::SetNodeValue<GenICam::gcstring>(
            pDevice->GetNodeMap(),
            "PixelFormat",
            PIXEL_FORMAT_TRANSFORM);
    }

    void WriteMatToFile(string path, cv::Mat& mat) {
        std::ofstream difference_file(path); // окрываем файл для чтения
        if (difference_file.is_open())
        {
            for (int i = 0; i < mat.size[0]; i++)
            {
                for (int j = 0; j < mat.size[1]; j++)
                {
                    difference_file
                        << "[ "
                        << mat.at<double>(i, j, 0) << ","
                        << mat.at<double>(i, j, 1) << ","
                        << mat.at<double>(i, j, 2) << "]\t";
                }
            }
            difference_file.close();     // закрываем файл
        }
    }

    ~CameraHandler() {
        if (converter != NULL) {
            delete converter;
        }
    }
private:
    Converter* converter = NULL;
    string IntrinsicMatrixFile;
};