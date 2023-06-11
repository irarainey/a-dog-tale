//
// A Dog Tale
//
// Ira Rainey 12/6/97
//
// ira.rainey@gmail.com
//

// Include all the relevant header files and all the globals from define.h
#include <libps.h>
#include <string.h>
#include <stdio.h>
#include <pad.h>
#include "define.h"
#include "world.h"

//
// Main program function
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void main(void)
	{
	int idx;
	
	dead = true;
	
	// Set background colour
	back_r = back_g = back_b = 0;

	// Initialise pad buffers
	GetPadBuf(&bb0, &bb1);

	// Set PAL
	SetVideoMode(MODE_PAL);

	// Initialise graphics system
	GsInitGraph(SCREEN_WIDTH,SCREEN_HEIGHT,4,0,0);
	GsDefDispBuff(0,0,0,SCREEN_HEIGHT);

	// Initialise the ordering table
	for (idx=0;idx<2;idx++)
		{
		WorldOT[idx].length = OT_LENGTH;
		WorldOT[idx].org = OTTags[idx];
		}

	while (!break_out)
		{
		if(dead)
			{
			// Init Objects
			InitObjects();

			InitSound();		// Sound initial setting
			PlaySEQ();

			break_out = false;

			// Setup the front end screen
			DisplayFrontEnd();
			}
			
		// Initialise game sprites here
		if(!break_out) SetupLevelOne();
	
		// play level one
		if(!break_out) PlayLevelOne();

		// Stop all drawing before quitting
		if(break_out)
			{
			StopSound();
			ResetGraph(1);
			}
		}
	
//
// end of main code loop
//
	
	}

//
// Function to get the low level controller data
//
// Requires:	Nothing
//
// Returns:		Value determining the controller state
//
unsigned long PadRead(void)
	{
	return(~(*(bb0+3) | *(bb0+2) << 8 | *(bb1+3) << 16 | *(bb1+2) << 24));
	}

//
// Function to check the controller and update game object positions
//
// Requires:	Nothing
//
// Returns:		Value 99 if select pushed
//
static long CheckPad (void)
	{
	unsigned long padd = PadRead();		
	char jump_dude, down, old_down;
	
	if(padd & PAD1_CROSS) down = true;
		else down = false;
		
	if(old_padd & PAD1_CROSS) old_down = true;
		else old_down = false;
		
	if((down)&&(!old_down)) jump_dude = true;
		else if((down)&&(old_down)) jump_dude = false;
			else if((!down)&&(old_down)) jump_dude = false;
				else if((!down)&&(!old_down)) jump_dude = false; 
			
	old_padd = padd;
	
	if(padd & PAD1_SELECT) return(99);			

	if(padd & PAD1_START) return(98);			
	
	if((padd & PAD1_TRIANGLE)&&(!display_level)&&(no_keys)) key_moving = true;			
	
	if((game_paused)&&((padd & PAD1_LEFT)||(padd & PAD1_RIGHT)||(padd & PAD1_UP)||(padd & PAD1_DOWN)||(padd & PAD1_CROSS)))
		{
		game_paused = false;			
		SsSeqReplay(seq);
		}
		
	if(!display_level)
		{
		// move left
		if ((padd & PAD1_RIGHT)&&(!jump_dude)&&(!jumping))
			{
			// set sprite facing the correct way
			SpriteObject[22]->sprite.u = 0;
			change_frame++;
			MoveLeft();
			}
		
		if ((padd & PAD1_RIGHT)&&(jump_dude)&&(!jumping))
			{
			// set sprite facing the correct way
			SpriteObject[22]->sprite.u = 0;
			change_frame++;
			MoveLeft();
			jumping = JUMP_RIGHT;
			}
				
		if ((padd & PAD1_RIGHT)&&(jumping))
			{
			// set sprite facing the correct way
			SpriteObject[22]->sprite.u = 0;
			}
				
		// move right
		if ((padd & PAD1_LEFT)&&(!jump_dude)&&(!jumping))
			{
			// set sprite facing the correct way
			SpriteObject[22]->sprite.u = 32;
			change_frame++;
			MoveRight();
			}
			
		if ((padd & PAD1_LEFT)&&(jump_dude)&&(!jumping))
			{
			// set sprite facing the correct way
			SpriteObject[22]->sprite.u = 32;
			change_frame++;
			MoveRight();
			jumping = JUMP_LEFT;
			}

		if ((padd & PAD1_LEFT)&&(jumping))
			{
			// set sprite facing the correct way
			SpriteObject[22]->sprite.u = 32;
			}

		// jump up
		if ((jump_dude)&&(!jumping))
			{
			jumping = JUMP_UP;
			}
		}
	}

//
// Function to set an object up as a sprite
//
// Requires:	Pointer to the object structure
//					Address of TIM file data in main memory
//
// Returns:		Nothing
//
void CreateObject(Object* obj, unsigned long tim_add, int extra)
	{
	RECT		rect;								// Define RECT structure

	// Get the TIM file info - skip 4 bytes from start of file to get to info we need
	GsGetTimInfo ((u_long *)(tim_add+4),obj->image);

	// Load the image into the frame buffer
	rect.x = obj->image->px;									// x pos in frame buffer
	rect.y = obj->image->py;									// y pos in frame buffer
	rect.w = obj->image->pw;									// width of image
	rect.h = obj->image->ph;									// height of image
	LoadImage(&rect, obj->image->pixel);					// load data into frame buffer

	// Load the CLUT into the frame buffer (if needed)
	if((obj->image->pmode>>3)&0x01)
		{
		rect.x = obj->image->cx;								// x pos in frame buffer
		rect.y = obj->image->cy;								// y pos in frame buffer
		rect.w = obj->image->cw;								// width of CLUT
		rect.h = obj->image->ch;								// height of CLUT
		LoadImage(&rect, obj->image->clut);					// load data into frame buffer
		obj->sprite.cx = obj->image->cx;						// CLUT x axis position
		obj->sprite.cy = obj->image->cy;						// CLUT y axis position
		}

	// Initialise sprite structure
	obj->sprite.attribute = ((obj->image->pmode & 0x03)<<24);	// sprite attribute * turn it on *
	obj->sprite.x = obj->xpos;												// sprite x axis position
	obj->sprite.y = obj->ypos;												// sprite y axis position
	obj->sprite.w = obj->width;											// sprite width
	obj->sprite.h = obj->height;											// sprite height
	obj->sprite.tpage = GetTPage((obj->image->pmode & 0x03),0, (obj->image->px+extra), obj->image->py);

	// Wait until non-blocking functions have finished before returning
	DrawSync(0);
	}

//
// Function to find the next available object within the maximum objects array
//
// Requires:	Nothing
//
// Returns:		Pointer to on object structure
//
Object* GetNextFreeObject (void)
	{
	Object* obj=NULL;
	int idx;
	for (idx=0; idx<MAX_OBJECTS; idx++)
		{
		if (ObjectArray[idx].alive==false)
			{
			obj=&ObjectArray[idx];
			break;
			}
		}
	
	ClearObject(obj);

	return obj;
	}

//
// Function to initialise an object
//
// Requires:	Pointer to the object structure to initialise
//
// Returns:		Nothing
//
void ClearObject(Object* obj)
	{
	obj->alive = false;
	obj->xpos = 0;
	obj->ypos = 0;
	obj->width = 0;
	obj->height = 0;
	obj->moving = false;
	obj->collide.x = 0;
	obj->collide.y = 0;
	obj->collide.w = 0;
	obj->collide.h = 0;
	obj->image = NULL;
	ClearSprite(&obj->sprite);
	}

