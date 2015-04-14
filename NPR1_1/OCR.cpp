//
//  OCR.cpp
//  NPR1_1
//
//  Created by Jacob on 15/3/15.
//  Copyright (c) 2015年 Jacob. All rights reserved.
//

#include "OCR.h"

//保存数据1
void OCR::savedata1(cv::Mat img,int num1,int num2){
    stringstream ss(stringstream::in | stringstream::out);
    ss << "/Users/setsufumimoto/Desktop/data/temporaryData/" << num1 << "_" <<num2 << ".png";
    imwrite(ss.str(), img);
}

//保存数据2
void OCR::savedata2(cv::Mat img,int num1){
    stringstream ss(stringstream::in | stringstream::out);
    ss << "/Users/setsufumimoto/Desktop/data/temporaryData/" << num1 << ".png";
    imwrite(ss.str(), img);
}



const char OCR::strCharacters[] = {'0','1','2','3','4','5','6','7','8','9','A','B','E','H','I','K','M','N','O','P','T','X','Y','Z'};
const int OCR::numCharacters=24;

//Charsegment构造函数
CharSegment::CharSegment(){}
CharSegment::CharSegment(Mat i, cv::Rect p){
    img=i;
    pos=p;
}

//默认构造函数
OCR::OCR(){
    
    charSize=20;
}

//构造函数
OCR::OCR(string trainFile){
    charSize=20;
    
    //导入OCR训练数据
    FileStorage fs;
    fs.open(trainFile, FileStorage::READ);
    Mat TrainingData;
    Mat Classes;
    fs["TrainingDataF15"] >> TrainingData;
    fs["classes"] >> Classes;
    
    train(TrainingData, Classes, 10);
    
}

//字符的尺寸验证
bool OCR::verifySizes(cv::Mat r){
    //字符尺寸58*77
    float aspect=58.0f/77.0f;
    float charAspect= (float)r.cols/(float)r.rows;
    float error=0.35;
    float minHeight=15;
    float maxHeight=33;
    //设置长宽比的范围
    float minAspect=0.1;
    float maxAspect=aspect+aspect*error;
    //像素值大于0的像素个数
    float area= cv::countNonZero(r);
    //像素面积
    float bbArea=r.cols*r.rows;
    //像素值为0的个数占所有像素的百分比超过90%，认为是一个黑色块
    float percPixels=area/bbArea;
    if (percPixels < 0.7) {
        if (charAspect > minAspect) {
            if (charAspect < maxAspect) {
                if (r.rows >= minHeight) {
                    if (r.rows <= maxHeight) {
                        std::cout << "no problem" << "\n";
                        return true;
                    }
                    else {
                        std::cout << "r.rows >= maxHeight"<< "\n";
                        return false;
                    }
                }
                else {
                    std::cout << "r.rows < minHeight"<< "\n";
                    return false;
                }
            }
            else {
                std::cout << "charAspect >= maxAspect"<< "\n";
                return false;
            }
        }
        else {
            std::cout << "charAspect <= minAspect"<< "\n";
            return false;
        }
    }
    else {
        std::cout << "percPixels >= 0.8" << "\n";
        return false;
    }
    
}

//对字符图像进行预处理
cv::Mat OCR::preprocessChar(cv::Mat in){
    //重绘图像
    int h=in.rows;
    int w=in.cols;
    cv::Mat transformMat = cv::Mat::eye(2,3,CV_32F);
    int m=cv::max(w,h);
    transformMat.at<float>(0,2)=m/2 - w/2;
    transformMat.at<float>(1,2)=m/2 - h/2;
    
    cv::Mat warpImage(m,m, in.type());
    cv::warpAffine(in, warpImage, transformMat, warpImage.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(0) );
    
    cv::Mat out;
    cv::resize(warpImage, out, cv::Size(20, 20) );
    
    
    return out;
}

