#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int fd = -1;
int *plcd = NULL;

/*
    lcd_init:初始化LCD屏幕
    @lcd_fd:LCD文件描述符
    @plcd:屏幕映射区域的首地址
    返回值：
        void
*/
void *lcd_init(int *lcd_fd)
{
    int fd = open("/dev/fb0", O_RDWR);
    if(fd == -1)
    {
        printf("open lcd fail!\n");
        return NULL;
    }
    *lcd_fd = fd;

    void *plcd = mmap(NULL, 800*480*4, PROT_READ | PROT_WRITE,
                MAP_SHARED, fd, 0);
    
    return plcd;
}

// void lcd_init(int *lcd_fd, int **plcd)
// {
//     int fd = open("/dev/fb0", O_RDWR);
//     if(fd == -1)
//     {
//         printf("open lcd fail!\n");
//         return ;
//     }
//     *lcd_fd = fd;

//     *plcd = mmap(NULL, 800*480*4, PROT_READ | PROT_WRITE,
//                 MAP_SHARED, fd, 0);
// }

/*
    lcd_draw_point:lcd屏幕画一个像素点
    @x:x坐标
    @y：y坐标
    @color:像素点的颜色
    @plcd:z屏幕映射区域的首地址
    返回值：
        int 
        失败 -1
        成功 0
*/
int lcd_draw_point(int x, int y, int color, int *plcd)
{
    if(plcd == NULL)
    {
        printf("plcd is NULL!\n");
        return -1;
    }

    if((x>=0)&&(x<800)&&(y>=0)&&(y<480))
    {
        *(plcd+y*800+x) = color;
    }
    else
    {
        return -1;
    }

    return 0;

}

/*
    lcd_draw_rectangle:画矩形函数
    @x:矩形起始坐标x
    @y:矩形起始坐标y
    @width:矩形的宽
    @heigth:矩形的高
    @color:需要显示的颜色
    @plcd:LCD映射区域的首地址
    返回值：
        int 
        失败 -1
        成功 0
*/
int lcd_draw_rectangle(int x, int y, int width, int height, int color, int *plcd)
{
    if((x<0) || (x>=800) || (y<0) || (y>=480))
    {
        printf("lcd_draw_rectangle parameters input error!\n");
        return -1;
    }

    if((x+width>=800) || (y+height)>=479)
    {
        printf("lcd_draw_rectangle parameters input error!\n");
        return -1;
    }

    if(plcd == NULL)
    {
        printf("lcd_draw_rectangle parameters input error!\n");
        return -1;
    }

    for(int i=0; i<height; i++)
    {
        for(int j=0; j<width; j++)
        {
            *(plcd+ (y+i)*800 + x+j) = color;
        }
    }

    return 0;

}

/*
    lcd_draw_sircle:LCD屏幕上画圆
    @x:圆心的x坐标
    @y:圆心的y坐标
    @r:圆的半径
    @color:圆的颜色
    @plcd:屏幕映射区域的首地址
    返回值：
        int 
        成功0
        失败-1
*/
int lcd_draw_circle(int x, int y, int r, int color, int *plcd)
{
    if((x-r<0) || (x+r>=800) || (y-r<0) || (y+r>=479))
    {
        printf("function lcd_draw_circle parameters input error!\n");
        return -1;
    }

    if(plcd == NULL)
    {
        printf("function lcd_draw_circle parameters input error!\n");
        return -1;
    }

    for(int i=y-r; i<y+r; i++)
    {
        for(int j=x-r; j<x+r; j++)
        {
            if((x-j)*(x-j) + (y-i)*(y-i) <= r*r)
            {
                lcd_draw_point(j, i, color, plcd);
            }
        }
    }
}

void display_point(int x,int y,int color)
{
    if(0<= x && x<800 && 0<= y && y<480)
    {
        *(plcd + 800 * y + x) = color;
    }
}

/*
   lcd_ uninit:解初始化屏幕
   @lcd_fd:LCD文件描述符
   @plcd:映射区域的首地址
   返回值：
    void
*/
void lcd_uninit(int lcd_fd, int *plcd)
{
    munmap(plcd, 800*480*4);
    close(lcd_fd);
}