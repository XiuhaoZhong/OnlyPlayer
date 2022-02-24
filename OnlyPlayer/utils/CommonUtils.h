//
//  CommonUtils.h
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/8/15.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface CommonUtils : NSObject

+ (NSString *)bundlePath:(NSString *)fileName;

+ (NSString *)documentPath:(NSString *)fileName;

@end

NS_ASSUME_NONNULL_END
