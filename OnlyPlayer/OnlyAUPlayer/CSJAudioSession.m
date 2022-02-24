//
//  CSJAudioSession.m
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/9/8.
//

#import "CSJAudioSession.h"

#import  "AVAudioSession+RouteUtils.h"

const NSTimeInterval AUAudioSessionLatency_Background = 0.0929;
const NSTimeInterval AUAudioSessionLatency_Default    = 0.0232;
const NSTimeInterval AUAudioSessionLatency_LowLatency = 0.0058;

@implementation CSJAudioSession

+ (CSJAudioSession *)sharedInstance {
    static CSJAudioSession *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[CSJAudioSession alloc] init];
    });
    
    return instance;
}

- (id)init {
    if (self = [super init]) {
        
        _preferredSampleRate = _currentSampleRate = 44100.0;
        _audioSession = [AVAudioSession sharedInstance];
        return self;
    }
    
    return nil;
}

- (void)setCategory:(NSString *)category {
    _category = category;
    
    NSError *error = nil;
    if (![self.audioSession setPreferredSampleRate:self.preferredSampleRate error:&error]) {
        NSLog(@"Error when setting sample rate on audio session: %@", error.localizedDescription);
    }
    
    if (![self.audioSession setActive:_active error:&error]) {
        NSLog(@"Error when setting active state of audio session: %@", error.localizedDescription);
    }
    
    _currentSampleRate = [self.audioSession sampleRate];
}

- (void)setPreferredLatency:(NSTimeInterval)preferredLatency {
    _preferredLatency = preferredLatency;
    
    NSError *error = nil;
    if (![self.audioSession setPreferredIOBufferDuration:_preferredLatency error:&error]) {
        NSLog(@"Error when setting preferred I/O buffer duration");
    }
}

- (void)addRouteChangeListener {
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onNotificationAudioRouteChange:) name:AVAudioSessionRouteChangeNotification object:nil];
    
    [self adjustOnRouteChange];
}

#pragma mark - notification observer;
- (void)onNotificationAudioRouteChange:(NSNotification *)notify {
    [self adjustOnRouteChange];
}
 
- (void)adjustOnRouteChange {
    AVAudioSessionRouteDescription *currentRoute = [[AVAudioSession sharedInstance] currentRoute];
    if (currentRoute) {
        if ([[AVAudioSession sharedInstance] usingWiredMicrophone]) {
            
        } else {
            if (![[AVAudioSession sharedInstance] usingBlueTooth]) {
                [[AVAudioSession sharedInstance] overrideOutputAudioPort:AVAudioSessionPortOverrideSpeaker error:nil];
            }
        }
    }
}

@end
