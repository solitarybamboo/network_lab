#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "ts.h"
#include "bmp.h"

static int mplayer_pid = -1;
extern volatile int flag_screen;
extern volatile int flag_video_run;
FILE* mplayer_stdin = NULL;

// 启动MPlayer进程
int start_mplayer(const char* filename) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe创建失败");
        return -1;
    }

    mplayer_pid = fork();
    if (mplayer_pid == -1) {
        perror("fork失败");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (mplayer_pid == 0) {  // 子进程 - MPlayer
        close(pipefd[1]);  // 关闭写端
        // 将标准输入重定向到管道读端
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        
        // 启动MPlayer，-slave选项使其进入从模式，接受命令输入
        execlp("mplayer", "mplayer", "-slave", "-quiet", filename, (char*)NULL);
        perror("execlp失败，可能未安装mplayer");
        exit(EXIT_FAILURE);
    } else {  // 父进程
        close(pipefd[0]);  // 关闭读端
        mplayer_stdin = fdopen(pipefd[1], "w");
        if (!mplayer_stdin) {
            perror("fdopen失败");
            close(pipefd[1]);
            return -1;
        }
        setbuf(mplayer_stdin, NULL);  // 禁用缓冲，确保命令立即发送
        return 0;
    }
}

// 发送命令到MPlayer
void send_command(const char* cmd) {
    if (mplayer_stdin && flag_video_run) {
        fprintf(mplayer_stdin, "%s\n", cmd);
    }
}

// 控制函数
void play_pause(void) {
    send_command("pause");  // 暂停/继续切换
}

void seek_forward(int seconds) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "seek %d 0", seconds);  // 相对当前位置快进
    send_command(cmd);
}

void seek_backward(int seconds) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "seek -%d 0", seconds);  // 相对当前位置后退
    send_command(cmd);
}

void set_volume(int volume) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "volume %d 1", volume);  // 设置音量(0-100)
    send_command(cmd);
}

void quit_mplayer(void) {
    if (mplayer_pid == -1) {
        return;  // 进程已经不存在
    }
    
    // 先尝试发送quit命令
    if (mplayer_stdin) {
        printf("发送quit命令到MPlayer\n");
        fprintf(mplayer_stdin, "quit\n");
        fflush(mplayer_stdin);  // 强制刷新缓冲区
        fclose(mplayer_stdin);
        mplayer_stdin = NULL;
    }
    
    // 等待进程结束，但设置超时
    int status;
    int wait_count = 0;
    int max_wait = 20;  // 最多等待2秒（20次 * 100ms）
    
    while (wait_count < max_wait) {
        pid_t result = waitpid(mplayer_pid, &status, WNOHANG);
        
        if (result == mplayer_pid) {
            // 进程已正常结束
            printf("MPlayer进程正常结束\n");
            mplayer_pid = -1;
            return;
        } else if (result == -1) {
            // waitpid出错，进程可能已经不存在
            perror("waitpid错误");
            mplayer_pid = -1;
            return;
        }
        
        // 进程还在运行，继续等待
        usleep(100000);  // 等待100ms
        wait_count++;
    }
    
    // 超时后强制终止
    printf("MPlayer未响应quit命令，发送SIGTERM信号\n");
    kill(mplayer_pid, SIGTERM);
    usleep(500000);  // 等待500ms
    
    // 再次检查
    pid_t result = waitpid(mplayer_pid, &status, WNOHANG);
    if (result == 0) {
        // 还没结束，使用SIGKILL强制杀死
        printf("MPlayer仍未结束，发送SIGKILL信号\n");
        kill(mplayer_pid, SIGKILL);
        waitpid(mplayer_pid, &status, 0);  // 阻塞等待，SIGKILL必定成功
    }
    
    mplayer_pid = -1;
    printf("MPlayer进程已清理\n");
}

// 显示帮助信息
void print_help(void) {
    printf("\n控制命令:\n");
    printf("  p - 暂停/继续\n");
    printf("  f - 快进10秒\n");
    printf("  b - 后退10秒\n");
    printf("  + - 音量增加10\n");
    printf("  - - 音量减少10\n");
    printf("  q - 退出程序\n");
    printf("  h - 显示帮助\n");
}

// 封装视频播放函数
void video_play(const char *filename)
{
    printf("正在启动MPlayer播放 %s...\n", filename);

    int mpl = start_mplayer(filename);

    if (mpl != 0) {
        fprintf(stderr, "启动MPlayer失败\n");
        return;
    }

    print_help();
    printf("MPlayer启动成功，PID=%d\n", mplayer_pid);
    
    // 主循环，等待退出信号或MPlayer进程结束
    int status;
    int loop_count = 0;

    bmp_display("/xixi/pic/list_start.bmp", 0, 380);
    
    while (flag_video_run && (flag_screen == 3)) {
        loop_count++;
        
        // 非阻塞检查MPlayer进程状态
        pid_t result = waitpid(mplayer_pid, &status, WNOHANG);
        
        if (result == mplayer_pid) {
            // MPlayer进程已结束（播放完毕）
            printf("MPlayer进程已自然结束，循环次数=%d\n", loop_count);
            flag_video_run = 0;
            flag_screen = 1;  // 通知触摸线程退出
            mplayer_pid = -1;  // 标记进程已结束
            break;
        } else if (result == -1) {
            perror("waitpid错误");
            break;
        }
        
        // 每秒输出一次状态（调试用）
        if (loop_count % 10 == 0) {
            printf("[播放中] 循环=%d, flag_video_run=%d, flag_screen=%d\n", 
                   loop_count, flag_video_run, flag_screen);
        }
        
        usleep(100000);  // 100ms 检查一次
    }
    
    printf("退出播放循环：flag_video_run=%d, flag_screen=%d\n", 
           flag_video_run, flag_screen);
    
    // 清理MPlayer（如果还在运行）
    quit_mplayer();
    
    // 确保标志位正确设置
    flag_video_run = 0;
    flag_screen = 1;
    
    printf("视频播放结束\n");
}