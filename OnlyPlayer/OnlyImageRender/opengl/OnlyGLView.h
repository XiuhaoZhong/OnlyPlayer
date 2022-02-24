//
//  OnlyGLView.h
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/10/6.
//

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface OnlyGLView : UIView

- (instancetype)initWithFrame:(CGRect)frame filePath:(NSString *)filePath;

- (void)render;

- (void)destory;

@end

NS_ASSUME_NONNULL_END
