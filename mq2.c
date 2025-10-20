#include <fcntl.h> 
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdio.h>
#include "mq2.h"
#include "beep.h"
#include <sys/select.h>  // 添加到文件开头

static int fd=-1;


int init_serial_mq2(const char *file, int baudrate)
{
	
	if(fd==-1)  fd = open(file, O_RDWR);
	if (fd == -1)
	{
		perror("open device error:");
		return -1;
	}

	struct termios myserial;
	//清空结构体
	memset(&myserial, 0, sizeof (myserial));
	//O_RDWR               
	myserial.c_cflag |= (CLOCAL | CREAD);
	//设置控制模式状态，本地连接，接受使能
	//设置 数据位
	myserial.c_cflag &= ~CSIZE;   //清空数据位
	myserial.c_cflag &= ~CRTSCTS; //无硬件流控制
	myserial.c_cflag |= CS8;      //数据位:8

	myserial.c_cflag &= ~CSTOPB;//   //1位停止位
	myserial.c_cflag &= ~PARENB;  //不要校验
	//myserial.c_iflag |= IGNPAR;   //不要校验
	//myserial.c_oflag = 0;  //输入模式
	//myserial.c_lflag = 0;  //不激活终端模式

	switch (baudrate)
	{
		case 9600:
			cfsetospeed(&myserial, B9600);  //设置波特率
			cfsetispeed(&myserial, B9600);
			break;
		case 115200:
			cfsetospeed(&myserial, B115200);  //设置波特率
			cfsetispeed(&myserial, B115200);
			break;
		case 19200:
			cfsetospeed(&myserial, B19200);  //设置波特率
			cfsetispeed(&myserial, B19200);
			break;
        case 38400:
        	cfsetospeed(&myserial, B38400);  //设置波特率
			cfsetispeed(&myserial, B38400);
			break;

	}
	
	/* 刷新输出队列,清除正接受的数据 */
	tcflush(fd, TCIFLUSH);

	/* 改变配置 */
	tcsetattr(fd, TCSANOW, &myserial);

	return fd;
}

void* get_mq2_data(void* arg)
{
    // printf("[MQ2] 线程启动\n");
    
    if(fd == -1) {
        fd = init_serial_mq2(COM4, 9600);
    }
    if (fd == -1)
    {
        printf("[MQ2] 串口初始化失败\n");
        return NULL;
    }
    // printf("[MQ2] 串口初始化成功, fd=%d\n", fd);

    int loop_count = 0;
    while (1)
    {
        loop_count++;
        // printf("[MQ2] 循环 %d 次\n", loop_count);
        
        send_cmd();
        // printf("[MQ2] 命令已发送\n");
        sleep(1);
        
        recv_data();
        // printf("[MQ2] 数据已接收\n");
        sleep(1);
    }
    
    return NULL;
}

void send_cmd(void)
{
	char cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
	write(fd,cmd,9);
}

void recv_data(void)
{
    char buf_yanwu[9] = {0};
    
    // printf("[MQ2] 开始读取数据...\n");
    myread(buf_yanwu, 9);
    // printf("[MQ2] 数据读取完成\n");

    // 打印原始数据
    // printf("[MQ2] 原始数据: ");
    for (int i = 0; i < 9; i++) {
        printf("%02x ", (unsigned char)buf_yanwu[i]);
    }
    printf("\n");

    // 计算烟雾值
    int ilux = (unsigned char)buf_yanwu[2] << 8 | (unsigned char)buf_yanwu[3];
    // printf("[MQ2] 烟雾值=%d, flag_screen=%d\n", ilux, flag_screen);
    
    // 显示逻辑
    if(flag_screen == 1) {
        // printf("[MQ2] 在主界面，准备显示烟雾值\n");
        show_tem_yw(ilux, 195, 45);
        // printf("[MQ2] 烟雾值已显示\n");
    } else {
        // printf("[MQ2] 不在主界面(flag_screen=%d)，跳过显示\n", flag_screen);
    }
    
    // 报警逻辑
    if(ilux > 230 && flag_screen == 1) {
        // printf("[MQ2] 烟雾浓度超标！触发报警\n");
        for(int i = 0; i < 5; i++) {
            open_beep();
            sleep(1);
            close_beep();
            sleep(1);
        }
    }
}


void myread(char *buf, int len)
{
    int ret = 0;
    int total = 0;
    int timeout_count = 0;
    
    // printf("[MQ2] myread: 需要读取 %d 字节\n", len);
    
    while(total < len)
    {
        // 设置超时
        fd_set readfds;
        struct timeval timeout;
        
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        
        timeout.tv_sec = 2;   // 2秒超时
        timeout.tv_usec = 0;
        
        int select_ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
        
        if (select_ret < 0) {
            perror("[MQ2] select错误");
            break;
        } else if (select_ret == 0) {
            // 超时
            timeout_count++;
            printf("[MQ2] 读取超时 (%d/%d)，已读取 %d/%d 字节\n", 
                   timeout_count, 3, total, len);
            
            if (timeout_count >= 3) {
                printf("[MQ2] 多次超时，放弃读取\n");
                break;
            }
            continue;
        }
        
        // 有数据可读
        ret = read(fd, buf + total, len - total);
        
        if(ret < 0) {
            perror("[MQ2] read错误");
            break;
        } else if(ret == 0) {
            printf("[MQ2] read返回0\n");
            break;
        }
        
        // printf("[MQ2] 本次读取 %d 字节，累计 %d/%d\n", ret, total + ret, len);
        total += ret;
    }
    
    // printf("[MQ2] myread完成，共读取 %d 字节\n", total);
}

