/**
 * for image extrace demo
 */
#include <stdio.h>
#include <getopt.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int ignore = 0;

/** learn how to extract out rectangle pic */

void foo()
{
    Point2f inputQuad[4]; 
    Point2f outputQuad[4];
         
    // Lambda Matrix
    Mat lambda( 2, 4, CV_32FC1 );
    //Input and Output Image;
    Mat input, output;
     
    //Load the image
    input = imread( "extractimg.png", 1 );
    // Set the lambda matrix the same type and size as input
    lambda = Mat::zeros( input.rows, input.cols, input.type() );
 
    // The 4 points that select quadilateral on the input , from top-left in clockwise order
    // These four pts are the sides of the rect box used as input 
    inputQuad[0] = Point2f(160, 70 );
    inputQuad[1] = Point2f(298, 130);
    inputQuad[2] = Point2f(180, 260);
    inputQuad[3] = Point2f(2, 200);
    // The 4 points where the mapping is to be done , from top-left in clockwise order
    outputQuad[0] = Point2f( 0,0 );
    outputQuad[1] = Point2f(150, 0);
    outputQuad[2] = Point2f( 170, 170);
    outputQuad[3] = Point2f( 0, 170);
 
    // Get the Perspective Transform Matrix i.e. lambda 
    lambda = getPerspectiveTransform( inputQuad, outputQuad );
    // Apply the Perspective Transform just found to the src image
    warpPerspective(input,output,lambda, Size(150, 170));
 
    //Display input and output
    imshow("Input",input);
    imshow("Output",output);
 
    waitKey(0);
}

// get all lines, and draw it.
void detectlines()
{
    cv::Mat img = cv::imread("extractimg.png", 1);
    cv::Mat gray;
    cv::cvtColor(img, gray, CV_BGR2GRAY);
    cv::Mat contours;
    cv::Canny(gray, contours, 125, 350);
    printf("finished reading,size=%d %d\n", img.rows, img.cols);

    cv::threshold(contours, contours, 128, 255, THRESH_BINARY);

    vector<Vec4i> lines;

    cv::HoughLinesP(contours, lines, 1, CV_PI/180, 80, 50, 10);


    printf("will try traverse the lines...\n");
    printf("the line whole length=%ld\n", lines.size());
    vector<Vec4i>::const_iterator it=lines.begin();
    while(it != lines.end()){
        //printf("a line...\n");
        Point pt1((*it)[0],(*it)[1]);
        Point pt2((*it)[2],(*it)[3]);
        line(img, pt1, pt2, Scalar(0, 255, 0), 2);
        ++it;
    }
    //
    namedWindow("lines");
    imshow("lines", img);
    waitKey();
}

void auto_detect()
{
    cv::Mat img = cv::imread("extractimg.png", 1);
    cv::Mat filtered;
    pyrMeanShiftFiltering(img, filtered, 25, 10);

    namedWindow("filter");
    imshow("not-filter", img);
    imshow("filter", filtered);
    waitKey();
}

int main(int argc, char **argv)
{

    //simplestdemo("extractimg.png", argc, argv);
    //foo();
    //detectlines();
    auto_detect();

    return 0;
}
