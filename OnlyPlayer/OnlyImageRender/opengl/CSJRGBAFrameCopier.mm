//
//  CSJRGBAFrameCopier.m
//  OnlyPlayer
//
//  Created by zhongxiuhao on 2021/10/6.
//

#import "CSJRGBAFrameCopier.h"

#define STRINGIZE(x) #x
#define STRINGIZE2(x) STRINGIZE(x)
#define SHADER_STRING(text) @ STRINGIZE2(text)

NSString *const vertexShaderString = SHADER_STRING(
                                                   attribute vec4 position;
                                                   attribute vec2 texcoord;
                                                   varying vec2 v_texcoord;
    
                                                   void main() {
                                                       gl_Position = position;
                                                       v_texcoord = texcoord.xy;
                                                   }
                                                   );

NSString * const rgbFragmentShaderString = SHADER_STRING(
                                                         varying highp vec2 v_texcoord;
                                                         uniform sampler2D inputImageTexture;
                                                         
                                                         void main() {
                                                             gl_FragColor = texture2D(inputImageTexture, v_texcoord);
                                                         });

@implementation CSJRGBAFrameCopier {
    NSInteger   mFrameWidth;
    NSInteger   mFrameHeight;
    
    GLuint      mFilterProgram;
    GLint       mFilterPositionAttribute;
    GLint       mFilterTextureCoordinateAttribute;
    GLint       mFilterInputTextureUniform;
    
    GLuint      _inputTexture;
}

- (BOOL)prepareRender:(NSInteger)textureWidth height:(NSInteger)textureHeight {
    BOOL ret = NO;
    
    mFrameWidth = textureWidth;
    mFrameHeight = textureHeight;
    if ([self buildProgram:vertexShaderString fragmentShader:rgbFragmentShaderString]) {
        glGenTextures(1, &_inputTexture);
        glBindTexture(GL_TEXTURE_2D, _inputTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)mFrameWidth, (GLsizei)mFrameHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        ret = YES;
    }
    
    return ret;
}

- (void)renderFrame:(uint8_t *)rgbaFrame {
    glUseProgram(mFilterProgram);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindTexture(GL_TEXTURE_2D, _inputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)mFrameWidth, (GLsizei)mFrameHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaFrame);
    
    static const GLfloat imageVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0, 1.0f,
    };
    
    GLfloat noRotationTextureCoordinates[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };
    
    glVertexAttribPointer(mFilterPositionAttribute, 2, GL_FLOAT, 0, 0, imageVertices);
    glEnableVertexAttribArray(mFilterPositionAttribute);
    glVertexAttribPointer(mFilterTextureCoordinateAttribute, 2, GL_FLOAT, 0, 0, noRotationTextureCoordinates);
    glEnableVertexAttribArray(mFilterTextureCoordinateAttribute);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _inputTexture);
    glUniform1i(mFilterInputTextureUniform, 0);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

- (BOOL)buildProgram:(NSString *)vertexShader fragmentShader:(NSString *)fragmentShader {
    BOOL result = NO;
    
    GLuint vertShader = 0, fragShader = 0;
    mFilterProgram = glCreateProgram();
    vertShader = compileShader(GL_VERTEX_SHADER, vertexShader);
    if (!vertShader) {
        
    }
    fragShader = compileShader(GL_FRAGMENT_SHADER, fragmentShader);
    if (!fragShader) {
        
    }
    
    glAttachShader(mFilterProgram, vertShader);
    glAttachShader(mFilterProgram, fragShader);
    
    glLinkProgram(mFilterProgram);
    
    mFilterPositionAttribute = glGetAttribLocation(mFilterProgram, "position");
    mFilterTextureCoordinateAttribute = glGetAttribLocation(mFilterProgram, "texcoord");
    mFilterInputTextureUniform = glGetUniformLocation(mFilterProgram, "inputImageTexture");
    
    GLint status;
    glGetProgramiv(mFilterProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        NSLog(@"Failed to link program %d", mFilterProgram);
    }
    result = validateProgram(mFilterProgram);
    
exit:
    if (vertShader) {
        glDeleteShader(vertShader);
    }
    
    if (fragShader) {
        glDeleteShader(fragShader);
    }
    
    if (result) {
        NSLog(@"Setup GL program OK!");
    } else {
        glDeleteProgram(mFilterProgram);
        mFilterProgram = 0;
    }
    
    return result;
}

- (void)releaseRender {
    if (mFilterProgram) {
        glDeleteProgram(mFilterProgram);
        mFilterProgram = 0;
    }
    
    if (_inputTexture) {
        glDeleteTextures(1, &_inputTexture);
    }
}

@end
