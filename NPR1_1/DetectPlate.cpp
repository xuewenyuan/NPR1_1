//
//  DetectPlate.cpp
//  NPR1_1
//
//  Created by Jacob on 15/3/10.
//  Copyright (c) 2015年 Jacob. All rights reserved.
//

#include "DetectPlate.h"

//保存数据1
void DetectPlate::savedata1(cv::Mat img,int num1,int num2){
    stringstream ss(stringstream::in | stringstream::out);
    ss << "/Users/setsufumimoto/Desktop/data/temporaryData/" << num1 << "_" <<num2 << ".png";
    imwrite(ss.str(), img);
}

//保存数据2
void DetectPlate::savedata2(cv::Mat img,int num1){
    stringstream ss(stringstream::in | stringstream::out);
    ss << "/Users/setsufumimoto/Desktop/data/temporaryData/plate" << num1 << ".png";
    imwrite(ss.str(), img);
}



//默认构造函数
DetectPlate::DetectPlate() {
    
}

//构造函数
DetectPlate::DetectPlate(string trainFile){
    
    //导入OCR训练数据
    cv::FileStorage fs;
    fs.open(trainFile, cv::FileStorage::READ);
    cv::Mat SVM_trainingData;
    cv::Mat SVM_classes;
    fs["TrainingData"] >> SVM_trainingData;
    fs["classes"] >> SVM_classes;
    //设置SVM参数
    CvSVMParams SVM_params;
    SVM_params.svm_type = CvSVM::C_SVC;
    SVM_params.kernel_type = CvSVM::LINEAR; //CvSVM::LINEAR;
    SVM_params.degree = 0;
    SVM_params.gamma = 1;
    SVM_params.coef0 = 0;
    SVM_params.C = 1;
    SVM_params.nu = 0;
    SVM_params.p = 0;
    SVM_params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 1000, 0.01);
    
    svmClassifier.train(SVM_trainingData, SVM_classes, cv::Mat(), cv::Mat(), SVM_params);

}

//尺寸验证，这里以欧洲车牌为准
bool DetectPlate::verifySizes(cv::RotatedRect candidate) {
    float error = 0.4;
    float aspect = 4.7272;
    int min = 15*aspect*15;
    int max = 125*aspect*125;
    float rmin = aspect - aspect*error;
    float rmax = aspect + aspect*error;
    
    int area = candidate.size.height * candidate.size.width;
    float r = (float)candidate.size.width/(float)candidate.size.height;
    if (r < 1) {
        r = 1/r;
    }
    if ((area < min || area > max) || (r < rmin || r >rmax)) {
        return false;
    }
    else {
        return true;
    }
}

//直方图均衡化
cv::Mat DetectPlate::histeq(cv::Mat input) {
    cv::Mat out(input.size(),input.type());
    if (input.channels() == 3) {
        cv::Mat hsv;
        vector<cv::Mat> hsvSplit;
        cv::cvtColor(input, hsv, CV_BGR2HSV);
        cv::split(hsv, hsvSplit);
        cv::equalizeHist(hsvSplit[2], hsvSplit[2]);
        cv::merge(hsvSplit, hsv);
        cv::cvtColor(hsv, out, CV_HSV2BGR);
    }
    else if (input.channels() == 1) {
        cv::equalizeHist(input, out);
    }
    
    return out;
}

//重写floodFill
//opencv的flooFill算法在xcode中参数不能显示完整，所以用原函数重写一下
int DetectPlate::myfloodFill( cv::InputOutputArray _image,cv::InputOutputArray _mask,
                cv::Point seedPoint, cv::Scalar newVal, cv::Rect* rect,
                cv::Scalar loDiff, cv::Scalar upDiff, int flags ) {
    CvConnectedComp ccomp;
    CvMat c_image = _image.getMat(), c_mask = _mask.getMat();
    cvFloodFill(&c_image, seedPoint, newVal,loDiff, upDiff, &ccomp, flags, c_mask.data.ptr ? &c_mask : 0);
    if( rect )
        *rect = ccomp.rect;
    return cvRound(ccomp.area);
}


