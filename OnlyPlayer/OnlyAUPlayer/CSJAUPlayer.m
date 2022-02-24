//
//  CSJAUPlayer.m
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/9/8.
//

#import "CSJAUPlayer.h"

#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
#import "CSJAudioSession.h"

@implementation CSJAUPlayer {
    AUGraph     mPlayerGraph;
    AUNode      mPlayerNode;
    AUNode      mSplitterNode;
    AudioUnit   mSplitterUnit;
    AUNode      mAccMixerNode;
    AudioUnit   mAccMixerUnit;
    AUNode      mVocalMixerNode;
    AudioUnit   mVocalMixerUnit;
    AudioUnit   mPlayerUnit;
    AUNode      mPlayerIONode;
    AudioUnit   mPlayerIOUnit;
    NSURL       *mPlayPath;
}

#pragma mark - public functions;
- (id)initWithFilePath:(NSString *)path {
    if (self = [super init]) {
        [[CSJAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord];
        [[CSJAudioSession sharedInstance] setPreferredSampleRate:44100];
        [[CSJAudioSession sharedInstance] setActive:YES];
        [[CSJAudioSession sharedInstance] addRouteChangeListener];
        [self addAudioSessionInterruptedObserver];
        mPlayPath = [NSURL URLWithString:path];
        [self initializePlayGraph];
        
        return self;
    }
    
    return nil;
}

- (BOOL)play {
    OSStatus status = AUGraphStart(mPlayerGraph);
    CheckStatus(status, @"couldn't start AUGraph", YES);
    
    return YES;
}

- (void)stop {
    Boolean isRunning = false;
    OSStatus status = AUGraphIsRunning(mPlayerGraph, &isRunning);
    if (isRunning) {
        status = AUGraphStop(mPlayerGraph);
        CheckStatus(status, @"couldn't stop AUGraph", YES);
    }
}

#pragma mark - private functions;
// AudioSession 被打断的通知;
- (void)addAudioSessionInterruptedObserver {
    [self removeAudioSessionInterruptedObserver];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onNotificationAudioInterrupted:) name:AVAudioSessionInterruptionNotification object:[AVAudioSession sharedInstance]];
}

- (void)removeAudioSessionInterruptedObserver {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:AVAudioSessionInterruptionNotification object:[AVAudioSession sharedInstance]];
}

- (void)onNotificationAudioInterrupted:(NSNotification *)notify {
    AVAudioSessionInterruptionType interruptionType = [[[notify userInfo] objectForKey:AVAudioSessionInterruptionNotification] unsignedIntValue];
    
    switch (interruptionType) {
        case AVAudioSessionInterruptionTypeBegan:
            [self stop];
            break;
        case AVAudioSessionInterruptionTypeEnded:
            [self play];
            break;
        default:
            break;
    }
}

