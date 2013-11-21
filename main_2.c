/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年11月17日 21时37分00秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wenxian Ni (Hello World~), niwenxianq@qq.com
 *   Organization:  AMS/ICT
 *
 * =====================================================================================
 */

/*
********************************************************************************************
* Copyright(C):
* Filename    : main.c
* Author      : sduzh
* Version     : V0.1.0
* Date        : 11/16/2013
* Description : 俄罗斯方块 Linux版
********************************************************************************************
*/

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <cdk.h>
#include "colors.h"
#include "tetris.h"
#include "shapes.h"

#define MAX(a,b)      ((a)>(b)?(a):(b))
#define MIN(a,b)      ((a)<(b)?(a):(b))

/*---------------------CONSTS--------------------------*/

/* 版本号，发布时间 */
#define VERSION             10   /* 产品版本号：主板本号，子版本号，编译版本号 */
#define DAY                 16   
#define MONTH               11
#define YEAR                2013
/* 按键 */
#define MY_KEY_ESC          27
#define MY_KEY_SPACE        32
/* 每个Block在Y和X方向占用字符数 */
#define BLOCK_CHARS_Y       4                                                                           
#define BLOCK_CHARS_X       8
/* Window左上角X Y 坐标 */
#define WINDOW_ORGN_X      20    /* 要保证窗口左侧有足够空间显示提示信息 */
#define WINDOW_ORGN_Y      2     /* 要保证窗口上策有足够空间显示提示信息 */
/* Window在X和Y方向可以容纳的Block数 */
#define WINDOW_BLOCKS_Y    8
#define WINDOW_BLOCKS_X    5     /* 这个值应尽量是奇数，保证新的Block从中间出现 */
/* Window的字符宽度和长度 */
#define WINDOW_HEIGHT      (WINDOW_BLOCKS_Y * BLOCK_CHARS_Y +2)
#define WINDOW_WIDTH       (WINDOW_BLOCKS_X * BLOCK_CHARS_X +2)
/* 子窗口大小 要至少可以容纳一个方块大小*/
#define SUBWND_HEIGHT      WINDOW_HEIGHT    
#define SUBWND_WIDTH       (BLOCK_CHARS_X*5/2)
/* 子窗口显示位置（左上角坐标） */
#define SUBWND_ORGN_X      (WINDOW_ORGN_X+WINDOW_WIDTH)
#define SUBWND_ORGN_Y      (WINDOW_ORGN_Y)
/* _board_table数组大小 */
#define BOARD_TABLE_X      (WINDOW_BLOCKS_X*4)
#define BOARD_TABLE_Y      (WINDOW_BLOCKS_Y*4)
/* 新的Block出现时的X Y坐标 */
#define BLOCK_INIT_X       (1+BLOCK_CHARS_X*((WINDOW_BLOCKS_X)/2)); 
#define BLOCK_INIT_Y       1
/* 每消除一行增加的分数 */
#define SCORE_ADD_PERLINE    100
/* 每升一级需要的分数 */
#define SCORE_PERLEVEL       1000 

/*--------- GLOBAL VARIABLES DECLARATION----------------*/
static char *_buttons_quit[] = {
	"Yes",
	"No",
};
static char *_buttons_over[] = {
	"Restart",
	"Exit",
};
static char *_buttons_pause[] = {
	"OK"
};
static char *_msg_over[] = {
	"Game Over!"
};
static char *_msg_pause[] = {
	"Pause!",
	"Press ENTER to continue",
};
static char *_msg_quit[] = {
	"Do you want to exit?",
};
static CDKDIALOG *_dialog[3];
static CDKSCREEN *_screen;
static int _block_x;					/* 方块左上角x坐标  */
static int _block_y;					/* 方块左上角y坐标  */
static int _block_curr;                                 /* 当前下落方块索引 */
static int _block_next;                                 /* 下个出现方块索引 */
static int _table_shape_curr[4][4];                     /* 当前下落方块形状 */
static int _table_shape_next[4][4];                     /* 下个下落方块形状 */

