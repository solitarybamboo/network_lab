#ifndef MPLAYER_H
#define MPLAYER_H

#include <stdio.h>

// 启动MPlayer进程
int start_mplayer(const char* filename);

// 发送命令到MPlayer
void send_command(const char* cmd);

// 控制函数
void play_pause(void);
void seek_forward(int seconds);
void seek_backward(int seconds);
void set_volume(int volume);
void quit_mplayer(void);
void print_help(void);
void video_play(const char *filename);

#endif