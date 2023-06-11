/*
 *		Copyright (C) 1996 by Sony Computer Entertainment Inc.
 *				All rights Reserved
 *
 *			pad.h: PadRead() accsess macro
 */

#define PAD1_UP     	(1<<12)
#define PAD1_DOWN   	(1<<14)
#define PAD1_LEFT   	(1<<15)
#define PAD1_RIGHT  	(1<<13)
#define PAD1_TRIANGLE   (1<< 4)
#define PAD1_CROSS   	(1<< 6)
#define PAD1_SQUARE   	(1<< 7)
#define PAD1_CIRCLE  	(1<< 5)
#define PADi       		(1<< 9)
#define PADj       		(1<<10)
#define PADk       		(1<< 8)
#define PADl       		(1<< 3)
#define PADm       		(1<< 1)
#define PADn       		(1<< 2)
#define PADo       		(1<< 0)
#define PADh       		(1<<11)
#define PAD1_L1    		PADn
#define PAD1_L2    		PADo
#define PAD1_R1    		PADl
#define PAD1_R2    		PADm
#define PAD1_START		PADh
#define PAD1_SELECT  	PADk