static int _line_noneempty_min;                         /* 最下非空行 */

static int _flag_new;					/* 新方块产生标志 */ 
static int _flag_done;					/* 游戏结束标志 */
static int _flag_pause;					/* 暂停标志 */
static int _flag_quit;
static int _time_down;					/* 方块下降时间,ms */

static int _window_height;
static int _window_width;

static short _bg_color;					/* 背景颜色 */
static short _fg_color;					/* 前景颜色 */

static WINDOW *_pwindow;       
static WINDOW *_subwnd;
static struct BOARD _board_table[ BOARD_TABLE_Y ][ BOARD_TABLE_X ];

static int _level_max;					/* 最大等级 */
static int _level_cur;					/* 当前等级 */
static unsigned int _score;				/* 分数 */
static struct itimerval _tick;
static const int _table_downtime[ ] = { /* 每个等级对应的下降时间,ms */
	490,440,390,340,290,240,200,250,100,50,20
};


/*----------FUNCTION DECLARATION----------------------*/

void    CalBlockShape(int index, int buf[4][4]);
void    Change(void);
int     CearFullLines(void);
int     Collision(void);
WINDOW *CreateNewWindow(int height, int weight, int y, int x);
void    CreateSquare(WINDOW *win, int y, int x);
void    DrawArrow(const unsigned char arrow[5][5], int y, int x );
void    DrawBackground(int l);
void    DrawNext(WINDOW *win,int y,int x);
void    DrawShape(WINDOW *win, int index, int table_shape[4][4], int tl_y, int tl_x);
void    DrawSquare(WINDOW *win, int y, int x);
int     HasFullLine(int start, int end, int *fullline);
void    InitNcurses(void);
void    Initial(void);
int     LeftBorder(void);
int     MaxRight(void);
int     MinLeft(void);
void    OnAlarm(int p);
void    OnColor(WINDOW *win, int index);
void    OffColor(WINDOW *win, int index);
void    RandomBlock(void);
void    Restart(void);
int     RightBorder(void);
void    ShowInfo(WINDOW *win);
void    UpdateBackground(int y, int x);


