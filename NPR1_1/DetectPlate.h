//
//  DetectPlate.h
//  NPR1_1
//
//  Created by Jacob on 15/3/10.
//  Copyright (c) 2015年 Jacob. All rights reserved.
//

#ifndef __NPR1_1__DetectPlate__
#define __NPR1_1__DetectPlate__

#include <stdio.h>
#include <string.h>
#include <vector>

#include "Plate.h"


//#include "ViewController.m"

using namespace std;
using namespace cv;

class DetectPlate {
public:
    DetectPlate();
    DetectPlate(string trainFile);
    string filename;
    void setFilename(string f);
    bool saveRegions;
    bool showSteps;
    vector<Plate> Detect(Mat input);
    
    
private:
    vector<Plate> segment(Mat input);
    bool verifySizes(RotatedRect mr);
    Mat histeq(Mat in);
    int myfloodFill( cv::InputOutputArray _image,cv::InputOutputArray _mask,
                    cv::Point seedPoint, cv::Scalar newVal, cv::Rect* rect,
                    cv::Scalar loDiff, cv::Scalar upDiff, int flags );
    
    void savedata1(cv::Mat img,int num1,int num2);
    void savedata2(cv::Mat img,int num1);
    CvSVM svmClassifier;
    
};

#endif /* defined(__NPR1_1__DetectPlate__) */