//直方图统计
cv::Mat OCR::ProjectedHistogram(cv::Mat img, int flag) {
    
    int sz = (flag)? img.rows: img.cols;
    cv::Mat hist = cv::Mat::zeros(1, sz, CV_32F);
    
    for (int i = 0; i < sz; i ++) {
        cv::Mat data = (flag)? img.row(i): img.col(i);
        hist.at<float>(i) = cv::countNonZero(data);
    }
    
    //正则化直方图
    double min, max;
    cv::minMaxLoc(hist, &min, &max);
    
    if (max > 0) {
        hist.convertTo(hist, -1, 1.0f/max, 0);
    }
    return hist;
}

//求特征向量
cv::Mat OCR::features(cv::Mat in, int sizeData) {
    //分别获取垂直与水平方向上的直方图
    cv::Mat vhist = ProjectedHistogram(in, VERTICAL);
    cv::Mat hhist = ProjectedHistogram(in, HORIZONTAL);
    
    //低分辨率图像特征
    cv::Mat lowData;
    cv::resize(in, lowData, cv::Size(sizeData, sizeData));
    int numCols =  vhist.cols + hhist.cols + lowData.cols*lowData.cols;
    cv::Mat out = cv::Mat::zeros(1, numCols, CV_32F);
    
    //为特征向量赋值,ANN的样本特征为水平、垂直直方图和低分辨率图像所组成的矢量
    int start = 0;
    /*/将in的阵列作为特征
    for(int i=0; i<in.rows; i++){
        for(int k=0; k<in.cols; k++){
            out.at<float>(start)=(float)in.at<unsigned char>(i,k);
            start++;
        }
        
    }
    */
    for (int i = 0; i < vhist.cols; i ++) {
        out.at<float>(start) = vhist.at<float>(i);
        start ++;
    }
    for (int i = 0; i < hhist.cols; i ++) {
        out.at<float>(start) = hhist.at<float>(i);
        start ++;
    }
    for (int i = 0; i < lowData.cols; i ++) {
        for (int j = 0; j < lowData.rows; j ++) {
            out.at<float>(start) = (float)lowData.at<unsigned char>(i,j);
            start ++;
        }
    }
    return out;
}

//初始化，并训练ANN类
void OCR::train(Mat TrainData, Mat classes, int nlayers){
    Mat layers(1,3,CV_32SC1);
    layers.at<int>(0)= TrainData.cols;
    layers.at<int>(1)= nlayers;
    layers.at<int>(2)= numCharacters;
    ann.create(layers, CvANN_MLP::SIGMOID_SYM, 1, 1);
    //ann
    //准备训练数据
    //创建一个矩阵，有n个样本，m个分类
    Mat trainClasses;
    trainClasses.create( TrainData.rows, numCharacters, CV_32FC1 );
    for( int i = 0; i <  trainClasses.rows; i++ )
    {
        for( int k = 0; k < trainClasses.cols; k++ )
        {
            //If class of data i is same than a k class
            if( k == classes.at<int>(i) )
                trainClasses.at<float>(i,k) = 1;
            else
                trainClasses.at<float>(i,k) = 0;
        }
    }
    Mat weights( 1, TrainData.rows, CV_32FC1, Scalar::all(1) );
    
    
    //训练分类器
    ann.train( TrainData, trainClasses, weights);

}

//分类，返回字符在strCharacters中的位置
int OCR::classify(Mat f){
    Mat output(1, numCharacters, CV_32FC1);
    ann.predict(f, output);
    cv::Point maxLoc;
    double maxVal;
    minMaxLoc(output, 0, &maxVal, 0, &maxLoc);
    //We need know where in output is the max val, the x (cols) is the class.
    
    return maxLoc.x;
}

