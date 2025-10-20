#include "common.h"

#define REC_CMD  "arecord -d3 -c1 -r16000 -traw -fS16_LE cmd.pcm"
#define PCM_FILE "./cmd.pcm"
/* -d：录音时长（duration）
-c：音轨（channels）
-r：采样频率（rate）
-t：封装格式（type）
-f：量化位数（format） */

void catch (int sig)
{
    if (sig == SIGPIPE)
    {
        printf("killed by SIGPIPE\n");
        exit(0);
    }
}

// 原有的voice函数保留（如果需要单独测试使用）
int voice(const char * ipaddr)
{
    signal(SIGPIPE, catch);
    // 初始化TCP客户端套接字，用于连接到语音识别服务器(即ubuntu)
    int sockfd_tcp = init_tcp_socket(ipaddr);

    int id_num = -1; // 识别后的指令id
    while (1)
    {
        // 1，调用arecord来录一段音频
        printf("please to start REC in 3s...\n");

        // 在程序中执行一条命令  "录音的命令"
        system(REC_CMD);

        // 2，将录制好的PCM音频发送给语音识别引擎
        send_pcm(sockfd_tcp, PCM_FILE);

        // 3，等待对方回送识别结果（字符串ID）
        xmlChar* id = wait4id(sockfd_tcp);
        if (id == NULL)
        {
            continue;
        }
        id_num = atoi((char*)id);
        if (id_num == 999)
        {
            printf("bye-bye!\n");
            return 999;
        }
        printf("recv id: %d \n", id_num);
    }

    return 0;
}