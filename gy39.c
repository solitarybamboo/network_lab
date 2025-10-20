#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "gy39.h"
#include "word.h"
#include "bmp.h"

extern volatile double tm_data_1;
volatile int flag_tq = 1;

//初始化串口
//file: 串口所对应的文件名
//baudrate：波特率
int init_serial(const char *file, int baudrate)
{ 
	int fd;
	
	fd = open(file, O_RDWR);
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

//解析并处理数据
void parse_data(unsigned char data[], int len)
{
    int i;

    //debug
    for (i = 0; i < len; i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\n");

    if (data[0] != 0x5A || data[1] != 0x5A)
    {
        printf(" ERROR data\n");
        return ;
    }

    int type = data[2];
    int l = data[3];
    

    if (type == 0x15)
    {
        int d =  (data[4]<< 24) | (data[5] << 16) | (data[6] << 8) | (data[7]) ;

        double lux = d/100.0;

        // printf("lux = %g\n", lux); 
        show_tem(lux, 380, 420);
        //在屏幕上合适位置上，显示这个光照强度
    }
    else if (type == 0x45)
    {
        //解析： 温度、湿度、气压、海拔
        int t = (data[4] << 8) | data[5];
        double tem = t / 100.0;
        tem += tm_data_1;
        // printf("tem = %g\n",tem);
        show_tem(tem, 150, 255);
        int p = (data[6] << 24) | (data[7] << 16) | (data[8] << 8) | data[9];
        double per = p / 10000.0;
        // printf("per = %g\n", per);
        show_tem(per, 380, 160);
        int h = (data[10] << 8) | data[11];
        double hum = h / 100.0;
        if(hum > 60) {
            flag_tq = 0;
        }
        else {
            flag_tq = 1;
        }
        if(flag_tq == 0) {
            bmp_display("./pic/rain.bmp", 100, 100);
        }
        else if(flag_tq == 1) {
            bmp_display("./pic/sun.bmp", 100, 100);
        }
        // printf("hum = %g\n", hum);
        show_tem(hum, 380, 290);
        int g = (data[12] << 8) | data[13];
        double hei = g / 100.0;
        // printf("hei = %g\n", hei);
        // show_tem(hei, 380, 160);
        // ....
    }

}


void* get_gy39_data(void* arg)
{
    int ret;
    int fd = init_serial(COM3, 9600);
    if (fd == -1)
    {
        printf("init_serial fail\n");
    }
    printf("yes\n");


    // 发送命令给  GY39模块。
    unsigned char cmd[3] = {0xA5, 0X83, 0x28};
    ret = write(fd, cmd, 3);

 

    unsigned char data[16] ={0x5A, 0x5A}; 
    int i ;


    while (1)
    {
        unsigned char ch;
        // 连续读取两个 0x5A    ,帧头
        while (1)
        {
            do
            {
                read(fd, &ch, 1);
            } while (ch != 0x5A);
            
            //
            read(fd, &ch, 1);
            if (ch != 0x5A)
            {
                continue;
            }
            else
            {
                break;
            }
        }
        i = 2;

        read(fd, &data[i++], 1); // type
        read(fd, &data[i++], 1); // len
        int len = data[i-1];
        int j;
        for (j = 0; j < len; j++)
        {
            read(fd, &data[i++], 1);
        }

        read(fd, &data[i++], 1); //checksum
    
        if(flag_screen == 1) {
            parse_data(data, i); //解析(处理)这一帧GY39的数据
        }
        sleep(2);

    }

    close(fd);
}