//锐化处理
void OCR::sharpen (const cv::Mat input, cv::Mat result){
    
    //调用滤波函数来完成图像的锐化
    //滤波器的核
    Mat kernel(3,3,CV_32F,Scalar(0));
    // 分配像素置
    kernel.at<float>(1,1) = 5.0;
    kernel.at<float>(0,1) = -1.0;
    kernel.at<float>(2,1) = -1.0;
    kernel.at<float>(1,0) = -1.0;
    kernel.at<float>(1,2) = -1.0;
    //调用滤波函数
    filter2D(input,result,input.depth(),kernel);
}

//去除车牌边缘
Mat OCR::deletedge (cv::Mat input){
    cv::Mat output;
    IplImage pI_1 = input, pI_2;
    CvScalar s1, s2;
    //用sum来记录白色像素的个数，用sum1来记录像素值间断的个数，用sum2来记录像素值连续的个数
    int sum = 0, sum1 = 0,sum2 = 0;
    int height_1 = 0, height_2 = 0;
    int width_1 = 0, width_2 = 0;
    int i, j;
    //cout << input.rows << " " << input.cols << endl;
    for(i=0; i<input.rows/2; i++){
        sum = 0;
        sum1 = 0,
        sum2 = 0;
        for(j=0; j<input.cols-1; j++){
            s1 = cvGet2D(&pI_1, i, j);
            s2 = cvGet2D(&pI_1, i, j+1);
            if(((int)s1.val[0]) != ((int)s2.val[0])){
                sum1 += 1;
                sum2 = 0;
            }else{
                sum2 += 1;
            }
            if ((int)s1.val[0] == 255) {
                sum ++;
            }
            if(sum2 != 0){
                if( sum2 > input.cols/7){
                    sum1 = 0;
                    break;
                }
            }
        }
        if((sum1 >= 14 && sum <= 20)|| sum <= 20){
            height_1 = i;
            break;
        }else{
            height_1 = i;
        }
    }
    for(i=input.rows-1; i>input.rows/2; i--){
        sum = 0;
        sum1 = 0,
        sum2 = 0;
        for(j=0; j<input.cols-1; j++){
            s1 = cvGet2D(&pI_1, i, j);
            s2 = cvGet2D(&pI_1, i, j+1);
            if(((int)s1.val[0]) != ((int)s2.val[0])){
                sum1 += 1;
                sum2 = 0;
            }else{
                sum2 += 1;
            }
            if ((int)s1.val[0] == 255) {
                sum ++;
            }
            if(sum2 != 0){
                if( sum2 > input.cols/7){
                    sum1 = 0;
                    break;
                }
            }
        }
        if((sum1 >= 14 && sum <= 20)|| sum <= 20){
            height_2 = i;
            break;
        }else{
            height_2 = i;
        }
    }
    
    for (i = 0; i <= input.cols/16; i++) {
        sum = 0, sum2 = 0;
        for (j = 0; j < input.rows; j++) {
            s1 = cvGet2D(&pI_1, j, i);
            if ((int)s1.val[0] == 255) {
                sum ++;
            }
        }
        if (sum<=8) {
            width_1 = i;
            break;
        }
        else{
            width_1 = i;
        }
    }
    
    for (i = input.cols-1; i > input.cols*15/16; i--) {
        sum = 0, sum2 = 0;
        for (j = 0; j < input.rows; j++) {
            s1 = cvGet2D(&pI_1, j, i);
            if ((int)s1.val[0] == 255) {
                sum ++;
            }
        }
        if (sum<=8) {
            width_2 = i;
            break;
        }
        else{
            width_2 = i;
        }
    }
    //cv::Mat resultROI = input(cv::Rect(height_1, width_1, width_2-width_1+1, height_2-height_1+1));
    /*
    cv:: Mat result = Mat(height_2-height_1+1, width_2-width_1+1,  CV_8UC1, 1);
    pI_2 = result;
    
    for(i=height_1; i<=height_2; i++){
        for(j=width_1; j<= width_2; j++){
            s1 = cvGet2D(&pI_1, i, j);
            cvSet2D(&pI_2, i-height_1, j-width_1, s1);
            //cout << i << " " << j << endl;
        }	
    }
    
    output.create(33, 144, CV_8UC1);
    cv::resize(result, output, output.size(), 0, 0, cv::INTER_CUBIC);
    return output;
    */
    cv:: Mat result = Mat(input.rows, input.cols,  CV_8UC1, 1);
    pI_2 = result;
    
    for(i=0; i<input.rows; i++){
        for(j=0; j<input.cols; j++){
            if ((i>=height_1 && i<=height_2) && (j>=width_1 && j<=width_2)) {
                s1 = cvGet2D(&pI_1, i, j);
                cvSet2D(&pI_2, i, j, s1);
                //cout << i << " " << j << endl;
            }
            else {
                cvSet2D(&pI_2, i, j, CvScalar());
            }
        }
    }
    
    output.create(33, 144, CV_8UC1);
    cv::resize(result, output, output.size(), 0, 0, cv::INTER_CUBIC);
    return output;
}

