#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "lcd.h"
#include "bmp.h"
#include "touch.h"
#include "voicectl.h"
#include "word.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "lcd.h"
#include "bmp.h"
#include "touch.h"
#include "voicectl.h"
#include "word.h"
#include "gy39.h"



extern int *plc;  // 声明全局 plc

int main(int argc ,const char * argv[])
{
    int lcd_fd;
    plc = lcd_init(&lcd_fd);  // 初始化全局 plc
    if (!plc) {
        printf("lcd init fail!\n");
        return -1;
    }

    // 实时获取传感器数据
    pthread_t t_tm;
    pthread_create(&t_tm, NULL, get_gy39_data, NULL);

    //主功能部分

    show_tem(28.98, 240, 240);

    lcd_uninit(lcd_fd, plc);
    return 0;
}