/* main */
int main(int argc, char**argv){
	int ch = 0;
	int i;	
	int max_x, max_y;

	Initial();
	InitNcurses();

	/* 测试屏幕是否足够大 */
	getmaxyx(stdscr,max_y,max_x);
	if ( max_x < WINDOW_ORGN_X+WINDOW_WIDTH+SUBWND_WIDTH+5 || 
		 max_y < WINDOW_ORGN_Y+WINDOW_HEIGHT+2 ){
		mvaddstr(max_y/2,1,"Your screen is too small!\n Press any key to quit!");
		getch();
		endwin();
		exit(1);
	}
	curs_set(0);    /* 隐藏光标 */

	/* 创建显示窗口 */
	_pwindow = CreateNewWindow(WINDOW_HEIGHT,
							   WINDOW_WIDTH,
							   WINDOW_ORGN_Y,
							   WINDOW_ORGN_X
							   );
	wattron(_pwindow,A_REVERSE);  /* 反白显示 */

	_subwnd = CreateNewWindow(SUBWND_HEIGHT,
							  SUBWND_WIDTH,
							  SUBWND_ORGN_Y,
							  SUBWND_ORGN_X
							  );
	wattron(_subwnd,A_REVERSE); /* 反白显示 */

	_screen = initCDKScreen(_pwindow);
	_dialog[0] = newCDKDialog(_screen,
								CENTER, CENTER,
							   _msg_quit,1,
							   _buttons_quit,2,
							   A_UNDERLINE,
							   true,true,true
							 );
   _dialog[1] = newCDKDialog(_screen,
							 CENTER, CENTER,
							 _msg_pause,2,
							 _buttons_pause,1,
							 A_UNDERLINE,
							 true,true,true
							 );
   _dialog[2] = newCDKDialog(_screen,
							CENTER,CENTER,
							_msg_over,1,
							_buttons_over,2,
							A_UNDERLINE,
							true,true,true
							);
   for ( i=0; i<3; i++ ){
		injectCDKDialog(_dialog[i],'\n');
		drawCDKDialog(_dialog[i],true);
		drawCDKDialogButtons(_dialog[i]);
   }
   /* 提示信息 */
	attron(COLOR_PAIR(7));
	mvaddstr(WINDOW_ORGN_Y+2,1,"Quit   : ESC   ");
	mvaddstr(WINDOW_ORGN_Y+3,1,"Change : UP    ");
	mvaddstr(WINDOW_ORGN_Y+4,1,"Move   : Arrows");
	mvaddstr(WINDOW_ORGN_Y+5,1,"Pause  : SPACE ");
	mvaddstr(WINDOW_ORGN_Y+6,1,"Restart: R     ");
	attroff(COLOR_PAIR(7));
	refresh();
	
	/* 创建定时任务 */
	signal(SIGALRM,OnAlarm);
	_tick.it_value.tv_sec = 0;
	_tick.it_value.tv_usec = 10;
	_tick.it_interval.tv_sec = _time_down/1000;
	_tick.it_interval.tv_usec = 1000*(_time_down%1000);
	setitimer(ITIMER_REAL, &_tick, NULL);

	timeout(0);
    while( 1 ){ 
		switch( ch=getch() ){
		case KEY_LEFT :                 /* 左移 */
			if ( !MinLeft() && !_flag_pause )
				_block_x = _block_x-2;  /* 注意这里不是减1 */
			break;
		case KEY_RIGHT :                /* 右移 */     
			if ( !MaxRight() && !_flag_pause )
				_block_x = _block_x+2;  /* 注意这里不是加1 */
			break;
		case KEY_DOWN :                 /* 向下 */
			while( !Collision() && !_flag_pause )
				_block_y = _block_y+1; 
			break;
		case KEY_UP :                   /* 变形 */
			if ( !_flag_pause )
				Change();
			break;
		case MY_KEY_SPACE :             /* 暂停 */
		    _flag_pause = 1;
			activateCDKDialog(_dialog[1],NULL);
			_flag_pause = 0;
			break;
		case 'r' :
		case 'R' :
			Restart();
			break;
		case MY_KEY_ESC :                /* 退出 */
			_flag_quit = 1;
			break;
		}
		
		if ( (_flag_quit!=0) && (0==activateCDKDialog(_dialog[0],NULL)) )  /* 退出游戏 */
			break;
		else
			_flag_quit = 0;
		if ( ch != ERR ){	
			werase(_pwindow); 
			DrawShape(_pwindow, _block_curr, _table_shape_curr, _block_y, _block_x);
			DrawBackground(_line_noneempty_min);
			box(_pwindow,'|','-');
			wrefresh(_pwindow);
		}else if ( _flag_done ){
			if ( 0 == activateCDKDialog(_dialog[2],NULL) ){
				Restart();
			}
			else{
				_flag_quit = 1;
				break;
			}
		}
	}
	destroyCDKDialog(_dialog[0]);
	destroyCDKDialog(_dialog[1]);
	destroyCDKDialog(_dialog[2]);
	destroyCDKScreen(_screen);
	delwin(_pwindow);
	delwin(_subwnd);
    endCDK();
//	endwin();
	return 0;
};


/********************************************
* CalBlockShape()
* 将图像的形状转换为数组buf中的值
* Input : index 图形在_shapes数组中的下标
* Output: buf 保存计算结果的数组
* Return：NONE
********************************************/
void CalBlockShape(int index, int buf[4][4]){
	int i;
	unsigned int value;
	
	value = (_shapes[index].box[0]<<8)|(_shapes[index].box[1]);
	
	for (i=0; i<4; i++){
		buf[0][i] = (value&(1U<<(15-i)))!=0 ? 1:0;
		buf[1][i] = (value&(1U<<(11-i)))!=0 ? 1:0;
		buf[2][i] = (value&(1U<<(7-i )))!=0 ? 1:0;
		buf[3][i] = (value&(1U<<(3-i )))!=0 ? 1:0;
	}		
}

