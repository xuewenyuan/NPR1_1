//
//  OCR.h
//  NPR1_1
//
//  Created by Jacob on 15/3/15.
//  Copyright (c) 2015年 Jacob. All rights reserved.
//

#ifndef __NPR1_1__OCR__
#define __NPR1_1__OCR__

#include <string.h>
#include <vector>

#include "Plate.h"

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define HORIZONTAL    1
#define VERTICAL    0

class CharSegment{
public:
    CharSegment();
    CharSegment(Mat i, cv::Rect p);
    Mat img;
    cv::Rect pos;
};

class OCR{
public:
    string filename;
    static const int numCharacters;
    static const char strCharacters[];
    OCR(string trainFile);
    OCR();
    int charSize;
    Mat preprocessChar(Mat in);
    int classify(Mat f);
    void train(Mat trainData, Mat trainClasses, int nlayers);
    Mat features(Mat input, int size);
    string segment(Plate input);
private:
    Mat Preprocess(Mat in, int newSize);
    Mat getVisualHistogram(Mat *hist, int type);
    void drawVisualFeatures(Mat character, Mat hhist, Mat vhist, Mat lowData);
    Mat ProjectedHistogram(Mat img, int t);
    bool verifySizes(Mat r);
    void sharpen (const cv::Mat input, cv::Mat result);
    Mat deletedge (cv::Mat input);
    CvANN_MLP  ann;
    void savedata1(cv::Mat img,int num1,int num2);
    void savedata2(cv::Mat img,int num1);
    //CvKNearest knnClassifier;
    //int K;
};

#endif /* defined(__NPR1_1__OCR__) */
