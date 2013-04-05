#ifndef PLOT_UTILS_H_
#define PLOT_UTILS_H_

#define GL_GLEXT_PROTOTYPES

#if defined(__linux__)
#include <GL/glut.h>
#elif defined(WIN32)
#undef GL_GLEXT_PROTOTYPES
#include <GLUT/glut.h>
#include <gl/GLU.h>
#include "..\..\..\glWinInit.h"
#else // assume OS X
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#endif

#include <stdio.h>
#include <math.h>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <FTGL/ftgl.h>

#include "../../../../JuceLibraryCode/JuceHeader.h"

#define WAVE1 0
#define WAVE2 1
#define WAVE3 2
#define WAVE4 3
#define PROJ1x2 4
#define PROJ1x3 5
#define PROJ1x4 6
#define PROJ2x3 7
#define PROJ2x4 8
#define PROJ3x4 9

void checkGlError();
void setViewportRange(int xMin,int xMax,int yMin,int yMax);

void drawString(float x, float y, void* f, const char* string);
void drawString(float x, float y, int size, String s, FTPixmapFont* f);

void drawViewportEdge();

void drawViewportCross();

int roundUp(int, int);

double ad16ToUv(int ad, int gain);

void makeLabel(int val, int gain, bool convert, char* s);

void n2ProjIdx(int i, int* p1, int* p2);

template< class T >
T* addressof(T& arg)
{
    return (T*)&(char&)arg;
}

bool isFrameBufferExtensionSupported();
bool checkFramebufferStatus();
#endif