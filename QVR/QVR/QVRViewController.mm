//
//  QVRViewController.m
//  QVR
//
//  Created by 谌启亮 on 2017/3/29.
//  Copyright © 2017年 Tencent. All rights reserved.
//

#import "QVRViewController.h"
#import "Engine.hpp"
#import <CoreMotion/CoreMotion.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "QVR-Swift.h"

using namespace QVR;

@interface QVRViewController () <QVRAdjustManagerDelegate>
{
    Engine *vrEngine;
    UIView *_controlView;
    UISlider *_degreesSlider;
    UISlider *_p1Slider;
    UISlider *_p2Slider;
    UILabel *_degressLabel;
    UILabel *_p1Label;
    UILabel *_p2Label;
}
@property (strong, nonatomic) CMMotionManager *motionManager;
@property (strong, nonatomic) AVPlayer *player;
@property (strong, nonatomic) AVPlayerItem *playerItem;
@property (strong, nonatomic) AVPlayerItemVideoOutput *videoOutput;
@property (strong, nonatomic) QVRAdjustManager *adjustManager;
@end

@implementation QVRViewController

- (instancetype)initWithURL:(NSURL *)url
{
    self = [super initWithNibName:nil bundle:nil];
    if (self) {
        NSDictionary *pixelBuffAttributes = @{(id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)};
        _videoOutput = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:pixelBuffAttributes];
        
        _playerItem = [AVPlayerItem playerItemWithURL:url];
        [_playerItem addOutput:_videoOutput];
        
        _player = [AVPlayer playerWithPlayerItem:_playerItem];
        [_player addObserver:self forKeyPath:@"status" options:0 context:nil];
        [_player play];
        
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(playerItemDidPlayToEndTime:)
                                                     name:AVPlayerItemDidPlayToEndTimeNotification
                                                   object:nil];
        _adjustManager = [QVRAdjustManager new];
        _adjustManager.delegate = self;
    }
    return self;
}

- (void)playerItemDidPlayToEndTime:(AVPlayerItem *)item{
    [_player seekToTime:kCMTimeZero];
    [_player play];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    GLKView *view = (GLKView *)self.view;
    EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    view.context = context;
    view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    [view bindDrawable];
    
    self.preferredFramesPerSecond = 60;
    
    [EAGLContext setCurrentContext:context];
    
    glClearColor(0, 0, 0, 0);

    self.motionManager = [[CMMotionManager alloc] init];
    self.motionManager.deviceMotionUpdateInterval = 1.0 / 60.0;
    self.motionManager.gyroUpdateInterval = 1.0f / 60;
    self.motionManager.showsDeviceMovementDisplay = YES;
    
    [self.motionManager startDeviceMotionUpdatesUsingReferenceFrame:CMAttitudeReferenceFrameXArbitraryCorrectedZVertical];

    NSString *vshMesh = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"ShaderMesh" ofType:@"vsh"] encoding:NSUTF8StringEncoding error:NULL];
    NSString *fshMesh = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"ShaderMesh" ofType:@"fsh"] encoding:NSUTF8StringEncoding error:NULL];

    vrEngine = new Engine();
    
    vrEngine->meshFsh = [fshMesh UTF8String];
    vrEngine->meshVsh = [vshMesh UTF8String];
    vrEngine->setVideoType(VideoType3D360UpDown);
    vrEngine->setPlayMode(VideoPlayModeHeadset);
    vrEngine->setViewSize(view.drawableWidth, view.drawableHeight);
    vrEngine->setViewDistortions(-0.4, -1.25, 0, 0);

    NSString *vsh = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"vsh"] encoding:NSUTF8StringEncoding error:NULL];
    NSString *fsh = [NSString stringWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"fsh"] encoding:NSUTF8StringEncoding error:NULL];
    
    vrEngine->begin([vsh UTF8String], [fsh UTF8String]);
    
    _controlView = [[UIView alloc] initWithFrame:self.view.bounds];
    _controlView.backgroundColor = [UIColor clearColor];
    [self.view addSubview:_controlView];

    _degreesSlider = [[UISlider alloc] initWithFrame:CGRectMake(50, 40, self.view.bounds.size.width-100, 32)];
    _degreesSlider.minimumValue = 60;
    _degreesSlider.maximumValue = 179;
    _degreesSlider.value = 100;
    [_degreesSlider addTarget:self action:@selector(degreesChanged:) forControlEvents:UIControlEventValueChanged];
    [_controlView addSubview:_degreesSlider];
    vrEngine->setViewDegrees(100);

    _degressLabel = [[UILabel alloc] initWithFrame:CGRectMake(self.view.bounds.size.width-45, 40, 60, 32)];
    _degressLabel.text = [NSString stringWithFormat:@"%.2f", _degreesSlider.value];
    _degressLabel.textColor = [UIColor whiteColor];
    [_controlView addSubview:_degressLabel];
    
    UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(0, 40, 45, 32)];
    label.text = @"视角";
    label.textAlignment = NSTextAlignmentRight;
    label.textColor = [UIColor whiteColor];
    [_controlView addSubview:label];

    _p1Slider = [[UISlider alloc] initWithFrame:CGRectMake(50, self.view.bounds.size.height-80, self.view.bounds.size.width-100, 32)];
    _p1Slider.minimumValue = -5;
    _p1Slider.maximumValue = 5;
    _p1Slider.value = -0.4;
    [_p1Slider addTarget:self action:@selector(pChanged:) forControlEvents:UIControlEventValueChanged];
    [_controlView addSubview:_p1Slider];

    _p1Label = [[UILabel alloc] initWithFrame:CGRectMake(self.view.bounds.size.width-45, self.view.bounds.size.height-80, 60, 32)];
    _p1Label.text = [NSString stringWithFormat:@"%.2f", _p1Slider.value];
    _p1Label.textColor = [UIColor whiteColor];
    [_controlView addSubview:_p1Label];
    
    label = [[UILabel alloc] initWithFrame:CGRectMake(0, self.view.bounds.size.height-80, 45, 32)];
    label.text = @"p1";
    label.textAlignment = NSTextAlignmentRight;
    label.textColor = [UIColor whiteColor];
    [_controlView addSubview:label];

    _p2Slider = [[UISlider alloc] initWithFrame:CGRectMake(50, self.view.bounds.size.height-40, self.view.bounds.size.width-100, 32)];
    _p2Slider.minimumValue = -5;
    _p2Slider.maximumValue = 5;
    _p2Slider.value = -1.25;
    [_p2Slider addTarget:self action:@selector(pChanged:) forControlEvents:UIControlEventValueChanged];
    [_controlView addSubview:_p2Slider];
    
    _p2Label = [[UILabel alloc] initWithFrame:CGRectMake(self.view.bounds.size.width-45, self.view.bounds.size.height-40, 60, 32)];
    _p2Label.text = [NSString stringWithFormat:@"%.2f", _p2Slider.value];
    _p2Label.textColor = [UIColor whiteColor];
    [_controlView addSubview:_p2Label];
    
    label = [[UILabel alloc] initWithFrame:CGRectMake(0, self.view.bounds.size.height-40, 45, 32)];
    label.text = @"p2";
    label.textAlignment = NSTextAlignmentRight;
    label.textColor = [UIColor whiteColor];
    [_controlView addSubview:label];
    
    _controlView.hidden = YES;

    
    UITapGestureRecognizer *tap3 = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tap3:)];
    tap3.numberOfTapsRequired = 3;
    [self.view addGestureRecognizer:tap3];

}

