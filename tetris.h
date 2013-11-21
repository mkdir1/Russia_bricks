#ifndef TEIRIS_H
#define TETRIS_H

/* Board */
struct BOARD{
	int value;
	int color;
};

/* Shape */
struct SHAPE{
/*一个字节8位，每4位表示游戏方块中的一行，例如:
box[0]=“0x88”，box[1]=“0xC0”表示：
1000
1000
1100
0000
*/
	unsigned char box[2];
	short color_id;  /* 每个方块的颜色索引 */
	int next;   /* 下个方块的编号 */
};


#endif /* TETRIS */
