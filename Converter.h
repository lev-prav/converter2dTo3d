#include "IntrinsicParameters.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"

using namespace cv;

class Converter
{
    IntrinsicParameters P;
    Mat* DistanceMatrix;
    Mat* K;
    Mat* Matrix_3D;
    int picture_width;
    int picture_height;
public:
    Converter(IntrinsicParameters& _P, int p_width = 640, int p_height = 480) :
        P(_P), picture_width(p_width), picture_height(p_height)
    {
        DistanceMatrix = new Mat(picture_height, picture_width, CV_64F);
        int dims[] = { picture_height, picture_width, 3 };
        K = new Mat();
        Matrix_3D = new Mat();
        Matrix_3D->create(3, dims, CV_64F);
        K->create(3, dims, CV_64F);

        //cout<<"INTRINSIC:\t"<<P.getMatrix()<<"\n";
    }

    void prepareKoefs() {
        cv::Mat* P_row = P.getMatrixPtr();
        cv::Mat P_mat = P_row->inv().t();
        //-------------------------------------------------------------------
        double ptr[3][3] = 
        {
            {0.0019, 0.0000, -0.6046 },
            { 0.0,   0.0019, -0.4459 },
            { 0.0,     0.0,   1.0000 }
        };
        //cv::Mat P_mat(3, 3, CV_64F, ptr);
        //---------------------------------------------------------------------

        //cout << "P_mat inv : \n" << P_mat << "\n";
        //--------------------------------------------------------------------------------------
        /*
        cv::Mat difference = P_matlab - P_mat;
        cout << "Difference \n" << difference << "\n";
        std::ofstream file("D:\\TryToGetPic\\C++\\To3D_NoCMake\\Intrinsic_Difference.txt"); // окрываем файл для чтения
        if (file.is_open())
        {
            for (int i = 0; i < 3; i++)
            {
                file << difference.at<double>(i, 0)<<"\t"
                     << difference.at<double>(i, 1)<<"\t"
                     << difference.at<double>(i, 2)<<"\n";
            }
            file.close();     // закрываем файл
        }
        */
        //---------------------------------------------------------------------------------------

        cv::Mat pixel_3d(3, 1, CV_64F, Scalar(1.0));
        
        double i_d = 0.0, j_d = 0.0;

        for (int i = 0; i < picture_height; i++) {
            pixel_3d.at<double>(0, 0) = i_d++;
            for (int j = 0; j < picture_width; j++) {
                pixel_3d.at<double>(1, 0) = j_d++;

                cv::Mat on_matrix_pos = (P_mat) * pixel_3d;

                cv::Mat sqr_on_matrix = on_matrix_pos.mul(on_matrix_pos);
                double sum_on_matrix = sum(sqr_on_matrix.t())[0];
                double dist_on_matrix = sqrt(sum_on_matrix);

                //!!cout << on_matrix_pos->mul(dist_on_matrix);
                Mat k_mat = on_matrix_pos.mul(1 / dist_on_matrix);

                /*
                cout << "on matr pos : \t" << on_matrix_pos << "\n" << "pixel_3d: \t" << pixel_3d << "\n"
                     << "k_mat:\t " << on_matrix_pos.mul(1 / dist_on_matrix) << "\n";
                */

                double* pixel = K->ptr<double>(i, j);
                pixel[0] = k_mat.at<double>(0, 0);
                pixel[1] = k_mat.at<double>(1, 0);
                pixel[2] = k_mat.at<double>(2, 0);

                //cout << pixel[0] << " " << pixel[1] << " " << pixel[2] << "\n";
            }
        }
    }

    void setDistance(const uint8_t* dist_arr) {

        for (int i = 0; i < picture_height; i++) {
            double* row = DistanceMatrix->ptr<double>(i);
            for (int j = 0; j < picture_width; j++) {
                row[j] = (double)(dist_arr[i * picture_height + j]);
            }
        }
    }

    cv::Mat get3D() {
        for (int i = 0; i < picture_height; i++) {
            for (int j = 0; j < picture_width; j++) {
                double* cord_3d = Matrix_3D->ptr<double>(i, j);
                double* cord_k = K->ptr<double>(i, j);
                cord_3d[0] = cord_k[0] * DistanceMatrix->at<double>(i,j);
                cord_3d[1] = cord_k[1] * DistanceMatrix->at<double>(i,j);
                cord_3d[2] = cord_k[2] * DistanceMatrix->at<double>(i,j);
            }
        }
        return *Matrix_3D;
    }

    void print3D() {
        cout << "3D PIC\n";
        for (int i = 0; i < picture_height; i++) {
            for (int j = 0; j < picture_width; j++) {
                double* cord_3d = Matrix_3D->ptr<double>(i, j);
                cout << cord_3d[0] << " " << cord_3d[1] << " " << cord_3d[2] << "|";
            }
            cout << "\n";
        }
        cout << "\n";
    }
    void printK() {
        cout << "K matrix\n";
        for (int i = 0; i < picture_height; i++) {
            for (int j = 0; j < picture_width; j++) {
                double* cord_k = K->ptr<double>(i, j);
                cout << cord_k[0] << " " << cord_k[1] << " " << cord_k[2] << "|";
            }
            cout << "\n";
        }
        cout << "\n";
    }
    void printDist() {
        cout << "Distance matrix\n";
        for (int i = 0; i < picture_height; i++) {
            for (int j = 0; j < picture_width; j++) {
                double* cord_3d = DistanceMatrix->ptr<double>(i, j);
                cout << cord_3d[0] << " " << cord_3d[1] << " " << cord_3d[2] << "|";
            }
            cout << "\n";
        }
        cout << "\n";
    }

    void printKandPicandDist() {
        print3D();
        printK();
        printDist();
    }

    ~Converter() {
        delete DistanceMatrix;
        delete K;
        delete Matrix_3D;
    }
};

