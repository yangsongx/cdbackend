
/**
 * For Caredear OS phone(M2), image size:
 *    540 x 960
 */
#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

void test_on_taiji_shiping()
{
    cv::Mat origin = cv::imread("test2.png", CV_LOAD_IMAGE_COLOR);
    if(!origin.data)
    {
        printf("failed open the image!\n");
        return;
    }

    namedWindow("demo");
    imshow("1", origin);
    waitKey();
}

void cut_img(Mat *img, int row, int col)
{
}

void crop_img(Mat *img)
{
    printf("image width=%d height=%d\n",
            img->cols, img->rows);
    printf("the x=%f\n", (img->cols*0.01));

    cv::Rect smallRect(0, img->rows/5, img->cols, (img->rows*0.6));

    cv::Mat tmp = img->clone();
    cv::Mat cropped = tmp(smallRect);

    cv::namedWindow("demo");
    imshow("original", *img);
    imshow("cropped", cropped);
    cv::waitKey();
}

//using range to crop the image
void crop_img2(Mat *img)
{
    printf("image width=%d height=%d\n",
            img->cols, img->rows);
    printf("the 2nd width=%d, height=%d\n",
            img->size().width, img->size().height);

    cv::Mat tmp = img->clone();
    cv::Range beg;
    beg.start = img->rows*0.20;
    beg.end= img->rows*0.38;

    cv::Range end;
    end.start = img->cols*0.023;
    end.end=img->cols*0.195;

    cv::Mat cropped(tmp, beg, end);

    cv::namedWindow("demo");
    imshow("original", *img);
    imshow("cropped", cropped);

    cv::Mat bin=cropped.clone();
    cv::threshold(cropped, bin, 210, 255, CV_THRESH_BINARY);
    imshow("binned", bin);

    // second
    end.start += (img->cols*0.002);
    end.start += (img->cols*0.161);
    end.end += img->cols*0.162;
    cv::Mat cropped2(tmp, beg, end);
    imshow("cropped2", cropped2);

    end.start += (img->cols*0.002);
    end.start += (img->cols*0.161);
    end.end += img->cols*0.162;
    cv::Mat cropped3(tmp, beg, end);
    imshow("cropped3", cropped3);


    cv::waitKey();
}

void handle_block_img(Mat *img)
{
}


int main(int argc, char **argv)
{
    cv::Mat origin = cv::imread("full.png", 0/*-1*/);
    if(!origin.data)
    {
        printf("failed open image\n");
        exit(1);
    }

    crop_img2(&origin);

    printf("exit the cut program\n");
    return 0;
}
