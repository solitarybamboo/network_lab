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



int main(int argc ,const char * argv[])
{
    //1.初始化屏幕
    int lcd_fd;
    int *plc;
    printf("4\n");
    plc = lcd_init(&lcd_fd);
    printf("1\n");
    show_tem(28.98, 240, 240);
    // voice(argv[1]);
    printf("2\n");
 
    lcd_uninit(lcd_fd, plc);
    printf("3\n");

    return 0;
}