//
// Function to initialise a world object
//
// Requires:	Pointer to the world object structure to initialise
//
// Returns:		Nothing
//
void ClearWObject(LevelObject* obj)
	{
	obj->global_pos = 0;
	obj->global_y = 0;
	obj->x_offset = 0;
	obj->y_offset = 0;
	obj->type = NULL;
	obj->display = false;
	obj->on_scr = false;
	obj->moving = false;
	obj->move_dir = false;
	}

//
// Function to clear all data from a sprite structure
//
// Requires:	Pointer to the sprite structure
//
// Returns:		Nothing
//
void ClearSprite (GsSPRITE* sprite)
	{
	sprite->attribute = SPRITE_OFF;
	sprite->x = 0;
	sprite->y = 0;
	sprite->w = 0;
	sprite->h = 0;
	sprite->tpage = 0;
	sprite->u = 0;
	sprite->v = 0;
	sprite->cx = 0;
	sprite->cy = 0;
	sprite->r = 128;
	sprite->g = 128;
	sprite->b = 128;
	sprite->mx = 0;
	sprite->my = 0;
	sprite->scalex = ONE;
	sprite->scaley = ONE;
	sprite->rotate = 0;
	}

//
// Function to clear all data from all objects
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void InitObjects (void)
	{
	int idx;
	for (idx=0; idx<MAX_OBJECTS; idx++)
		{
		ClearObject(&ObjectArray[idx]);
		ClearWObject(&WorldArray[idx]);
		}
	}

//
// Function to define sprites
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void DefineObject (int no, int x, int y, int w, int h, int x_off, int y_off, unsigned long add, int extra)
	{
	SpriteObject[no]->xpos = SpriteObject[no]->collide.x = x;
	SpriteObject[no]->ypos = SpriteObject[no]->collide.y = y;
	SpriteObject[no]->width = SpriteObject[no]->collide.w = w;
	SpriteObject[no]->height = SpriteObject[no]->collide.h = h;
	SpriteObject[no]->sprite.u = x_off;
	SpriteObject[no]->sprite.v = y_off;
	CreateObject(SpriteObject[no], add, extra);
	}

//
// Function to refresh the screen using various objects
//
// Requires:	A definition of which objects to display
//
// Returns:		Nothing
//
void RefreshScreen(int tof_background)
	{
	int idx;
	
	// Find out which buffer is being used
	activeBuff = GsGetActiveBuff();

	// Packet area
	GsSetWorkBase((PACKET *)GpuPacketArea[activeBuff]);

	// Clear the ordering table
	GsClearOt(0,0,&WorldOT[activeBuff]);

	// Decide which objects to register to the OT
	switch(tof_background)
		{
		case	LOGO_FADE:	GsSortFastSprite(&SpriteObject[0]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[1]->sprite,&WorldOT[activeBuff],0);
								break;
		case	PAUSED:		GsSortFastSprite(&SpriteObject[0]->sprite,&WorldOT[activeBuff],7);
								GsSortFastSprite(&SpriteObject[1]->sprite,&WorldOT[activeBuff],7);
								GsSortFastSprite(&SpriteObject[2]->sprite,&WorldOT[activeBuff],6);
								GsSortFastSprite(&SpriteObject[3]->sprite,&WorldOT[activeBuff],6);
								GsSortFastSprite(&SpriteObject[4]->sprite,&WorldOT[activeBuff],6);
								GsSortFastSprite(&SpriteObject[5]->sprite,&WorldOT[activeBuff],6);
								GsSortFastSprite(&SpriteObject[6]->sprite,&WorldOT[activeBuff],5);
								GsSortFastSprite(&SpriteObject[7]->sprite,&WorldOT[activeBuff],5);
								GsSortFastSprite(&SpriteObject[8]->sprite,&WorldOT[activeBuff],5);
								GsSortFastSprite(&SpriteObject[9]->sprite,&WorldOT[activeBuff],5);
								GsSortFastSprite(&SpriteObject[10]->sprite,&WorldOT[activeBuff],3);
								GsSortFastSprite(&SpriteObject[11]->sprite,&WorldOT[activeBuff],3);
								GsSortFastSprite(&SpriteObject[12]->sprite,&WorldOT[activeBuff],3);
								GsSortFastSprite(&SpriteObject[13]->sprite,&WorldOT[activeBuff],2);
								for(idx=0;idx<life;idx++)
									{
									GsSortSprite(&SpriteObject[14+idx]->sprite,&WorldOT[activeBuff],2);
									}
								GsSortFastSprite(&SpriteObject[19]->sprite,&WorldOT[activeBuff],2);
								for(idx=0;idx<MAX_LEVEL_ONE_OBJ;idx++)
									{
									scr_pos = ((WorldArray[idx].global_pos + WorldArray[idx].x_offset)-where_you_are);
									if((scr_pos<=SCREEN_WIDTH)&&((scr_pos+WorldArray[idx].type->width)>=0)&&(WorldArray[idx].display))
										{
										WorldArray[idx].type->sprite.x = WorldArray[idx].type->collide.x = scr_pos;
										if(WorldArray[idx].disc != PLATFORM)
											{
											if(WorldArray[idx].disc == BONE) GsSortFastSprite(&SpriteObject[50+idx]->sprite,&WorldOT[activeBuff],2);
												else GsSortFastSprite(&SpriteObject[50+idx]->sprite,&WorldOT[activeBuff],4);
											}
											else GsSortFastSprite(&SpriteObject[50+idx]->sprite,&WorldOT[activeBuff],3);
										}
									}
								GsSortFastSprite(&SpriteObject[20]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[21]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[22]->sprite,&WorldOT[activeBuff],1);
								GsSortFastSprite(&SpriteObject[24]->sprite,&WorldOT[activeBuff],0);
								for(idx=0;idx<no_keys;idx++)
									{
									GsSortFastSprite(&SpriteObject[26+idx]->sprite,&WorldOT[activeBuff],2);
									}
								break;
		case	DEADGEEZER:	GsSortFastSprite(&SpriteObject[2]->sprite,&WorldOT[activeBuff],0);
								break;
		case	START_GAME:	if(!end_of_game)
									{
									GsSortSprite(&SpriteObject[0]->sprite,&WorldOT[activeBuff],1);
									GsSortSprite(&SpriteObject[1]->sprite,&WorldOT[activeBuff],1);
									}
								GsSortFastSprite(&SpriteObject[2]->sprite,&WorldOT[activeBuff],0);
								break;
		case	BONES:		ScrollText();
								// sky
								GsSortFastSprite(&SpriteObject[0]->sprite,&WorldOT[activeBuff],4);
								GsSortFastSprite(&SpriteObject[1]->sprite,&WorldOT[activeBuff],4);
								// back
								GsSortFastSprite(&SpriteObject[2]->sprite,&WorldOT[activeBuff],3);
								GsSortFastSprite(&SpriteObject[3]->sprite,&WorldOT[activeBuff],3);
								// front
								GsSortFastSprite(&SpriteObject[4]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[5]->sprite,&WorldOT[activeBuff],2);
								// floor
								GsSortFastSprite(&SpriteObject[6]->sprite,&WorldOT[activeBuff],1);
								GsSortFastSprite(&SpriteObject[7]->sprite,&WorldOT[activeBuff],1);
								// Bones
								GsSortSprite(&SpriteObject[8]->sprite,&WorldOT[activeBuff],0);
								GsSortSprite(&SpriteObject[9]->sprite,&WorldOT[activeBuff],0);
								// Logo
								GsSortSprite(&SpriteObject[10]->sprite,&WorldOT[activeBuff],0);
								// Message
								GsSortFastSprite(&SpriteObject[11]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[12]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[13]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[14]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[15]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[16]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[17]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[18]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[19]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[20]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[21]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[22]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[23]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[24]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[25]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[26]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[27]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[28]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[29]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[30]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[31]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[32]->sprite,&WorldOT[activeBuff],0);
								GsSortFastSprite(&SpriteObject[34]->sprite,&WorldOT[activeBuff],0);
								break;
		case	ABOUT_BOX:	ScrollText();
								// sky
								GsSortFastSprite(&SpriteObject[0]->sprite,&WorldOT[activeBuff],4);
								GsSortFastSprite(&SpriteObject[1]->sprite,&WorldOT[activeBuff],4);
								// back
								GsSortFastSprite(&SpriteObject[2]->sprite,&WorldOT[activeBuff],3);
								GsSortFastSprite(&SpriteObject[3]->sprite,&WorldOT[activeBuff],3);
								// front
								GsSortFastSprite(&SpriteObject[4]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[5]->sprite,&WorldOT[activeBuff],2);
								// floor
								GsSortFastSprite(&SpriteObject[6]->sprite,&WorldOT[activeBuff],1);
								GsSortFastSprite(&SpriteObject[7]->sprite,&WorldOT[activeBuff],1);
								// Bones
								GsSortSprite(&SpriteObject[8]->sprite,&WorldOT[activeBuff],1);
								GsSortSprite(&SpriteObject[9]->sprite,&WorldOT[activeBuff],1);
								// Logo
								GsSortSprite(&SpriteObject[10]->sprite,&WorldOT[activeBuff],1);
								GsSortFastSprite(&SpriteObject[34]->sprite,&WorldOT[activeBuff],1);
								// Box and about
								GsSortBoxFill(&overlay, &WorldOT[activeBuff],0);
								GsSortSprite(&SpriteObject[33]->sprite,&WorldOT[activeBuff],0);
								// message
								GsSortFastSprite(&SpriteObject[11]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[12]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[13]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[14]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[15]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[16]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[17]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[18]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[19]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[20]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[21]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[22]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[23]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[24]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[25]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[26]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[27]->sprite,&WorldOT[activeBuff],2);
								GsSortFastSprite(&SpriteObject[28]->sprite,&WorldOT[activeBuff],1);
								GsSortFastSprite(&SpriteObject[29]->sprite,&WorldOT[activeBuff],1);
								GsSortFastSprite(&SpriteObject[30]->sprite,&WorldOT[activeBuff],1);
								GsSortFastSprite(&SpriteObject[31]->sprite,&WorldOT[activeBuff],1);
								GsSortFastSprite(&SpriteObject[32]->sprite,&WorldOT[activeBuff],1);
								break;
		default :			break;
		}

	// Wait for all drawing to finish
	DrawSync(0);

	// Wait for vertical blank interrupt
	VSync(0);

	// Switch display buffer
	GsSwapDispBuff();

	// Clear the ordering table
	GsSortClear(back_r,back_g,back_b,&WorldOT[activeBuff]);

	// Draw the objects registered in the Ordering table
	GsDrawOt(&WorldOT[activeBuff]);
	}

