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

#define CJ_ADD 0x68
#define LM_ADD 0x2D4A

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

	//u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0; //In these variables there will be information about keys detected in the previous frame

	//printf("\x1b[0;0HPress Start to exit.");
	//printf("\x1b[1;0HCirclePad position:");
	int currpos = 0;

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


			//These two lines must be rewritten because we cleared the whole console
			printf("\x1b[0;0HPokemon Shuffle Editor 0.2 by suloku");
			printf("\x1b[2;0HCurrent values:");

			printf("\x1b[4;5HJewels: %3d", jewels);
			printf("\x1b[5;5HCoins: %5d", coins);
			printf("\x1b[6;5HMegaSpeedups: %2d", megas);
			printf("\x1b[7;5HStocked lives: %2d", lives);
			
			if (currpos == 0){printf("\x1b[4;0H-->\x1b[5;0H   \x1b[6;0H   \x1b[7;0H   ");}
			if (currpos == 1){ printf("\x1b[5;0H-->\x1b[4;0H   \x1b[6;0H   \x1b[7;0H   ");}
			if (currpos == 2){ printf("\x1b[6;0H-->\x1b[4;0H   \x1b[5;0H   \x1b[7;0H   ");}
			if (currpos == 3){ printf("\x1b[7;0H-->\x1b[4;0H   \x1b[5;0H   \x1b[6;0H   ");}
			
			printf("\x1b[10;0H");
			printf("Controls:\n");
			printf("\tDpad Up/Down: select value\n");
			printf("\tDpad Right/Left: value inc/dec (except coins)\n");
			printf("\tHold R trigger + Dpad Right/Left: value +/- 1\n");
			printf("\tHold L trigger + Dpad Right/Left:\n\t\tvalue +/- 10000 (coins only)\n");
			printf("\n\n\tX: max jewels");
			printf("\n\tY: max coins");
			printf("\n\tB: max mega speed ups");
			printf("\n\tA: max stocked lives");
			printf("\n\n\nPress START to exit.");
			printf("\nPress SELECT to save and exit.");

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

			//These two lines must be rewritten because we cleared the whole console
			printf("\x1b[0;0HPokemon Shuffle Editor 0.2 by suloku");
			printf("\x1b[2;0HCurrent values:");

			printf("\x1b[4;5HJewels: %3d", jewels);
			printf("\x1b[5;5HCoins: %5d", coins);
			printf("\x1b[6;5HMegaSpeedups: %2d", megas);
			printf("\x1b[7;5HStocked lives: %2d", lives);
			
			if (currpos == 0){printf("\x1b[4;0H-->\x1b[5;0H   \x1b[6;0H   \x1b[7;0H   ");}
			if (currpos == 1){ printf("\x1b[5;0H-->\x1b[4;0H   \x1b[6;0H   \x1b[7;0H   ");}
			if (currpos == 2){ printf("\x1b[6;0H-->\x1b[4;0H   \x1b[5;0H   \x1b[7;0H   ");}
			if (currpos == 3){ printf("\x1b[7;0H-->\x1b[4;0H   \x1b[5;0H   \x1b[6;0H   ");}
			
			printf("\x1b[10;0H");
			printf("Controls:\n");
			printf("\tDpad Up/Down: select value\n");
			printf("\tDpad Right/Left: value inc/dec (except coins)\n");
			printf("\tHold R trigger + Dpad Right/Left: value +/- 1\n");
			printf("\tHold L trigger + Dpad Right/Left:\n\t\tvalue +/- 1000 (coins only)\n");
			printf("\n\n\tX: max jewels");
			printf("\n\tY: max coins");
			printf("\n\tB: max mega speed ups");
			printf("\n\tA: max stocked lives");
			printf("\n\n\nPress START to exit.");
			printf("\nPress SELECT to save and exit.");
			
			if (kDown & KEY_DRIGHT && kHeld & KEY_R){
				switch(currpos){
					case 0  :
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
					   lives++;
					   if (lives > 99) lives = 99;
					   break; /* optional */
				}
			}else if (kDown & KEY_DLEFT && kHeld & KEY_R){
				switch(currpos){
					case 0  :
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
					   lives--;
					   if (lives < 0) lives = 0;
					   break; /* optional */
				}
			}else if (kHeld & KEY_DRIGHT && !(kHeld & KEY_R)){
				switch(currpos){
					case 0  :
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
					   lives++;
					   if (lives > 99) lives = 99;
					   break; /* optional */
				}
			}else if (kHeld & KEY_DLEFT && !(kHeld & KEY_R)){
				switch(currpos){
					case 0  :
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
					   lives--;
					   if (lives < 0) lives = 0;
					   break; /* optional */
				}
			}else if (kDown & KEY_DUP){
				currpos--;
				if (currpos < 0) currpos = 3;
			}else if (kDown & KEY_DDOWN){
				currpos++;
				if (currpos > 3) currpos=0;
			}else if (kDown & KEY_X){
				jewels = 150;
			}else if (kDown & KEY_Y){
				coins = 99999;
			}else if (kDown & KEY_B){
				megas = 99;
			}else if (kDown & KEY_A){
				lives = 99;
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
