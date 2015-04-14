//
//  ViewController.m
//  NPR1_1
//
//  Created by Jacob on 15/2/5.
//  Copyright (c) 2015年 Jacob. All rights reserved.
//

#import "ViewController.h"


//Mat格式转化到UIImage格式
static UIImage* MatToUIImage(const cv::Mat& image) {
    
    NSData *data = [NSData dataWithBytes:image.data length:image.elemSize()*image.total()];
    
    CGColorSpaceRef colorSpace;
    
    if (image.elemSize() == 1) {
        colorSpace = CGColorSpaceCreateDeviceGray();
    } else {
        colorSpace = CGColorSpaceCreateDeviceRGB();
    }
    
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)data);
    
    // Creating CGImage from cv::Mat
    CGImageRef imageRef = CGImageCreate(image.cols,                                 //width
                                        image.rows,                                 //height
                                        8,                                          //bits per component
                                        8 * image.elemSize(),                       //bits per pixel
                                        image.step.p[0],                            //bytesPerRow
                                        colorSpace,                                 //colorspace
                                        kCGImageAlphaNone|kCGBitmapByteOrderDefault,// bitmap info
                                        provider,                                   //CGDataProviderRef
                                        NULL,                                       //decode
                                        false,                                      //should interpolate
                                        kCGRenderingIntentDefault                   //intent
                                        );
    
    
    // Getting UIImage from CGImage
    UIImage *finalImage = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    
    return finalImage;
}

//UIImage格式转化到Mat格式
static void UIImageToMat(const UIImage* image, cv::Mat& m, bool alphaExist = false) {
    CGColorSpaceRef colorSpace = CGImageGetColorSpace(image.CGImage);
    CGFloat cols = image.size.width, rows = image.size.height;
    CGContextRef contextRef;
    CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedLast;
    if (CGColorSpaceGetModel(colorSpace) == 0)
    {
        m.create(rows, cols, CV_8UC1); // 8 bits per component, 1 channel
        bitmapInfo = kCGImageAlphaNone;
        if (!alphaExist)
            bitmapInfo = kCGImageAlphaNone;
        contextRef = CGBitmapContextCreate(m.data, m.cols, m.rows, 8,
                                           m.step[0], colorSpace, bitmapInfo);
    }
    else
    {
        m.create(rows, cols, CV_8UC4); // 8 bits per component, 4 channels
        if (!alphaExist)
            bitmapInfo = kCGImageAlphaNoneSkipLast | kCGBitmapByteOrderDefault;
        contextRef = CGBitmapContextCreate(m.data, m.cols, m.rows, 8,
                                           m.step[0], colorSpace, bitmapInfo);
    }
    CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows), image.CGImage);
    CGContextRelease(contextRef);
    CGColorSpaceRelease(colorSpace);
}



//保存实验数据1
void saveData1(Mat ScreenShot,int datanum1,int dataNum2) {
    UIImage* OutputImage = MatToUIImage(ScreenShot);
    NSString *name = [NSString stringWithFormat:@"/Users/setsufumimoto/Desktop/data/temporaryData/%d_%d.png",datanum1,dataNum2];
    NSFileManager * fileManager = [NSFileManager defaultManager];
    NSData * data = UIImageJPEGRepresentation(OutputImage, 1);
    BOOL bo = [fileManager createFileAtPath:name contents:data attributes:nil];
    //验证是否成功
    if (bo) {
        NSLog(@"yes");
    }else{
        NSLog(@"no");
    }
}

//保存实验数据2
void saveData2(Mat ScreenShot, int datanum1) {
    UIImage* OutputImage = MatToUIImage(ScreenShot);
    NSString *name = [NSString stringWithFormat:@"/Users/setsufumimoto/Desktop/data/temporaryData/%d.png",datanum1];
    NSFileManager * fileManager = [NSFileManager defaultManager];
    NSData * data = UIImageJPEGRepresentation(OutputImage, 1);
    BOOL bo = [fileManager createFileAtPath:name contents:data attributes:nil];
    //验证是否成功
    if (bo) {
        NSLog(@"yes");
    }else{
        NSLog(@"no");
    }
}




