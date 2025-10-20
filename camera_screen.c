#include "common.h"
#include "camera.h"
#include <stdint.h>

char formats[5][16] = {0};
struct v4l2_fmtdesc fmtdesc;
struct v4l2_format  fmt;
struct v4l2_capability cap;

int CAMERA_W, CAMERA_H;
int R[256][256];
int G[256][256][256];
int B[256][256];

// 获取摄像头格式信息（固定）
bool get_caminfo(int camfd)
{
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    bool ret = false;
    printf("当前摄像头所支持的格式有: \n");
    while((ret=ioctl(camfd, VIDIOC_ENUM_FMT, &fmtdesc)) == 0)
    {
        printf("[%d]", fmtdesc.index);
        sprintf(formats[fmtdesc.index]+0, "%c", (fmtdesc.pixelformat>>8*0)&0xFF);
        sprintf(formats[fmtdesc.index]+1, "%c", (fmtdesc.pixelformat>>8*1)&0xFF);
        sprintf(formats[fmtdesc.index]+2, "%c", (fmtdesc.pixelformat>>8*2)&0xFF);
        sprintf(formats[fmtdesc.index]+3, "%c", (fmtdesc.pixelformat>>8*3)&0xFF);

        printf("\"%s\"", formats[fmtdesc.index]);
        printf("（详细描述: %s）\n", fmtdesc.description);
        fmtdesc.index++;
        ret = true;
    }
    printf("\n");

    return ret;
}