/*******************************************
* Change();
* 如果满足变形的条件则变换形状
* Input : NONE
* Output: NONE
* Return: 可以变形返回 1，不可以返回 0
*-------------------------------------------
* 备注：判断依据是变形之后是否有部分方块在
* 右边界之外（不可能从左边界出去）
*******************************************/
void Change(void){
	int tmp_index;

	/* 如果已经到底，不能变形 */
	if ( Collision() )
		return;

	tmp_index = _block_curr;	
	_block_curr = _shapes[_block_curr].next;
	CalBlockShape(_block_curr,_table_shape_curr);

	/* 有可能发生碰撞 */
	if ( _block_x+8 > WINDOW_WIDTH-2 ){ 
		_block_x = _block_x-2;
		if ( RightBorder() ){
			_block_x = _block_x+2;
			_block_curr = tmp_index;
			CalBlockShape(_block_curr,_table_shape_curr);
			return;
		}
		_block_x = _block_x+2;
		return;
	}
}


/**************************************
* ClearFullLines()
* 清除满行
* Input : NONE
* Output: NONE
* Return: 清除掉的满行数量
**************************************/
int ClearFullLines(void){
	int i, j;
	int end = BOARD_TABLE_Y-1;
	int start = _line_noneempty_min;
	int fullline;
	int cnt=0;
	while( HasFullLine(start,end,&fullline) ){
		for ( i=fullline; i>=_line_noneempty_min && i>=1; i-- ){
			for( j=0; j<BOARD_TABLE_X; j++ ){
				_board_table[i][j] = _board_table[i-1][j];
			}
		}
		_line_noneempty_min++; /* 注意这里不是减 */
		end = fullline;
		start = _line_noneempty_min;
		cnt++;
	}
	return cnt; /* 返回清除的行数 */
}


/**************************************
* Collition()
* 判断是否会和下方产生碰撞
* Input : NONE
* Output: NONE
* Return: 会碰撞返回值非0，碰撞返回指为0
**************************************/
int Collision(void){
	int i, j, x,y;

	x = (_block_x-1)/2;
	y = (_block_y-1)/1;
	for (i=0; i<4 && y<BOARD_TABLE_Y; i++ ){
		for (j=0; j<4 && x<BOARD_TABLE_X; j++){
			if (_table_shape_curr[i][j] != 0 &&
				_board_table[y+1][x].value != 0 
				)
				return 1;
			x++;
		}
		if ( i<3 &&
			 _table_shape_curr[i+1][0] == 0 &&
			 _table_shape_curr[i+1][1] == 0 &&
			 _table_shape_curr[i+1][2] == 0 &&
			 _table_shape_curr[i+1][3] == 0
			 )
			break;

		if ( i == 3)
			break;
		x = (_block_x-1)/2;
		y++;
	}
	if ( y+1 >= WINDOW_BLOCKS_Y*BLOCK_CHARS_Y ) /* 到底了 */
		return 1;
	return 0;
}

/**************************************
* CreateNewWindow()
* 在特定位置创建一定大小的窗口并返回窗口指针
* Input :  height 窗口高度（行数）
*          width  窗口宽度（列数）
*          y 窗口左上角纵坐标
*          x 窗口左上角横坐标
* Output: NONE
* Return: 指向窗口的指针
**************************************/
WINDOW *CreateNewWindow(int height, int width, int y, int x){
	WINDOW *pwin = newwin(height, width, y, x);
	box(pwin,'|','-');
	wrefresh(pwin);
	
	return pwin;
}

