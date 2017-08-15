//
//  Engine.cpp
//  
//
//  Created by 谌启亮 on 2017/3/29.
//  Copyright © 2017年 Tencent. All rights reserved.
//

#include "Engine.hpp"
#include <cmath>
#include <cstdlib>
#include "Matrix.hpp"
#include <string>

using namespace QVR;

static const char *_fsh = "\
\
precision mediump float;\
\
uniform sampler2D Texture;\
varying vec2 TextureCoordsOut;\
uniform int pixelFormat;\
\
void main(void)\
{\
    vec4 mask = texture2D(Texture, TextureCoordsOut);\
    if (pixelFormat == 1) {\
        gl_FragColor = vec4(mask.bgr, 1.0);\
    }\
    else{\
        gl_FragColor = vec4(mask.rgb, 1.0);\
    }\
}\
";

static const char *_vsh = "\
\
attribute vec2 TextureCoords;\
attribute vec4 Position;\
varying vec2 TextureCoordsOut;\
uniform mat4 modelViewProjectionMatrix;\
\
void main(void)\
{\
    gl_Position = modelViewProjectionMatrix * Position;\
    TextureCoordsOut = TextureCoords;\
}\
";


#define LOG printf

int QVCreateSphere(int numSlices, float radius, float **vertices,
                float **texCoords, GLshort **indices, int *numVertices_out);


GLuint QVCreateShader(GLenum shaderType, const char *source){
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    
    if (status != GL_TRUE) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderInfoLog(shader, logLength, &logLength, log);
            LOG("Compile Shader Error: %s", log);
            free(log);
        }
    }

    return shader;
}

GLint QVCreateProgram(const char *vsh, const char *fsh){
    GLuint vertexShader = QVCreateShader(GL_VERTEX_SHADER, vsh);
    GLuint fragmentShader = QVCreateShader(GL_FRAGMENT_SHADER, fsh);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    
    glLinkProgram(program);
    
    int status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetProgramInfoLog(program, logLength, &logLength, log);
            LOG("Link Program Error: %s", log);
            free(log);
        }
    }

    return program;
}

Engine::Engine() {
    distortionParamsId = -1;
}


void Engine::begin(){
    this->begin(_vsh, _fsh);
}

void Engine::begin(const char *vsh, const char *fsh){
    
    numIndices = QVCreateSphere(200, 1.0f, &vertices, &texCoords, &indices, &numVertices);
    
    if (videoType == VideoType3D360UpDown) {
        for (int i = 0; i < numVertices; i++) {
            texCoords[2*i+1]/=2.0;
        }
    }
    else if (videoType == VideoType3D360LeftRight) {
        for (int i = 0; i < numVertices; i++) {
            texCoords[2*i]/=2.0;
        }
    }

    
    if (playMode == VideoPlayModeHeadset) {
        texCoords2 = (GLfloat *)malloc(numVertices * sizeof(GLfloat) *2);
        memcpy(texCoords2, texCoords, numVertices * sizeof(GLfloat) *2);
        if (videoType == VideoType3D360UpDown) {
            for (int i = 0; i < numVertices; i++) {
                texCoords2[2*i+1]+=0.5;
            }
        }
        else if (videoType == VideoType3D360LeftRight) {
            for (int i = 0; i < numVertices; i++) {
                texCoords2[2*i]+=0.5;
            }
        }
    }
    
    
    program = QVCreateProgram(vsh, fsh);
    
    positionAttribLocation = glGetAttribLocation(program, "Position");
    texCoordAttribLocation = glGetAttribLocation(program, "TextureCoords");

    modelViewMatrixId = glGetUniformLocation(program, "modelViewMatrix");
    projectionMatrixId = glGetUniformLocation(program, "projectionMatrix");
    pixelFormatId = glGetUniformLocation(program, "pixelFormat");
    distortionParamsId = glGetUniformLocation(program, "p");

    glEnable(GL_TEXTURE_2D);

    texUniformId = glGetUniformLocation(program, "Texture");

    glGenBuffers(1, &vertexIndicesBuff);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndicesBuff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices*sizeof(GLushort), indices, GL_STATIC_DRAW);
    
    //顶点数组buff
    glGenBuffers(1, &vertexBuff);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuff);
    glBufferData(GL_ARRAY_BUFFER, numVertices*3*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionAttribLocation);
    glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glGenBuffers(1, &textCoordBuff);
    glBindBuffer(GL_ARRAY_BUFFER, textCoordBuff);
    glBufferData(GL_ARRAY_BUFFER, numVertices*2*sizeof(GLfloat), texCoords, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(texCoordAttribLocation);
    glVertexAttribPointer(texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
    if (playMode == VideoPlayModeHeadset) {
        glGenBuffers(1, &textCoordBuff2);
        glBindBuffer(GL_ARRAY_BUFFER, textCoordBuff2);
        glBufferData(GL_ARRAY_BUFFER, numVertices*2*sizeof(GLfloat), texCoords2, GL_DYNAMIC_DRAW);
    }
    
    GLint numUniform = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniform);

    
    glValidateProgram(program);
    int status = 0;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);

    glUseProgram(program);
    
    
    glUniform4fv(distortionParamsId, 1, distortionParams);


    GLfloat matrix[16] = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    };
    glUniformMatrix4fv(modelViewMatrixId, 1, GL_FALSE, matrix);
    this->updateProjectionMatrix();
    
    glGenFramebuffers(1, &interFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, interFrameBuffer);
    
    glGenTextures(1, &interTexure);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewWidth, viewHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, interTexure, 0);
    
    glGenRenderbuffers(1, &interDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, interDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, viewWidth, viewHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, interDepthBuffer);
    
    meshProgram = QVCreateProgram(meshVsh.c_str(), meshFsh.c_str());
    meshTexCoordAttribLoc = glGetAttribLocation(meshProgram, "a_tex_coord");
    meshPositionAttribLoc = glGetAttribLocation(meshProgram, "a_pos");
    
    glEnable(GL_TEXTURE_2D);
    meshTexUniformId = glGetUniformLocation(meshProgram, "u_texture");
    
    meshMVPUniformId = glGetUniformLocation(meshProgram, "u_mvpMat");
    
    char name[20] = {0};
    glGetActiveUniform(meshProgram, 0, 20, NULL, NULL, NULL, name);
}

