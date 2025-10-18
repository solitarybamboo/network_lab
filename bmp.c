#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "lcd.h"

/*
    show_bmp:显示bmp图片
    @bmp_path:需要显示的图片路径
    @x_0:图片显示的起始x坐标
    @y_0:图片显示的起始y坐标
    @plcd:映射区域的首地址
    返回值：
        int
        成功 0
        失败 -1
*/
int show_bmp(char *bmp_path, int x_0, int y_0, int *plcd)
{
    //1.打开图片
    int bmp_fd = open(bmp_path, O_RDONLY);
    if(bmp_fd == -1)
    {
        printf("open %s fail!\n", bmp_path);
        return -1;
    }

    //2.获取图片的宽、高、色深
    //获取宽
    int width = 0;
    lseek(bmp_fd, 0x12, SEEK_SET);
    read(bmp_fd, &width, 4);

    //获取高
    int heigth = 0;
    lseek(bmp_fd, 0x16, SEEK_SET);
    read(bmp_fd, &heigth, 4);

    //获取色深
    short depth = 0;
    lseek(bmp_fd, 0x1C, SEEK_SET);
    read(bmp_fd, &depth, 2);

    //3.获取像素数组
    int line_effctive_bytes = abs(width)*(depth/8);//一行的有效字节数
    int line_total_bytes = 0;//一行的总字节数
    int laizi = 0;//赖子数
    if(line_effctive_bytes % 4 != 0)
    {
        laizi = 4-line_effctive_bytes%4;
    }
    line_total_bytes = line_effctive_bytes + laizi;

    int total_bytes = line_total_bytes * abs(heigth);//像素数组的大小
    unsigned char piexl_arr[total_bytes];
    lseek(bmp_fd, 0x36, SEEK_SET);
    read(bmp_fd, piexl_arr, total_bytes);

    //4.把像素点一一显示在屏幕上，考虑小端模式存放
    int a, r, g, b;
    int i=0;
    int color;

    for(int y=0; y<abs(heigth); y++)
    {
        for(int x=0; x<abs(width); x++)
        {
            b = piexl_arr[i++];
            g = piexl_arr[i++];
            r = piexl_arr[i++];
            if(depth == 32)
            {
                a = piexl_arr[i++];
            }
            else
            {
                a = 0;
            }
            color = (a<<24) | (r<<16) | (g<<8) | b;
            lcd_draw_point(width>0?x_0+x:x_0+abs(width)-1-x, 
                        heigth>0?y_0+abs(heigth)-1-y:y_0+y, color, plcd);
        }

        i+=laizi;
    }

    //5.关闭图片文件
    close(bmp_fd);
    return 0;
}