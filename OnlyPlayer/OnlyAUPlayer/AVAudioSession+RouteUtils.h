//
//  AVAudioSession+AVAudioSession_RouteUtils.h
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/9/8.
//

#import <AVFoundation/AVFoundation.h>

@interface AVAudioSession (RouteUtils)

- (BOOL)usingBlueTooth;

- (BOOL)usingWiredMicrophone;

- (BOOL)shouldShowEarphoneAlert;

@end

