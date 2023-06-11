//
// Basic definitions & globals
//
// Ira Rainey 6/6/97
//
// ira.rainey@gmail.com
//

#include <libps.h>

// Define the basics
#define		true 	1
#define		false 0

// Set ordering table length
#define 		OT_LENGTH	10

// Masks for setting sprite attribute
#define		SIXTEEN_BIT_ON		(1<<25)
#define		EIGHT_BIT_ON		(1<<24)
#define		SPRITE_OFF			(1<<31)

// Primitive related stuff
#define 		PACKETMAX	2048
#define 		PACKETMAX2	(PACKETMAX*24)

// sound stuff
#define 		MVOL		127		//  Main volume level
#define 		SVOL		127		//  SEQ volume level

// Total number of sprite objects
#define 		MAX_OBJECTS 256

// Screen size detail
#define		SCREEN_WIDTH	320
#define		SCREEN_HEIGHT	240

// Define some constants for refresh screen function
#define		LOGO_FADE	0
#define		BONES			1
#define		ABOUT_BOX	2
#define		START_GAME	3
#define		DEADGEEZER	4
#define		PAUSED		5

// types of world objects
#define		BONE			0
#define		PLATFORM		1
#define		CACTUS		2
#define		KEY			3
#define		DOG			4

// Define data addresses
#define	LogoData		0x80112A20
#define	CactusData	0x80116FB0
#define	CopyData		0x801175D0
#define	BothData		0x801185C4
#define	CharData		0x8011974C
#define	TextData		0x8012196C
#define	SkyData		0x801275AC
#define	BackData		0x8012D58C
#define	FrontData	0x8013676C
#define	FloorData	0x8013FF8C
#define	SmlogoData	0x801415AC
#define	HeadData		0x80141C2C
#define	BBoneData	0x8014215C
#define	BoneNum		0x8014243C
#define	PlatData		0x8014465C
#define	ReadyData	0x80144C7C
#define	SBoneData	0x80145E9C
#define	PauseData	0x80145FBC
#define	ManData		0x801474DC
#define	KeyData		0x80149EFC
#define	DogData		0x8014A51C
#define	OverData		0x8014A97C
#define	EndData		0x8014BB9C
#define	SeqData		0x801575BC
#define	VHData		0x8016107C
#define	VBData		0x8016FC9C

// Define size of playing area
#define	NO_OF_SCREENS 12
#define	MAX_POINT	(SCREEN_WIDTH*NO_OF_SCREENS)
#define	MAX_LEVEL_ONE_OBJ	256

// Define the buffers
volatile u_char *bb0, *bb1;

// Sound related variables
short vab, seq;

// Define sprite control structure
typedef struct
	{
	int		alive;
	int		xpos;
	int		ypos;
	int		width;
	int		height;
	GsIMAGE*	image;
	GsSPRITE	sprite;
	RECT		collide;
	char		moving;
	}Object;

// Define the object array for the maximum number of objects
Object ObjectArray[MAX_OBJECTS];

Object *SpriteObject[MAX_OBJECTS];

// Define world object structure
typedef struct
	{
	unsigned int	global_pos;
	int				global_y;
	int				x_offset;
	int				y_offset;
	Object 			*type;
	char				display;
	char				on_scr;
	char				disc;
	char				moving;
	char				move_dir;
	}LevelObject;
	
// Define the global object array for the maximum number of global objects
LevelObject WorldArray[MAX_OBJECTS];

// Define the packet work area
PACKET		GpuPacketArea[2][PACKETMAX2];

// Ordering table variables
GsOT		WorldOT[2];
GsOT_TAG	OTTags[2][1<<OT_LENGTH];

// Which buffer variable
int			activeBuff;

// Box structure for about box
GsBOXF overlay;

// Global position variables
unsigned long	where_you_are;
int scr_pos;

// Background colours
unsigned int back_r, back_g, back_b;

// scrolling message and text look up tables
int char_ylut[64]	= {0,0,0,0,0,0,0,0,
							32,32,32,32,32,32,32,32,
							64,64,64,64,64,64,64,64,
							96,96,96,96,96,96,96,96,
							128,128,128,128,128,128,128,128,
							160,160,160,160,160,160,160,160,
							192,192,192,192,192,192,192,192,
							224,224,224,224,224,224,224,224
							};

int char_xlut[64]	= {0,16,32,48,64,80,96,112,
							0,16,32,48,64,80,96,112,
							0,16,32,48,64,80,96,112,
							0,16,32,48,64,80,96,112,
							0,16,32,48,64,80,96,112,
							0,16,32,48,64,80,96,112,
							0,16,32,48,64,80,96,112,
							0,16,32,48,64,80,96,112,
							};