// 获取摄像头格式信息（可调）
bool get_camfmt(int camfd)
{
    bzero(&fmt, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(ioctl(camfd, VIDIOC_G_FMT, &fmt) == -1)
    {
        printf("获取摄像头格式信息失败: %s\n", strerror(errno));
        return false;
    }

    printf("摄像头当前的配置信息:\n");
    printf("分辨率: %d×%d\n", fmt.fmt.pix.width, fmt.fmt.pix.height);

    printf("像素格式: ");
    switch(fmt.fmt.pix.pixelformat)
    {
    case V4L2_PIX_FMT_MJPEG:
        printf("V4L2_PIX_FMT_MJPEG\n");
        break;
    case V4L2_PIX_FMT_JPEG:
        printf("V4L2_PIX_FMT_JPEG\n");
        break;
    case V4L2_PIX_FMT_MPEG:
        printf("V4L2_PIX_FMT_MPEG\n");
        break;
    case V4L2_PIX_FMT_MPEG1:
        printf("V4L2_PIX_FMT_MPEG1\n");
        break;
    case V4L2_PIX_FMT_MPEG2:
        printf("V4L2_PIX_FMT_MPEG2\n");
        break;
    case V4L2_PIX_FMT_MPEG4:
        printf("V4L2_PIX_FMT_MPEG4\n");
        break;
    case V4L2_PIX_FMT_H264:
        printf("V4L2_PIX_FMT_H264\n");
        break;
    case V4L2_PIX_FMT_XVID:
        printf("V4L2_PIX_FMT_XVID\n");
        break;
    case V4L2_PIX_FMT_RGB24:
        printf("V4L2_PIX_FMT_RGB24\n");
        break;
    case V4L2_PIX_FMT_BGR24:
        printf("V4L2_PIX_FMT_BGR24\n");
        break;
    case V4L2_PIX_FMT_YUYV:
        printf("V4L2_PIX_FMT_YUYV\n");
        break;
    case V4L2_PIX_FMT_YYUV:
        printf("V4L2_PIX_FMT_YYUV\n");
        break;
    case V4L2_PIX_FMT_YVYU:
        printf("V4L2_PIX_FMT_YVYU\n");
        break;
    case V4L2_PIX_FMT_YUV444:
        printf("V4L2_PIX_FMT_YUV444\n");
        break;
    case V4L2_PIX_FMT_YUV410:
        printf("V4L2_PIX_FMT_YUV410\n");
        break;
    case V4L2_PIX_FMT_YUV420:
        printf("V4L2_PIX_FMT_YUV420\n");
        break;
    case V4L2_PIX_FMT_YVU420:
        printf("V4L2_PIX_FMT_YVU420\n");
        break;
    case V4L2_PIX_FMT_YUV422P:
        printf("V4L2_PIX_FMT_YUV422P\n");
        break;
    default:
        printf("未知\n");
    }
    printf("\n");

    return true;
}

// 获取摄像头设备的基本参数
bool get_camcap(int camfd)
{
    bzero(&cap, sizeof(cap));
    if(ioctl(camfd, VIDIOC_QUERYCAP, &cap) == -1)
    {
        printf("获取摄像头基本信息失败: %s\n", strerror(errno));
        return false;
    }

    printf("驱动：%s\n", cap.driver);
    printf("显卡：%s\n", cap.card);
    printf("总线：%s\n", cap.bus_info);
    printf("版本：%d\n", cap.version);

    if((cap.capabilities&V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE)
    {
        printf("该设备为视频采集设备\n");
    }
    if((cap.capabilities&V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING)
    {
        printf("该设备支持流IO操作\n\n");
    }

    return true;
}

// 配置摄像头像素格式
void set_camfmt(int camfd)
{
    struct v4l2_format *tmp = calloc(1, sizeof(*tmp));
    bzero(tmp, sizeof(*tmp));

    tmp->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tmp->fmt.pix.width  = 640;
    tmp->fmt.pix.height = 480;

    int n = 0;

    if(!strncmp(formats[n], "JPEG", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
    else if(!strncmp(formats[n], "MJPG", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    else if(!strncmp(formats[n], "MPEG", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_MPEG;
    else if(!strncmp(formats[n], "YUYV", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    else if(!strncmp(formats[n], "YVYU", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_YVYU;
    else if(!strncmp(formats[n], "H264", 4)) tmp->fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
    else
    {
        printf("对不起，所选格式无法配置.\n");
        exit(0);
    }

    tmp->fmt.pix.field = V4L2_FIELD_INTERLACED;

    if(ioctl(camfd, VIDIOC_S_FMT, tmp) == -1)
    {
        printf("ioctl() VIDIOC_S_FMT 失败了: %s\n", strerror(errno));
    }
    free(tmp);
}

void *convert(void *arg __attribute__((unused)))
{
    pthread_detach(pthread_self());

    for(int i=0; i<256; i++)
    {
        for(int j=0; j<256; j++)
        {
            R[i][j] = i + 1.042*(j-128);
            R[i][j] = R[i][j]>255 ? 255 : R[i][j];
            R[i][j] = R[i][j]<0   ? 0   : R[i][j];

            B[i][j] = i + 1.772*(j-128);
            B[i][j] = B[i][j]>255 ? 255 : B[i][j];
            B[i][j] = B[i][j]<0   ? 0   : B[i][j];

            for(int k=0; k<256; k++)
            {
                G[i][j][k] = i + 0.344*(j-128)-0.714*(k-128);
                G[i][j][k] = G[i][j][k]>255 ? 255 : G[i][j][k];
                G[i][j][k] = G[i][j][k]<0   ? 0   : G[i][j][k];
            }
        }
    }
    pthread_exit(NULL);
}

// BMP文件头结构
typedef struct {
    uint8_t  bfType[2];
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMP_FILE_HEADER;

// BMP信息头结构
typedef struct {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMP_INFO_HEADER;

// YUV转BMP
void yuv2bmp(uint8_t *yuv, const char *filename)
{
    int width = CAMERA_W;
    int height = CAMERA_H;
    int bit_count = 24;
    int row_bytes = (width * bit_count + 31) / 32 * 4;
    int pixel_data_size = row_bytes * height;
    int file_total_size = sizeof(BMP_FILE_HEADER) + sizeof(BMP_INFO_HEADER) + pixel_data_size;

    uint8_t *rgb_data = (uint8_t *)calloc(1, pixel_data_size);
    if (rgb_data == NULL) {
        printf("分配RGB缓存失败: %s\n", strerror(errno));
        return;
    }

    uint8_t Y0, U, Y1, V;
    int yuv_offset, rgb_row_offset, rgb_col_offset;

    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < width; j += 2) {
            yuv_offset = (width * i + j) * 2;
            rgb_row_offset = (height - 1 - i) * row_bytes;
            rgb_col_offset = j * 3;

            Y0 = *(yuv + yuv_offset + 0);
            U  = *(yuv + yuv_offset + 1);
            Y1 = *(yuv + yuv_offset + 2);
            V  = *(yuv + yuv_offset + 3);

            *(rgb_data + rgb_row_offset + rgb_col_offset + 0) = B[Y0][U];
            *(rgb_data + rgb_row_offset + rgb_col_offset + 1) = G[Y0][U][V];
            *(rgb_data + rgb_row_offset + rgb_col_offset + 2) = R[Y0][V];

            *(rgb_data + rgb_row_offset + rgb_col_offset + 3) = B[Y1][U];
            *(rgb_data + rgb_row_offset + rgb_col_offset + 4) = G[Y1][U][V];
            *(rgb_data + rgb_row_offset + rgb_col_offset + 5) = R[Y1][V];
        }
    }

    BMP_FILE_HEADER file_header = {0};
    BMP_INFO_HEADER info_header = {0};

    file_header.bfType[0] = 'B';
    file_header.bfType[1] = 'M';
    file_header.bfSize = file_total_size;
    file_header.bfOffBits = sizeof(BMP_FILE_HEADER) + sizeof(BMP_INFO_HEADER);

    info_header.biSize = sizeof(BMP_INFO_HEADER);
    info_header.biWidth = width;
    info_header.biHeight = height;
    info_header.biPlanes = 1;
    info_header.biBitCount = bit_count;
    info_header.biCompression = 0;
    info_header.biSizeImage = pixel_data_size;

    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        printf("创建BMP文件[%s]失败: %s\n", filename, strerror(errno));
        free(rgb_data);
        return;
    }

    fwrite(&file_header, sizeof(BMP_FILE_HEADER), 1, fp);
    fwrite(&info_header, sizeof(BMP_INFO_HEADER), 1, fp);
    fwrite(rgb_data, pixel_data_size, 1, fp);

    printf("创建BMP文件[%s]成功\n", filename);

    fclose(fp);
    free(rgb_data);
}
