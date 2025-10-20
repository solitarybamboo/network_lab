#ifndef __GY39_H__
#define __GY39_H__
#include "word.h"


//串口所对应的文件名
#define COM2 "/dev/ttySAC1"
#define COM3 "/dev/ttySAC2"
#define COM4 "/dev/ttySAC3"

extern volatile int flag_screen;  // 声明全局 flag

//初始化串口
//file: 串口所对应的文件名
//baudrate：波特率
int init_serial(const char *file, int baudrate);


void parse_data(unsigned char data[], int len);

void* get_gy39_data(void* arg);
#endif 