- (void)initializePlayGraph {
    OSStatus status = noErr;
    
    // 1、构造AUGraph;
    status = NewAUGraph(&mPlayerGraph);
    CheckStatus(status, @"Couldn't create a new AUGraph", YES);
    
    // 2-1、添加IONode;
    AudioComponentDescription ioDescription;
    bzero(&ioDescription, sizeof(ioDescription));
    ioDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
    // kAudioUnitType_Output可以作为AudioGraph或者AVAudioEngine的一部分独立使用，
    // 通过苹果提供的一系列output Unit直接与audio device连接;
    ioDescription.componentType = kAudioUnitType_Output;
    // kAudioUnitSubType_RemoteIO 接入音频系统，Bus 0 将音频输出，Bus 1 输入音频;
    ioDescription.componentSubType = kAudioUnitSubType_RemoteIO;
    status = AUGraphAddNode(mPlayerGraph, &ioDescription, &mPlayerIONode);
    CheckStatus(status, @"Couldn't add I/O node to AUGraph", YES);
    
    // 2-2：添加PlayerNode;
    AudioComponentDescription playerDescription;
    bzero(&playerDescription, sizeof(playerDescription));
    playerDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
    // kAudioUnitType_Generator 表示没有音频输入，只会产出音频，类似于kAudioUnitType_MusicDevice
    playerDescription.componentType = kAudioUnitType_Generator;
    // kAudioUnitSubType_AudioFilePlayer 播放音频文件，可以展示订制的UI;
    playerDescription.componentSubType = kAudioUnitSubType_AudioFilePlayer;
    status = AUGraphAddNode(mPlayerGraph, &playerDescription, &mPlayerNode);
    CheckStatus(status, @"Couldn't add Player node to AUGraph",  YES);
    
    // 2-3: 添加Splitter;
    AudioComponentDescription splitterDescription;
    bzero(&splitterDescription, sizeof(splitterDescription));
    splitterDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
    // kAudioUnitType_FormatConverter 将input的audio转换格式之后，由output输出;
    splitterDescription.componentType = kAudioUnitType_FormatConverter;
    // kAudioUnitSubType_Splitter 当前Unit是一个bus输入，但可以提供两个buses输出;
    splitterDescription.componentSubType = kAudioUnitSubType_Splitter;
    status = AUGraphAddNode(mPlayerGraph, &splitterDescription, &mSplitterNode);
    CheckStatus(status, @"Couldn't add Splitter node to AUGraph", YES);
    
    // 2-4: 添加两个mixer;
    AudioComponentDescription mixerDescription;
    bzero(&mixerDescription, sizeof(mixerDescription));
    mixerDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
    mixerDescription.componentType = kAudioUnitType_Mixer;
    mixerDescription.componentSubType = kAudioUnitSubType_MultiChannelMixer;
    status = AUGraphAddNode(mPlayerGraph, &mixerDescription, &mVocalMixerNode);
    CheckStatus(status, @"Couldn't add VocalMixer node to AUGraph", YES);
    status = AUGraphAddNode(mPlayerGraph, &mixerDescription, &mAccMixerNode);
    CheckStatus(status, @"Couldn't add AccMixer node to AUGraph", YES);
    
    // 3: 打开Graph，打开Graph之后才会实例化；每一个Node;
    status = AUGraphOpen(mPlayerGraph);
    CheckStatus(status, @"Couldn't open AUGraph", YES);
    
    // 4-1: 获取IONode的AudioUnit;
    status = AUGraphNodeInfo(mPlayerGraph, mPlayerIONode, NULL, &mPlayerIOUnit);
    CheckStatus(status, @"Couldn't retrieve node info for I/O node", YES);
    
    // 4-2: 获取PlayerNode的AudioUnit;
    status = AUGraphNodeInfo(mPlayerGraph, mPlayerNode, NULL, &mPlayerUnit);
    CheckStatus(status, @"Couldn't retrieve node info for Player node", YES);
    
    // 4-3: 获取SplitterNode的AudioUnit;
    status = AUGraphNodeInfo(mPlayerGraph, mSplitterNode, NULL, &mSplitterUnit);
    CheckStatus(status, @"Couldn't retrieve node info for Splitter node", YES);
    
    // 4-4: 获取VocalMixer的AudioUnit;
    status = AUGraphNodeInfo(mPlayerGraph, mVocalMixerNode, NULL, &mVocalMixerUnit);
    CheckStatus(status, @"Couldn't retrieve node info for VocalMixer node", YES);
    
    // 4-5: 获取AccMixer的AudioUnit;
    status = AUGraphNodeInfo(mPlayerGraph, mAccMixerNode, NULL, &mAccMixerUnit);
    CheckStatus(status, @"Couldn't retrieve node info for AccMixer node", YES);
    
    // 5-1: 给AudioUnit设置参数;
    AudioStreamBasicDescription stereoStreamFormat;
    UInt32 bytesPerSample = sizeof(Float32);
    bzero(&stereoStreamFormat, sizeof(stereoStreamFormat));
    stereoStreamFormat.mFormatID = kAudioFormatLinearPCM;
    stereoStreamFormat.mFormatFlags = kAudioFormatFlagsNativeFloatPacked | kAudioFormatFlagIsNonInterleaved;
    stereoStreamFormat.mBytesPerPacket = bytesPerSample;
    stereoStreamFormat.mFramesPerPacket = 1;
    stereoStreamFormat.mChannelsPerFrame = 2;   // 立体声;
    stereoStreamFormat.mBitsPerChannel = 8 * bytesPerSample;
    stereoStreamFormat.mSampleRate = 48000.0;
    status = AudioUnitSetProperty(mPlayerIOUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &stereoStreamFormat, sizeof(stereoStreamFormat));
    CheckStatus(status, @"set remote IO output element stream format!", YES);
    
    status = AudioUnitSetProperty(mPlayerUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &stereoStreamFormat, sizeof(stereoStreamFormat));
    CheckStatus(status, @"Couldn't set StreamFormat for Player Unit", YES);
    
    // 5-2: 配置Splitter属性;
    status = AudioUnitSetProperty(mSplitterUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &stereoStreamFormat, sizeof(stereoStreamFormat));
    CheckStatus(status, @"Couldn't set StreamFormat for Splitter Unit output element", YES);
    status = AudioUnitSetProperty(mSplitterUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &stereoStreamFormat, sizeof(stereoStreamFormat));
    CheckStatus(status, @"Couldn't set StreamFormat for Splitter Unit input element", YES);
    
    // 5-3: 配置VocalMixer属性;
    status = AudioUnitSetProperty(mVocalMixerUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &stereoStreamFormat, sizeof(stereoStreamFormat));
    CheckStatus(status, @"Couldn't set StreamFormat for AccMixer Unit output element", YES);
    status = AudioUnitSetProperty(mVocalMixerUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &stereoStreamFormat, sizeof(stereoStreamFormat));
    CheckStatus(status, @"Couldn't set StreamFormat for AccMixer Unit input element", YES);
    int mixerElementCount = 1;
    status = AudioUnitSetProperty(mVocalMixerUnit, kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0, &mixerElementCount, sizeof(mixerElementCount));
    CheckStatus(status, @"Couldn't set elementCount for VocalMixer Unit input element", YES);
    
    // 5-4: 配置AccMixer属性;
    status = AudioUnitSetProperty(mAccMixerUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &stereoStreamFormat, sizeof(stereoStreamFormat));
    CheckStatus(status, @"Couldn't set StreamFormat for AccMixer Unit output element", YES);
    status = AudioUnitSetProperty(mAccMixerUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &stereoStreamFormat, sizeof(stereoStreamFormat));
    CheckStatus(status, @"Couldn't set StreamFormat for AccMixer Unit input element", YES);
    mixerElementCount = 2;
    status = AudioUnitSetProperty(mAccMixerUnit, kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0, &mixerElementCount, sizeof(mixerElementCount));
    CheckStatus(status, @"Couldn't set elementCount for AccMixer Unit input element", YES);
    
    [self setInputSource:NO];
    
    // 6: 将Node连接起来;
    status = AUGraphConnectNodeInput(mPlayerGraph, mPlayerNode, 0, mSplitterNode, 0);
    CheckStatus(status, @"Player Node connect to IONode error!", YES);
    status = AUGraphConnectNodeInput(mPlayerGraph, mSplitterNode, 0, mVocalMixerNode, 0);
    CheckStatus(status, @"Splitter node connect to VocalMixer node error!", YES);
    status = AUGraphConnectNodeInput(mPlayerGraph, mSplitterNode, 1, mAccMixerNode, 0);
    CheckStatus(status, @"splitter node connect to accMixer node error!", YES);
    status = AUGraphConnectNodeInput(mPlayerGraph, mVocalMixerNode, 1, mAccMixerNode, 1);
    CheckStatus(status, @"VocalMixer node connect to AccMixer node error!", YES);
    status = AUGraphConnectNodeInput(mPlayerGraph, mAccMixerNode, 0, mPlayerIONode, 0);
    CheckStatus(status, @"AccMixer node connect to PlayerIO node error!", YES);
    
    // 7: 初始化Graph;
    status = AUGraphInitialize(mPlayerGraph);
    CheckStatus(status, @"Couldn't initialize mPlayerGraph", YES);
    
    // 8: 现实Graph的结构;
    CAShow(mPlayerGraph);
    
    // 9: Graph初始化之后才可以设置AudioPlayer的参数;
    [self setUpFilePlayer];
}