@interface ViewController ()

@end

@implementation ViewController
{
    UIImageView * _imageView;//图片
    UITextField * _text;//输出信息
    UIImage * _compareImage;//中间比较图片变量
    BOOL displayingPrimary;//翻转变量
    UILabel * _zhengLable;//按钮正面View
    UILabel * _fanLable;//按钮反面View
}

/*
- (void)viewDidLoad {
    [super viewDidLoad];
 
    //UIImage* image = [UIImage imageNamed:@"89.JPG"];
    UIImage* image = [UIImage imageNamed:@"106.png"];
    
    UIImageToMat(image, cvImage);
    
    string licensePlate;
    if (!cvImage.empty()){
        
        NSString *dataPath0 = [[NSBundle mainBundle]pathForResource:@"SVM" ofType:@"xml"];
        const char *dataPath0C = [dataPath0 UTF8String];
        DetectPlate newplate(dataPath0C);
        vector<Plate> plates = newplate.Detect(cvImage);
        NSString *dataPath1 = [[NSBundle mainBundle]pathForResource:@"OCR" ofType:@"xml"];
        const char *dataPath1C = [dataPath1 UTF8String];
        OCR ocr(dataPath1C);
        for(int i=0; i< plates.size(); i++) {
            Plate plate = plates[i];
            string temp = ocr.segment(plate);
            if (temp.size() >= 7) {
                licensePlate = temp;
                break;
            }
        }
        cout << licensePlate << "\n";
        
    }
 
    // Do any additional setup after loading the view, typically from a nib.

}
*/

- (void)viewDidLoad {
    [super viewDidLoad];
    if([self respondsToSelector:@selector(setAutomaticallyAdjustsScrollViewInsets:)]){
        self.automaticallyAdjustsScrollViewInsets = NO;
    }
    self.view.backgroundColor = [UIColor colorWithRed:35/255.0 green:67/255.0 blue:84/255.0 alpha:1];
    displayingPrimary = YES;
    [self creatMainView];
    // Do any additional setup after loading the view, typically from a nib.
    
    
    }

