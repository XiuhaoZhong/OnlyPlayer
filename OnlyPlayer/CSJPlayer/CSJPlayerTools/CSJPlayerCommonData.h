//
//  Header.h
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2022/3/9.
//

#ifndef Header_h
#define Header_h

// player status;
typedef enum {
    CSJPlayerStatus_None = -1,
    CSJPlayerStatus_Ready,
    CSJPlayerStatus_Started,
    CSJPlayerStatus_Pause,
    CSJPlayerStaus_Stop
} CSJPlayerStatus;

// decoder status;
typedef enum {
    CSJDecoderStatus_None = -1,
    CSJDecoderStatus_Decoding,
    CSJDecoderStatus_Puase,
    CSJDecoderStatus_Stop
} CSJDecoderStatus;



#endif /* Header_h */
