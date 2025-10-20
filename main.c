#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "lcd.h"
#include "bmp.h"
#include "touch.h"
#include "voicectl.h"
#include "word.h"
#include "mplayer.h"
#include "gy39.h"
#include "ts.h"
#include "beep.h"
#include "mq2.h"
#include "camera.h"
#include "camera_screen.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#define REC_CMD  "arecord -d3 -c1 -r16000 -traw -fS16_LE cmd.pcm"
#define PCM_FILE "./cmd.pcm"

void clean_screen(void);
void main_screen(void);
void camera_screen(void);
void vedio_screen(void);
void *touch_thread(void *arg);
void *voice_thread(void *arg);
void *data_process(void *arg);

extern int *plc;
extern volatile int flag_tq;
volatile int flag_screen = -1;
volatile int flag_led = -1;
volatile int flag_lock = -1;
volatile int flag_camera_run = 0;
volatile int flag_video_run = 0;
volatile int flag_video_paused = 0;
volatile int flag_voice_exit = 0;  // 语音线程退出标志
volatile double tm_data_1 = 0;

pthread_t t_camera;
pthread_t t_touch;
pthread_t t_voice;
pthread_t tm_data;

int main(int argc, const char *argv[])
{
    int lcd_fd;
    plc = lcd_init(&lcd_fd);
    if (!plc) {
        printf("lcd init fail!\n");
        return -1;
    }

    // 实时获取传感器数据
    pthread_t t_tm;
    int ret_tm = pthread_create(&t_tm, NULL, get_gy39_data, NULL);
    printf("GY39线程创建: %s\n", ret_tm == 0 ? "成功" : "失败");
    
    pthread_t t_mq;
    int ret_mq = pthread_create(&t_mq, NULL, get_mq2_data, NULL);
    printf("MQ2线程创建: %s\n", ret_mq == 0 ? "成功" : "失败");
    
    // 创建语音控制线程
    int ret_voice = pthread_create(&t_voice, NULL, voice_thread, NULL);
    printf("语音控制线程创建: %s\n", ret_voice == 0 ? "成功" : "失败");

    int ret_data = pthread_create(&tm_data, NULL, data_process, NULL);
    printf("语音控制线程创建: %s\n", ret_data == 0 ? "成功" : "失败");
    
    clean_screen();

    char *bmp_path = "./pic/beijing.bmp";
    int x_0 = 0;
    int y_0 = 0;
    bmp_display(bmp_path, x_0, y_0);
    bmp_display("./pic/switch_off.bmp", 640, 120);
    bmp_display("./pic/switch_off.bmp", 640, 240);
    if(flag_tq == 0) {
        bmp_display("./pic/rain.bmp", 100, 100);
    }
    else if(flag_tq == 1) {
        bmp_display("./pic/sun.bmp", 100, 100);
    }
    close_led();
    main_screen();

    // 程序退出时设置语音线程退出标志
    flag_voice_exit = 1;
    
    pthread_join(t_tm, NULL);
    pthread_join(t_mq, NULL);
    pthread_join(t_voice, NULL);
    lcd_uninit(lcd_fd, plc);
    return 0;
}

void clean_screen(void) {
    for(int i = 0; i < 480; i++) {
        for(int j = 0; j < 800; j++) {
            display_point(j, i, 0xffffff);
        }
    }
}

void camera_screen(void)
{
    printf("进入摄像头界面\n");
    flag_screen = 2;
    flag_camera_run = 1;

    // 创建触摸线程处理退出
    pthread_t t_touch;
    pthread_create(&t_touch, NULL, touch_thread, NULL);

    // 清空屏幕
    for(int i = 0; i < 480; i++) {
        for(int j = 0; j < 800; j++) {
            display_point(j, i, 0x000000);  // 黑色背景
        }
    }
    
    // 绘制退出按钮区域提示（红色区域）
    for(int i=0; i<80; i++) {
        for(int j=700; j<800; j++) {
            display_point(j, i, 0xFF0000);
        }
    }

    // 阻塞显示摄像头（直到 flag_camera_run=0）
    camera_display("/dev/video7");

    // 等待触摸线程结束
    pthread_join(t_touch, NULL);

    printf("返回主界面\n");
    flag_screen = 1;
    
    // 短暂延迟，确保资源完全释放
    usleep(100000);
    
    // 重新绘制主界面
    for(int i = 0; i < 480; i++) {
        for(int j = 0; j < 800; j++) {
            display_point(j, i, 0xffffff);
        }
    }
    
    bmp_display("./pic/beijing.bmp", 0, 0);
    
    // 根据当前状态恢复开关显示
    if(flag_led == 1) {
        bmp_display("./pic/switch_on.bmp", 640, 120);
    } else {
        bmp_display("./pic/switch_off.bmp", 640, 120);
    }
    
    if(flag_lock == 1) {
        bmp_display("./pic/switch_on.bmp", 640, 240);
    } else {
        bmp_display("./pic/switch_off.bmp", 640, 240);
    }
}

