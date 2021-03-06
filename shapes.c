/*
 * =====================================================================================
 *
 *       Filename:  shapes.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年11月15日 16时49分48秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Wenxian Ni (Hello World~), niwenxianq@qq.com
 *   Organization:  AMS/ICT
 *
 * =====================================================================================
 */


#include "tetris.h"
#include "shapes.h"

struct SHAPE  _shapes[MAX_BOX] = {
/*
*  1     1 1 1   1 1       1
*  1     1         1   1 1 1
*  1 1             1
*/
	{ {0x88,0xc0}, 1, 1 },
	{ {0xe8,0x00}, 1, 2 },
	{ {0xc4,0x40}, 1, 3 },
	{ {0x2e,0x00}, 1, 0 },

/*
*    1	 1       1 1  1 1 1
*    1	 1 1 1	 1        1
*  1 1	         1    
*/
	{ {0x44,0xc0}, 2, 5 },
	{ {0x8e,0x00}, 2, 6 },
	{ {0xc8,0x80}, 2, 7 },
	{ {0xe2,0x00}, 2, 4 },

/*
*   1        1 1
*   1 1    1 1
*     1
*/
	{ {0x8c,0x40}, 3, 9 },
	{ {0x6c,0x00}, 3, 8 },

/*
*     1    1 1
*   1 1      1 1
*   1
*/
	{ {0x4c,0x80}, 4, 11 },
	{ {0xc6,0x00}, 4, 10 },

/*
*     1        1       1 1 1      1
*   1 1 1      1 1       1      1 1
*              1                  1      
*/
	{ {0x4e,0x00}, 5, 13 },
	{ {0x8c,0x80}, 5, 14 },
	{ {0xe4,0x00}, 5, 15 },
	{ {0x4c,0x40}, 5, 12 },

/*
*    1     1 1 1 1
*    1
*    1
*    1
*/
	{ {0x88,0x88}, 6, 17 },
	{ {0xf0,0x00}, 6, 16 },

/*
*   1 1
*   1 1
*/
	{ {0xcc,0x00}, 7, 18 },

};

