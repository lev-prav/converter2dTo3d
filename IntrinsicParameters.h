
// TODO добавть енам дл€ искажени€ объектива
// TODO вы€снить однозначно, можно ли читать эти значени€ через јренајпи
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include <fstream>
#include <vector>

using namespace std;

struct Point {
    double X;
    double Y;
};

class IntrinsicParameters {
public:
    Point FocalLength;
    Point OpticalCenter;
    double Skew;
    IntrinsicParameters() {
        ParametersMatrix.create(3, 3, CV_64F);
    }
    bool readFromFile(string path, bool fromMatlab = false) {
        cv::Mat& data = ParametersMatrix;

        std::ifstream file(path); // окрываем файл дл€ чтени€
        if (file.is_open())
        {
            for (int i = 0; !file.eof(); i++)
            {
                file >> data.at<double>(i, 0)
                    >> data.at<double>(i, 1)
                    >> data.at<double>(i, 2);
            }
            file.close();     // закрываем файл
            return true;
        }
        return false;
    }

    void setMatrix(cv::Mat& mat) {
        ParametersMatrix = mat;
    }

    cv::Mat getMatrix() {
        return ParametersMatrix;
    }
    cv::Mat* getMatrixPtr() {
        return &ParametersMatrix;
    }

    void operator=(IntrinsicParameters& P) {
        if (&P == this) return;

        this->FocalLength = P.FocalLength;
        this->OpticalCenter = P.OpticalCenter;
        this->Skew = P.Skew;
        this->ParametersMatrix = P.getMatrix();
    }
private:
    void setParameters(bool isTransposed) {
        if (!isTransposed) {
            cv::Mat transposed(ParametersMatrix);
            transpose(transposed, ParametersMatrix);
        }
        cv::Mat& data = ParametersMatrix;

        FocalLength.X = data.at<double>(0, 0);
        FocalLength.Y = data.at<double>(1, 1);
        OpticalCenter.X = data.at<double>(2, 0);
        OpticalCenter.Y = data.at<double>(2, 1);
        Skew = data.at<double>(1, 1);
    }
    cv::Mat ParametersMatrix;
};