- (void)degreesChanged:(id)sender{
    vrEngine->setViewDegrees(_degreesSlider.value);
    _degressLabel.text = [NSString stringWithFormat:@"%.2f", _degreesSlider.value];
}

- (void)pChanged:(id)sender{
    vrEngine->setViewDistortions(_p1Slider.value, _p2Slider.value, 0, 0);
    _p1Label.text = [NSString stringWithFormat:@"%.2f", _p1Slider.value];
    _p2Label.text = [NSString stringWithFormat:@"%.2f", _p2Slider.value];
}


- (void)tap3:(UIGestureRecognizer *)tap3{
    if (tap3.state == UIGestureRecognizerStateEnded) {
        _controlView.hidden = !_controlView.hidden;
    }
}

- (BOOL)prefersStatusBarHidden{
    return NO;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context{
    if (object == _player) {
        if ([keyPath isEqualToString:@"status"]) {
            AVPlayerStatus status = _player.status;
            NSLog(@"Player Status: %d", (int)status);
            if (status == AVPlayerStatusFailed) {
                NSLog(@"Player Fail: %@", _player.error);
            }
        }
    }
}

- (void)viewAngleChange:(NSString * _Nonnull)viewAngle{
    _degreesSlider.value = viewAngle.floatValue;
    [self degreesChanged:_degreesSlider];
}

- (void)k1Change:(NSString * _Nonnull)k1 {
    _p1Slider.value = k1.floatValue;
    [self pChanged:_p1Slider];
}

- (void)k2Change:(NSString * _Nonnull)k2 {
    _p2Slider.value = k2.floatValue;
    [self pChanged:_p2Slider];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    CMDeviceMotion *deviceMotion = self.motionManager.deviceMotion;
    if (deviceMotion != nil) {
        CMAttitude *attitude = deviceMotion.attitude;
        
        float cRoll = -attitude.roll; // Up/Down landscape
        float cYaw = attitude.yaw;  // Left/ Right landscape
        float cPitch = attitude.pitch; // Depth landscape
        
        UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
        if (orientation == UIDeviceOrientationLandscapeRight ){
            cPitch = cPitch*-1; // correct depth when in landscape right
        }
        vrEngine->updateRotation(cRoll, cPitch, cYaw);
    }
    else{
        vrEngine->updateRotation(0, 0, 0);
    }
    
    CMTime currentTime = [_playerItem currentTime];
    if ([self.videoOutput hasNewPixelBufferForItemTime:currentTime]) {
        CVPixelBufferRef pixelBuffer = [self.videoOutput copyPixelBufferForItemTime:currentTime itemTimeForDisplay:nil];
        if (pixelBuffer) {
            CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
            int width = (int)CVPixelBufferGetWidth(pixelBuffer);
            int height = (int)CVPixelBufferGetHeight(pixelBuffer);
            vrEngine->update(PixelFormatBGRA, (unsigned char *)CVPixelBufferGetBaseAddress(pixelBuffer), width, height);
            CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
        }
        CVPixelBufferRelease(pixelBuffer);
    }
    
    vrEngine->render();
    glFlush();
}

- (void)dealloc{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