//字符分割
string OCR::segment(Plate inputplate){
    Mat input=inputplate.plateimg;
    //savedata2(input , 1111111);
    vector<CharSegment> segmentchar;
    savedata2(input, 6666);
    //锐化
    sharpen (input, input);
    savedata2(input, 7777);
    
    //二值化图像
    Mat img_threshold;
    //threshold(input, img_threshold, 60, 255, CV_THRESH_BINARY_INV);
    adaptiveThreshold(input, img_threshold, 255, ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 37, 31);
    //threshold(input, img_threshold, 60, 255, CV_THRESH_OTSU+CV_THRESH_BINARY_INV);
    savedata2(img_threshold, 8888);
    
    //去除边缘
    Mat afterdelete;
    afterdelete = deletedge(img_threshold);
    //sharpen (afterdelete, afterdelete);
    //morphologyEx(afterdelete, afterdelete, MORPH_OPEN, MORPH_RECT);
    savedata2(afterdelete, 9999);
    
    Mat img_contours;
    afterdelete.copyTo(img_contours);
    
    //寻找可能的字符边缘
    vector<vector<cv::Point> > contours;
    findContours(img_contours,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    
    //在图像上描出边缘
    cv::Mat result;
    img_threshold.copyTo(result);
    cvtColor(result, result, CV_GRAY2RGB);
    //cv::drawContours(result,contours,-1,cv::Scalar(255,0,0),1);
    //savedata2(result, 555);
    //Start to iterate to each contour founded
    vector<vector<cv::Point> >::iterator itc= contours.begin();
    
    //去除在限制外的区域
    int i = 0;
    while (itc!=contours.end()) {
        
        //绘制边缘的外接矩形
        cv::Rect mr= boundingRect(Mat(*itc));
        //rectangle(result, mr, Scalar(0,255,0));
        //裁剪
        Mat auxRoi(afterdelete, mr);
        savedata2(auxRoi, i*100+11);
        
        if(verifySizes(auxRoi)){
            auxRoi=preprocessChar(auxRoi);
            //savedata2(auxRoi, i*100+22);
            segmentchar.push_back(CharSegment(auxRoi, mr));
            //rectangle(result, mr, Scalar(0,125,255));
        }
        ++itc;
        i++;
    }
    
    
    for(int i=0; i<segmentchar.size(); i++){
        
        //进一步处理分割的字符，让他们拥有相同的大小
        Mat ch=preprocessChar(segmentchar[i].img);
        cout << segmentchar.size() << endl;
        savedata2(ch, i*100+33);
        //提取每个分割字符的特征
        Mat f=features(ch,15);
        
        //对每个分割字符分类
        int character=classify(f);
        //cout << strCharacters[character] << " ";
        inputplate.chars.push_back(strCharacters[character]);
        inputplate.charsPos.push_back(segmentchar[i].pos);
        
    }
    
    
    return inputplate.str();
}