//车牌检测
vector<Plate> DetectPlate::Detect(Mat input) {
    
#pragma mark -- 从彩色车牌图片中发现可能的车牌区域，并对这些区域做标准化处理，存入 possibleRegions
    vector<Plate> possibleRegions;
    
    //转换为灰度图像
    cv::Mat cvGrayImage;
    cv::cvtColor(input, cvGrayImage, CV_BGR2GRAY);
    imwrite("/Users/setsufumimoto/Desktop/data/temporaryData/灰度.png", cvGrayImage);
    //利用5*5的高斯模糊来去噪
    cv::blur(cvGrayImage, cvGrayImage, cv::Size(5,5));
    imwrite("/Users/setsufumimoto/Desktop/data/temporaryData/高斯去噪.png", cvGrayImage);
    //Sobel滤波器
    cv::Mat cvSobelImage;
    cv::Sobel(cvGrayImage, cvSobelImage, CV_8U, 1, 0, 3, 1, 0);
    //imwrite("/Users/setsufumimoto/Desktop/data/temporaryData/Sobel.png", cvSobelImage);
    //阀值算子
    cv::Mat cvThresholdImage;
    threshold(cvSobelImage, cvThresholdImage, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
    imwrite("/Users/setsufumimoto/Desktop/data/temporaryData/阀值算子.png", cvThresholdImage);
    //闭形态学算子
    cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(17, 3));
    morphologyEx(cvThresholdImage, cvThresholdImage, CV_MOP_CLOSE, element);
    imwrite("/Users/setsufumimoto/Desktop/data/temporaryData/闭形态学算子.png", cvThresholdImage);
    //寻找可能的车牌轮廓
    vector<vector<cv::Point>> contours;
    cv::findContours(cvThresholdImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    //std::cout << contours.size();
    
    //对每个轮廓检测和提取最小区域的有界矩形区域
    vector<vector<cv::Point>>::iterator itc = contours.begin();
    vector<cv::RotatedRect> rects;
    
    while (itc != contours.end()) {
        cv::RotatedRect mr = cv::minAreaRect(cv::Mat(*itc));
        if (!verifySizes(mr)) {
            itc = contours.erase(itc);
        }
        else {
            ++ itc;
            rects.push_back(mr);
        }
    }
    //cout << rects.size();
    
    cv::Mat result;
    //input.copyTo(result);
    cvGrayImage.copyTo(result);
    //cv::drawContours(result, contours, -1, cv::Scalar(255, 0, 0), 1);
    //imwrite("/Users/setsufumimoto/Desktop/data/temporaryData/轮廓.png", result);
    //使用漫水填充算法来获取旋转矩形
    for (int i = 0; i < rects.size(); i ++) {
        
        cv::circle(result, rects[i].center, 3, cv::Scalar(0, 255, 0), -1);
        
        //取长和宽的最小值
        float minSize = (rects[i].size.width < rects[i].size.height)? rects[i].size.width: rects[i].size.height;
        minSize = minSize - minSize*0.5;
        
        //为漫水填充算法随机取5个点
        srand((int)time(NULL));
        
        //初始化漫水填充法的参数和变量
        cv::Mat mask;
        mask.create(input.rows+2, input.cols+2, CV_8UC1);
        mask = cv::Scalar::all(0);
        int loDiff = 30;
        int upDiff = 30;
        int connectivity = 4;
        int newMaskVal = 255;
        int NumSeeds = 10;
        cv::Rect ccomp;
        int flags = connectivity + (newMaskVal << 8) + CV_FLOODFILL_FIXED_RANGE + CV_FLOODFILL_MASK_ONLY;
        for (int j = 0; j < NumSeeds; j ++) {
            cv::Point seed;
            seed.x = rects[i].center.x + rand()%(int)minSize - (minSize/2);
            seed.y = rects[i].center.y + rand()%(int)minSize - (minSize/2);
            cv::circle(result, seed, 1, cv::Scalar(0, 255, 255), -1);
            int area = myfloodFill(cvGrayImage, mask, seed, cv::Scalar(255, 0, 0), &ccomp, cv::Scalar(loDiff, loDiff, loDiff), cv::Scalar(upDiff, upDiff, upDiff), flags);
            //savedata1(mask, i, j);
            //查看由漫水填充法得到的掩膜能否匹配矩形
            //首先获取所有用来绘制最小旋转矩形的点
            std::vector<cv::Point> pointsInterest;
            cv::Mat_<uchar>::iterator itMask = mask.begin<uchar>();
            cv::Mat_<uchar>::iterator end = mask.end<uchar>();
            for (; itMask != end; ++ itMask) {
                if (*itMask == 255) {
                    pointsInterest.push_back(itMask.pos());
                }
            }
            
            cv::RotatedRect minRect = cv::minAreaRect(pointsInterest);
            
            if (verifySizes(minRect)) {
                
                //绘制旋转矩形
                cv::Point2f rect_points[4];
                minRect.points(rect_points);
                for (int j = 0; j < 4; j ++) {
                    cv::line(result, rect_points[j], rect_points[(j+1)%4], cv::Scalar(0, 0, 255), 1, 8);
                
                }
                //获取旋转矩形的矩阵
                float r = (float)minRect.size.width/(float)minRect.size.height;
                float angle = minRect.angle;
                if (r<1) {
                    angle = 90 + angle;
                }
                cv::Mat rotmat = cv::getRotationMatrix2D(minRect.center, angle, 1);
                
                //创建一个旋转图像，并进行仿射变换（可以理解为图像的旋转）
                cv::Mat img_rotated;
                cv::warpAffine(cvGrayImage, img_rotated, rotmat, cvGrayImage.size(),CV_INTER_CUBIC);
                
                //对图像进行裁剪
                cv::Size rect_size = minRect.size;
                if (r<1) {
                    std::swap(rect_size.width, rect_size.height);
                }
                cv::Mat img_crop;
                cv::getRectSubPix(img_rotated, rect_size, minRect.center, img_crop);
                
                //调整一下尺寸
                cv::Mat resultResized;
                resultResized.create(33, 144, CV_8UC3);
                cv::resize(img_crop, resultResized, resultResized.size(), 0, 0, cv::INTER_CUBIC);
                
                //高斯滤波去噪
                cv::blur(resultResized, resultResized, cv::Size(3, 3));
                resultResized = histeq(resultResized);
                //savedata1(resultResized, 10+i, j);
                //将矩形区域压入输出栈
                possibleRegions.push_back(Plate(resultResized, minRect.boundingRect()));
            }
            
        }

    }
    
    std::cout << possibleRegions.size() << "\n";
#pragma mark -- 从可能的车牌区域中筛选出车牌
    /*
    //导入离线训练数据
    cv::FileStorage fs;
    fs.open("/Users/setsufumimoto/Documents/develope/DesignForGratuate/NPR1_1/SVM.xml", cv::FileStorage::READ);
    cv::Mat SVM_trainingData;
    cv::Mat SVM_classes;
    fs["TrainingData"] >> SVM_trainingData;
    fs["classes"] >> SVM_classes;
    //std::cout << SVM_trainingData.rows;
    
    //设置SVM参数
    CvSVMParams SVM_params;
    SVM_params.svm_type = CvSVM::C_SVC;
    SVM_params.kernel_type = CvSVM::LINEAR; //CvSVM::LINEAR;
    SVM_params.degree = 0;
    SVM_params.gamma = 1;
    SVM_params.coef0 = 0;
    SVM_params.C = 1;
    SVM_params.nu = 0;
    SVM_params.p = 0;
    SVM_params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 1000, 0.01);
    
    //创建SVM分类器类
    CvSVM svmClassifier(SVM_trainingData, SVM_classes, cv::Mat(), cv::Mat(), SVM_params);
    */
     
    //利用分类器的predict函数进行判断，返回筛选结果
    vector<Plate> plates;
    
    for(int i=0; i< possibleRegions.size(); i++){
        
        Mat img=possibleRegions[i].plateimg;
        Mat p= img.reshape(1, 1);
        p.convertTo(p, CV_32FC1);
        int response = (int)svmClassifier.predict( p );
        if(response==1){
            plates.push_back(possibleRegions[i]);
            //savedata2(img, i);
        }
        printf("%d.png分类结果：%d\n",i,response);
    }
    
    return plates;
}