void vedio_screen(void)
{
    printf("进入视频播放界面\n");
    flag_screen = 3;
    flag_video_run = 1;
    flag_video_paused = 0;  // 初始化为播放状态

    // 创建触摸线程处理控制
    pthread_t t_touch;
    pthread_create(&t_touch, NULL, touch_thread, NULL);

    // 清空屏幕
    for(int i = 0; i < 480; i++) {
        for(int j = 0; j < 800; j++) {
            display_point(j, i, 0x000000);  // 黑色背景
        }
    }

    // 显示初始播放图标
    bmp_display("./pic/list_start.bmp", 0, 0);  // 左上角显示播放图标

    // 播放视频
    video_play("./vedio/2.mp4");

    // 等待触摸线程结束
    pthread_join(t_touch, NULL);

    printf("返回主界面\n");
    flag_screen = 1;
    flag_video_paused = 0;  // 重置状态
    
    // 短暂延迟，确保资源完全释放
    usleep(100000);
    
    // 重新绘制主界面
    for(int i = 0; i < 480; i++) {
        for(int j = 0; j < 800; j++) {
            display_point(j, i, 0xffffff);
        }
    }
    
    bmp_display("./pic/beijing.bmp", 0, 0);
    
    // 根据当前状态恢复开关显示
    if(flag_led == 1) {
        bmp_display("./pic/switch_on.bmp", 640, 120);
    } else {
        bmp_display("./pic/switch_off.bmp", 640, 120);
    }
    
    if(flag_lock == 1) {
        bmp_display("./pic/switch_on.bmp", 640, 240);
    } else {
        bmp_display("./pic/switch_off.bmp", 640, 240);
    }
}

void *touch_thread(void *arg)
{
    ts_point pe;

    while (flag_screen == 2) {  // 摄像头界面
        get_ts_point(&pe);
        
        if (pe.x > 700 && pe.x < 800 && pe.y > 0 && pe.y < 80) {
            printf("检测到摄像头退出点击\n");
            flag_camera_run = 0;
            break;
        }
        
        usleep(50000);
    }

    // 为视频播放界面添加触摸处理
    while (flag_screen == 3) {  // 视频播放界面
        get_ts_point(&pe);
        
        // 左上角暂停/播放区域 (0~100, 0~100)
        if (pe.x >= 200 && pe.x <= 400 && pe.y >= 380 && pe.y <= 480) {
            printf("点击暂停/播放区域\n");
            play_pause();
            
            // 切换暂停状态
            flag_video_paused = !flag_video_paused;
            
            // 显示对应图标
            if (flag_video_paused) {
                printf("显示暂停图标\n");
                bmp_display("./pic/list_pause.bmp", 0, 380);
            } else {
                printf("显示播放图标\n");
                bmp_display("./pic/list_start.bmp", 0, 380);
            }
        }
        // 右上角快进区域 (700~800, 0~100)
        else if (pe.x >= 400 && pe.x <= 600 && pe.y >= 380 && pe.y <= 480) {
            printf("点击快进区域\n");
            seek_forward(10);
        }
        // 左下角后退区域 (0~100, 380~480)
        else if (pe.x >= 0 && pe.x <= 200 && pe.y >= 380 && pe.y <= 480) {
            printf("点击后退区域\n");
            seek_backward(10);
        }
        // 右下角退出区域 (700~800, 380~480)
        else if (pe.x >= 600 && pe.x <= 800 && pe.y >= 380 && pe.y <= 480) {
            printf("检测到视频退出点击\n");
            flag_video_run = 0;
            flag_screen = 1;
            break;
        }
        
        usleep(50000);
    }
    
    printf("触摸线程退出, flag_screen=%d\n", flag_screen);
    pthread_exit(NULL);
}