- (void)setInputSource:(BOOL)isAcc {
    OSStatus status;
    AudioUnitParameterValue value;
    
    // 获取VocalMixer Unit的音量;
    status = AudioUnitGetParameter(mVocalMixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 0, &value);
    CheckStatus(status, @"get VocalMixer Unit volume failed!", YES);
    NSLog(@"VocalMixer Unit volume: %lf", value);
  
    // 设置VocalMixer Unit的音量;
//    status = AudioUnitSetProperty(mVocalMixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 0, 1.0, 0);
//    CheckStatus(status,  @"set VocalMixer Unit volume failed!", YES);
    
    // 获取AccMixer Unit element 0 的音量;
    status = AudioUnitGetParameter(mAccMixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 0, &value);
    CheckStatus(status, @"get AccMixer Unit element 0 volume failed!", YES);
    NSLog(@"AccMixer Unit element 0 volume: %lf", value);
    
    // 获取AccMixer Uint element 1 的音量;
    status = AudioUnitGetParameter(mAccMixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 1, &value);
    CheckStatus(status, @"get AccMixer unit element 1 volume failed!", YES);
    NSLog(@"AccMixer Unit element 1 volume: %lf", value);
    
    if (isAcc) {
        status = AudioUnitSetParameter(mAccMixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 0, 0.1, 0);
        CheckStatus(status, @"set AccMixer Unit element 0 volume failed!", YES);
        
        status = AudioUnitSetParameter(mAccMixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 1, 1, 0);
        CheckStatus(status, @"set AccMixer Unit element 1 volume failed!", YES);
    } else {
        status = AudioUnitSetParameter(mAccMixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 0, 0, 0);
        CheckStatus(status, @"set AccMixer Unit element 0 volume failed!", YES);
        
        status = AudioUnitSetParameter(mAccMixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 1, 0, 0);
        CheckStatus(status, @"set AccMixer Unit element 1 volume failed!", YES);
    }
}

