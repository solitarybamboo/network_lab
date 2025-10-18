#ifndef __WORD_H__
#define __WORD_H__

extern unsigned char word[14][16*16/8];


/*
	:用户指定在开发板的合适位置显示一个字符。
	@x0,y0:用户指定的显示位置。
	@word[]:用户指定显示的字符
	@w,h:显示字符的大小。
		字符大小取决于取模的字体大小，就是宽度和高度
*/
void word_display(char word[],int x0,int y0,int w,int h);
void show_tem(double temperature, int center_x, int center_y);





#endif