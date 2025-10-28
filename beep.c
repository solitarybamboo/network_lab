#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>//头文件。
#include <unistd.h>



void open_beep(void)
{
    int fd = open("/sys/kernel/gec_ctrl/beep",O_RDWR);
    if(fd == -1)
    {
        perror("open beep file failed\n");
        return;
    }
    
    int k;//保存当前需要写入到文件中的数据
    k = 1;
    write(fd,&k,4);
    close(fd);
}

void close_beep(void)
{
    int fd = open("/sys/kernel/gec_ctrl/beep",O_RDWR);
    if(fd == -1)
    {
        perror("close beep file failed\n");
        return;
    }
    
    int k;//保存当前需要写入到文件中的数据
    k = 0;
    write(fd,&k,4);
    close(fd);
}

void open_led(void)
{
    int fd = open("/sys/kernel/gec_ctrl/led_all",O_RDWR);
    if(fd == -1)
    {
        perror("open led file failed\n");
        return ;
    }
    int k;//保存当前需要写入到文件中的数据
    k = 1;
    write(fd,&k,4);
    close(fd);
}

void close_led(void)
{
    int fd = open("/sys/kernel/gec_ctrl/led_all",O_RDWR);
    if(fd == -1)
    {
        perror("close led file failed\n");
        return;
    }
    
    int k;//保存当前需要写入到文件中的数据
    k = 0;
    write(fd,&k,4);
    close(fd);
}