/**************************************
* DrawArrow()
* 画箭头形状图形 
* !!!!!有问题
**************************************/
void DrawArrow(const unsigned char arrow[5][5], int y, int x ){
	int i,j;
	int x0 = x;
	for ( i=0; i<5; i++ ){
		for ( j=0; j<5; j++ ){
			if ( arrow[i][j] )
				mvaddch(y,x,'.');
			x++;
		}
		x = x0;
		y++;
	}
}


/**************************************
* DrawBackground()
* 画背景图形
* Input : line 起始绘图行
* Output: NONE
* Return: NONE
**************************************/
void DrawBackground(int line){
	int i, j;
	int r_x, r_y;

	r_x = 1;
	r_y = line+1;
	for ( i=line; i<BOARD_TABLE_Y; i++ ){
		for ( j=0; j<BOARD_TABLE_X; j++ ){
			if ( _board_table[i][j].value != 0 ){
				OnColor(_pwindow, _board_table[i][j].color);
				DrawSquare(_pwindow,r_y,r_x);
				OffColor(_pwindow,_board_table[i][j].color);
			}
			r_x+=2;
		}
		r_x = 1;
		r_y += 1;
	}
}


/**************************************
* DrawNext
* 画下个图形的预览图
* Input : win 绘图窗口
*         y , x ：左上角相对与win的坐标
* Output: NONE
* Return: NONE
**************************************/
void DrawNext(WINDOW *win, int y, int x){
	
	CalBlockShape(_block_next, _table_shape_next);
	DrawShape(win, _block_next, _table_shape_next,y, x);
}


/**************************************
* DrawSquare()
* 画最小正方形
* Input : win 画图窗口
*         x, y正方形左上角坐标
* Output: NONE
* Return: NONE
**************************************/
void DrawSquare(WINDOW *win,int y, int x){
	if ( win == NULL )
		win = stdscr;

	mvwaddch(win,y,x,'[');
	mvwaddch(win,y,x+1,']');
}


/**************************************
* DrawShape()
* 画指定形状图形
* Input : win 做图窗口指针
*         index 形状在_shapes数组中的下标值
*         tl_y tl_x 指定形状的左上角坐标值
* Output: NONE
* Return: NONE
**************************************/
void DrawShape(WINDOW *win, int index, int table_shape[4][4], int tl_y, int tl_x){
	int i, j, y, x;
	y = tl_y;
	x = tl_x;
	
	if ( win == NULL )
		win = stdscr;

	OnColor(win, _shapes[index].color_id);

	for (i=0; i<4; i++){
		for (j=0; j<4; j++){
			if( table_shape[i][j] != 0 )
				DrawSquare(win,y,x);
			x+=2;
		}
		x = tl_x;
		y++;
	}
	OffColor(win, _shapes[index].color_id); 
}

/**************************************
* HasFullLine()
* 从下往上判断是否有填充满的行，一旦找到便退出
* Input : start 判断的起始行
*         end   判断结束行
* Output: fullline 记录满行值
* Return: 找到返回 1，否则返回 0
**************************************/
int HasFullLine(int start, int end,  int *fullline){
	int i, j;

	if ( start<0 || end>BOARD_TABLE_Y-1 )
		return 0;

	/* 从下往上找，一旦找到便退出 */
	for ( i=end; i>=start; i-- ){
		for ( j=0; j<BOARD_TABLE_X; j++ ){
			if ( _board_table[i][j].value == 0 )
				break;
		}
		if ( j == BOARD_TABLE_X ){
			*fullline = i;
			return 1;
		}
	}
	return 0;
}


