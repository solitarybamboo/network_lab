#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "lcd.h"
#include "bmp.h"
#include "touch.h"
#include "voicectl.h"
#include "word.h"



int main(int argc ,const char * argv[])
{
    //1.初始化屏幕
    int lcd_fd;
    int *plcd;
    plcd = lcd_init(&lcd_fd);
    
    show_tem(28.98, 240, 240);
    // voice(argv[1]);
 
    lcd_uninit(lcd_fd, plcd);

    return 0;
}



