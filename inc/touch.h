#ifndef __TOUCH_H__
#define __TOUCH_H__

int init_touch(int *touch_fd);
int get_user_input(int *x, int *y, int touch_fd);
int uninit_touch(int touch_fd);
int judge_slider_direction(int touch_fd);

#endif