//
// Function to setup all the main sprites for the actual game loop
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void SetupLevelOne(void)
	{
	int idx;
	
	if(dead)
		{
		// assign objects for sprites
		for(idx=0;idx<MAX_OBJECTS;idx++)
			{
			SpriteObject[idx]=GetNextFreeObject();
			ClearObject(SpriteObject[idx]);
			SpriteObject[idx]->alive = true;
			}

		life = 4;
		restart = 0;
		r_position_x = 30;
		r_position_y = 148;
		r_max_bottom = 148;
		spinning = false;
		no_bones = 0;
		no_keys = 0;
		end_of_game = false;
		}

	where_you_are = restart;
	game_paused = false;
	which_plat = -1;

	// Sky
	DefineObject (0, 0, 0, 160, 75, 0, 0, SkyData, 0);
	DefineObject (1, 160, 0, 160, 75, 32, 0, SkyData, 80);

	// Back mountains
	DefineObject (2, 0, 30, 160, 115, 0, 0, BackData, 0);
	DefineObject (3, 160, 30, 160, 115, 32, 0, BackData, 80);
	DefineObject (4, 320, 30, 160, 115, 0, 0, BackData, 0);
	DefineObject (5, 480, 30, 160, 115, 32, 0, BackData, 80);

	// Front mountains
	DefineObject (6, 0, 70, 160, 120, 0, 0, FrontData, 0);
	DefineObject (7, 160, 70, 160, 120, 32, 0, FrontData, 80);
	DefineObject (8, 320, 70, 160, 120, 0, 0, FrontData, 0);
	DefineObject (9, 480, 70, 160, 120, 32, 0, FrontData, 80);

	// Floor
	DefineObject (10, 0, 180, 160, 32, 0, 0, FloorData, 0);
	DefineObject (11, 160, 180, 160, 32, 0, 0, FloorData, 0);
	DefineObject (12, 320, 180, 160, 32, 0, 0, FloorData, 0);

	// small dog tale logo
	DefineObject (13, 10, 212, 40, 28, 0, 0, SmlogoData, 0);

	// lifes
	DefineObject (14, 80, 227, 28, 28, 40, 0, HeadData, 0);
	SpriteObject[14]->sprite.mx = 14;
	SpriteObject[14]->sprite.my = 14;
	SpriteObject[14]->sprite.rotate = ONE;
	DefineObject (15, 108, 227, 28, 28, 40, 0, HeadData, 0);
	SpriteObject[15]->sprite.mx = 14;
	SpriteObject[15]->sprite.my = 14;
	SpriteObject[15]->sprite.rotate = ONE;
	DefineObject (16, 136, 227, 28, 28, 40, 0, HeadData, 0);
	SpriteObject[16]->sprite.mx = 14;
	SpriteObject[16]->sprite.my = 14;
	SpriteObject[16]->sprite.rotate = ONE;
	DefineObject (17, 164, 227, 28, 28, 40, 0, HeadData, 0);
	SpriteObject[17]->sprite.mx = 14;
	SpriteObject[17]->sprite.my = 14;
	SpriteObject[17]->sprite.rotate = ONE;
	DefineObject (18, 192, 227, 28, 28, 40, 0, HeadData, 0);
	SpriteObject[18]->sprite.mx = 14;
	SpriteObject[18]->sprite.my = 14;
	SpriteObject[18]->sprite.rotate = ONE;

	// big bone
	DefineObject (19, 222, 212, 48, 28, 0, 28, BBoneData, 0);

	// bone counter
	DefineObject (20, 273, 211, 16, 32, (char_xlut[(no_bones/10)+16]), ((char_ylut[(no_bones/10)+16])+108), BoneNum, 0);
	DefineObject (21, 288, 211, 16, 32, (char_xlut[(no_bones%10)+16]), ((char_ylut[(no_bones%10)+16])+108), BoneNum, 0);

	//Keys collected
	DefineObject (26, 96, -32, 32, 32, 0, 120, KeyData, 0);
	DefineObject (27, 120, -32, 32, 32, 0, 120, KeyData, 0);
	DefineObject (28, 144, -32, 32, 32, 0, 120, KeyData, 0);
	DefineObject (29, 168, -32, 32, 32, 0, 120, KeyData, 0);
	DefineObject (30, 192, -32, 32, 32, 0, 120, KeyData, 0);

	key_moving = false;
	key_counter = 0;

	// man
	DefineObject (22, r_position_x, r_position_y, 32, 32, 0, 0, ManData, 0);
	SpriteObject[22]->sprite.u = 0;
	SpriteObject[22]->sprite.v = 64;
	
	going_up = true;
	jumping = STILL;
	max_bottom = r_max_bottom;
	on_plat = fall = false;
	change_frame = 0;
	frame_counter = 3;
	SpriteObject[22]->moving = false;
	SpriteObject[22]->collide.h = (SpriteObject[22]->sprite.h-6);

	// level one start
	DefineObject (23, 96, 104, 128, 32, 0, 108, ReadyData, 0);
	display_level = true;

	// game paused
	DefineObject (24, 84, 104, 152, 32, 0, 0, PauseData, 0);

	SetupWorld();
	}

