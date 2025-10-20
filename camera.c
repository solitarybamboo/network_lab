#include "camera.h"
#include "camera_screen.h"
#include "common.h"

#define SCREENSIZE 800*480*4
#define MIN(a, b) ((a) < (b) ? (a) : (b))

extern volatile int flag_camera_run;

// 全局变量
int redoffset, greenoffset, blueoffset;
static int fd;
extern int *plc;
int camfd;
int lcd;
struct fb_var_screeninfo lcdinfo;
uint8_t *fb;
int SCREEN_W, SCREEN_H;
int CAMERA_W, CAMERA_H;
int R[256][256];
int G[256][256][256];
int B[256][256];
sem_t s;
uint8_t *gyuv;

// 添加全局变量用于资源管理
static uint8_t *g_start[3] = {NULL, NULL, NULL};
static int g_length[3] = {0};
static int g_nbuf = 3;

// ======================== 显示函数 ========================
void display_cam(uint8_t *yuv)
{
    gyuv = yuv;

    uint8_t Y0, U, Y1, V;
    int w = MIN(SCREEN_W, CAMERA_W);
    int h = MIN(SCREEN_H, CAMERA_H);

    uint8_t *fbtmp = fb;
    int yuv_offset, lcd_offset;
    for(int y=0; y<h; y++)
    {
        for(int x=0; x<w; x+=2)
        {
            yuv_offset = (CAMERA_W*y + x) * 2;
            lcd_offset = (SCREEN_W*y + x) * 4;
            
            Y0 = *(yuv + yuv_offset + 0);
            U  = *(yuv + yuv_offset + 1);
            Y1 = *(yuv + yuv_offset + 2);
            V  = *(yuv + yuv_offset + 3);

            *(fbtmp + lcd_offset + redoffset  +0) = R[Y0][V];
            *(fbtmp + lcd_offset + greenoffset+0) = G[Y0][U][V];
            *(fbtmp + lcd_offset + blueoffset +0) = B[Y0][U];

            *(fbtmp + lcd_offset + redoffset  +4) = R[Y1][V];
            *(fbtmp + lcd_offset + greenoffset+4) = G[Y1][U][V];
            *(fbtmp + lcd_offset + blueoffset +4) = B[Y1][U];
        }
    }
}

