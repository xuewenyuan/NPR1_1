//
//  ViewController.h
//  NPR1_1
//
//  Created by Jacob on 15/2/5.
//  Copyright (c) 2015年 Jacob. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <opencv2/imgproc/imgproc.hpp>

#import "Plate.h"
#import "DetectPlate.h"
#import "OCR.h"

@interface ViewController : UIViewController <UIActionSheetDelegate,UIImagePickerControllerDelegate,UIScrollViewDelegate,UITextFieldDelegate,UINavigationControllerDelegate>{
    cv::Mat cvImage;
}



@end