//
// Function to setup all world objects
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void SetupWorld (void)
	{
	int idx;
	
	// place dog at end of game
	WorldArray[0].global_pos = ((MAX_POINT+SCREEN_WIDTH)-18);
	WorldArray[0].global_y = 149;
	WorldArray[0].type = SpriteObject[50];
	WorldArray[0].display = true;
	WorldArray[0].disc = DOG;
	DefineObject (50, WorldArray[0].global_pos, WorldArray[0].global_y, 18, 32, 0, 0, DogData, 0);

	// enter platforms into world array
	for(idx=1;idx<(TOTAL_PLATFORMS+1);idx++)
		{
		WorldArray[idx].global_pos = platx_lut[idx-1];
		WorldArray[idx].global_y = platy_lut[idx-1];
		WorldArray[idx].type = SpriteObject[50+idx];
		WorldArray[idx].display = true;
		WorldArray[idx].disc = PLATFORM;
		DefineObject ((50+idx), WorldArray[idx].global_pos, WorldArray[idx].global_y, 64, 16, 0, 92, PlatData, 0);
		}
	
	WorldArray[5].moving = true;
	WorldArray[13].moving = true;
	WorldArray[19].moving = true;
	WorldArray[24].moving = true;
	WorldArray[152].moving = true;
	
	// if end of game then re-enter bones and keys into world array
	if(dead)
		{
		// enter bones into world array
		for(idx=(TOTAL_PLATFORMS+1);idx<((TOTAL_PLATFORMS+1)+TOTAL_BONES);idx++)
			{	
			WorldArray[idx].global_pos = bonex_lut[idx-(TOTAL_PLATFORMS+1)];
			WorldArray[idx].global_y = boney_lut[idx-(TOTAL_PLATFORMS+1)];
			WorldArray[idx].type = SpriteObject[(50+idx)];
			WorldArray[idx].display = true;
			WorldArray[idx].disc = BONE;
			DefineObject ((50+idx), WorldArray[idx].global_pos, WorldArray[idx].global_y, 28, 16, 136, 0, SBoneData, 0);
			}

		// enter keys into world array	
		for(idx=200;idx<206;idx++)
			{	
			WorldArray[idx].global_pos = keyx_lut[idx-200];
			WorldArray[idx].global_y = keyy_lut[idx-200];
			WorldArray[idx].type = SpriteObject[(50+idx)];
			WorldArray[idx].display = true;
			WorldArray[idx].disc = KEY;
			DefineObject ((50+idx), WorldArray[idx].global_pos, WorldArray[idx].global_y, 32, 32, 0, 120, KeyData, 0);
			}

		dead = false;
		}
	
	// enter cactus into world array
	for(idx=((TOTAL_PLATFORMS+1)+TOTAL_BONES);idx<((TOTAL_PLATFORMS+1)+TOTAL_BONES+TOTAL_CACTUS);idx++)
		{	
		WorldArray[idx].global_pos = cactusx_lut[idx-((TOTAL_PLATFORMS+1)+TOTAL_BONES)];
		WorldArray[idx].global_y = cactusy_lut[idx-((TOTAL_PLATFORMS+1)+TOTAL_BONES)];
		WorldArray[idx].type = SpriteObject[(50+idx)];
		WorldArray[idx].display = true;
		WorldArray[idx].disc = CACTUS;
		DefineObject ((50+idx), WorldArray[idx].global_pos, WorldArray[idx].global_y, 32, 32, 0, 0, CactusData, 0);
		SpriteObject[(50+idx)]->collide.x += 4;
		SpriteObject[(50+idx)]->collide.y += 4;
		SpriteObject[(50+idx)]->collide.w -= 4;
		SpriteObject[(50+idx)]->collide.h -= 4;
		}

	}

//
// Function to display and handle title screen
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void DisplayFrontEnd(void)
	{
	unsigned int idx, scale_value = ONE, scale_count = 0;
	char start_game = false, offset, play, this_char;
	unsigned long padd;

	if(!already_run)
		{
		play = false;
		offset = 0;
		}
		else
			{
			play = true;
			offset = 1;
			}

	already_run = true;
	
	// assign objects for sprites
	for(idx=0;idx<35;idx++)
		{
		SpriteObject[idx]=GetNextFreeObject();
		ClearObject(SpriteObject[idx]);
		SpriteObject[idx]->alive = true;
		}

	// Sky
	DefineObject (0, 0, 0, 160, 75, 0, 0, SkyData, 0);
	DefineObject (1, 160, 0, 160, 75, 32, 0, SkyData, 80);

	// Back mountains
	DefineObject (2, 0, 30, 160, 115, 0, 0, BackData, 0);
	DefineObject (3, 160, 30, 160, 115, 32, 0, BackData, 80);

	// Front mountains
	DefineObject (4, 0, 70, 160, 120, 0, 0, FrontData, 0);
	DefineObject (5, 160, 70, 160, 120, 32, 0, FrontData, 80);

	// Floor
	DefineObject (6, 0, 180, 160, 32, 0, 0, FloorData, 0);
	DefineObject (7, 160, 180, 160, 32, 0, 0, FloorData, 0);

	// Setup sprite for about bone
	DefineObject (8, 39, 25, 58, 34, 58, 0, BothData, 0);
	SpriteObject[8]->sprite.mx = 29;
	SpriteObject[8]->sprite.my = 17;

	// Setup sprite for play bone
	DefineObject (9, 281, 25, 58, 34, 0, 0, BothData, 0);
	SpriteObject[9]->sprite.mx = 29;
	SpriteObject[9]->sprite.my = 17;

	// Setup sprite for logo
	DefineObject (10, 154, 112, 166, 104, 0, 0, LogoData, 0);
	SpriteObject[10]->sprite.mx = 83;
	SpriteObject[10]->sprite.my = 52;
  	SpriteObject[10]->sprite.scalex = SpriteObject[10]->sprite.scaley = (ONE*8);

	// set message pointer to start of message
	mess_pos = 0;
	
	// setup sprites for scrolling message
	for(idx=0;idx<22;idx++)
		{
		SpriteObject[idx+11]->width = 16;
		SpriteObject[idx+11]->height = 32;
		SpriteObject[idx+11]->xpos = 0+(idx*15);
		SpriteObject[idx+11]->ypos = 208;
		this_char = *(message+mess_pos);
		mess_pos++;
		SpriteObject[idx+11]->sprite.u = (char_xlut[(this_char-32)]);
		SpriteObject[idx+11]->sprite.v = (char_ylut[(this_char-32)]);
		CreateObject(SpriteObject[idx+11], CharData, 0);
		}
	
	mess_pos--;
	
	// Scale in
	for (idx=(ONE*8);idx>ONE;idx-=(ONE/5))
		{
	  	SpriteObject[10]->sprite.scalex = SpriteObject[10]->sprite.scaley = idx;
	  	SpriteObject[10]->sprite.rotate = (SpriteObject[10]->sprite.rotate+(ONE*10));
		RefreshScreen(BONES);
		}

	DefineObject (34, 42, 164, 236, 15, 0, 180, CopyData, 0);

	// Wait for one of the options to be selected
	while(!start_game)
		{
		padd = PadRead();
		
		// programmer key [break] 		
		if((padd & PAD1_L2)&&(padd & PAD1_TRIANGLE)&&(padd & PAD1_R1))
			{
			//break_out = true;
			//start_game = true;
			}
			
		if((padd & PAD1_RIGHT)&&(!play)) play = true;
		
		if((padd & PAD1_LEFT)&&(play)) play = false;
		
		if((padd & PAD1_CROSS)&&(!play)) GameAbout();

		if((padd & PAD1_CROSS)&&(play)) start_game = true;

		// run counter to decide whether to scale bone up or down
		scale_count = (scale_count+1)%25;
		
		// scale bone up or down
		if(scale_count<12) scale_value += (ONE/32);
			else if(scale_count>12) scale_value -= (ONE/32);
		
		if((play)&&(!scale_count)) offset = 1;
			else if((!play)&&(!scale_count)) offset = 0;
			
		// adjust the bones size in the structure
		SpriteObject[(8+offset)]->sprite.scalex = SpriteObject[(8+offset)]->sprite.scaley = scale_value;		

		// draw the screen
		RefreshScreen(BONES);
		}

	for(idx=0;idx<35;idx++)
		{
		ClearObject(SpriteObject[idx]);
		}

	}