// ======================== 封装函数 ========================
void camera_display(const char *dev_name)
{
    // 使用局部变量避免干扰主程序的 LCD
    int cam_lcd;
    struct fb_var_screeninfo cam_lcdinfo;
    uint8_t *cam_fb;
    
    // ---------- 初始化 LCD ----------
    cam_lcd = open("/dev/fb0", O_RDWR);
    if(cam_lcd == -1)
    {
        perror("open /dev/fb0 failed");
        return;
    }

    ioctl(cam_lcd, FBIOGET_VSCREENINFO, &cam_lcdinfo);
    SCREEN_W = cam_lcdinfo.xres;
    SCREEN_H = cam_lcdinfo.yres;
    cam_fb = mmap(NULL, cam_lcdinfo.xres * cam_lcdinfo.yres * cam_lcdinfo.bits_per_pixel / 8,
              PROT_READ | PROT_WRITE, MAP_SHARED, cam_lcd, 0);
    if(cam_fb == MAP_FAILED)
    {
        perror("mmap failed");
        close(cam_lcd);
        return;
    }
    
    // 使用全局 fb 指针供 display_cam 使用
    fb = cam_fb;
    bzero(fb, cam_lcdinfo.xres * cam_lcdinfo.yres * 4);

    redoffset  = cam_lcdinfo.red.offset/8;
    greenoffset= cam_lcdinfo.green.offset/8;
    blueoffset = cam_lcdinfo.blue.offset/8;

    // ---------- 启动颜色转换线程 ----------
    pthread_t tide;
    pthread_create(&tide, NULL, convert, NULL);
    usleep(100000);  // 等待转换表生成

    // ---------- 打开摄像头 ----------
    camfd = open(dev_name, O_RDWR);
    if(camfd == -1)
    {
        printf("open \"%s\" failed: %s\n", dev_name, strerror(errno));
        munmap(cam_fb, cam_lcdinfo.xres * cam_lcdinfo.yres * 4);
        close(cam_lcd);
        return;
    }

    printf("\n摄像头的基本参数：\n");
    get_camcap(camfd);
    get_camfmt(camfd);
    get_caminfo(camfd);
    set_camfmt(camfd);
    get_camfmt(camfd);

    CAMERA_W = fmt.fmt.pix.width;
    CAMERA_H = fmt.fmt.pix.height;

    // ---------- 申请缓存 ----------
    // 将 buffer 定义提前，避免 goto 跳过变长数组初始化
    struct v4l2_buffer buffer[3];  // 固定大小为3
    
    struct v4l2_requestbuffers reqbuf;
    bzero(&reqbuf, sizeof(reqbuf));
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    reqbuf.count = g_nbuf;
    if(ioctl(camfd, VIDIOC_REQBUFS, &reqbuf) == -1)
    {
        printf("VIDIOC_REQBUFS failed\n");
        goto cleanup_cam;
    }

    for(int i=0; i<g_nbuf; i++)
    {
        bzero(&buffer[i], sizeof(buffer[i]));
        buffer[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer[i].memory = V4L2_MEMORY_MMAP;
        buffer[i].index = i;
        
        if(ioctl(camfd, VIDIOC_QUERYBUF, &buffer[i]) == -1)
        {
            printf("VIDIOC_QUERYBUF failed for buffer %d\n", i);
            goto cleanup_buffers;
        }
        
        g_length[i] = buffer[i].length;
        g_start[i] = mmap(NULL, buffer[i].length, PROT_READ | PROT_WRITE,
                        MAP_SHARED, camfd, buffer[i].m.offset);
        
        if(g_start[i] == MAP_FAILED)
        {
            printf("mmap failed for buffer %d\n", i);
            g_start[i] = NULL;
            goto cleanup_buffers;
        }
        
        ioctl(camfd, VIDIOC_QBUF, &buffer[i]);
    }

    // ---------- 启动采集 ----------
    enum v4l2_buf_type vtype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(camfd, VIDIOC_STREAMON, &vtype) == -1)
    {
        printf("VIDIOC_STREAMON failed\n");
        goto cleanup_buffers;
    }

    struct v4l2_buffer v4lbuf;
    bzero(&v4lbuf, sizeof(v4lbuf));
    v4lbuf.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4lbuf.memory= V4L2_MEMORY_MMAP;

    // ---------- 主循环 ----------
    int i = 0;
    while(flag_camera_run)
    {
        v4lbuf.index = i % g_nbuf;
        if(ioctl(camfd, VIDIOC_DQBUF, &v4lbuf) == -1)
        {
            if(errno == EINTR) continue;  // 被信号中断，继续
            printf("VIDIOC_DQBUF failed: %s\n", strerror(errno));
            break;
        }

        display_cam(g_start[i % g_nbuf]);

        v4lbuf.index = i % g_nbuf;
        ioctl(camfd, VIDIOC_QBUF, &v4lbuf);
        i++;
    }

    // ---------- 清理资源 ----------
    vtype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(camfd, VIDIOC_STREAMOFF, &vtype);

cleanup_buffers:
    for(int i=0; i<g_nbuf; i++)
    {
        if(g_start[i] != NULL && g_start[i] != MAP_FAILED)
        {
            munmap(g_start[i], g_length[i]);
            g_start[i] = NULL;
        }
    }

cleanup_cam:
    close(camfd);
    camfd = -1;
    
    munmap(cam_fb, cam_lcdinfo.xres * cam_lcdinfo.yres * 4);
    close(cam_lcd);
    fb = NULL;  // 重要：清空全局指针
    
    printf("摄像头资源已完全释放\n");
}