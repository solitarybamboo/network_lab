#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int fd = -1;
int *plc = NULL;   // 全局 frame buffer 指针（每像素 4 字节，即 int）

#define LCD_W 800
#define LCD_H 480
#define LCD_BYTES (LCD_W * LCD_H * 4)

/*
    lcd_init: 初始化 LCD 屏幕
    返回值：
        成功 -> 全局 plc 被设置，返回 plc 指针
        失败 -> 返回 NULL
*/
int *lcd_init(int *lcd_fd)
{
    int local_fd = open("/dev/fb0", O_RDWR);
    if (local_fd == -1)
    {
        printf("open /dev/fb0 fail! errno=%d (%s)\n", errno, strerror(errno));
        return NULL;
    }

    void *map = mmap(NULL, LCD_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED, local_fd, 0);
    if (map == MAP_FAILED)
    {
        printf("mmap fail! errno=%d (%s)\n", errno, strerror(errno));
        close(local_fd);
        return NULL;
    }

    // 设置全局并返回
    fd = local_fd;
    plc = (int *)map;

    if (lcd_fd) *lcd_fd = local_fd;
    return plc;
}

/*
    lcd_draw_point: 在屏幕画一个像素点（使用传入的 plcd 指针或全局 plc）
    返回：
      0 成功，-1 失败
*/
int lcd_draw_point(int x, int y, int color, int *plcd)
{
    int *fb = plcd ? plcd : plc;  // 优先使用传入指针，否则用全局
    if (fb == NULL)
    {
        printf("lcd_draw_point: fb is NULL!\n");
        return -1;
    }

    if (x < 0 || x >= LCD_W || y < 0 || y >= LCD_H)
    {
        // 越界不绘制
        return -1;
    }

    fb[y * LCD_W + x] = color;
    return 0;
}

/*
    lcd_draw_rectangle: 画矩形
*/
int lcd_draw_rectangle(int x, int y, int width, int height, int color, int *plcd)
{
    int *fb = plcd ? plcd : plc;
    if (fb == NULL)
    {
        printf("lcd_draw_rectangle: fb is NULL!\n");
        return -1;
    }

    if (width <= 0 || height <= 0) return -1;
    if (x < 0 || x >= LCD_W || y < 0 || y >= LCD_H) return -1;
    if (x + width > LCD_W || y + height > LCD_H) return -1;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            fb[(y + i) * LCD_W + (x + j)] = color;
        }
    }
    return 0;
}

/*
    lcd_draw_circle: 在屏幕画圆
*/
int lcd_draw_circle(int cx, int cy, int r, int color, int *plcd)
{
    int *fb = plcd ? plcd : plc;
    if (fb == NULL)
    {
        printf("lcd_draw_circle: fb is NULL!\n");
        return -1;
    }

    if (r <= 0) return -1;
    if (cx - r < 0 || cx + r >= LCD_W || cy - r < 0 || cy + r >= LCD_H) return -1;

    for (int y = cy - r; y <= cy + r; y++)
    {
        for (int x = cx - r; x <= cx + r; x++)
        {
            int dx = cx - x;
            int dy = cy - y;
            if (dx*dx + dy*dy <= r*r)
            {
                fb[y * LCD_W + x] = color;
            }
        }
    }
    return 0;
}

/*
    display_point: 便捷版，使用全局 plc
*/
void display_point(int x, int y, int color)
{
    if (plc == NULL)
    {
        // 不打印堆栈信息以免阻塞，简单返回
        return;
    }
    if (x < 0 || x >= LCD_W || y < 0 || y >= LCD_H) return;
    plc[y * LCD_W + x] = color;
}

/*
   lcd_uninit: 解除映射并关闭设备
*/
void lcd_uninit(int lcd_fd, int *plcd)
{
    int *fb = plcd ? plcd : plc;
    if (fb)
    {
        munmap(fb, LCD_BYTES);
    }
    if (lcd_fd != -1)
    {
        close(lcd_fd);
    }
    // 清理全局
    plc = NULL;
    fd = -1;
}