//
// Function to display the about box
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void GameAbout(void)
	{
	int idx;

	// Assign object to sprite for about text
	SpriteObject[33]=GetNextFreeObject();
	ClearObject(SpriteObject[33]);
	SpriteObject[33]->alive = true;

	// setup the text
	DefineObject (33, 160, 120, 256, 184, 0, 0, TextData, 0);
	SpriteObject[33]->sprite.mx = 128;
	SpriteObject[33]->sprite.my = 92;
	SpriteObject[33]->sprite.scalex = SpriteObject[33]->sprite.scaley = 0;

	// setup the start position, size and detail of the about box
	overlay.attribute = (1<<30);
	overlay.x = 160;
	overlay.y = 120;
	overlay.w = 0;
	overlay.h = 0;
	overlay.r = overlay.g = overlay.b = 0;
	
	// scale about box in
	for(idx=0;idx<21;idx++)
		{
		overlay.x -=7;
		overlay.y -=5;
		overlay.w +=14;
		overlay.h +=10;
		SpriteObject[33]->sprite.scalex = SpriteObject[33]->sprite.scaley += (ONE/21);
		RefreshScreen(ABOUT_BOX);
		}

	// wait until select is pressed
	while(!((PadRead()) & PAD1_SELECT))
		{
		RefreshScreen(ABOUT_BOX);
		}

	// scale about box back out
	for(idx=19;idx>0;idx--)
		{		
		RefreshScreen(ABOUT_BOX);
		overlay.x +=7;
		overlay.y +=5;
		overlay.w -=14;
		overlay.h -=10;
		SpriteObject[33]->sprite.scalex = SpriteObject[33]->sprite.scaley -= (ONE/21);
		}
	
	// Clear the about text sprite
	ClearObject(SpriteObject[33]);
		
	}

//
// Function to move the world right
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void MoveRight(void)
	{
	if(((SpriteObject[22]->sprite.x<124)&&(!where_you_are))||((SpriteObject[22]->sprite.x>123)&&(where_you_are>=MAX_POINT)))
		{
		if(SpriteObject[22]->sprite.x>0) SpriteObject[22]->sprite.x -= 3;
		CheckUnderAgain();
		}
		else
			{
			if(where_you_are)
				{
				CheckUnderAgain();
				where_you_are-=4;
				if(SpriteObject[6]->sprite.x!=0)
					{
					SpriteObject[6]->sprite.x+=2;
					SpriteObject[7]->sprite.x+=2;
					SpriteObject[8]->sprite.x+=2;
					SpriteObject[9]->sprite.x+=2;
					}
					else
						{
						SpriteObject[6]->sprite.x=-318;
						SpriteObject[7]->sprite.x=-158;
						SpriteObject[8]->sprite.x=2;
						SpriteObject[9]->sprite.x=162;
						}
				if(SpriteObject[10]->sprite.x!=0)
					{
					SpriteObject[10]->sprite.x+=4;
					SpriteObject[11]->sprite.x+=4;
					SpriteObject[12]->sprite.x+=4;
					}
					else
						{
						SpriteObject[10]->sprite.x=-156;
						SpriteObject[11]->sprite.x=4;
						SpriteObject[12]->sprite.x=164;
						}
				if(SpriteObject[2]->sprite.x!=0)
					{
					SpriteObject[2]->sprite.x+=1;
					SpriteObject[3]->sprite.x+=1;
					SpriteObject[4]->sprite.x+=1;
					SpriteObject[5]->sprite.x+=1;
					}
					else
						{
						SpriteObject[2]->sprite.x=-319;
						SpriteObject[3]->sprite.x=-159;
						SpriteObject[4]->sprite.x=1;
						SpriteObject[5]->sprite.x=161;
						}
				}
			}
	}

//
// Function to move the world left
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void MoveLeft(void)
	{
	if(((SpriteObject[22]->sprite.x!=123)&&(where_you_are<MAX_POINT))||((SpriteObject[22]->sprite.x>=123)&&(where_you_are>=MAX_POINT)))
		{
		if((SpriteObject[22]->sprite.x > 266)&&(no_keys == 5))
			{
			end_of_game = true;
			}
			else if(SpriteObject[22]->sprite.x < 272) SpriteObject[22]->sprite.x += 3;
		CheckUnderAgain();
		}
		else
			{
			if(where_you_are<MAX_POINT)
				{
				CheckUnderAgain();
				where_you_are+=4;
				if(SpriteObject[8]->sprite.x!=0)
					{
					SpriteObject[6]->sprite.x-=2;
					SpriteObject[7]->sprite.x-=2;
					SpriteObject[8]->sprite.x-=2;
					SpriteObject[9]->sprite.x-=2;
					}
					else
						{
						SpriteObject[6]->sprite.x=-2;
						SpriteObject[7]->sprite.x=158;
						SpriteObject[8]->sprite.x=318;
						SpriteObject[9]->sprite.x=478;
						}
				if(SpriteObject[12]->sprite.x!=160)
					{
					SpriteObject[10]->sprite.x-=4;
					SpriteObject[11]->sprite.x-=4;
					SpriteObject[12]->sprite.x-=4;
					}
					else
						{
						SpriteObject[10]->sprite.x=-4;
						SpriteObject[11]->sprite.x=156;
						SpriteObject[12]->sprite.x=316;
						}
				if(SpriteObject[4]->sprite.x!=0)
					{
					SpriteObject[2]->sprite.x-=1;
					SpriteObject[3]->sprite.x-=1;
					SpriteObject[4]->sprite.x-=1;
					SpriteObject[5]->sprite.x-=1;
					}
					else
						{
						SpriteObject[2]->sprite.x=-1;
						SpriteObject[3]->sprite.x=159;
						SpriteObject[4]->sprite.x=319;
						SpriteObject[5]->sprite.x=479;
						}
				}
			}
	}
	
