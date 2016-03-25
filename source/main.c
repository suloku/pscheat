/*
	Circle Pad example made by Aurelio Mannara for ctrulib
	Please refer to https://github.com/smealum/ctrulib/blob/master/libctru/include/3ds/services/hid.h for more information
	This code was modified for the last time on: 12/13/2014 2:20 UTC+1

	This wouldn't be possible without the amazing work done by:
	-Smealum
	-fincs
	-WinterMute
	-yellows8
	-plutoo
	-mtheall
	-Many others who worked on 3DS and I'm surely forgetting about
*/

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "filesystem.h"

//Megaspeedups/stock lives
//00000000   0000101    000     1100011    0010000
//   8?       7 MS       3?     7 lives       7?

#define MEGAMASK 4278321151
#define LIVEMASK 4294951039
#define LIVEINDEX_LO 7
#define LIVEINDEX_HI 31-18
#define MEGAINDEX_LO 17
#define MEGAINDEX_HI 31-8

//COINS/jewels
//0100       10010110  11000110010011111     000
// 4 ?       8 jewels        17 coins        3 ?
#define JEWELMASK 4027580415
#define COINMASK 4293918727
#define JEWELINDEX_LO 20
#define JEWELINDEX_HI 31-4
#define COININDEX_LO 3
#define COININDEX_HI 31-12

#define ITEMMASK 0x1

#define CJ_ADD 		0x68 //Coin/jewel address
#define LM_ADD 		0x2D4A //Mega speedup/lives address
#define ITEM_ADD 	0x2D4D
/*
2D4D: Raise Max Level
2D4E: Level Up
2D4F: Exp. Booster S
2D50: Exp. Booster M
2D51: Exp. Booster L
2D52: Skill Booster S
2D53: Skill Booster M
2D54: Skill Booster L
2D55: Unknown Enhancement
*/
#define BAT_ADD		0xd0
/*
+5moves
+time
+exp
complexity
DD
attackup
*/

u64 size;
u8* buffer = NULL;

int exitnow = 0;

void myexit_error(char* msg){
	printf("\x1b[0;0H");
	printf("%s", msg);
	printf("\n Press START to exit");
	while(1){
		hidScanInput();
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) break;
	}
	gfxExit();
	exitnow = 1;
	return ;
}

void myexit_exit(){
	// Exit services
	if (buffer != NULL) free(buffer);
	filesystemExit();
	gfxExit();
	return;
}

