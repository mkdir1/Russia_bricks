/*
*************************************************************************************************************
* Copyright(C):
* Filename    :
* Author      :
* Version     :
* Date        :
* Description :
**************************************************************************************************************
*/

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <ncurses.h>
#include "colors.h"
#include "tetris.h"
#include "shapes.h"

#define MAX(a,b)      ((a)>(b)?(a):(b))
#define MIN(a,b)      ((a)<(b)?(a):(b))

/*---------------------CONSTS--------------------------*/

#define MYKEY_ESC          27	
/* 每个Block在Y和X方向占用字符数 */
#define BLOCK_CHARS_Y       4                                                                           
#define BLOCK_CHARS_X       8
/* Window左上角X Y 坐标 */
#define WINDOW_ORGN_X      10
#define WINDOW_ORGN_Y      5
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

static int _block_x;           /* 方块左上角x坐标 */
static int _block_y;           /* 方块左上角y坐标 */
static int _block_index;  
static int _block_next;
static int _table_shape_curr[4][4];
static int _table_shape_next[4][4];

static int _line_noneempty_min;

static int _flag_new;          /* 新方块产生标志 */ 
static int _flag_done;         /* 游戏结束标志 */
static int _time_down;         /* 方块下降时间,ms */

static int _window_height;
static int _window_width;

static short _bg_color;        /* 背景颜色 */
static short _fg_color;        /* 前景颜色 */

static WINDOW *_pwindow;       
static WINDOW *_subwnd;
static struct BOARD _board_table[ BOARD_TABLE_Y ][ BOARD_TABLE_X ];

static int _level_max;                /* 最大等级 */
static int _level_cur;                /* 当前等级 */
static unsigned int _score;           /* 分数 */

static int _table_downtime[ ] = { /* 每个等级对应的下降时间,ms */
	500,450,400,350,300,250,200,250,100,50,20
};

static const unsigned char _arrow_up[5][5] = {
    {0,0,1,0,0},
    {0,1,1,1,0},
    {0,0,1,0,0},
    {0,0,1,0,0},
	{0,0,1,0,0},
};

static const unsigned char _arrow_right[5][5] = {
	{0,0,0,0,0},
	{0,0,0,1,0},
	{1,1,1,1,1},
	{0,0,0,1,0},
	{0,0,0,0,0},
};

static const unsigned char _arrow_left[5][5] = {
	{0,0,0,0,0},
	{0,1,0,0,0},
	{1,1,1,1,1},
	{0,1,0,0,0},
	{0,0,0,0,0},
};

static const unsigned char _arrow_down[5][5] = {
	{0,0,1,0,0},
	{0,0,1,0,0},
	{0,0,1,0,0},
	{0,1,1,1,0},
	{0,0,1,0,0},
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
int     RightBorder(void);
void    ShowInfo(WINDOW *win);
void    UpdateBackground(int y, int x);

/* main */
int main(int argc, char**argv)
{
	int ch = 0;
	int quit = 0;
	struct itimerval tick;
	
	int max_x, max_y;

	Initial();
	InitNcurses();
	
	getmaxyx(stdscr,max_y,max_x);
	if ( max_x < WINDOW_ORGN_X+WINDOW_WIDTH+SUBWND_WIDTH+5 || 
		 max_y < WINDOW_ORGN_Y+WINDOW_HEIGHT+2 ){
		addstr("your screen is to small! press any key to quit!");
		getch();
		endwin();
		exit(1);
	}

	curs_set(0);    /* 隐藏光标 */

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

	attron(COLOR_PAIR(7));
	mvaddstr(0,1,"Press ESC to quit");
	mvaddstr(1,1,"Press Up to change");
	mvaddstr(2,1,"Press Arrows to move");
	attroff(COLOR_PAIR(7));
	refresh();
	
	signal(SIGALRM,OnAlarm);
	tick.it_value.tv_sec = _time_down/1000;
	tick.it_value.tv_usec = 1000*(_time_down%1000);

	tick.it_interval.tv_sec = _time_down/1000;
	tick.it_interval.tv_usec = 1000*(_time_down%1000);

	setitimer(ITIMER_REAL, &tick, NULL);

    while( 1 ){ /* press ESC to quit*/

		switch( ch=getch() ){
		case KEY_LEFT :                 /* 左移 */
			if ( !MinLeft() )
				_block_x = _block_x-2;  /* 注意这里不是减1 */
			break;
		case KEY_RIGHT :                /* 右移 */     
			if ( !MaxRight() )
				_block_x = _block_x+2;  /* 注意这里不是加1 */
			break;
		case KEY_DOWN :                 /* 向下 */
			while( !Collision() )
				_block_y = _block_y+1; 
			break;
		case KEY_UP :                   /* 变形 */
			Change();
			break;
		case MYKEY_ESC :                /* 退出 */
			quit = 1;
			break;
		}
		
		if ( quit != 0 )  /* 推出游戏 */
			break;
	    
		werase(_pwindow); /* 用werase() 用wclear()屏幕会闪烁 */
		DrawShape(_pwindow, _block_index, _table_shape_curr, _block_y, _block_x);
		DrawBackground(_line_noneempty_min);
		box(_pwindow,'|','-');
		wrefresh(_pwindow);
	}
	delwin(_pwindow);
	delwin(_subwnd);
	endwin();
	return 0;
};

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

	tmp_index = _block_index;	
	_block_index = _shapes[_block_index].next;
	CalBlockShape(_block_index,_table_shape_curr);

	/* 有可能发生碰撞 */
	if ( _block_x+8 > WINDOW_WIDTH-2 ){ 
		_block_x = _block_x-2;
		if ( RightBorder() ){
			_block_x = _block_x+2;
			_block_index = tmp_index;
			CalBlockShape(_block_index,_table_shape_curr);
			return;
		}
		_block_x = _block_x+2;
		return;
	}
}

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
		_line_noneempty_min++; /* 注意这里不是减奥 */
		end = fullline;
		start = _line_noneempty_min;
		cnt++;
	}
	return cnt; /* 返回清除的行数 */
}

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