char *message = {"                              A DOG TALE      A VIDEO GAME OF AMAZINGLY THIN PROPORTIONS FOR THE SONY PLAYSTATION          ALL CODE AND GRAPHICS KNOCKED TOGETHER BY IRA RAINEY, AND THE SNAZZY TUNE WAS 'BORROWED' FROM SONY - I WAS GONNA STICK AN ORIGINAL TUNE IN BUT I NEEDED TO GET THE GAME FINISHED BEFORE I MOVED HOUSE SO I USED THIS TUNE MOSTLY DUE TO TIME CONSTRAINTS, THANKS GUYS!          WELL HERE IT IS THEN MY VERY FIRST PLAYSTATION GAME DEVELOPED ON MY NET YAROZE, WHAT DO YOU RECKON?         YEAH OK I KNOW IT'S NOT EXACTLY CRASH BANDICOOT OR TOMB RAIDER BUT HEY IT'S ONLY MY FIRST ATTEMPT!      JUST THINK OF IT AS MARIO MEETS MANIC MINER            THE STORY?    WELL BASICALLY YOU'RE OUT TO RESCUE YOUR DOG WHO'S BEEN DOGNAPPED BY A NASTY NASTY GANG FOR SOME REASON, AND THEY'VE CHAINED HIM TO A RAILING WITH FIVE HUGE PADLOCKS   SO, ALL YOU NEED TO DO IS COLLECT ALL FIVE KEYS AND UNLOCK HIM   THAT'S ABOUT IT REALLY   A BIT THIN I KNOW BUT WHO GIVES A TOSS - IT'S A GAME AND I DON'T INTEND TO WRITE A BIOGRAPHY FOR A BUNCH OF PIXELS     AND WHO WOULD REALLY CARE IF I DID?      OH YEAH I FORGOT, COLLECT ALL THE BONES YOU CAN ALONG THE WAY - GET FIFTY AND YOU GET AN EXTRA LIFE!      WOW!     A BIG SHOUT HAS GOTTA GO OUT TO ALL THE FOLKS AT SONY, THANKS TO WHOM ALL THIS HAS BECOME POSSIBLE      - NET YAROZE -     PROBABLY THE BEST LUMP OF CASH I'VE EVER SPENT, WELL SINCE THAT CASE OF BEERS AND THE CHICKEN VINDALOO LAST NIGHT ANYWAY!        THANKS SONY! YOU'VE BROUGHT FUN AND ENJOYMENT BACK INTO MY MUNDANE PROGRAMMING EXISTENCE   YOU'VE TAKEN ME BACK TO THE COMMODORE 64 DAYS OF MY YOUTH - CHRIST NOW THAT I THINK ABOUT IT THAT WAS A HELL OF A LONG TIME AGO!         I DON'T WANT TO THINK ABOUT IT!         AM I REALLY THAT OLD ALREADY?   ANYONE ELSE HAPPILY REMEMBER HONG KONG PHOOEY AND INCH HIGH PRIVATE EYE? - THE FIRST TIME AROUND!        THOSE WERE THE DAYS, BACK WHEN I WAS JUST A BROKE, STRUGGLING, BEERSWILLING SEVENTEEN YEAR OLD PROGRAMMER!      AND HERE I AM NOW AS AN ADULT - A BROKE, STRUGGLING, BEERSWILLING TWENTY SEVEN YEAR OLD PROGRAMMER!         HEY NOTHING'S CHANGED!       IS THAT GOOD??         WHO CARES?      WHO'S STILL READING THIS?      YOU SHOULD BE PLAYING THE GAME BY NOW!        SHIT! WILL YOU LOOK AT THE TIME I'VE GOTTA BE UP AT SIX, I'M GONNA WRAP THIS SUCKER UP FOR THE NIGHT, SEE YOU ALL NEXT TIME FOLKS    - KEEP UP THE CODING AND LETS SEE MORE DEMOS -      IRA RAINEY - AUGUST 1997      CONTACT IRA@NETCOMUK.CO.UK   "};
						
int mess_pos;

int life, spinning, dead_spinning;
int no_bones, no_keys, key_moving, key_counter;
int display_level, change_frame, frame_counter;
int going_up, jumping, start_h, max_bottom, on_plat, fall;
int break_out, game_paused, dead, already_run = false;
char quit_game, end_of_game, which_plat;
unsigned long old_padd = 0, restart = 0;
int r_position_x, r_position_y, r_max_bottom;

#define	STILL			0
#define	JUMP_UP 		1
#define	JUMP_LEFT 	2
#define	JUMP_RIGHT	3
						
// Function Prototypes
Object* GetNextFreeObject (void);
void ClearObject(Object*);
void ClearSprite(GsSPRITE*);
void CreateObject(Object*, unsigned long, int);
void InitObjects (void);
void ShowYarozeLogo(void);
void RefreshScreen(int);
void SetupLevelOne(void);
void DisplayFrontEnd(void);
void GameAbout(void);
void ClearWObject(LevelObject*);
unsigned long PadRead(void);
void MoveLeft(void);
void MoveRight(void);
void ScrollText(void);
void DefineObject (int, int, int, int, int, int, int, unsigned long, int);
void PlayLevelOne(void);
void Jump(void);
void CheckBones (int);
void CheckUnder (void);
char RectanglesOverlap (RECT*, RECT*);
char XOverlap (RECT* , RECT* );
char YOverlap (RECT* , RECT* );
void SetupWorld (void);
void Falling (void);
void CheckUnderAgain (void);
void Pause (int);
static long CheckPad (void);
void InitSound (void);
void StopSound (void);
void PlaySEQ (void);
void MoveKeys (void);