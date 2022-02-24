//
//  CSJAudioSession.h
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/9/8.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

extern const NSTimeInterval AUAudioSessionLatency_Background;
extern const NSTimeInterval AUAudioSessionLatency_Default;
extern const NSTimeInterval AUAudioSessionLatency_LowLatency;

NS_ASSUME_NONNULL_BEGIN

@interface CSJAudioSession : NSObject

@property (nonatomic, strong) AVAudioSession    *audioSession;
@property (nonatomic, assign) Float64           preferredSampleRate;
@property (nonatomic, assign, readonly) Float64 currentSampleRate;
@property (nonatomic, assign) NSTimeInterval    preferredLatency;
@property (nonatomic, assign) BOOL              active;
@property (nonatomic, strong) NSString          *category;

+ (CSJAudioSession *)sharedInstance;

- (void)addRouteChangeListener;

@end

NS_ASSUME_NONNULL_END
