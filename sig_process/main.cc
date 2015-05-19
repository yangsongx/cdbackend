/**
 * For Caredear OS phone(M2), image size:
 *    540 x 960
 */
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

void scale_img(double param, Mat *pOrigin)
{
    Size scaledSize(pOrigin->cols*param, pOrigin->rows*param);
    Mat scaledImg(scaledSize, CV_32S);
    //cv::Resize(*pOrigin, scaledImg, scaledSize);
    cv::resize(*pOrigin, scaledImg, scaledSize);

    vector<int> p;
    p.push_back(CV_IMWRITE_PNG_COMPRESSION);
    p.push_back(9);

    if(cv::imwrite("caled.png", scaledImg, p) == true)
    {
        printf("OK to scale img to %f\n", param);
    }
    else
    {
        printf("failed scale img\n");
    }
}

void rotate_img(double degree, Mat *pImg)
{
}


int main(int argc, char **argv)
{
    cv::Mat origin = cv::imread("a.png");

    printf("open file [OK]\n");
    printf("cols=%d, rows=%d\n", origin.cols, origin.rows);
    cv::Size size = origin.size();
    printf("After got the size,x=%d,y=%d\n", size.width, size.height);

    printf("will try scal image...\n");
    scale_img(0.5, &origin);

    Mat tmp = origin.clone();
    Rect rect(origin.cols/4, origin.rows/4, origin.cols/2, origin.rows/2);
    cv::Mat small_img = tmp(rect);


    // next try jpg->png
    Mat jpg = cv::imread("b.jpg");
    if(!jpg.data)
    {
        printf("failed open the jpg file\n");
    }
    else
    {
        printf("JPEG cols=%d, rows=%d\n", jpg.cols, jpg.rows);
        Mat newpng = jpg.clone();

        vector<int> param;
        param.push_back(CV_IMWRITE_PNG_COMPRESSION);
        param.push_back(9);

        if(imwrite("key.png", newpng/*, param*/) == true)
        {
            printf("cool to jpg->PNG\n");
        }
        else
        {
            printf("failed convert to PNG\n");
        }
    }
    

    printf("show the gui...\n");
    namedWindow("win");

    imshow("image", small_img);
    waitKey();

    printf("exit the program\n");
    return 0;
}
