//
//  OnlyGLView.m
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/10/6.
//

#import "OnlyGLView.h"

#import "RGBAFrame.h"
#import "CSJRGBAFrameCopier.h"
#import "png_decoder.h"

@interface OnlyGLView ()

@property (nonatomic, assign) BOOL   readyToRender;
@property (nonatomic, assign) BOOL   shouldEnableOpenGL;
@property (nonatomic, strong) NSLock *shouldEnableOpenGLLock;

@end

@implementation OnlyGLView {
    dispatch_queue_t    _contextQueue;
    EAGLContext         *_context;
    GLuint              _displayFrameBuffer;
    GLuint              _renderBuffer;
    GLint               _backingWidth;
    GLint               _backingHeight;
    BOOL                _stopping;
    
    CSJRGBAFrameCopier  *_frameCopier;
    RGBAFrame           *_frame;
}

+ (Class) layerClass {
    return [CAEAGLLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame filePath:(NSString *)filePath {
    if (self = [super initWithFrame:frame]) {
        
        _shouldEnableOpenGLLock = [NSLock new];
        [_shouldEnableOpenGLLock lock];
        _shouldEnableOpenGL = [UIApplication sharedApplication].applicationState == UIApplicationStateActive;
        [_shouldEnableOpenGLLock unlock];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive:) name:UIApplicationWillResignActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive:) name:UIApplicationDidBecomeActiveNotification object:nil];
        
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,  kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat ,nil];
        
        _contextQueue = dispatch_queue_create("com.csj.pictureRenderQueue", NULL);
        dispatch_sync(_contextQueue, ^{
            _context = [self buildEAGLContext];
            if (!_context || ![EAGLContext setCurrentContext:_context]) {
                NSLog(@"Setup EAGLContext failed...");
            }
            
            if (![self createDisplayFrameBuffer]) {
                NSLog(@"create Display FrameBuffer failed...");
            }
            
            _frame = [self getRGBAFrame:filePath];
            _frameCopier = [[CSJRGBAFrameCopier alloc] init];
            if (![_frameCopier prepareRender:_frame->width height:_frame->height]) {
                NSLog(@"RGBAFrameCopier prepareRender failed...");
            }
            
            self.readyToRender = YES;
        });
        
        return self;
    }
    
    return nil;
}

- (void)render {
    if (_stopping) {
        return ;
    }
    
    dispatch_async(_contextQueue, ^{
        if (self->_frame) {
            [self.shouldEnableOpenGLLock lock];
            if (!self.readyToRender || !self.shouldEnableOpenGL) {
                // glFinish 将缓冲区中的指令立即发送给图形硬件执行，执行完成之后才返回，即使缓冲区没有满;
                // 注意与glFlush的区别，glFlush同样是将缓冲区中的指令发送给图形硬件执行，但是会立即返回，同样不会判断缓冲区是否满;
                glFinish();
                [self.shouldEnableOpenGLLock unlock];
                return ;
            }
            [self.shouldEnableOpenGLLock unlock];
            
            [EAGLContext setCurrentContext:self->_context];
            glBindFramebuffer(GL_FRAMEBUFFER, self->_displayFrameBuffer);
            glViewport(0, 0, self->_backingWidth, self->_backingHeight);
            [self->_frameCopier renderFrame:self->_frame->pixels];
            glBindRenderbuffer(GL_RENDERBUFFER, self->_renderBuffer);
            [self->_context presentRenderbuffer:GL_FRAMEBUFFER];
        }
    });
}

- (void)destory {
    _stopping = YES;
    dispatch_sync(_contextQueue, ^{
        if (_frameCopier) {
            [_frameCopier releaseRender];
        }
        
        if (_displayFrameBuffer) {
            glDeleteFramebuffers(1, &_displayFrameBuffer);
            _displayFrameBuffer = 0;
        }
        
        if (_renderBuffer) {
            glDeleteRenderbuffers(1, &_renderBuffer);
            _renderBuffer = 0;
        }
        
        if ([EAGLContext currentContext] == _context) {
            [EAGLContext setCurrentContext:nil];
        }
    });
}

- (EAGLContext *)buildEAGLContext {
    return [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
}

- (BOOL)createDisplayFrameBuffer {
    
    // 注意区分RenderBuffer和FrameBuffer;
    // 简单来说，RenderBuffer是一个可以绘制的缓冲区，类似一个off screen
    // FrameBuffer是一个集合，里面包含了颜色、深度、漏字板等等;
    glGenFramebuffers(1, &_displayFrameBuffer);
    glGenRenderbuffers(1, &_renderBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _displayFrameBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffer);
    // 将renderBuffer与当前的CAEAGLLayer连接起来;
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_backingHeight);
    // 将缓冲区绑定到帧缓冲区;
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderBuffer);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"failed to make complete framebuffer object: %x", status);
        return NO;
    }
    
    GLenum glError = glGetError();
    if (glError != GL_NO_ERROR) {
        NSLog(@"failed to setup GL %x", glError);
        return NO;
    }
    
    return YES;
}

- (RGBAFrame *)getRGBAFrame:(NSString *)pngFilePath {
    
    PngPicDecoder *pDecoder = new PngPicDecoder();
    char *pngPath = (char *)[pngFilePath cStringUsingEncoding:NSUTF8StringEncoding];
    pDecoder->openFile(pngPath);
    RawImageData data = pDecoder->getRawImageData();
    RGBAFrame *frame = new RGBAFrame();
    frame->width = data.width;
    frame->height = data.height;
    int expectedLength = data.width * data.height * 4;
    uint8_t * pixels = new uint8_t[expectedLength];
    memset(pixels, 0, sizeof(uint8_t) * expectedLength);
    int pixelsLength = MIN(expectedLength, data.size);
    memcpy(pixels, (byte *)data.data, pixelsLength);
    frame->pixels = pixels;
    pDecoder->releaseRawImageData(&data);
    pDecoder->closeFile();
    delete pDecoder;
    
    return frame;
}

#pragma mark - notification handler;
- (void)applicationWillResignActive:(NSNotification *)notify {
    [self.shouldEnableOpenGLLock lock];
    self.shouldEnableOpenGL = NO;
    [self.shouldEnableOpenGLLock unlock];
}

- (void)applicationDidBecomeActive:(NSNotification *)notify {
    [self.shouldEnableOpenGLLock lock];
    self.shouldEnableOpenGL = YES;
    [self.shouldEnableOpenGLLock unlock];
}

@end
