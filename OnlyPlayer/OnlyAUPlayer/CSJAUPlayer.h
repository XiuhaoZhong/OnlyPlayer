//
//  CSJAUPlayer.h
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/9/8.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface CSJAUPlayer : NSObject

- (id)initWithFilePath:(NSString *)path;

- (BOOL)play;

- (void)stop;

- (void)setInputSource:(BOOL)isAcc;

@end

NS_ASSUME_NONNULL_END