//
// Function to scroll text message on start screen
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void ScrollText(void)
	{
	int idx;
	char this_char;

	// move sprites for scrolling message
	for(idx=0;idx<22;idx++)
		{
		SpriteObject[idx+11]->sprite.x -= 3;
		}

	// check to see if they need resetting
	if((SpriteObject[12]->sprite.x)<1)
		{
		for(idx=0;idx<22;idx++)
			{
			// move all sprites back to start position
			SpriteObject[idx+11]->sprite.x = 0+(idx*15);
	
			// change characters
			SpriteObject[idx+11]->sprite.u = SpriteObject[idx+12]->sprite.u;
			SpriteObject[idx+11]->sprite.v = SpriteObject[idx+12]->sprite.v;
			}

		mess_pos++;
		
		if(mess_pos==(strlen(message))) mess_pos = 0;

		this_char = *(message+mess_pos);

		SpriteObject[32]->sprite.u = (char_xlut[(this_char-32)]);
		SpriteObject[32]->sprite.v = (char_ylut[(this_char-32)]);
						
		}
	}

//
// Function to play level one from game
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void PlayLevelOne(void)
	{
	int idx, left, right;
	unsigned long timer = 0;

	int shrink;
	unsigned long rotate_title;
	RECT grab_rect;
	
	quit_game = dead_spinning = false;

	// Keep playing until quit from pause
	while(!quit_game)
		{
		if(!dead_spinning)
			{
			// pause
			if((CheckPad())==99)
				{
				game_paused = true;
				SsSeqPause(seq);
				}
				
			while(game_paused)
				{
				if((CheckPad()) == 98)
					{
					quit_game = true;
					game_paused = false;
					dead = true;
					StopSound();
					}
				if(display_level)
					{
					display_level = false;
					ClearObject(SpriteObject[23]);
					}
				RefreshScreen(PAUSED);
				}
			}

	// this is where the end of game sequence gets called - well done that man
	if(end_of_game)
		{
		life = 0;
		already_run = false;
		}
			
	if(!life)
		{
		Pause(30);
		quit_game = true;
		dead = true;

		if(!end_of_game)
			{
			// Grab the screen from the display buffer into another part of frame buffer
			grab_rect.x = 0;
			if(!activeBuff) grab_rect.y = 0;
				else grab_rect.y = 240; 
			grab_rect.w = 320; 
			grab_rect.h = 240;
			
			// get screen image data
			MoveImage(&grab_rect, 320, 0);		

			// wait until done
			DrawSync(0);
			}
			
		// unassign all objects from sprites
		for(idx=0;idx<30;idx++)
			{
			ClearObject(SpriteObject[idx]);
			}

		if(!end_of_game)
			{
			// Assign object to sprite for left of sprite
			SpriteObject[0]=GetNextFreeObject();
			ClearObject(SpriteObject[0]);
			SpriteObject[0]->alive = true;

			// Initialise sprite structure
			SpriteObject[0]->sprite.attribute = SIXTEEN_BIT_ON;	
			SpriteObject[0]->sprite.x = 160;	
			SpriteObject[0]->sprite.y = 120;	
			SpriteObject[0]->sprite.w = 160;
			SpriteObject[0]->sprite.h = 240;
			SpriteObject[0]->sprite.mx = 160;
			SpriteObject[0]->sprite.my = 120;
			SpriteObject[0]->sprite.tpage = 5;
			
			// Assign object to sprite for right of sprite
			SpriteObject[1]=GetNextFreeObject();
			ClearObject(SpriteObject[1]);
			SpriteObject[1]->alive = true;

			// Initialise sprite structure
			SpriteObject[1]->sprite.attribute = SIXTEEN_BIT_ON;	
			SpriteObject[1]->sprite.x = 160;	
			SpriteObject[1]->sprite.y = 120;	
			SpriteObject[1]->sprite.w = 160;
			SpriteObject[1]->sprite.h = 240;
			SpriteObject[1]->sprite.mx = 0;
			SpriteObject[1]->sprite.my = 120;
			SpriteObject[1]->sprite.u = 32;
			SpriteObject[1]->sprite.v = 0;
			SpriteObject[1]->sprite.tpage = 7;
			}
					
		if(!end_of_game) DefineObject (2, 96, 104, 128, 32, 0, 0, OverData, 0);
			else 	DefineObject (2, 32, 28, 256, 184, 0, 0, EndData, 0);
			
		left = right = 127;
		
		for(shrink=ONE;shrink>(0-(ONE/56));shrink-=(ONE/64))
	 		{
	 		left = right -=2;
	 		if(!end_of_game)
	 			{
				rotate_title = (rotate_title+((ONE*5)))%(ONE*360);
				SpriteObject[0]->sprite.rotate = SpriteObject[1]->sprite.rotate = rotate_title;
				SpriteObject[0]->sprite.scalex = SpriteObject[0]->sprite.scaley = shrink;
				SpriteObject[1]->sprite.scalex = SpriteObject[1]->sprite.scaley = shrink;
				}
			RefreshScreen(START_GAME);
			SsSetMVol(left,right);
			}

		if(!end_of_game)
			{
			// wipe out all the sprites
			ClearObject(SpriteObject[0]);
			ClearObject(SpriteObject[1]);
			for(idx=0;idx<60;idx++)
			{
			RefreshScreen(DEADGEEZER);
			}
			}
			else
				{
				for(idx=0;idx<1200;idx++)
					{
					if(CheckPad()==99) break;
					RefreshScreen(DEADGEEZER);
					}
				}

		StopSound();

		}
		
		if(!dead)
			{	
			timer++;
			if(timer>75) display_level = false;
			
			// Find out which buffer is being used
			activeBuff = GsGetActiveBuff();

			// Packet area
			GsSetWorkBase((PACKET *)GpuPacketArea[activeBuff]);

			// Clear the ordering table
			GsClearOt(0,0,&WorldOT[activeBuff]);

	  		// Register sprites etc into ordering table here
	  		// Sky sprites
			GsSortFastSprite(&SpriteObject[0]->sprite,&WorldOT[activeBuff],5);
			GsSortFastSprite(&SpriteObject[1]->sprite,&WorldOT[activeBuff],5);
			// Rear mountain sprites
			GsSortFastSprite(&SpriteObject[2]->sprite,&WorldOT[activeBuff],4);
			GsSortFastSprite(&SpriteObject[3]->sprite,&WorldOT[activeBuff],4);
			GsSortFastSprite(&SpriteObject[4]->sprite,&WorldOT[activeBuff],4);
			GsSortFastSprite(&SpriteObject[5]->sprite,&WorldOT[activeBuff],4);
			// Front mountain sprites
			GsSortFastSprite(&SpriteObject[6]->sprite,&WorldOT[activeBuff],3);
			GsSortFastSprite(&SpriteObject[7]->sprite,&WorldOT[activeBuff],3);
			GsSortFastSprite(&SpriteObject[8]->sprite,&WorldOT[activeBuff],3);
			GsSortFastSprite(&SpriteObject[9]->sprite,&WorldOT[activeBuff],3);
			// Floor sprites
			GsSortFastSprite(&SpriteObject[10]->sprite,&WorldOT[activeBuff],1);
			GsSortFastSprite(&SpriteObject[11]->sprite,&WorldOT[activeBuff],1);
			GsSortFastSprite(&SpriteObject[12]->sprite,&WorldOT[activeBuff],1);
			// small logo
			GsSortFastSprite(&SpriteObject[13]->sprite,&WorldOT[activeBuff],0);
			// Big bone
			GsSortFastSprite(&SpriteObject[19]->sprite,&WorldOT[activeBuff],0);
			if(display_level)GsSortFastSprite(&SpriteObject[23]->sprite,&WorldOT[activeBuff],0);

			// display world objects
			for(idx=0;idx<MAX_LEVEL_ONE_OBJ;idx++)
				{
				if((WorldArray[idx].moving)&&(WorldArray[idx].move_dir))
					{
					WorldArray[idx].x_offset +=4;
					if((SpriteObject[22]->moving)&&(!jumping)&&(idx==which_plat)) MoveLeft();
					if(WorldArray[idx].x_offset>128) WorldArray[idx].move_dir = false;
					}
					else if((WorldArray[idx].moving)&&(!WorldArray[idx].move_dir))
						{
						WorldArray[idx].x_offset -=4;
						if((SpriteObject[22]->moving)&&(!jumping)&&(idx==which_plat)) MoveRight();
						if(WorldArray[idx].x_offset<1) WorldArray[idx].move_dir = true;
						}
				scr_pos = ((WorldArray[idx].global_pos + WorldArray[idx].x_offset)-where_you_are);
				if((scr_pos<=SCREEN_WIDTH)&&((scr_pos+WorldArray[idx].type->width)>=0)&&(WorldArray[idx].display))
					{
					WorldArray[idx].on_scr = true;
					WorldArray[idx].type->sprite.x = WorldArray[idx].type->collide.x = scr_pos;
					if(WorldArray[idx].disc != PLATFORM)
						{
						if(!dead_spinning) CheckBones (50+idx);
						if(WorldArray[idx].disc == BONE) GsSortFastSprite(&SpriteObject[50+idx]->sprite,&WorldOT[activeBuff],0);
							else GsSortFastSprite(&SpriteObject[50+idx]->sprite,&WorldOT[activeBuff],2);
						}
						else GsSortFastSprite(&SpriteObject[50+idx]->sprite,&WorldOT[activeBuff],1);
					}
					else WorldArray[idx].on_scr = false;
				}
			
			// Bone count
			SpriteObject[20]->sprite.u = (char_xlut[(no_bones/10)+16]);
			SpriteObject[20]->sprite.v = ((char_ylut[(no_bones/10)+16])+108);
			SpriteObject[21]->sprite.u = (char_xlut[(no_bones%10)+16]);
			SpriteObject[21]->sprite.v = ((char_ylut[(no_bones%10)+16])+108);
			GsSortFastSprite(&SpriteObject[20]->sprite,&WorldOT[activeBuff],0);
			GsSortFastSprite(&SpriteObject[21]->sprite,&WorldOT[activeBuff],0);

			if(key_moving) MoveKeys();

			// Keys collected
			for(idx=0;idx<no_keys;idx++)
				{
				GsSortFastSprite(&SpriteObject[26+idx]->sprite,&WorldOT[activeBuff],2);
				}
			
			// man
			if(!dead_spinning)
				{
				if((jumping)&&(!display_level)) Jump();
					else if((jumping)&&(display_level)) jumping = false;
				if(fall) Falling();
				if(change_frame>1)
					{
					change_frame = 0;
				   switch (frame_counter)
		    			{
		    			case  	1:
			   					SpriteObject[22]->sprite.v = 32;
			   					frame_counter++;
			   					break;
		    			case  	2:
			   					SpriteObject[22]->sprite.v = 64;
			   					frame_counter++;
			   					break;
		    			case  	3:
			   					SpriteObject[22]->sprite.v = 96;
			   					frame_counter++;
			   					break;
		    			case  	4:
			   					SpriteObject[22]->sprite.v = 128;
			   					frame_counter++;
			   					break;
		    			case  	5:
			   					SpriteObject[22]->sprite.v = 96;
			   					frame_counter++;
			   					break;
		    			case  	6:
			   					SpriteObject[22]->sprite.v = 64;
			   					frame_counter++;
			   					break;
		    			case  	7:
			   					SpriteObject[22]->sprite.v = 32;
			   					frame_counter++;
			   					break;
		    			case  	8:
			   					SpriteObject[22]->sprite.v = 0;
			   					frame_counter = 1 ;
			   					break;
						default:
									break;
						}
					}
				}				
			
			GsSortFastSprite(&SpriteObject[22]->sprite,&WorldOT[activeBuff],0);

			// lives
			if((spinning)||(dead_spinning))
				{
				for(idx=0;idx<life;idx++)
					{
					SpriteObject[14+idx]->sprite.rotate += (ONE*10);
					if(SpriteObject[14+idx]->sprite.rotate >= (ONE*350))
						{
						SpriteObject[14+idx]->sprite.rotate = ONE;
						spinning = false;
						if(where_you_are > 1700)
							{
							restart = 1584;
							r_position_x = 123;
							r_position_y = 89;
							r_max_bottom = 89;
							}
						if(dead_spinning)	quit_game = true;
						}
					GsSortSprite(&SpriteObject[14+idx]->sprite,&WorldOT[activeBuff],0);
					}
				}
				else
					{
					for(idx=0;idx<life;idx++)
						{
						GsSortSprite(&SpriteObject[14+idx]->sprite,&WorldOT[activeBuff],0);
						}
					}
			
			// Wait for all non-blocking stuff to finish
			DrawSync(0);

			// Wait for vertical blank interrupt
			VSync(0);

			// Switch display buffer & offscreen buffer
			GsSwapDispBuff();

			// Sort the ordering table
			GsSortClear(back_r,back_g,back_b,&WorldOT[activeBuff]);

			// Draw the objects registered in the Ordering table
			GsDrawOt(&WorldOT[activeBuff]);
			
			}
		}
	}
	