#pragma mark - 主视图
- (void)creatMainView
{
    //滑动视图
    UIScrollView * imageScrollView = [[UIScrollView alloc]initWithFrame:CGRectMake(0,20,[UIScreen mainScreen].bounds.size.width,[UIScreen mainScreen].bounds.size.height-200)];
    imageScrollView.contentSize = CGSizeMake([UIScreen mainScreen].bounds.size.width,[UIScreen mainScreen].bounds.size.height-200);
    imageScrollView.delegate = self;
    imageScrollView.maximumZoomScale = 3;
    imageScrollView.bounces = NO;
    imageScrollView.bouncesZoom = NO;
    imageScrollView.showsHorizontalScrollIndicator = NO;
    imageScrollView.showsVerticalScrollIndicator = NO;
    
    //输出TEXT
    _text = [[UITextField alloc]initWithFrame:CGRectMake(0,[UIScreen mainScreen].bounds.size.height-180,[UIScreen mainScreen].bounds.size.width,60)];
    _text.placeholder = @"这里是结果";
    _text.font = [UIFont systemFontOfSize:25];
    _text.textColor = [UIColor colorWithRed:255/255.0 green:255/255.0 blue:255/255.0 alpha:1];
    _text.textAlignment = NSTextAlignmentCenter;
    _text.inputView = [[UIView alloc]init];
    _text.inputView.hidden = YES;
    _text.delegate =self;
    
    //显示图片的View
    _imageView = [[UIImageView alloc]initWithFrame:CGRectMake(0, 0, [UIScreen mainScreen].bounds.size.width, [UIScreen mainScreen].bounds.size.height-200)];
    _imageView.backgroundColor = [UIColor clearColor];
    _imageView.contentMode = UIViewContentModeScaleAspectFit;
    _imageView.userInteractionEnabled = YES;
    
    //按钮双面效果View
    _zhengLable = [[UILabel alloc]initWithFrame:CGRectMake(0, 0, 100, 100)];
    _zhengLable.backgroundColor = [UIColor colorWithRed:74/255.0 green:174/255.0 blue:146/255.0 alpha:1];
    _fanLable = [[UILabel alloc]initWithFrame:CGRectMake(0, 0, 100, 100)];
    _fanLable.backgroundColor =[UIColor colorWithRed:216/255.0 green:32/255.0 blue:101/255.0 alpha:1];
    _zhengLable.text = @"选择照片";
    _fanLable.text = @"检测";
    _zhengLable.textAlignment = NSTextAlignmentCenter;
    _fanLable.textAlignment = NSTextAlignmentCenter;
    _zhengLable.textColor = [UIColor whiteColor];
    _fanLable.textColor =[UIColor whiteColor];
    _zhengLable.userInteractionEnabled =NO;
    _fanLable.userInteractionEnabled = NO;
    _zhengLable.layer.cornerRadius = 50;
    _zhengLable.layer.masksToBounds = YES;
    _fanLable.layer.cornerRadius = 50;
    _fanLable.layer.masksToBounds = YES;
    
    //选择照片按钮
    UIButton * button = [[UIButton alloc]initWithFrame:CGRectMake([UIScreen mainScreen].bounds.size.width/2-50, [UIScreen mainScreen].bounds.size.height-110, 100, 100)];
    button.backgroundColor = [UIColor clearColor];
    button.tag = 1000;
    [button addTarget:self action:@selector(change) forControlEvents:UIControlEventTouchUpInside];
    
    //视图添加
    [self.view addSubview:button];
    [button addSubview:_fanLable];
    [button addSubview:_zhengLable];
    [imageScrollView addSubview:_imageView];
    [self.view addSubview:imageScrollView];
    [self.view addSubview:_text];
}

#pragma mark - 按钮点击事件
- (void)change
{
    //按钮逻辑判断
    if (_imageView.image == nil) {
        UIActionSheet * actionSheet = [[UIActionSheet alloc]initWithTitle:@"选择照片" delegate:self cancelButtonTitle:@"取消" destructiveButtonTitle:nil otherButtonTitles:@"选择相册",@"照相机" ,nil];
        [actionSheet showInView:self.view];
    }else{
        if ([_compareImage isEqual:_imageView.image]) {
            UIActionSheet * actionSheet = [[UIActionSheet alloc]initWithTitle:@"选择照片" delegate:self cancelButtonTitle:@"取消" destructiveButtonTitle:nil otherButtonTitles:@"选择相册",@"照相机" ,nil];
            [actionSheet showInView:self.view];
        }else{
            //这里加入检测算法，_imageView.image为待检测的image对象
            
            //UIImage* image = [UIImage imageNamed:@"89.JPG"];
            //UIImage* image = [UIImage imageNamed:@"75.png"];
            
            UIImageToMat(_imageView.image, cvImage);
            
            string licensePlate;
            string relicense;
            if (!cvImage.empty()){
                
                NSString *dataPath0 = [[NSBundle mainBundle]pathForResource:@"SVM" ofType:@"xml"];
                const char *dataPath0C = [dataPath0 UTF8String];
                DetectPlate newplate(dataPath0C);
                vector<Plate> plates = newplate.Detect(cvImage);
                NSString *dataPath1 = [[NSBundle mainBundle]pathForResource:@"OCR" ofType:@"xml"];
                const char *dataPath1C = [dataPath1 UTF8String];
                OCR ocr(dataPath1C);
                //OCR ocr("/Users/setsufumimoto/Documents/develope/DesignForGratuate/NPR1_1/OCR.xml");
                for(int i=0; i< plates.size(); i++) {
                    Plate plate = plates[i];
                    string temp = ocr.segment(plate);
                    if (temp.size() >= 7) {
                        licensePlate = temp;
                        break;
                    }
                }
                if (licensePlate.length() == 0) {
                    relicense = "请在视图中央放大车牌重新检测！";
                }
                for(int i=0; i< licensePlate.length(); i++) {
                    if (i<=2) {
                        if (licensePlate[i] == '0') {
                            relicense.append(1, 'O');
                        }
                        else if (licensePlate[i] == '1') {
                            relicense.append(1, 'I');
                        }
                        else if (licensePlate[i] == '8') {
                            relicense.append(1, 'B');
                        }
                        else if (licensePlate[i] == '6') {
                            relicense.append(1, 'E');
                        }
                        else if (licensePlate[i] == '4') {
                            relicense.append(1, 'A');
                        }
                        else relicense.append(1,licensePlate[i]);
                    }
                    else if (i>2) {
                        if (licensePlate[i] == 'O') {
                            relicense.append(1, '0');
                        }
                        else if (licensePlate[i] == 'I') {
                            relicense.append(1, '1');
                        }
                        else if (licensePlate[i] == 'B' || licensePlate[i] == 'P') {
                            relicense.append(1, '8');
                        }
                        else if (licensePlate[i] == 'E') {
                            relicense.append(1, '6');
                        }
                        else if (licensePlate[i] == 'A') {
                            relicense.append(1, '4');
                        }
                        else relicense.append(1,licensePlate[i]);
                    }
                    
                }
                
                cout << licensePlate << "\n";
                cout << relicense << "\n";
               
            }
            NSString *licensePlateresult  = [NSString stringWithUTF8String:relicense.c_str()];
            _text.text = licensePlateresult;
            _compareImage = _imageView.image;
            [self overturnButton];
        }
    }
}

