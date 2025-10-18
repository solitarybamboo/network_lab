#include "bmp.h"
#include "2048.h"

/*
    count_arr_zero:计算二维数组中有多少个0
    @arr:需要统计的数组名
    返回值：
        int 返回计算的0的个数
*/
int count_arr_zero(int arr[][4])
{
    int count = 0;
    for(int i=0; i<4; i++)
    {
        for( int j=0; j<4; j++)
        {
            if(arr[i][j] == 0)
            {
                count++;
            }
        }
    }

    return count;
}

/*
    rand_num:在棋盘中随机生成2或者4
    @arr:数组名
    返回值：
        void
*/
void rand_num(int arr[][4])
{
    //1.计算有多少个0
    int count = count_arr_zero(arr);

    //2.随机生成1-count之间的随机数，找到随机位置
    int k = rand()%count + 1;
    int temp = 1;

    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            if(arr[i][j] == 0)
            {
                if(temp == k)
                {
                    arr[i][j] = (rand()%10>3)?2:4;//生成2或4
                }
                temp++;
            }
        }
    }
}

/*
    show_interface:显示游戏界面
    @arr:棋盘数组
    @plcd：映射区域的首地址
    返回值：
        void
*/
void show_interface(int arr[][4], int *plcd)
{
    int ccoord_x[4] = {100, 220, 340, 460};
    int ccoord_y[4] = {20, 140, 260, 380};

    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            char bmp_path_arr[256] = {0};
            sprintf(bmp_path_arr, "./%d.bmp", arr[i][j]);//把图片路径保存到数组中
            show_bmp(bmp_path_arr, ccoord_x[j], ccoord_y[i], plcd);//显示对应数字的bmp图片，从左至右，从上到下显示
        }
    }
}

/*
    get_slide:获取滑动方向
    @touch_fd:触摸屏文件描述符
    @arr:棋盘数组
    返回值：
        int
*/
int get_slide(int touch_fd, int arr[][4], int move_flag)
{
    switch(move_flag)
    {
        case 1:
            slide_up();//上滑
            break;
        case 2:
            slide_down();
            break;
        case 3:
            slide_left();
            break;
        case 4:
            slide_right();
            break;
        default:
            break;
    }
}

/*
    slide_left:左滑的操作
    @arr:棋盘数组
    返回值：
        void
*/
void slide_left(int arr[][4])
{
    int temp_arr[4] = {0};

    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            temp_arr[j] = arr[i][j]; 
        }

        //平移操作，去0
        rm_zero(temp_arr);

        //合并。相邻的合并
        hebing(temp_arr);

        //q去0
        rm_zero(temp_arr);

        for(int j=0; j<4; j++)
        {
            arr[i][j] = temp_arr[j];//将操作过后的一行数据重新写回到我么的棋盘二维数组中
        }
    
    }
}

/*
    rm_zero:把0放到与滑动方向相反的一遍，去0d平移操作
    @temp_arr:数组的一行或一列
    返回值：
        void
        0 0 2 4
        2 4 0 0

        2 4 0 0
*/
void rm_zero(int temp_arr[])
{
    int k = 0;//有效数字的下标

    for(int i=0; i<4; i++)
    {
        if(temp_arr[i] != 0)
        {
            temp_arr[k] = temp_arr[i];
            if(k != i)
            {
                temp_arr[i] = 0;
            }
            k++;
        }
    }
}

/*
    hebing:合并相邻相等的元素
    @temp_arr:棋盘数组的一行或一列
    返回值：
        void
*/
void hebing(int temp_arr[])
{
    for(int i=0; i<3; i++)
    {
        if(temp_arr[i] == temp_arr[i+1] && temp_arr[i] != 0)
        {
            temp_arr[i] *= 2;
            temp_arr[i+1] = 0;
        }
    }
}