// 语音控制线程
void *voice_thread(void *arg)
{
    printf("语音控制线程启动\n");
    
    // 语音识别服务器IP地址（根据实际情况修改）
    const char *server_ip = "192.168.1.100";  // 修改为你的Ubuntu服务器IP
    
    signal(SIGPIPE, SIG_IGN);  // 忽略SIGPIPE信号
    
    int sockfd_tcp = -1;
    int id_num = -1;
    int reconnect_count = 0;
    
    while (!flag_voice_exit) {
        // 只在主界面时进行语音识别
        if(flag_screen != 1) {
            usleep(500000);  // 非主界面时休眠
            continue;
        }
        
        // 检查连接状态，如果未连接则尝试连接
        if(sockfd_tcp < 0) {
            printf("正在连接语音识别服务器...\n");
            sockfd_tcp = init_tcp_socket(server_ip);
            if(sockfd_tcp < 0) {
                printf("连接失败，3秒后重试\n");
                usleep(3000000);
                continue;
            }
            printf("语音识别服务器连接成功\n");
            reconnect_count = 0;
        }
        
        printf("请开始语音输入(3秒)...\n");
        
        // 录音
        system(REC_CMD);
        
        // 发送音频数据
        send_pcm(sockfd_tcp, PCM_FILE);
        
        // 等待识别结果
        xmlChar* id = wait4id(sockfd_tcp);
        if (id == NULL) {
            printf("识别失败，关闭连接准备重连\n");
            close(sockfd_tcp);
            sockfd_tcp = -1;
            usleep(1000000);
            continue;
        }
        
        id_num = atoi((char*)id);
        printf("识别ID: %d\n", id_num);
        xmlFree(id);  // 释放内存
        
        // 根据ID执行对应操作
        switch(id_num) {
            case 1:  // 开灯
                printf("语音指令: 开灯\n");
                if(flag_led != 1) {
                    flag_led = 1;
                    open_led();
                    bmp_display("./pic/switch_on.bmp", 640, 120);
                }
                break;
                
            case 2:  // 关灯
                printf("语音指令: 关灯\n");
                if(flag_led != -1) {
                    flag_led = -1;
                    close_led();
                    bmp_display("./pic/switch_off.bmp", 640, 120);
                }
                break;
                
            case 3:  // 开锁
                printf("语音指令: 开空调\n");
                if(flag_lock != 1) {
                    flag_lock = 1;
                    bmp_display("./pic/switch_on.bmp", 640, 240);
                }
                break;
                
            case 4:  // 关锁
                printf("语音指令: 关空调\n");
                if(flag_lock != -1) {
                    flag_lock = -1;
                    bmp_display("./pic/switch_off.bmp", 640, 240);
                }
                break;
                
            case 5:  // 打开监控/摄像头
                printf("语音指令: 打开监控\n");
                if(flag_screen == 1) {
                    camera_screen();
                }
                break;
                
            case 6:  // 打开视频
                printf("语音指令: 打开视频\n");
                if(flag_screen == 1) {
                    vedio_screen();
                }
                break;
                
            case 7:  // 返回主界面（用于从子界面返回）
                printf("语音指令: 返回主界面\n");
                if(flag_screen == 2) {
                    flag_camera_run = 0;
                } else if(flag_screen == 3) {
                    flag_video_run = 0;
                }
                break;
                
            case 999:  // 退出系统
                printf("语音指令: 退出系统\n");
                flag_voice_exit = 1;
                break;
                
            default:
                printf("未识别的指令ID: %d\n", id_num);
                break;
        }
        
        usleep(100000);  // 防止过快连续识别
    }
    
    close(sockfd_tcp);
    printf("语音控制线程退出\n");
    pthread_exit(NULL);
}

void main_screen(void) {
    flag_screen = 1;
    ts_point pt;
    display_word();
    if(flag_tq == 0) {
        bmp_display("./pic/rain.bmp", 100, 100);
    }
    else if(flag_tq == 1) {
        bmp_display("./pic/sun.bmp", 100, 100);
    }
    while(1) {
        display_word();
        if(flag_tq == 0) {
            bmp_display("./pic/rain.bmp", 100, 100);
        }
        else if(flag_tq == 1) {
            bmp_display("./pic/sun.bmp", 100, 100);
        }
        get_ts_point(&pt);
        
        // 只在主界面处理点击
        if(flag_screen != 1) {
            usleep(100000);
            continue;
        }
        
        // LED 控制区域
        if(pt.x > 520 && pt.x < 780 && pt.y > 100 && pt.y < 200) {
            flag_led = -flag_led;
            printf("flag_led = %d\n", flag_led);
            
            if(flag_led == 1) {
                open_led();
                bmp_display("./pic/switch_on.bmp", 640, 120);
            } else {
                close_led();
                bmp_display("./pic/switch_off.bmp", 640, 120);
            }
        }
        
        // 锁控制区域
        if(pt.x > 520 && pt.x < 780 && pt.y > 220 && pt.y < 320) {
            flag_lock = -flag_lock;
            printf("flag_lock = %d\n", flag_lock);
            
            if(flag_lock == 1) {
                bmp_display("./pic/switch_on.bmp", 640, 240);
            } else {
                bmp_display("./pic/switch_off.bmp", 640, 240);
            }
        }
        
        // 摄像头图标区域
        if (pt.x > 40 && pt.x < 260 && pt.y > 320 && pt.y < 460) {
            printf("点击摄像头图标\n");
            camera_screen();  // 阻塞调用，返回后已经清理完毕
            printf("摄像头界面已退出\n");
        }
        if (pt.x > 520 && pt.x < 780 && pt.y > 340 && pt.y < 440) {
            printf("点击视频图标\n");
            vedio_screen();  // 阻塞调用，返回后已经清理完毕
            printf("视频界面已退出\n");
        }
        
        usleep(50000);  // 防抖延迟
    }
}

void *data_process(void *arg) {
    while(1) {
        if(flag_lock == 1) {
            tm_data_1 -= 0.3;
        }
        else if(flag_lock == -1) {
            if(tm_data_1 < 0){
                tm_data_1 += 1;
            }
        }
        sleep(1);
    }
}

