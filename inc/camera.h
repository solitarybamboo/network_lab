#ifndef __CAMERA_H
#define __CAMERA_H

#include "common.h"
//#include "jpeglib.h"

extern int CAMERA_W;
extern int CAMERA_H;

extern int R[256][256];
extern int G[256][256][256];
extern int B[256][256];

extern char formats[5][16];
extern struct v4l2_fmtdesc fmtdesc;
extern struct v4l2_format  fmt;
extern struct v4l2_capability cap;

bool get_caminfo(int camfd);
bool get_camfmt(int camfd);
bool get_camcap(int camfd);

void set_camfmt(int camfd);

void *convert(void *arg);
//void yuv2jpg(uint8_t *yuv);
// YUV转BMP（替换原yuv2jpg函数）
void yuv2bmp(uint8_t *yuv, const char *filename);

#endif