/**************************************
* Initial()
* 初始化
* Input : NONE
* Output: NONE
* Return: NONE
***************************************/
void Initial(void){
	int i,j;

	srand(time(NULL));
	_block_next = rand()%MAX_BOX;

	_bg_color = COLOR_BLACK;
	_fg_color = COLOR_WHITE;
	_block_x = BLOCK_INIT_X;
	_block_y = BLOCK_INIT_Y;
	_flag_new = 1;
	_window_height = WINDOW_BLOCKS_Y*BLOCK_CHARS_Y+2;
	_window_width  = WINDOW_BLOCKS_X*BLOCK_CHARS_X+2;
	_line_noneempty_min = BOARD_TABLE_Y-1;

	_score = 0;
	_level_max = sizeof(_table_downtime)/sizeof(_table_downtime[0])-1;
	_level_cur = 0;
	_time_down = _table_downtime[_level_cur];
	for (i=0; i<BOARD_TABLE_Y; i++ ){
		for (j=1; j<BOARD_TABLE_X; j++){
			_board_table[ i ][ j ].value = 0;
		}
	}
	_flag_done = 0;
}


/**************************************
* InitNcurses()
* 使用ncurses库之前的初始化
* Input : NONE
* Output: NONE
* Return：NONE
**************************************/
void InitNcurses(void){
	initscr();
	start_color();
	init_pair(1, _tab_color[1], _bg_color);
	init_pair(2, _tab_color[2], _bg_color);
	init_pair(3, _tab_color[3], _bg_color);
	init_pair(4, _tab_color[4], _bg_color);
	init_pair(5, _tab_color[5], _bg_color);
	init_pair(6, _tab_color[6], _bg_color);
	init_pair(7, _tab_color[7], _bg_color);
	cbreak();
	nonl();
	noecho();
	intrflush(stdscr,FALSE);
	keypad(stdscr,TRUE);
}


/**************************************
* LeftBorder()
* 判断当前下落图形是否在左边界
* Input : NONE
* Output: NONE
* Return: 是返回 1，否返回 0
**************************************/
int LeftBorder(void){
    if ( _block_x <= 1 ){
		if ( _shapes[_block_curr].box[0] & 0x88 || 
			 _shapes[_block_curr].box[1] & 0x88  
			 )
			return 1;
		else
			return 0;
	}
	return 0;
}

/**************************************
* MaxRight()
* 判断当前下落图形是否可以向右移动
* Input : NONE
* Output: NONE
* Return: 不可以返回 1 ，可以返回 0
**************************************/
int MaxRight(void){
	int i,j;
	int x,y;
	if ( RightBorder() )
		return 1;
	
	y = (_block_y-1)/1;
	for ( i=0; i<4 && y<BOARD_TABLE_Y && y>=0; i++ ){
		x = (_block_x-1)/2+3;
		for ( j=3; j>=0 && x+1<BOARD_TABLE_X && x+1>=0; j-- ){
			if ( _table_shape_curr[i][j] != 0 ){
				if ( _board_table[y][x+1].value != 0 )
					return 1;
				else
					break;
			}
			x--;
		}
		y++;
	}

	return 0;
}

/**************************************
* MinLeft()
* 判断当前下落图形是否可以向左移动
* Input : NONE
* Output: NONE
* Return: 不可以返回 1，可以返回 0
***************************************/
int MinLeft(void){
	int i,j;
	int x,y;
	if ( LeftBorder() )
		return 1;

	x = (_block_x-1)/2;
	y = (_block_y-1)/1;
	for ( i=0; i<4 && y<BOARD_TABLE_Y && y>=0; i++ ){
		for ( j=0; j<4 && x-1<BOARD_TABLE_X && x-1>=0; j++ ){
			if ( _table_shape_curr[i][j] != 0 ){
				if ( _board_table[y][x-1].value != 0 )
					return 1;
				else
					break;
			}
			x++;
		}
		x = (_block_x-1)/2;
		y++;
	}

	return 0;
}


