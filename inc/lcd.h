#ifndef __LCD_H__
#define __LCD_H__

void *lcd_init(int *lcd_fd);
// void lcd_init(int *lcd_fd, int **plcd);
int lcd_draw_point(int x, int y, int color, int *plcd);
int lcd_draw_rectangle(int x, int y, int width, int height, int color, int *plcd);
int lcd_draw_circle(int x, int y, int r, int color, int *plcd);
void lcd_uninit(int lcd_fd, int *plcd);
void display_point(int x,int y,int color);

#endif