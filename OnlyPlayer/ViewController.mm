//
//  ViewController.m
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/8/5.
//

#import "ViewController.h"

// FFmpeg Header File
#ifdef __cplusplus
extern "C" {
#endif
    
#include "libswscale/swscale.h"
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
#include "libavcodec/avcodec.h"

#ifdef __cplusplus
};
#endif

#include "CSJDecoder.h"
#include "CSJDecoderController.hpp"
#include "CommonUtils.h"
#include "CSJFFmpegVideoDecoder.hpp"
#include "OnlyAUPlayer/CSJAUPlayer.h"
#include <thread>
#include <iostream>

#import "Masonry/Masonry.h"

#import "OnlyGLView.h"

@interface ViewController () {
    CSJFFmpegVideoDecoder *m_pVideoDecoder;
}

@property (nonatomic, strong) OnlyGLView *onlyGLView;

@property (nonatomic, strong) UIButton   *showImageBtn;
@property (nonatomic, strong) UIButton   *decodeBtn;
@property (nonatomic, strong) UIButton   *videoDecodeBtn;
@property (nonatomic, strong) UIButton   *decodePauseBtn;
@property (nonatomic, strong) UIButton   *decodeResumeBtn;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    [self ffmpegInit];
    
    //[self setupUI];
    
    //[self startPlayWithAUPlayer];
}

- (void)ffmpegInit {
    const char *ffmpegConfig = avformat_configuration();
    std::cout << *ffmpegConfig << std::endl;
    
    char info[10000] = {0};
    sprintf(info, "%s\n", avcodec_configuration());
    NSString * info_us = [NSString stringWithFormat:@"%s", info];
    std::cout << "configurations: " << std::endl;
    NSLog(@"%@", info_us);
    
    char inf[40000] = {0};
    
    std::cout << "procotols: " << std::endl;
    struct URLProtocol *pup = NULL;
    struct URLProtocol **p_tmp = &pup;
    avio_enum_protocols((void **)p_tmp, 0);
    while ((*p_tmp) != NULL) {
        sprintf(inf, "%s[In ][%10s]\n", inf, avio_enum_protocols((void **)p_tmp, 0));
    }
    
    pup = NULL;
    avio_enum_protocols((void **)p_tmp, 1);
    while ((*p_tmp) != NULL) {
        sprintf(inf, "%s[Out][%10s]\n", inf, avio_enum_protocols((void **)p_tmp, 1));
    }
    
    sprintf(inf, "%s%s\n", inf, "Formats: ");
    
    struct AVInputFormat *input_format_iter = NULL;
    av_demuxer_iterate((void **)&input_format_iter);
    
    
    struct AVOutputFormat *output_format_iter = NULL;
    av_muxer_iterate((void **)&output_format_iter);
    
    input_format_iter = input_format_iter->next;
    while (input_format_iter != NULL) {
        sprintf(inf, "%s[InFormat: ]%10s\n", inf, input_format_iter->name);
        input_format_iter = input_format_iter->next;
    }
    
    while (output_format_iter != NULL) {
        sprintf(inf, "%s[OutFormat: ]%10s\n", inf, output_format_iter->name);
        output_format_iter = output_format_iter->next;
    }
    
    NSLog(@"%s", inf);
}

- (void)setupUI {
    
    [self.view addSubview:self.decodeBtn];
    [self.view addSubview:self.showImageBtn];
    [self.view addSubview:self.decodePauseBtn];
    [self.view addSubview:self.decodeResumeBtn];
    
    [self.decodeBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.mas_offset(200);
        make.centerX.mas_equalTo(0);
    }];
    
    [self.showImageBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.equalTo(self.decodeBtn.mas_bottom).offset(10);
        make.centerX.equalTo(self.view);
    }];
    
    NSString *pngFilePath = [CommonUtils bundlePath:@"1.png"];
    _onlyGLView = [[OnlyGLView alloc] initWithFrame:CGRectMake(100, 300, 210, 210) filePath:pngFilePath];
    _onlyGLView.contentMode = UIViewContentModeScaleToFill;
    [self.view addSubview:_onlyGLView];
//    [_onlyGLView mas_makeConstraints:^(MASConstraintMaker *make) {
//        make.top.equalTo(self.showImageBtn.mas_bottom).offset(10);
//        make.centerX.mas_offset(0);
//        make.size.mas_equalTo(CGSizeMake(100, 100));
//    }];
    
    [self.view addSubview:self.videoDecodeBtn];
    [self.videoDecodeBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.equalTo(_onlyGLView.mas_bottom).offset(5);
        make.centerX.equalTo(self.decodeBtn);
    }];
    
    [self.decodePauseBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.equalTo(_videoDecodeBtn.mas_bottom).offset(5);
        make.centerX.equalTo(self.videoDecodeBtn);
    }];
    
    [self.decodeResumeBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.equalTo(_decodePauseBtn.mas_bottom).offset(5);
        make.centerX.equalTo(self.videoDecodeBtn);
    }];
}

- (void)btnClicked:(id)sender {
    
    if (sender == _decodeBtn) {
        [self decodeAudio];
    } else if (sender == _showImageBtn) {
        [self showGLImage];
    } else if (sender == _videoDecodeBtn) {
        [self decodeVideo];
    } else if (sender == _decodePauseBtn) {
        m_pVideoDecoder->stop();
    } else if (sender == _decodeResumeBtn) {
        m_pVideoDecoder->resume();
    }
}