//
// Function to jump dog
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void Jump (void)
	{
	int obj_vel=8, obj_acc=2;
	
	switch (jumping)
		{
		case JUMP_UP:	if(going_up)
							{
							on_plat =false;
							SpriteObject[22]->sprite.y -= (obj_vel>>1);
							obj_vel-=obj_acc;
							if(!obj_vel) obj_vel=1;
							if(SpriteObject[22]->sprite.y < (max_bottom-55)) going_up = false;
							}
							else
								{
								CheckUnder();
								SpriteObject[22]->sprite.y += (obj_vel>>1);
								obj_vel+=obj_acc;
								if(SpriteObject[22]->sprite.y >= max_bottom)
									{
									CheckUnderAgain();
									going_up = true;
									jumping = STILL;
									if(!fall) SpriteObject[22]->sprite.y = max_bottom;
									}
								}
							break;
		case	JUMP_LEFT:	if(going_up)
								{
								on_plat = false;
								SpriteObject[22]->sprite.y -= (obj_vel>>1);
								obj_vel-=obj_acc;
								if(!obj_vel) obj_vel=1;
								if(SpriteObject[22]->sprite.y < (max_bottom-55)) going_up = false;
								MoveRight();
								}
								else
									{
									CheckUnder();
									SpriteObject[22]->sprite.y += (obj_vel>>1);
									obj_vel+=obj_acc;
									if(SpriteObject[22]->sprite.y >= max_bottom)
										{
										CheckUnderAgain();
										going_up = true;
										jumping = STILL;
										if(!fall) SpriteObject[22]->sprite.y = max_bottom;
										}
										else MoveRight();
									}
								break;
		case	JUMP_RIGHT:	if(going_up)
								{
								on_plat = false;
								SpriteObject[22]->sprite.y -= (obj_vel>>1);
								obj_vel-=obj_acc;
								if(!obj_vel) obj_vel=1;
								if(SpriteObject[22]->sprite.y < (max_bottom-55)) going_up = false;
								MoveLeft();
								}
								else
									{
									CheckUnder();
									SpriteObject[22]->sprite.y += (obj_vel>>1);
									obj_vel+=obj_acc;
									if(SpriteObject[22]->sprite.y >= max_bottom)
										{
										CheckUnderAgain();
										going_up = true;
										jumping = STILL;
										if(!fall) SpriteObject[22]->sprite.y = max_bottom;
										}
										else MoveLeft();
									}
								break;
		}
	}

