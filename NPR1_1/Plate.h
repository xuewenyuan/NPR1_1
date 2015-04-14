//
//  Plate.h
//  NPR1_1
//
//  Created by Jacob on 15/3/7.
//  Copyright (c) 2015年 Jacob. All rights reserved.
//

#ifndef __NPR1_1__Plate__
#define __NPR1_1__Plate__

#include <stdio.h>
#include <string.h>
#include <vector>

using namespace std;
using namespace cv;

class Plate {
public:
    Plate();
    Plate(cv::Mat img, cv::Rect pos);
    string str();
    cv::Rect position;
    cv::Mat plateimg;
    vector<char> chars;
    vector<cv::Rect> charsPos;
};

#endif /* defined(__NPR1_1__Plate__) */
