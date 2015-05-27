/**
 * For Caredear OS phone(M2), image size:
 *    540 x 960
 */
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
#if 1
/************************************************************************/
/* 
Description:        手势检测
                                先滤波去噪
                                -->转换到HSV空间
                                -->根据皮肤在HSV空间的分布做出阈值判断，这里用到了inRange函数，
                                然后进行一下形态学的操作，去除噪声干扰，是手的边界更加清晰平滑
                                -->得到的2值图像后用findContours找出手的轮廓，去除伪轮廓后，再用convexHull函数得到凸包络
Author:                        Yang Xian
Email:                        yang_xian521@163.com
Version:                2011-11-2
History:                
*/
/************************************************************************/
#include <iostream>        // for standard I/O
#include <string>   // for strings
#include <iomanip>  // for controlling float print precision
#include <sstream>  // string to number conversion

#include <opencv2/imgproc/imgproc.hpp>  // Gaussian Blur
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/highgui/highgui.hpp>  // OpenCV window I/O

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
        std::string sourceReference = "./demo.mp4";
    if(argc > 1) {
        sourceReference = argv[1];
    }
        printf("the imag:%s\n", sourceReference.c_str());
        int delay = 1;

        char c;
        int frameNum = -1;                        // Frame counter

        VideoCapture captRefrnc(sourceReference);

        if ( !captRefrnc.isOpened())
        {
                cout  << "Could not open reference " << sourceReference << endl;
                return -1;
        }

        Size refS = Size( (int) captRefrnc.get(CV_CAP_PROP_FRAME_WIDTH),
                (int) captRefrnc.get(CV_CAP_PROP_FRAME_HEIGHT) );

        bool bHandFlag = false;

        const char* WIN_SRC = "Source";
        const char* WIN_RESULT = "Result";

        // Windows
        namedWindow(WIN_SRC, CV_WINDOW_AUTOSIZE );
        namedWindow(WIN_RESULT, CV_WINDOW_AUTOSIZE);

        Mat frame;        // 输入视频帧序列
        Mat frameHSV;        // hsv空间
        Mat mask(frame.rows, frame.cols, CV_8UC1);        // 2值掩膜
        Mat dst(frame);        // 输出图像

//         Mat frameSplit[4];

        vector< vector<Point> > contours;        // 轮廓
        vector< vector<Point> > filterContours;        // 筛选后的轮廓
        vector< Vec4i > hierarchy;        // 轮廓的结构信息
        vector< Point > hull;        // 凸包络的点集

        while(true) //Show the image captured in the window and repeat
        {
                captRefrnc >> frame;

                if( frame.empty() )
                {
                        cout << " < < <  Game over!  > > > ";
                        break;
                }
                imshow( WIN_SRC, frame);

                // Begin

                // 中值滤波，去除椒盐噪声
                medianBlur(frame, frame, 5);
//                 GaussianBlur( frame, frameHSV, Size(9, 9), 2, 2 );
//                 imshow("blur2", frameHSV);
//                pyrMeanShiftFiltering(frame, frameHSV, 10, 10);
//                 imshow(WIN_BLUR, frameHSV);
                // 转换到HSV颜色空间，更容易处理
                cvtColor( frame, frameHSV, CV_BGR2HSV );

//                 split(frameHSV, frameSplit);
//                 imshow(WIN_H, frameSplit[0]);
//                 imshow(WIN_S, frameSplit[1]);
//                 imshow(WIN_V, frameSplit[2]);

                Mat dstTemp1(frame.rows, frame.cols, CV_8UC1);
                Mat dstTemp2(frame.rows, frame.cols, CV_8UC1);
                // 对HSV空间进行量化，得到2值图像，亮的部分为手的形状
                inRange(frameHSV, Scalar(0,30,30), Scalar(40,170,256), dstTemp1);
                inRange(frameHSV, Scalar(156,30,30), Scalar(180,170,256), dstTemp2);
                bitwise_or(dstTemp1, dstTemp2, mask);
//                 inRange(frameHSV, Scalar(0,30,30), Scalar(180,170,256), dst);                

                // 形态学操作，去除噪声，并使手的边界更加清晰
                Mat element = getStructuringElement(MORPH_RECT, Size(3,3));
                erode(mask, mask, element);
                morphologyEx(mask, mask, MORPH_OPEN, element);
                dilate(mask, mask, element);
                morphologyEx(mask, mask, MORPH_CLOSE, element);

                frame.copyTo(dst, mask);

                contours.clear();
                hierarchy.clear();
                filterContours.clear();
                // 得到手的轮廓
                findContours(mask, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
                // 去除伪轮廓
                for (size_t i = 0; i < contours.size(); i++)
                {
//                         approxPolyDP(Mat(contours[i]), Mat(approxContours[i]), arcLength(Mat(contours[i]), true)*0.02, true);
                        if (fabs(contourArea(Mat(contours[i]))) > 30000)        //判断手进入区域的阈值
                        {
                                filterContours.push_back(contours[i]);
                        }
                }
                // 画轮廓
                drawContours(dst, filterContours, -1, Scalar(0,0,255), 3/*, 8, hierarchy*/);
                // 得到轮廓的凸包络
                for (size_t j=0; j<filterContours.size(); j++)
                {
                        convexHull(Mat(filterContours[j]), hull, true);
                        int hullcount = (int)hull.size();

                        for (int i=0; i<hullcount-1; i++)
                        {
                                line(dst, hull[i+1], hull[i], Scalar(255,0,0), 2, CV_AA);
                        }
                        line(dst, hull[hullcount-1], hull[0], Scalar(255,0,0), 2, CV_AA);
                }
                
                imshow(WIN_RESULT, dst);
                dst.release();
                // End

                c = cvWaitKey(delay);
                if (c == 27) break;
        }
}
#else

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
#endif