//
// Function to check for collision with bones and stuff
//
// Requires:	index of the current world object to check
//
// Returns:		Nothing
//
void CheckBones (int thing)
	{
	int idx;
	
	SpriteObject[22]->collide.w = 16;
	SpriteObject[22]->collide.x = (SpriteObject[22]->sprite.x+8);
	SpriteObject[22]->collide.y = (SpriteObject[22]->sprite.y+3);

	// check for collision with bones
	if ((RectanglesOverlap(&SpriteObject[22]->collide, &SpriteObject[thing]->collide))&&(WorldArray[thing-50].disc == BONE))
		{
		WorldArray[thing-50].display = false;
		
		if(((no_bones<50)&&(life<5))||((no_bones<25)&&(life==5))) no_bones ++;
		
		if((no_bones==50)&&(life<5))
				{
				life++;
				spinning = true;
				no_bones = 0;
				}
		}
	
	// check for collision with cactus
	if ((RectanglesOverlap(&SpriteObject[22]->collide, &SpriteObject[thing]->collide))&&(WorldArray[thing-50].disc == CACTUS))
		{
		life--;
		dead_spinning = true;
		}
	
	// check for collision with keys
	if ((RectanglesOverlap(&SpriteObject[22]->collide, &SpriteObject[thing]->collide))&&(WorldArray[thing-50].disc == KEY))
		{
		no_keys ++;
		WorldArray[thing-50].display = false;
		if(!key_moving) key_moving = true;
			else
				{
				for(idx=0;idx<5;idx++)
					{
					SpriteObject[26+idx]->sprite.y = -32;
					}
				key_counter = 0;
				key_moving = true;
				}

		}
	
	}

//
// Function to check for collision with platforms
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void CheckUnder (void)
	{
	int idx=0, on_something = false;
	
	SpriteObject[22]->collide.w = 8;
	if(SpriteObject[22]->sprite.u == 32) SpriteObject[22]->collide.x = (SpriteObject[22]->sprite.x+15);
		else SpriteObject[22]->collide.x = (SpriteObject[22]->sprite.x+9);
	SpriteObject[22]->collide.y = (SpriteObject[22]->sprite.y+3);

	while(!on_something)
		{
		if((WorldArray[idx].disc == PLATFORM)&&(WorldArray[idx].on_scr))
			{
			if((XOverlap(&SpriteObject[22]->collide,&SpriteObject[idx+50]->collide))&&((SpriteObject[22]->sprite.y+SpriteObject[22]->sprite.h)<=SpriteObject[idx+50]->sprite.y))
				{
				if(which_plat == -1) which_plat = idx;
				if(which_plat!=idx)
					{
					if((jumping)&&(!going_up)) max_bottom = (WorldArray[idx].global_y-(SpriteObject[22]->sprite.h));
					if(WorldArray[idx].moving) SpriteObject[22]->moving = true;
						else SpriteObject[22]->moving = false;
					if(!jumping)
						{
						jumping = JUMP_UP;
						going_up = false;
						}
					which_plat = idx;
					on_something = true;
					}
					else
						{
						on_plat = true;
						if((jumping)&&(!going_up)) max_bottom = (WorldArray[idx].global_y-(SpriteObject[22]->sprite.h));
						if(WorldArray[idx].moving) SpriteObject[22]->moving = true;
							else SpriteObject[22]->moving = false;
						on_something = true;
						}
				}
				else if(on_plat == false)
					{
					fall = true;
					}
			}
		idx++;
		if(idx>MAX_OBJECTS) on_something = true;
		}
	}

//
// Function to check under dog to see if he's fallen off a platform
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void CheckUnderAgain (void)
	{
	on_plat = false;
	SpriteObject[22]->sprite.y -=10;
	CheckUnder();
	if(((!on_plat)&&(!going_up))||((!on_plat)&&(!jumping)))
		{
		max_bottom = 148;
		fall = true;
		}
	SpriteObject[22]->sprite.y +=10;
	}

//
// Function to make dog fall when no longer on a platform
//
// Requires:	Nothing
//
// Returns:		Nothing
//
void Falling (void)
	{
	int obj_vel=10, obj_acc=2;
	
	if(!jumping)
		{
		obj_vel+=obj_acc;
		SpriteObject[22]->sprite.y += (obj_vel>>1);
		if(!on_plat) SpriteObject[22]->moving = false;
		if(SpriteObject[22]->sprite.y >= max_bottom)
			{
			going_up = true;
			jumping = STILL;
			SpriteObject[22]->sprite.y = max_bottom;
			fall = false;
			}
		}
	}
		
//
// Function to see if a collision has occured on either axis
//
// Requires:	Pointers to the two collision RECTs
//
// Returns:		Boolean value - true if occured
//
char RectanglesOverlap (RECT* first, RECT* second)
	{
	return((XOverlap(first,second))&&(YOverlap(first, second)));
	}


//
// Function to see if a collision has occured on x axis
//
// Requires:	Pointers to the two collision RECTs
//
// Returns:		Boolean value - true if occured
//
char XOverlap (RECT* first, RECT* second)
	{
	if (first->x >= second->x) return (first->x < second->x + second->w);
		else return (first->x + first->w > second->x);
	}


//
// Function to see if a collision has occured on y axis
//
// Requires:	Pointers to the two collision RECTs
//
// Returns:		Boolean value - true if occured
//
char YOverlap (RECT* first, RECT* second)
	{
	if (first->y >= second->y) return (first->y < second->y + second->h);
		else return (first->y + first->h > second->y);
	}

//
// Function to create a basic pause
//
// Requires:	Total of duration
//
// Returns:		Nothing
//
void Pause (int duration)
	{
	int idx;
	
	for (idx=0;idx<duration;idx++)
		{
		VSync(0);
		}
	}

// This section of code has pretty much been robbed from the sony check demo - I'm not ashamed!
// I just haven't had time to sort any original sound stuff or music out yet

// Sound data on memory playback preparation
void InitSound(void)
	{
	// VAB opening and transmission to sound buffer
	vab = SsVabTransfer( (u_char*)VHData, (u_char*)VBData, 1, 1 );
	if (vab < 0)
		{
		printf("SsVabTransfer failed (%d)\n", vab);
		return;
		}

	// SEQ opening
	seq = SsSeqOpen((u_long *)SeqData, vab);
	if (seq < 0) printf("SsSeqOpen failed (%d)\n", seq);
	}

// Sound Playback
void PlaySEQ (void)
	{
	SsSetMVol(MVOL, MVOL);									// Main volume setting
	SsSeqSetVol(seq, SVOL, SVOL);							// Volume setting for SEQ
	SsSeqPlay(seq, SSPLAY_PLAY, SSPLAY_INFINITY);	// Playback switch ON
	}
	
// Sound playback termination
void StopSound(void)
	{
	SsSeqStop(seq);			// Playback switch OFF
	VSync(0);
	VSync(0);
	SsSeqClose(seq);			// SEQ close
	SsVabClose(vab);			// VAB close
	}

//
// Move keys up and down
//
void MoveKeys (void)
	{
	int idx;
	
	if (key_counter == 0)
		{
		for(idx=0;idx<no_keys;idx++)
			{
			SpriteObject[26+idx]->sprite.y += 2;
			}
		}
	else if (key_counter == 100)
		{
		for(idx=0;idx<no_keys;idx++)
			{
			SpriteObject[26+idx]->sprite.y -= 2;
			}
		}

	if (SpriteObject[26]->sprite.y == 0) key_counter += 1;
		else if ((SpriteObject[26]->sprite.y == -32)&&(key_counter == 100))
			{
			key_moving = false;
			key_counter = 0;
			}

	}
