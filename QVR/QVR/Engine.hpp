//
//  Engine.hpp
//  
//
//  Created by 谌启亮 on 2017/3/29.
//  Copyright © 2017年 Tencent. All rights reserved.
//

#ifndef Engine_hpp
#define Engine_hpp

#include <stdio.h>
#include <cstdlib>
#include <OpenGLES/ES2/gl.h>
#include <string>

namespace QVR {
    
    typedef enum VideoType{
        VideoType360 = 0,
        VideoType3D360UpDown = 1,
        VideoType3D360LeftRight = 2,
    }VideoType;
    
    typedef enum VideoPlayMode{
        VideoPlayModeNormal = 0,
        VideoPlayModeHeadset = 1,
    }VideoPlayMode;
    
    typedef enum PixelFormat{
        PixelFormatRGBA = 0,
        PixelFormatBGRA = 1,
    }PixelFormat;
    
    class Engine
    {
    private:
        GLint positionAttribLocation;
        GLint texCoordAttribLocation;
        
        VideoType videoType;
        VideoPlayMode playMode;
        
        int texUniformId;
        int modelViewMatrixId;
        int projectionMatrixId;
        int pixelFormatId;
        int distortionParamsId;
        
        
        GLuint interFrameBuffer;
        GLuint interTexure;
        GLuint interDepthBuffer;
        GLint meshProgram;
        GLint meshTexUniformId;
        int meshTexCoordAttribLoc;
        int meshPositionAttribLoc;
        int meshMVPUniformId;
        
        
        GLint program;
        
        GLuint vertexBuff;
        GLuint vertexIndicesBuff;
        GLuint textCoordBuff;
        GLuint textCoordBuff2;
        GLuint textureId;
        
        GLint textureWidth;
        GLint textureHeight;
        
        GLfloat *vertices;
        GLfloat *texCoords;
        GLfloat *texCoords2;
        GLshort *indices;
        int numVertices;
        int numIndices;
        
        float viewWidth;
        float viewHeight;
        float degrees = 90;
        
        GLfloat distortionParams[4];
        
        void updateProjectionMatrix();

    public:
        
        Engine();
        
        std::string meshFsh;
        std::string meshVsh;
        
        //配置视频类型(2D, 3D上下，3D左右格式)
        void setVideoType(VideoType videoType);
        //配置播放模式（普通播放，头戴式设备播放）
        void setPlayMode(VideoPlayMode mode);
        //配置显示宽度和高度
        void setViewSize(float width, float height);
        //配置视角大小
        void setViewDegrees(float degrees);
        //配置反畸变参数
        void setViewDistortions(float p1, float p2, float p3, float p4);
        //开始渲染，分配资源
        void begin();
        void begin(const char *vsh, const char *fsh);//自定义shader
        //更新各个轴旋转角度
        void updateRotation(float rotationX, float rotationY, float rotationZ);
        //更新图片数据
        void update(PixelFormat pixelFormat, unsigned char *bitmap, int width, int height);
        //渲染
        void render();
        //结束渲染释放资源
        void end();
    };
}


#endif /* Engine_hpp */
