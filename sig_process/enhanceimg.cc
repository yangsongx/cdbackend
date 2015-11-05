/**
 * For Caredear OS phone(M2), image size:
 *    540 x 960
 */
#include <stdio.h>
#include <getopt.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// below for libpng
#include <png.h>

using namespace cv;

int ignore = 0;

/** learn how to extract out rectangle pic */

void mydemo(const char *fn, const char *newfn, int level)
{
    cv::Mat img = cv::imread(fn, 0);
    assert(img.data!=NULL);

    Size size(img.cols, img.rows);
    printf("image size:x=%d,y=%d\n", img.cols, img.rows);
    printf("the level = %d\n", level);

    cv::Mat bin(size, IPL_DEPTH_8U) ;
    //cv::cvtColor(img, bin,CV_BGR2GRAY);
    //bin = c
    //cv::threshold(img, bin, 147, 255, CV_THRESH_BINARY);
    cv::threshold(img, bin, level, 255, CV_THRESH_BINARY);

    //imwrite("test.png", txtSection);

    if (ignore == 0) {
    namedWindow("win");

    imshow("image", img);
    imshow("bin", bin);
    //imshow("txt", txtSection);
    //imshow("final", result);
    waitKey();
    }

    cv::imwrite(newfn, bin);
}

void simpleopenpng(const char *fn)
{
    cv::Mat img = cv::imread(fn);
    if(!img.data)
        return;

    printf("image's x=%d,y=%d\n", img.cols, img.rows);
    printf("elemsize=%ld, total=%ld\n", img.elemSize(), img.total());
}


int main(int argc, char **argv)
{
    int c;
    char newfile[1024];
    int level = 45;
    if(argc < 2) {
        printf("enimg -l N imagefile\n   N the threshold level(0-255)\n");
        exit(1);
    }

    while((c = getopt(argc, argv, "il:")) != -1) {
        switch(c)
        {
            case 'l':
                level = atoi(optarg);
                break;

            case 'i':
                ignore = 1;
                break;
        }
    }

    snprintf(newfile, sizeof(newfile),
            "en-%s", argv[argc - 1]);

    mydemo(argv[argc - 1], newfile, level);

    printf("%s ===> %s (with threshold %d) [OK] \nexit the program\n",
            argv[argc - 1], newfile, level);
    return 0;
}