#pragma mark -- 按钮翻转效果
- (void)overturnButton
{
    //按钮翻转效果
    [UIView transitionFromView:(displayingPrimary ? _zhengLable : _fanLable)
                        toView:(displayingPrimary ? _fanLable : _zhengLable)
                      duration: 0.5
                       options: UIViewAnimationOptionTransitionFlipFromLeft+UIViewAnimationOptionCurveEaseInOut
                    completion:^(BOOL finished) {
                        displayingPrimary=!displayingPrimary;
                    }
     ];
}

#pragma mark - UIActionSheet delegate
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (buttonIndex == 0) {
        [self localPhoto];
    }else if(buttonIndex == 1){
        [self takePhoto];
    }
}
#pragma mark - 选择本地相册
- (void)localPhoto
{
    UIImagePickerController *picker = [[UIImagePickerController alloc]init];
    picker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
    picker.allowsEditing = YES;
    picker.delegate = self;
    //    [self overturnButton];
    [self presentViewController:picker animated:YES completion:nil];
}
#pragma mark - 照相机
- (void)takePhoto
{
    UIImagePickerControllerSourceType sourceType = UIImagePickerControllerSourceTypeCamera;
    if ([UIImagePickerController isSourceTypeAvailable: UIImagePickerControllerSourceTypeCamera])
    {
        UIImagePickerController *picker = [[UIImagePickerController alloc] init];
        picker.delegate = self;
        picker.allowsEditing = YES;
        picker.sourceType = sourceType;
        [self presentViewController:picker animated:YES completion:nil];
        //        [self overturnButton];
    }else
    {
        UIAlertView * alert = [[UIAlertView alloc]initWithTitle:@"提示" message:@"模拟器没有该功能" delegate:nil cancelButtonTitle:@"好" otherButtonTitles: nil];
        [alert show];
    }
}

#pragma mark - UIImagePickerController delegate
- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info
{
    //    for (NSString * str in info) {
    //        NSLog(@"%@",str);
    //    }
    UIImage* image = [info objectForKey:@"UIImagePickerControllerEditedImage"];
    _imageView.image = image;
    [picker dismissViewControllerAnimated:YES completion:nil];
    [self overturnButton];
}
- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker
{
    [picker dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark - UIScrollView delegate
- (UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView
{
    return  _imageView;
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end