void OnColor(WINDOW *win, int index){
	if ( win == NULL )
		win = stdscr;

	switch( index ){
	case 1 :
		wattron(win,COLOR_PAIR(1));
		break;
	case 2 :
		wattron(win,COLOR_PAIR(2));
		break;
	case 3 :
		wattron(win,COLOR_PAIR(3));
		break;
	case 4 :
		wattron(win,COLOR_PAIR(4));
		break;
	case 5 :
		wattron(win,COLOR_PAIR(5));
		break;
	case 6 :
		wattron(win,COLOR_PAIR(6));
		break;
	case 7 :
		wattron(win,COLOR_PAIR(7));
		break;
	default:	
		break;
	}
}


void OffColor(WINDOW *win, int index){
	if ( win == NULL )
		win = stdscr;

	switch( index ){
	case 1 :
		wattroff(win,COLOR_PAIR(1));
		break;
	case 2 :
		wattroff(win,COLOR_PAIR(2));
		break;
	case 3 :
		wattroff(win,COLOR_PAIR(3));
		break;
	case 4 :
		wattroff(win,COLOR_PAIR(4));
		break;
	case 5 :
		wattroff(win,COLOR_PAIR(5));
		break;
	case 6 :
		wattroff(win,COLOR_PAIR(6));
		break;
	case 7 :
		wattroff(win,COLOR_PAIR(7));
		break;
	default:	
		break;
	}
}

/**************************************
* OnAlarm()
* 定时器的信号响应函数
* Input : NONE
* Output: NONE
* Return: NONE
**************************************/
void OnAlarm(int p){
	int lines = 0;

	if ( _flag_pause || _flag_quit || _flag_done)
		return;

   	if ( _flag_new!=0 ){
		_flag_new = 0;
		RandomBlock();
		ShowInfo(_subwnd);
	}else if ( !Collision() ){
		_block_y++;
	}else{
		UpdateBackground(_block_y,_block_x);
		_flag_new = 1;
		if ( _block_y <= BLOCK_INIT_Y )
			_flag_done = 1;
	}	
	werase(_pwindow);
	DrawShape(_pwindow,_block_curr, _table_shape_curr,_block_y,_block_x);
	lines = ClearFullLines();
	DrawBackground(_line_noneempty_min);
	box(_pwindow,'|','-');

	if ( lines ){
		int tmp = _level_cur;
		_score += lines*SCORE_ADD_PERLINE;
		_level_cur = MIN(_score/SCORE_PERLEVEL,_level_max);
		/* 升级之后加快下落时间 */
		if ( _level_cur > tmp ){
			_time_down = _table_downtime[ _level_cur ];
			_tick.it_value.tv_sec = _time_down/1000;
			_tick.it_value.tv_usec = 1000*(_time_down%1000);
			_tick.it_interval.tv_sec = _time_down/1000;
			_tick.it_interval.tv_usec = 1000*(_time_down%1000);
			setitimer(ITIMER_REAL, &_tick, NULL);
		}
		ShowInfo(_subwnd);
	}

	wrefresh(_pwindow);
}

/**************************************
* RandomBlock()
* 产生一个随机的图形
* Input : NONE
* Output: NONE
* Return: NONE
**************************************/
void RandomBlock(void){
	
	_block_curr = _block_next;
	_block_y = BLOCK_INIT_Y;
	_block_x = BLOCK_INIT_X;
	CalBlockShape(_block_curr,_table_shape_curr);
	/* 随机产生下一个方块 */
	srand(time(NULL));
	_block_next = rand()%MAX_BOX;
}