- (void)setUpFilePlayer {
    OSStatus status;
    AudioFileID musicFile;
    CFURLRef songURL = (__bridge CFURLRef)mPlayPath;
    
    // open the input audio file;
    status = AudioFileOpenURL(songURL, kAudioFileReadPermission, 0, &musicFile);
    CheckStatus(status, @"Open AudioFile... ", YES);
    
    // tell the file player unit to load the file we want to play;
    status = AudioUnitSetProperty(mPlayerUnit, kAudioUnitProperty_ScheduledFileIDs, kAudioUnitScope_Global, 0, &musicFile, sizeof(musicFile));
    CheckStatus(status, @"Tell Player Unit load which file.... ", YES);
    
    AudioStreamBasicDescription fileASBD;
    // get the audio data format from the file;
    UInt32 propSize = sizeof(fileASBD);
    status = AudioFileGetProperty(musicFile, kAudioFilePropertyDataFormat, &propSize, &fileASBD);
    CheckStatus(status, @"get the audio data format failed!", YES);
    
    UInt64 nPackets;
    propSize = sizeof(nPackets);
    status = AudioFileGetProperty(musicFile, kAudioFilePropertyAudioDataPacketCount, &propSize, &nPackets);
    CheckStatus(status, @"get the audio data packets failed!", YES);
    
    // tell the file player AU to play the entire file;
    ScheduledAudioFileRegion rgn;
    memset(&rgn.mTimeStamp, 0, sizeof(rgn.mTimeStamp)) ;
    rgn.mTimeStamp.mFlags = kAudioTimeStampSampleTimeValid;
    rgn.mTimeStamp.mSampleTime = 0;
    rgn.mCompletionProc = NULL;
    rgn.mCompletionProcUserData = NULL;
    rgn.mAudioFile = musicFile;
    rgn.mLoopCount = 0;
    rgn.mStartFrame = 0;
    rgn.mFramesToPlay = (UInt32)nPackets * fileASBD.mFramesPerPacket;
    status = AudioUnitSetProperty(mPlayerUnit, kAudioUnitProperty_ScheduledFileRegion, kAudioUnitScope_Global, 0, &rgn, sizeof(rgn));
    CheckStatus(status, @"set PlayerUnit region failed!", YES);
    
    // prime the file player AU with default values;
    UInt32 defaultVal = 0;
    status = AudioUnitSetProperty(mPlayerUnit, kAudioUnitProperty_ScheduledFilePrime, kAudioUnitScope_Global, 0, &defaultVal, sizeof(defaultVal));
    CheckStatus(status, @"set PlayerUnit file prime failed!", YES);
    
    // tell the file player AU when to start playing (-1 sample time means next render cycle);
    AudioTimeStamp startTime;
    memset(&startTime, 0, sizeof(startTime));
    startTime.mFlags = kAudioTimeStampSampleTimeValid;
    startTime.mSampleTime = -1;
    status = AudioUnitSetProperty(mPlayerUnit, kAudioUnitProperty_ScheduleStartTimeStamp, kAudioUnitScope_Global,  0, &startTime, sizeof(startTime));
    CheckStatus(status, @"set PlayerUnit start time failed!", YES);
}

static void CheckStatus(OSStatus status, NSString *message, BOOL fatal) {
    if (status == noErr) {
        return ;
    }
    
    char fourCC[16];
    *(UInt32 *)fourCC = CFSwapInt32HostToBig(status);
    fourCC[4] = '\0';
    
    // isprintf用来判断字符是否可以print，如果是控制字符，则返回false;
    if (isprint(fourCC[0]) && isprint(fourCC[1]) && isprint(fourCC[2]) && isprint(fourCC[3])) {
        NSLog(@"%@: %s", message, fourCC);
    } else {
        NSLog(@"%@: %d", message, (int)status);
    }
    
    if (fatal) {
        exit(-1);
    }
}
@end