void Engine::setVideoType(VideoType type){
    videoType = type;
}

void Engine::setViewSize(float width, float height){
    viewWidth = width;
    viewHeight = height;
}

void Engine::setViewDegrees(float degrees){
    this->degrees = degrees;
    this->updateProjectionMatrix();
}

void Engine::setViewDistortions(float p1, float p2, float p3, float p4){
    distortionParams[0] = p1;
    distortionParams[1] = p2;
    distortionParams[2] = p3;
    distortionParams[3] = p4;
    if (distortionParamsId >= 0) {
        glUniform4fv(distortionParamsId, 1, distortionParams);
    }
}

void Engine::setPlayMode(VideoPlayMode mode){
    playMode = mode;
}

void Engine::update(PixelFormat pixelFormat, unsigned char *bitmap, int width, int height){
    if (textureWidth == width && textureHeight == height) {
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
    }
    else{
        textureWidth = width;
        textureHeight = height;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);
        glActiveTexture(GL_TEXTURE5);
        glUniform1i(texUniformId, 5);
    }
    glUniform1i(pixelFormatId, pixelFormat);
}

void Engine::updateProjectionMatrix(){
    float aspect = 1;
    if (viewHeight > __FLT_EPSILON__) {
        if (playMode == VideoPlayModeHeadset) {
            aspect = fabs((viewWidth/2.0) / viewHeight);
        }
        else{
            aspect = fabs(viewWidth / viewHeight);
        }
    }
    float radians = degrees * (M_PI / 180);
    Mat4 projectionMatrix = Mat4MakePerspective(radians, aspect, 0.1f, 400.0f);
    //projectionMatrix = Mat4MakeOrtho(-120, 120, -120/aspect, 120/aspect, 150, 500.0f);
    projectionMatrix = Mat4Rotate(projectionMatrix, M_PI, 1.0f, 0.0f, 0.0f);
    glUniformMatrix4fv(projectionMatrixId, 1, GL_FALSE, projectionMatrix.m);
}

void Engine::updateRotation(float rotationX, float rotationY, float rotationZ){
    
    Mat4 modelViewMatrix = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    };
    float scale = 300;
    modelViewMatrix = Mat4Scale(modelViewMatrix, scale, scale, scale);
    modelViewMatrix = Mat4RotateX(modelViewMatrix, rotationX); // Up/Down axis
    modelViewMatrix = Mat4RotateY(modelViewMatrix, rotationY);
    modelViewMatrix = Mat4RotateZ(modelViewMatrix, rotationZ);
    modelViewMatrix = Mat4RotateX(modelViewMatrix, M_PI/2);
    glUniformMatrix4fv(modelViewMatrixId, 1, GL_FALSE, modelViewMatrix.m);
}

