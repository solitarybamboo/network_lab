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
void show_tem(double temperature, int center_x, int center_y)
{
    int w = 16;      // 字模宽
    int h = 16;      // 字模高
    int spacing = 4; // 字符间距
    int x, y;

    // 1️⃣ 判断负号
    int is_negative = 0;
    if (temperature < 0) {
        is_negative = 1;
        temperature = -temperature;
    }

    // 2️⃣ 分离整数和小数部分（保留两位）
    int integer = (int)temperature;
    int decimal = (int)((temperature - integer) * 100); 

    // 3️⃣ 处理整数部分字符串
    char buf_int[10];
    sprintf(buf_int, "%d", integer);
    int num_len = 0;
    for (int i = 0; buf_int[i] != '\0'; i++) num_len++;

    // 4️⃣ 处理小数部分两位
    char buf_dec[3];
    sprintf(buf_dec, "%02d", decimal); // 确保两位

    // 5️⃣ 总字符数 = 负号 + 整数 + 小数点 + 小数两位
    int char_count = num_len + 1 + 2 + (is_negative ? 1 : 0);

    // 6️⃣ 计算居中起始坐标
    int total_width = char_count * w + (char_count - 1) * spacing;
    int x0 = center_x - total_width / 2;
    int y0 = center_y - h / 2;

    x = x0;
    y = y0;

    // 7️⃣ 显示负号
    if (is_negative) {
        word_display(word[12], x, y, w, h);
        x += w + spacing;
    }

    // 8️⃣ 显示整数部分
    for (int i = 0; buf_int[i] != '\0'; i++) {
        int num = buf_int[i] - '0';
        word_display(word[num], x, y, w, h);
        x += w + spacing;
    }

    // 9️⃣ 显示小数点
    word_display(word[13], x, y, w, h);
    x += w + spacing;

    // 🔟 显示两位小数
    for (int i = 0; i < 2; i++) {
        int num = buf_dec[i] - '0';
        word_display(word[num], x, y, w, h);
        x += w + spacing;
    }
}



void* get_gy39_data()
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
    

        parse_data(data, i); //解析(处理)这一帧GY39的数据
        sleep(2);

    }

    close(fd);
}