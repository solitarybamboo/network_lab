#ifndef __MQ2_H__
#define __MQ2_H__
#include "word.h"


//串口所对应的文件名
#define COM2 "/dev/ttySAC1"
#define COM3 "/dev/ttySAC2"
#define COM4 "/dev/ttySAC3"

extern volatile int flag_screen;  // 声明全局 flag

int init_serial_mq2(const char *file, int baudrate);

void myread(char *buf,int len);

void* get_mq2_data(void* arg);

void send_cmd(void);

void recv_data(void);


#endif 