- (void)decodeAudio {
    NSLog(@"decode test...");
    
    const char *mp3FilePath = [[CommonUtils bundlePath:@"131.aac"] cStringUsingEncoding:NSUTF8StringEncoding];
    const char *pcmFilePath = [[CommonUtils documentPath:@"131.pcm"] cStringUsingEncoding:NSUTF8StringEncoding];
    
//    const char *mp3FilePath = [[CommonUtils bundlePath:@"Twins.mp3"] cStringUsingEncoding:NSUTF8StringEncoding];
//    const char *pcmFilePath = [[CommonUtils documentPath:@"twins.pcm"] cStringUsingEncoding:NSUTF8StringEncoding];
    
    CSJDecoderController *decoderController = new CSJDecoderController();
    int ret = decoderController->init(mp3FilePath, pcmFilePath);
    if (ret != 0) {
        NSLog(@"CSJDecoderController init failed!");
        return ;
    }
    decoderController->Decode();
    decoderController->Destroy();
    delete decoderController;
    NSLog(@"decode test over!");
}

- (void)showGLImage {
//    NSString *pngFilePath = [CommonUtils bundlePath:@"1.png"];
//    _onlyGLView = [[OnlyGLView alloc] initWithFrame:CGRectZero filePath:pngFilePath];
//    [self.view addSubview:_onlyGLView];
//    [_onlyGLView mas_makeConstraints:^(MASConstraintMaker *make) {
//        make.top.equalTo(self.showImageBtn.mas_bottom).offset(10);
//        make.centerX.mas_offset(0);
//        make.size.mas_equalTo(CGSizeMake(100, 100));
//    }];
    
    [_onlyGLView render];
}

- (void)decodeVideo {
    CSJFFmpegVideoDecoder *videoDecoder = new CSJFFmpegVideoDecoder();
    NSString *pngFilePath = [CommonUtils bundlePath:@"zl2-sm.mp4"];
    
    //NSString *pngFilePath = [CommonUtils bundlePath:@"dzd4-mp3.mp3"];
    
    int res = videoDecoder->openFile([pngFilePath UTF8String]);
    if (res < 0) {
        delete videoDecoder;
        return ;
    }
    
    //videoDecoder->decodeFrame(0);
    videoDecoder->start();
    
    m_pVideoDecoder = videoDecoder;
}

- (void)startPlayWithAUPlayer {
    CSJAUPlayer *auPlayer = [[CSJAUPlayer alloc] initWithFilePath:[CommonUtils bundlePath:@"131.aac"]];
    if (!auPlayer) {
        return ;
    }
    
    [auPlayer play];
}

#pragma mark - getters;
- (UIButton *)showImageBtn {
    if (!_showImageBtn) {
        _showImageBtn = [UIButton new];
        [_showImageBtn setTitle:@"render Image" forState:UIControlStateNormal];
        [_showImageBtn setTitleColor:[UIColor redColor] forState:UIControlStateNormal];
        [_showImageBtn setBackgroundColor:[UIColor greenColor]];
        [_showImageBtn addTarget:self action:@selector(btnClicked:) forControlEvents:UIControlEventTouchUpInside];
    }
    
    return _showImageBtn;
}

- (UIButton *)decodeBtn {
    if (!_decodeBtn) {
        _decodeBtn = [UIButton new];
        [_decodeBtn setTitle:@"Decode" forState:UIControlStateNormal];
        [_decodeBtn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        [_decodeBtn setBackgroundColor:[UIColor greenColor]];
        _decodeBtn.layer.cornerRadius = 5;
        _decodeBtn.layer.masksToBounds = YES;
        [_decodeBtn addTarget:self action:@selector(btnClicked:) forControlEvents:UIControlEventTouchUpInside];
    }
    
    return _decodeBtn;
}

- (UIButton *)videoDecodeBtn {
    if (!_videoDecodeBtn) {
        _videoDecodeBtn = [UIButton new];
        [_videoDecodeBtn setTitle:@"video decode" forState:UIControlStateNormal];
        [_videoDecodeBtn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        [_videoDecodeBtn setBackgroundColor:[UIColor greenColor]];
        _videoDecodeBtn.layer.cornerRadius = 5;
        _videoDecodeBtn.layer.masksToBounds = YES;
        [_videoDecodeBtn addTarget:self action:@selector(btnClicked:) forControlEvents:UIControlEventTouchUpInside];
    }
    
    return _videoDecodeBtn;
}

- (UIButton *)decodePauseBtn {
    if (!_decodePauseBtn) {
        _decodePauseBtn = [UIButton new];
        [_decodePauseBtn setTitle:@"pause" forState:UIControlStateNormal];
        [_decodePauseBtn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        [_decodePauseBtn setBackgroundColor:[UIColor greenColor]];
        _decodePauseBtn.layer.cornerRadius = 5;
        _decodePauseBtn.layer.masksToBounds = YES;
        [_decodePauseBtn addTarget:self action:@selector(btnClicked:) forControlEvents:UIControlEventTouchUpInside];
    }
    
    return _decodePauseBtn;
}

- (UIButton *)decodeResumeBtn {
    if (!_decodeResumeBtn) {
        _decodeResumeBtn = [UIButton new];
        [_decodeResumeBtn setTitle:@"resume" forState:UIControlStateNormal];
        [_decodeResumeBtn setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        [_decodeResumeBtn setBackgroundColor:[UIColor greenColor]];
        _decodeResumeBtn.layer.cornerRadius = 5;
        _decodeResumeBtn.layer.masksToBounds = YES;
        [_decodeResumeBtn addTarget:self action:@selector(btnClicked:) forControlEvents:UIControlEventTouchUpInside];
    }
    
    return _decodeResumeBtn;
}


@end
