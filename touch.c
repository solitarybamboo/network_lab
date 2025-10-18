#include <unistd.h>
#include <stdio.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

/*
    init_touch:初始化触摸屏
    @touch_fd:保存触摸屏文件描述符
    返回值：
        int
        成功 0
        失败 -1
*/
int init_touch(int *touch_fd)
{
    int fd = open("/dev/input/event0", O_RDONLY);
    if(fd == -1)
    {
        printf("open touch fail!\n");
        return -1;
    }
    *touch_fd = fd;
    return 0;
}

/*
    get_user_input:获取用户输入
    @x:保存获取到的x坐标
    @y:保存获取到的y坐标
    @touch_fd:触摸屏文件描述符
    返回值：
        int 
        失败 -1
        成功 0
*/
int get_user_input(int *x, int *y, int touch_fd)
{
    //1.打开触摸屏文件
    // int touch_fd = open("/dev/input/event0", O_RDONLY);
    // if(touch_fd == -1)
    // {
    //     printf("open touch fail!\n");
    //     return -1;
    // }

    //2.读输入事件
    struct input_event ev;
    int flag_x = 0;
    int flag_y = 0;
    while(1)
    {
        int res = read(touch_fd, &ev, sizeof(ev));
        if(res != sizeof(ev))
        {
            continue;
        }

        //3.解析对应的标准输入事件
        if(ev.type == EV_ABS && ev.code == ABS_X)
        {
            *x = ev.value*(1.0*800/1040);
            flag_x = 1;
            if(flag_y)
            {
                break;
            }
        }
        if(ev.type == EV_ABS && ev.code == ABS_Y)
        {
            *y = ev.value*(1.0*480/600);
            flag_y = 1;
            if(flag_x)
            {
                break;
            }
        }
    }

    //4.关闭触摸屏
    // close(touch_fd);
}

/*
    judge_slider_direction:判断滑动方向
    @touch_fd:触摸屏文件描述符
    返回值：
        int
        1 上滑
        2 下滑
        3 左滑
        4 右滑
*/
int judge_slider_direction(int touch_fd)
{
    struct input_event ev;
    int x_0 = -1, y_0 = -1;
    int x_1 = -1, y_1 = -1;

    while(1)
    {
        read(touch_fd, &ev, sizeof(ev));

        if(ev.type == EV_ABS && ev.code == ABS_X)
        {
            if(x_0 == -1)//起始x坐标
            {
                x_0 = ev.value*(1.0*800/1040);
                x_1 = ev.value*(1.0*800/1040);
            }
            else//终点x坐标
            {
                x_1 = ev.value*(1.0*800/1040);
            }
        }
        if(ev.type == EV_ABS && ev.code == ABS_Y)
        {
            if(y_0 == -1)//起始x坐标
            {
                y_0 = ev.value*(1.0*480/600);
                y_1 = ev.value*(1.0*480/600);
            }
            else//终点x坐标
            {
                y_1 = ev.value*(1.0*480/600);
            }
        }

        if(ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0)
        {
            if(abs(x_1-x_0)<10 && abs(y_1-y_0)<10)//点击事件
            {
                return 0;
            }
            if(abs(y_1-y_0)>abs(x_1-x_0) && (y_1<y_0))
            {
                return 1;
            }
            if(abs(y_1-y_0)>abs(x_1-x_0) && (y_1>y_0))
            {
                return 2;
            }
            if(abs(x_1-x_0)>abs(y_1-y_0) && (x_1<x_0))
            {
                return 3;
            }
            if(abs(x_1-x_0)>abs(y_1-y_0) && (x_1>x_0))
            {
                return 4;
            }
        }
    }
}

/*
    uninit_touch:解初始化触摸屏
    @touch_fd:触摸屏文件描述符
    返回值：
        int
        成功 0
        失败 -1
*/
int uninit_touch(int touch_fd)
{
    int ret = close(touch_fd);
    if(ret == -1)
    {
        printf("close touch fail!\n");
        return -1;
    }

    return 0;
}