WINDOW *CreateNewWindow(int height, int weight, int y, int x){
	WINDOW *pwin = newwin(height, weight, y, x);
	box(pwin,'|','-');
	wrefresh(pwin);
	
	return pwin;
}

void DrawArrow(const unsigned char arrow[5][5], int y, int x ){
	int i,j;
	int x0 = x;
	for ( i=0; i<5; i++ ){
		for ( j=0; j<5; j++ ){
			if ( arrow[i][j] )
				mvaddch(y,x,'|');
			x++;
		}
		x = x0;
		y++;
	}
}

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


void DrawSquare(WINDOW *win,int y, int x){
	if ( win == NULL )
		win = stdscr;

	mvwaddch(win,y,x,'[');
	mvwaddch(win,y,x+1,']');
}


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


int LeftBorder(void){
    if ( _block_x <= 1 ){
		if ( _shapes[_block_index].box[0] & 0x88 || 
			 _shapes[_block_index].box[1] & 0x88  
			 )
			return 1;
		else
			return 0;
	}
	return 0;
}


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


void OnAlarm(int p){
	int lines = 0;
	
	if ( _flag_done != 0 ){
		return;
	}

   	if ( _flag_new!=0 ){
		_flag_new = 0;
		RandomBlock();
		ShowInfo(_subwnd);
	}else if ( !Collision() ){
		_block_y++;
	}else{
		if ( _block_y <= BLOCK_INIT_Y )
			_flag_done = 1;
		UpdateBackground(_block_y,_block_x);
		_flag_new = 1;
	}	
	werase(_pwindow);
	DrawShape(_pwindow,_block_index, _table_shape_curr,_block_y,_block_x);
	lines = ClearFullLines();
	DrawBackground(_line_noneempty_min);
	box(_pwindow,'|','-');

	if ( lines ){
		_score += lines*SCORE_ADD_PERLINE;
		_level_cur = MIN(_score/SCORE_PERLEVEL,_level_max);
		ShowInfo(_subwnd);
	}

	wrefresh(_pwindow);
	if ( _flag_done != 0 ){
		mvaddstr(WINDOW_ORGN_Y-1,WINDOW_ORGN_X,"Done!");
		wrefresh(stdscr);
	}
}


void RandomBlock(void){
	
	_block_index = _block_next;
	_block_y = BLOCK_INIT_Y;
	_block_x = BLOCK_INIT_X;
	CalBlockShape(_block_index,_table_shape_curr);
	/* 随机产生下一个方块 */
	srand(time(NULL));
	_block_next = rand()%MAX_BOX;
}

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

void ShowInfo(WINDOW *win){
	int x, y;
	int i;
	werase(win);
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

	y = SUBWND_HEIGHT/2;
	x = SUBWND_WIDTH/10;
	mvwprintw(win,y,x,"%s: %d","Score",_score);
	y += 2;
	mvwprintw(win,y,x,"%s: %d","Level",_level_cur);
	y += 4;
	mvwaddstr(win,y,x,"Version: V1.0.0");
	y += 1;
	mvwaddstr(win,y,x,"Author : sduzh");
	wattron(win,A_REVERSE);  /* 打开反白 */
	
	box(win,'|','-');
	wrefresh(win);
}

void DrawNext(WINDOW *win, int y, int x){
	
	CalBlockShape(_block_next, _table_shape_next);
	DrawShape(win, _block_next, _table_shape_next,y, x);
}


void Initial(void){
	int i,j;

	srand(time(NULL));
	_block_next = rand()%MAX_BOX;

	_bg_color = COLOR_BLACK;
	_fg_color = COLOR_WHITE;
	_block_x = BLOCK_INIT_X;
	_block_y = BLOCK_INIT_Y;
	_flag_new = 1;
	_flag_done = 0;
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
}

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

// x, y:发生碰撞时方块左上角x和y坐标
void UpdateBackground(int y, int x){
	int i,j;
	int row, line;
	short color = _shapes[_block_index].color_id;
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