/**************************************
* Restart()
* 重新开始游戏
* Input : NONE
* Output: NONE
* Return：NONE
**************************************/
void Restart(void){
	int i,j;
	
	_flag_pause = 1;

	_score = 0;
	_level_cur = 0;
	_time_down = _table_downtime[_level_cur];
	_tick.it_value.tv_sec = 0;//_time_down/1000;
	_tick.it_value.tv_usec = 10;//1000*(_time_down%1000);
	_tick.it_interval.tv_sec = _time_down/1000;
	_tick.it_interval.tv_usec = 1000*(_time_down%1000);
	setitimer(ITIMER_REAL, &_tick, NULL);
	
	for ( i=0; i<BOARD_TABLE_Y; i++ ){
		for ( j=0; j<BOARD_TABLE_X; j++ ){
			_board_table[i][j].value = 0;
		}
	}
	_block_x = BLOCK_INIT_X;
	_block_y = BLOCK_INIT_Y;

	_block_curr = _block_next;
	_block_next = rand()%MAX_BOX;
	_line_noneempty_min = BOARD_TABLE_Y;
	CalBlockShape(_block_curr, _table_shape_curr);
	ShowInfo(_subwnd);
	_flag_pause = 0;
	_flag_done = 0;
}


/**************************************
* RightBorder()
* 判断当前下落图形是否在右边界处
* Input : NONE
* Output: NONE
* Return: 是返回 1， 否返回 0
**************************************/
int RightBorder(void){
	int i;
	int x = (WINDOW_WIDTH-1-_block_x)/2-1;
	if ( _block_x+8 >= WINDOW_WIDTH-1  ){ /* +8是因为x方向每个小方块占两个字符间距 */
		for (i=0;i<4;i++){
			if (_table_shape_curr[i][x] != 0 )
				return 1;
		}
	}
	return 0;
}

/**************************************
* ShowInfo()
* 显示预览，分数，等级等信息
* Input : NONE
* Output：NONE
* Return: NONE
***************************************/
void ShowInfo(WINDOW *win){
	int x, y;
	int i;
	werase(win);
	/* 显示下个图形预览 */
	x = 1+SUBWND_WIDTH/2-BLOCK_CHARS_X/2;
	y = 4;
	DrawNext(win, y, x);
	y += BLOCK_CHARS_Y+1;
	for ( i=1; i<SUBWND_WIDTH-1; i++ ){
		mvwaddch(win,y,i,'-');
	}
	wattroff(win,A_REVERSE);  /* 关闭反白 */
	y += 1;
	mvwaddstr(win,y,x,"Next");
	
	/* 显示分数，等级 */
	y = SUBWND_HEIGHT/2;
	x = SUBWND_WIDTH/10;
	mvwprintw(win,y,x,"%s: %d","Score",_score);
	y += 2;
	mvwprintw(win,y,x,"%s: %d/%d","Level",_level_cur,_level_max);
	
	/* 显示版本，时间，作者,联系方式 */
	y += 4;
	mvwprintw(win,y,x,"Date: %d/%d/%d", DAY,MONTH,YEAR);
	y += 1;
	mvwprintw(win,y,x,"Version: V%d.%d.%d",VERSION/100,VERSION%100/10,VERSION%100%10);
	y += 1;
	mvwaddstr(win,y,x,"Author : sduzh");
	y += 1;
	mvwaddstr(win,y,x,"QQ : 1239009982");
	wattron(win,A_REVERSE);  /* 打开反白 */
	box(win,'|','-');
	wrefresh(win);
}




/**************************************
* UpdateBackground()
* 更新背景数据结构
* Input ：y,x 更新时最后下落的图形的坐标
* Output: NONE
* Return: NONE
**************************************/
void UpdateBackground(int y, int x){
	int i,j;
	int row, line;
	short color = _shapes[_block_curr].color_id;
	/* 先将字符坐标x，y转换为在_board_table数组中对应的位置 */
	line = y-1;
	row  = (x-1)/2;
	for ( i=0; (i<4) && (line+i<BOARD_TABLE_Y); i++ ){
		for ( j=0; (j<4) && (row+j<BOARD_TABLE_X); j++ ){
			_board_table[line+i][row+j].value |= _table_shape_curr[i][j];
			if ( _table_shape_curr[i][j] != 0 )
				_board_table[line+i][row+j].color = color;
		}
	}

	_line_noneempty_min = MIN(line,_line_noneempty_min);
}

/******************************************************************************
	END FILE
*******************************************************************************/