int main(int argc, char **argv)
{
	int jewels = 0;
	int coins = 0;
	int megas = 0;
	int lives = 0;
	u8 lvlraise = 0;
	u8 battle[7];
	u8 items[8];

	//Matrix containing the name of each key. Useful for printing when a key is pressed
/*	char keysNames[32][32] = {
		"KEY_A", "KEY_B", "KEY_SELECT", "KEY_START",
		"KEY_DRIGHT", "KEY_DLEFT", "KEY_DUP", "KEY_DDOWN",
		"KEY_R", "KEY_L", "KEY_X", "KEY_Y",
		"", "", "KEY_ZL", "KEY_ZR",
		"", "", "", "",
		"KEY_TOUCH", "", "", "",
		"KEY_CSTICK_RIGHT", "KEY_CSTICK_LEFT", "KEY_CSTICK_UP", "KEY_CSTICK_DOWN",
		"KEY_CPAD_RIGHT", "KEY_CPAD_LEFT", "KEY_CPAD_UP", "KEY_CPAD_DOWN"
	};
*/
	// Initialize services
//#ifdef __cia
	filesystemInit(0x0004000000141000, 1);
//#else
//	filesystemInit();
//#endif
	gfxInitDefault();

	//Initialize console on top screen. Using NULL as the second argument tells the console library to use the internal console structure as current one
	consoleInit(GFX_TOP, NULL);


	printf("\x1b[0;0HPokemon Shuffle Editor 0.3 by suloku");

	//u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0; //In these variables there will be information about keys detected in the previous frame

	//printf("\x1b[0;0HPress Start to exit.");
	//printf("\x1b[1;0HCirclePad position:");
	int currpos = 0;
	
	int menu = 0; //Menu 0= normal // 1 = battle items // 2= extra items

	// Load savegame
	//Try to open savefile
	Result res = getSaveGameFileSize("/savedata.bin",&size);
	if (res != 0 ){
		myexit_error("Can't open savedata.bin\n");
		if (exitnow){ myexit_exit(); return 0;}
	} else{
		buffer = (u8*)malloc(size);
		res = readBytesFromSaveFile("/savedata.bin",0,buffer,size);
		if (res != 0 ){
		myexit_error("Failed to read savedata.bin\n\n");
		if (exitnow){ myexit_exit(); return 0;}
		}
	}
	//Save is loaded in buffer

//Read values from file
	u32 lvMsbuff;
	u32 coinjewbuff;
	memcpy(&lvMsbuff, buffer+LM_ADD, 4);
	memcpy(&coinjewbuff, buffer+CJ_ADD, 4);
	//lvMsbuff = be32 (lvMsbuff);
	
	//Jewels and coins
	int msb = MEGAINDEX_HI;
	int lsb = MEGAINDEX_LO;
	megas = (lvMsbuff >> lsb) & ~(~0 << (msb-lsb+1));
	msb = LIVEINDEX_HI;
	lsb = LIVEINDEX_LO;
	lives = (lvMsbuff >> lsb) & ~(~0 << (msb-lsb+1));
	 //printf("MegaS and lives value: %#.8X\n", (unsigned int)lvMsbuff);
	 //printf("Original mega %d\n", megas);
	 //printf("Original lives %d\n", lives);
 
	//Megas and lives
    msb = JEWELINDEX_HI;
    lsb = JEWELINDEX_LO;
    jewels = (coinjewbuff >> lsb) & ~(~0 << (msb-lsb+1));
    msb = COININDEX_HI;
    lsb = COININDEX_LO;
    coins = (coinjewbuff >> lsb) & ~(~0 << (msb-lsb+1));
    //printf("Coins and jewels value: %#.8X\n", (unsigned int)coinjewbuff);
    //printf("Original jewels %d\n", jewels);
    //printf("Original coins %d\n", coins);
	
	//Battle items
	int i = 0;
	u16 temp;
	for (i=0;i<7;i++)
	{
		memcpy(&temp, buffer+BAT_ADD+i, 2);
		battle[i] = ((temp >> 7) & 0x7F);
	}
	
	//Other items
	
	memcpy(&lvlraise, buffer+ITEM_ADD, 1);
	lvlraise = lvlraise>>1;
	for (i=0;i<8;i++)
	{
		memcpy(items+i, buffer+ITEM_ADD+1+i, 1);
		items[i]=items[i]>>1;
	}

	// Main loop
	while (aptMainLoop())
	{
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();
		//hidKeysHeld returns information about which buttons have are held down in this frame
		u32 kHeld = hidKeysHeld();
		//hidKeysUp returns information about which buttons have been just released
		//u32 kUp = hidKeysUp();

		if (kDown & KEY_X){
			menu ++;
			if (menu > 2) menu = 0;
			currpos = 0;
			consoleClear();
		}

		if (kDown & KEY_START){
			break; // break in order to return to hbmenu
		}
		if (kDown & KEY_SELECT){
			//Change values
				//Change values for jewels and coins
				lvMsbuff = lvMsbuff & LIVEMASK;
				lvMsbuff = lvMsbuff | (lives << LIVEINDEX_LO );
				lvMsbuff = lvMsbuff & MEGAMASK;
				lvMsbuff = lvMsbuff | (megas << MEGAINDEX_LO );
				msb = MEGAINDEX_HI;
				lsb = MEGAINDEX_LO;
				megas = (lvMsbuff >> lsb) & ~(~0 << (msb-lsb+1));
				msb = LIVEINDEX_HI;
				lsb = LIVEINDEX_LO;
				lives = (lvMsbuff >> lsb) & ~(~0 << (msb-lsb+1));
				//	printf("MegaS and lives edited value: %#.8X\n", (unsigned int)lvMsbuff);
				//	printf("Edited mega %d\n", megas);
				//	printf("Edited lives %d\n", lives);

				//Change values for megas and lives
				coinjewbuff = coinjewbuff & COINMASK;
				coinjewbuff = coinjewbuff | (coins << COININDEX_LO );
				coinjewbuff = coinjewbuff & JEWELMASK;
				coinjewbuff = coinjewbuff | (jewels << JEWELINDEX_LO );
				msb = JEWELINDEX_HI;
				lsb = JEWELINDEX_LO;
				jewels = (coinjewbuff >> lsb) & ~(~0 << (msb-lsb+1));
				msb = COININDEX_HI;
				lsb = COININDEX_LO;
				coins = (coinjewbuff >> lsb) & ~(~0 << (msb-lsb+1));
				//printf("Coins and jewels edited value: %#.8X\n", (unsigned int)coinjewbuff);
				//printf("Edited jewels %d\n", jewels);
				//printf("Edited coins %d\n", coins);

			//Add back to buffer
				memcpy(buffer+LM_ADD, &lvMsbuff, 4);
				memcpy(buffer+CJ_ADD, &coinjewbuff, 4);
				
			//Items
			
			//Lvl raise
				memcpy(&temp, buffer+ITEM_ADD, 1);
				temp = temp & ITEMMASK; //get the last bit stat
				lvlraise = lvlraise<<1; //shift
				temp = lvlraise|temp; //Merge values
				memcpy(buffer+ITEM_ADD, &temp, 1);//Add to buffer
			//Battle items
				for(i=0;i<7;i++)
				{
					u16 temp = 0;
					memcpy(&temp, buffer+BAT_ADD+i, 2);
					temp &= 0x7F;
					temp |= battle[i] << 7;
					memcpy(buffer+BAT_ADD+i, &temp, 2);//Add to buffer
				}
			//Other items
				for(i=0;i<8;i++)
				{
					memcpy(&temp, buffer+ITEM_ADD+1+i, 1);
					temp = temp & ITEMMASK; //get the last bit stat
					items[i] = items[i]<<1; //shift
					temp = items[i]|temp; //Merge values
					memcpy(buffer+ITEM_ADD+1+i, &temp, 1);//Add to buffer
				}

			// Write Save savegame
				res = writeBytesToSaveFile("/savedata.bin", 0, buffer, size);
				if (res != 0 ){
					myexit_error("Failed to write savedata.bin\n\n");
					if (exitnow) break;
				}
				break;
		}

		//Do the keys printing only if keys have changed
		//if (kDown != kDownOld || kHeld != kHeldOld || kUp != kUpOld)
		{
			//Clear console
			//consoleClear();
			printf("\x1b[0;0HPokemon Shuffle Editor 0.3 by suloku");

			if (!menu)
			{
				printf("\x1b[2;0HGeneral Items:");

				printf("\x1b[4;5HStocked lives:    %2d", lives);
				printf("\x1b[5;5HCoins:         %5d", coins);
				printf("\x1b[6;5HMegaSpeedups:     %2d", megas);
				printf("\x1b[7;5HMax 1 level:      %2d", lvlraise);
				printf("\x1b[8;5HJewels:          %3d", jewels);				

			}else if (menu == 1)
			{
				printf("\x1b[2;0HBattle Items:");

				printf("\x1b[4;5HMoves +5:         %2d", battle[0]);
				printf("\x1b[5;5HTime +10:         %2d", battle[1]);
				printf("\x1b[6;5HExp Plus:         %2d", battle[2]);
				printf("\x1b[7;5HMega Start:       %2d", battle[3]);
				printf("\x1b[8;5HComplexity -1:    %2d", battle[4]);
				printf("\x1b[9;5HDisruption Delay: %2d", battle[5]);
				printf("\x1b[10;5HAttack Up:        %2d", battle[6]);

			}else if (menu == 2)
			{
				printf("\x1b[2;0HUpgrade Items:");

				printf("\x1b[4;5HLevel up:         %2d", items[0]);
				printf("\x1b[5;5HExp. Booster S:   %2d", items[1]);
				printf("\x1b[6;5HExp. Booster M:   %2d", items[2]);
				printf("\x1b[7;5HExp. Booster L:   %2d", items[3]);
				printf("\x1b[8;5HSkill Booster S:  %2d", items[4]);
				printf("\x1b[9;5HSkill Booster M:  %2d", items[5]);
				printf("\x1b[10;5HSkill Booster L:  %2d", items[6]);
				printf("\x1b[11;5HUnknown:          %2d", items[7]);
			}
			
				if (currpos == 0){printf("\x1b[4;0H-->\x1b[5;0H   \x1b[6;0H   \x1b[7;0H   \x1b[8;0H   \x1b[9;0H   \x1b[10;0H   \x1b[11;0H   ");}
				if (currpos == 1){ printf("\x1b[5;0H-->\x1b[4;0H   \x1b[6;0H   \x1b[7;0H   \x1b[8;0H   \x1b[9;0H   \x1b[10;0H   \x1b[11;0H   ");}
				if (currpos == 2){ printf("\x1b[6;0H-->\x1b[4;0H   \x1b[5;0H   \x1b[7;0H   \x1b[8;0H   \x1b[9;0H   \x1b[10;0H   \x1b[11;0H   ");}
				if (currpos == 3){ printf("\x1b[7;0H-->\x1b[4;0H   \x1b[5;0H   \x1b[6;0H   \x1b[8;0H   \x1b[9;0H   \x1b[10;0H   \x1b[11;0H   ");}
				if (currpos == 4){ printf("\x1b[8;0H-->\x1b[4;0H   \x1b[5;0H   \x1b[6;0H   \x1b[7;0H   \x1b[9;0H   \x1b[10;0H   \x1b[11;0H   ");}
				if (currpos == 5){ printf("\x1b[9;0H-->\x1b[4;0H   \x1b[5;0H   \x1b[6;0H   \x1b[7;0H   \x1b[8;0H   \x1b[10;0H   \x1b[11;0H   ");}
				if (currpos == 6){ printf("\x1b[10;0H-->\x1b[4;0H   \x1b[5;0H   \x1b[6;0H   \x1b[7;0H   \x1b[8;0H   \x1b[9;0H   \x1b[11;0H   ");}
				if (currpos == 7){ printf("\x1b[11;0H-->\x1b[4;0H   \x1b[5;0H   \x1b[6;0H   \x1b[7;0H   \x1b[8;0H   \x1b[9;0H   \x1b[10;0H   ");}
			
			printf("\x1b[13;0H");
			printf("Controls:\n");
			printf("\tDpad Up/Down: select value\n");
			printf("\tDpad Right/Left: value inc/dec\n");
			printf("\tHold R trigger + Dpad Right/Left: value +/- 1\n");
			printf("\tHold L trigger + Dpad Right/Left:\n\t\tvalue +/- 1000 (coins only)\n");
			switch(menu)
			{
				case 0:
					printf("\n\n\tX: change to battle items menu");
					break;
				case 1:
					printf("\n\n\tX: change to upgrade items menu");
					break;
				case 2:
					printf("\n\n\tX: change to general items menu");
					break;
			}

			printf("\n\tY: set selected to MAX");
			//printf("\n\tB: max mega speed ups");
			//printf("\n\tA: max stocked lives");
			printf("\n\n\nPress START to exit.");
			printf("\nPress SELECT to save and exit.");
			
			if (!menu) //General items
			{
				if (kDown & KEY_DRIGHT && kHeld & KEY_R){
					switch(currpos){
						case 4  :
						   jewels++;
						   if (jewels > 150) jewels = 150;
						   break; /* optional */
						case 1 :
						   coins ++;
						   if (coins > 99999) coins = 99999;
						   break; /* optional */
						case 2 :
						   megas++;
						   if (megas > 99) megas = 99;
						   break; /* optional */
						case 3 :
						   lvlraise++;
						   if (lvlraise > 99) lvlraise = 99;
						   break; /* optional */
						case 0 :
						   lives++;
						   if (lives > 99) lives = 99;
						   break; /* optional */
					}
				}else if (kDown & KEY_DLEFT && kHeld & KEY_R){
					switch(currpos){
						case 4  :
						   jewels--;
						   if (jewels < 0) jewels = 0;
						   break; /* optional */
						case 1 :
						   coins --;
						   if (coins < 0 ) coins = 0;
						   break; /* optional */
						case 2 :
						   megas--;
						   if (megas < 0) megas = 0;
						   break; /* optional */
						case 3 :
						   if (lvlraise <= 0) lvlraise = 0;
						   else
								lvlraise--;
						   break; /* optional */
						case 0 :
						   lives--;
						   if (lives < 0) lives = 0;
						   break; /* optional */
					}
				}else if (kHeld & KEY_DRIGHT && !(kHeld & KEY_R)){
					switch(currpos){
						case 4  :
						   jewels++;
						   if (jewels > 150) jewels = 150;
						   break; /* optional */
						case 1 :
						   if (kHeld & KEY_L){
							 coins += 1000;
						   }else{
							 coins += 100;
						   }
						   if (coins > 99999) coins = 99999;
						   break; /* optional */
						case 2 :
						   megas++;
						   if (megas > 99) megas = 99;
						   break; /* optional */
						case 3 :
						   lvlraise++;
						   if (lvlraise > 99) lvlraise = 99;
						   break; /* optional */
						case 0 :
						   lives++;
						   if (lives > 99) lives = 99;
						   break; /* optional */
					}
				}else if (kHeld & KEY_DLEFT && !(kHeld & KEY_R)){
					switch(currpos){
						case 4  :
						   jewels--;
						   if (jewels < 0) jewels = 0;
						   break; /* optional */
						case 1 :
						   if (kHeld & KEY_L){
							 coins -= 1000;
						   }else{
							 coins -= 100;
						   }
						   if (coins < 0 ) coins = 0;
						   break; /* optional */
						case 2 :
						   megas--;
						   if (megas < 0) megas = 0;
						   break; /* optional */
						case 3 :
						   if (lvlraise <= 0) lvlraise = 0;
						   else
								lvlraise--;
						   break; /* optional */
						case 0 :
						   lives--;
						   if (lives < 0) lives = 0;
						   break; /* optional */
					}
				}else if (kDown & KEY_DUP){
					currpos--;
					if (currpos < 0) currpos = 4;
				}else if (kDown & KEY_DDOWN){
					currpos++;
					if (currpos > 4) currpos=0;
				}else if (kDown & KEY_Y){
					switch(currpos)
					{
						case 4:
							jewels = 150;
							break;
						case 1:
							coins = 99999;
							break;
						case 2:
							megas = 99;
							break;
						case 3:
							lvlraise = 99;
							break;
						case 0:
							lives = 99;
							break;
					}
					
				}else if (kDown & KEY_B){
					
				}else if (kDown & KEY_A){
					
				}
			}else if (menu == 1)
			{
				if (kDown & KEY_DRIGHT && kHeld & KEY_R){
					battle[currpos]++;
					if (battle[currpos] > 99) battle[currpos] = 99;
				}else if (kDown & KEY_DLEFT && kHeld & KEY_R){
					if (battle[currpos] <= 0) battle[currpos] = 0;
					else
						battle[currpos]--;
				}else if (kHeld & KEY_DRIGHT && !(kHeld & KEY_R)){
					battle[currpos]++;
					if (battle[currpos] > 99) battle[currpos] = 99;
				}else if (kHeld & KEY_DLEFT && !(kHeld & KEY_R)){
					if (battle[currpos] <= 0) battle[currpos] = 0;
					else
						battle[currpos]--;
				}else if (kDown & KEY_DUP){
					currpos--;
					if (currpos < 0) currpos = 6;
				}else if (kDown & KEY_DDOWN){
					currpos++;
					if (currpos > 6) currpos=0;
				}else if (kDown & KEY_Y){
					battle[currpos] = 99;
				}else if (kDown & KEY_B){
					
				}else if (kDown & KEY_A){
					
				}
			}else if (menu == 2)
			{
				if (kDown & KEY_DRIGHT && kHeld & KEY_R){
					items[currpos]++;
					if (items[currpos] > 99) items[currpos] = 99;
				}else if (kDown & KEY_DLEFT && kHeld & KEY_R){
					if (items[currpos] <= 0) items[currpos] = 0;
					else
						items[currpos]--;
				}else if (kHeld & KEY_DRIGHT && !(kHeld & KEY_R)){
					items[currpos]++;
					if (items[currpos] > 99) items[currpos] = 99;
				}else if (kHeld & KEY_DLEFT && !(kHeld & KEY_R)){
					if (items[currpos] <= 0) items[currpos] = 0;
					else
						items[currpos]--;
				}else if (kDown & KEY_DUP){
					currpos--;
					if (currpos < 0) currpos = 7;
				}else if (kDown & KEY_DDOWN){
					currpos++;
					if (currpos > 7) currpos=0;
				}else if (kDown & KEY_Y){
					items[currpos] = 99;
				}else if (kDown & KEY_B){
					
				}else if (kDown & KEY_A){
					
				}
			}

			//printf("\x1b[3;0H"); //Move the cursor to the fourth row because on the third one we'll write the circle pad position

			//Check if some of the keys are down, held or up
			/*
			int i;
			for (i = 0; i < 32; i++)
			{
				if (kDown & BIT(i)) printf("%s down\n", keysNames[i]);
				if (kHeld & BIT(i)) printf("%s held\n", keysNames[i]);
				if (kUp & BIT(i)) printf("%s up\n", keysNames[i]);
				
			}
			*/
		}

		//Set keys old values for the next framezy
/*		kDownOld = kDown;
		kHeldOld = kHeld;
		kUpOld = kUp;
*/
		circlePosition pos;

		//Read the CirclePad position
		hidCircleRead(&pos);

		//Print the CirclePad position
		//printf("\x1b[2;0H%04d; %04d", pos.dx, pos.dy);

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

		//Wait for VBlank
		gspWaitForVBlank();
	}

	//Clean up and exit
	myexit_exit();
	return 0;
}