void Engine::render(){
    
//    GLint originFrameBuff;
//    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originFrameBuff);
//    
//    glBindFramebuffer(GL_FRAMEBUFFER, interFrameBuffer);

    glUseProgram(program);
    if (playMode == VideoPlayModeHeadset) {
        glBindBuffer(GL_ARRAY_BUFFER, textCoordBuff);
        glVertexAttribPointer(texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glViewport(0, 0, viewWidth/2, viewHeight);
    }
    glBindTexture(GL_TEXTURE_2D, textureId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndicesBuff);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
    
    if (playMode == VideoPlayModeHeadset) {
        glViewport(viewWidth/2, 0, viewWidth/2, viewHeight);
        glClear(GL_DEPTH_BUFFER_BIT);
        glBindBuffer(GL_ARRAY_BUFFER, textCoordBuff2);
        glVertexAttribPointer(texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndicesBuff);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
    }
    
    
//    glBindFramebuffer(GL_FRAMEBUFFER, originFrameBuff);
//
//    
//    GLfloat meshPostions[] = {
//        0, 0, 0,
//        0, 1, 0,
//        1, 0, 0,
//        1, 1, 0,
//    };
//    
//    GLfloat meshTexCoord[] = {
//        0, 0,
//        0, 1,
//        1, 1,
//        1, 0,
//    };
//    
//    Mat4 modelViewMatrixMesh = {
//        1.0, 0.0, 0.0, 0.0,
//        0.0, 1.0, 0.0, 0.0,
//        0.0, 0.0, 1.0, 0.0,
//        0.0, 0.0, 0.0, 1.0,
//    };
//    
//    modelViewMatrixMesh = Mat4MakeOrtho(-1, 1, -1, 1, -1, 1);
//    
//
//    
//    glUseProgram(meshProgram);
////    glViewport(0, 0, viewWidth, viewHeight);
//    
//    glBindTexture(GL_TEXTURE_2D, textureId);
//    glActiveTexture(GL_TEXTURE0);
//    glUniform1i(meshTexUniformId, 0);
//    
//
//    glUniformMatrix4fv(meshMVPUniformId, 1, GL_FALSE, modelViewMatrixMesh.m);
//    glEnableVertexAttribArray(meshPositionAttribLoc);
//    glVertexAttribPointer(meshPositionAttribLoc, 3, GL_FLOAT, GL_FALSE, 0, meshPostions);
//    glEnableVertexAttribArray(meshTexCoordAttribLoc);
//    glVertexAttribPointer(meshTexCoordAttribLoc, 2, GL_FLOAT, GL_FALSE, 0, meshTexCoord);
//    
//    
//    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//    
}


int QVCreateSphere(int numSlices, float radius, float **vertices,
                float **texCoords, GLshort **indices, int *numVertices_out) {
    int numParallels = numSlices / 2;
    int numVertices = (numParallels + 1) * (numSlices + 1);
    int numIndices = numParallels * numSlices * 6;
    float angleStep = (2.0f * M_PI) / ((float) numSlices);
    
    if (vertices != NULL) {
        *vertices = (float*)malloc(sizeof(float) * 3 * numVertices);
    }
    
    if (texCoords != NULL) {
        *texCoords = (float*)malloc(sizeof(float) * 2 * numVertices);
    }
    
    if (indices != NULL) {
        *indices = (GLshort*)malloc(sizeof(GLshort) * numIndices);
    }
    
    for (int i = 0; i < numParallels + 1; i++) {
        for (int j = 0; j < numSlices + 1; j++) {
            int vertex = (i * (numSlices + 1) + j) * 3;
            
            if (vertices) {
                (*vertices)[vertex + 0] = radius * sinf(angleStep * (float)i) * sinf(angleStep * (float)j);
                (*vertices)[vertex + 1] = radius * cosf(angleStep * (float)i);
                (*vertices)[vertex + 2] = radius * sinf(angleStep * (float)i) * cosf(angleStep * (float)j);
            }
            
            if (texCoords) {
                int texIndex = (i * (numSlices + 1) + j) * 2;
                (*texCoords)[texIndex + 0] = (float)j / (float)numSlices;
                (*texCoords)[texIndex + 1] = 1.0f - ((float)i / (float)numParallels);
            }
        }
    }
    
    // Generate the indices
    if (indices != NULL) {
        GLshort *indexBuf = (*indices);
        for (int i = 0; i < numParallels ; i++) {
            for (int j = 0; j < numSlices; j++) {
                *indexBuf++ = i * (numSlices + 1) + j;
                *indexBuf++ = (i + 1) * (numSlices + 1) + j;
                *indexBuf++ = (i + 1) * (numSlices + 1) + (j + 1);
                
                *indexBuf++ = i * (numSlices + 1) + j;
                *indexBuf++ = (i + 1) * (numSlices + 1) + (j + 1);
                *indexBuf++ = i * (numSlices + 1) + (j + 1);
            }
        }
    }
    
    if (numVertices_out) {
        *numVertices_out = numVertices;
    }
    
    return numIndices;
}
