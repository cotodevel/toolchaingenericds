/* Contributed by Steven
 * 20071213
 * - Minor modifications and SDL code by Ant
 */

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <string.h>
#include "woopsifuncs.h"
#include "graphics.h"
#include "keypadTGDS.h"
#include "consoleTGDS.h"
#include "videoTGDS.h"
#include "biosTGDS.h"
#include "InterruptsARMCores_h.h"

#ifdef USING_SDL

#include "defines.h"

WoopsiUI::FrameBuffer* frameBuffer[2];
WoopsiUI::GadgetStyle* defaultGadgetStyle;

_pads Pad;
_stylus Stylus;

Uint32 initflags = SDL_INIT_VIDEO;
SDL_Surface *screen;
Uint8 video_bpp = 0;
Uint32 videoflags = SDL_SWSURFACE;
SDL_Event event;
s32 mouseX;
s32 mouseY;

void initWoopsiGfxMode() {

	initflags = SDL_INIT_VIDEO;
	video_bpp = 0;
	videoflags = SDL_SWSURFACE;

	// Initialize the SDL library
	if (SDL_Init(initflags) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	// Set video mode
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT * 2, video_bpp, videoflags);
	if (screen == NULL) {
		fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, video_bpp, SDL_GetError());
		SDL_Quit();
		exit(2);
	}

	// Create framebuffers
	frameBuffer[0] = new WoopsiUI::FrameBuffer(screen, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_HEIGHT);
	frameBuffer[1] = new WoopsiUI::FrameBuffer(screen, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	// Initialise default style
	woopsiInitDefaultGadgetStyle();

	// Initialise both arrays
	WoopsiUI::Graphics* graphics = frameBuffer[0]->newGraphics();
	graphics->drawFilledRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	delete graphics;

	graphics = frameBuffer[1]->newGraphics();
	graphics->drawFilledRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	delete graphics;
}

void woopsiVblFunc() {
	SDL_UpdateRect(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT * 2);

	if (Stylus.Newpress) {
		Stylus.Held = true;
		Stylus.Newpress = false;
	}

	Stylus.Released = false;
	
	if (Pad.Held.Left) Pad.HeldTime.Left++;
	if (Pad.Held.Right) Pad.HeldTime.Right++;
	if (Pad.Held.Up) Pad.HeldTime.Up++;
	if (Pad.Held.Down) Pad.HeldTime.Down++;
	if (Pad.Held.A) Pad.HeldTime.A++;
	if (Pad.Held.B) Pad.HeldTime.B++;
	if (Pad.Held.X) Pad.HeldTime.X++;
	if (Pad.Held.Y) Pad.HeldTime.Y++;
	if (Pad.Held.Start) Pad.HeldTime.Start++;
	if (Pad.Held.Select) Pad.HeldTime.Select++;
	if (Pad.Held.L) Pad.HeldTime.L++;
	if (Pad.Held.R) Pad.HeldTime.R++;

	if (Pad.Newpress.Left) Pad.Held.Left = true;
	if (Pad.Newpress.Right) Pad.Held.Right = true;
	if (Pad.Newpress.Up) Pad.Held.Up = true;
	if (Pad.Newpress.Down) Pad.Held.Down = true;
	if (Pad.Newpress.A) Pad.Held.A = true;
	if (Pad.Newpress.B) Pad.Held.B = true;
	if (Pad.Newpress.X) Pad.Held.X = true;
	if (Pad.Newpress.Y) Pad.Held.Y = true;
	if (Pad.Newpress.Start) Pad.Held.Start = true;
	if (Pad.Newpress.Select) Pad.Held.Select = true;
	if (Pad.Newpress.L) Pad.Held.L = true;
	if (Pad.Newpress.R) Pad.Held.R = true;

	Pad.Released.Left = false;
	Pad.Released.Right = false;
	Pad.Released.Up = false;
	Pad.Released.Down = false;
	Pad.Released.A = false;
	Pad.Released.B = false;
	Pad.Released.X = false;
	Pad.Released.Y = false;
	Pad.Released.L = false;
	Pad.Released.R = false;
	Pad.Released.Start = false;
	Pad.Released.Select = false;

	Pad.Newpress.Left = false;
	Pad.Newpress.Right = false;
	Pad.Newpress.Up = false;
	Pad.Newpress.Down = false;
	Pad.Newpress.A = false;
	Pad.Newpress.B = false;
	Pad.Newpress.X = false;
	Pad.Newpress.Y = false;
	Pad.Newpress.L = false;
	Pad.Newpress.R = false;
	Pad.Newpress.Start = false;
	Pad.Newpress.Select = false;
	
	// Read mouse state
	int mState;
	int mX;
	int mY;
	
	mState = SDL_GetMouseState(&mX, &mY);
	
	// Update mouse position
	mouseX = mX;
	mouseY = mY - SCREEN_HEIGHT;
	
	// Check buttons
	if ((mState & SDL_BUTTON_LEFT) && (!Stylus.Held)) {
		
		// New click
		Stylus.Newpress = true;
		Stylus.Held = true;
		Stylus.Released = false;

	} else if ((!(mState & SDL_BUTTON_LEFT)) && (Stylus.Held)) {
		
		// Release
		Stylus.Released = true;
		Stylus.Held = false;
		Stylus.Newpress = false;
	}
	
	// Get key state
    Uint8* keyState = SDL_GetKeyState(NULL);

	updatePadState(SDLK_UP, &(Pad.Held.Up), &(Pad.Newpress.Up), &(Pad.Released.Up), &(Pad.HeldTime.Up));
	updatePadState(SDLK_DOWN, &(Pad.Held.Down), &(Pad.Newpress.Down), &(Pad.Released.Down), &(Pad.HeldTime.Down));
	updatePadState(SDLK_LEFT, &(Pad.Held.Left), &(Pad.Newpress.Left), &(Pad.Released.Left), &(Pad.HeldTime.Left));
	updatePadState(SDLK_RIGHT, &(Pad.Held.Right), &(Pad.Newpress.Right), &(Pad.Released.Right), &(Pad.HeldTime.Right));
	updatePadState(SDLK_z, &(Pad.Held.A), &(Pad.Newpress.A), &(Pad.Released.A), &(Pad.HeldTime.A));
	updatePadState(SDLK_x, &(Pad.Held.B), &(Pad.Newpress.B), &(Pad.Released.B), &(Pad.HeldTime.B));
	updatePadState(SDLK_c, &(Pad.Held.X), &(Pad.Newpress.X), &(Pad.Released.X), &(Pad.HeldTime.X));
	updatePadState(SDLK_v, &(Pad.Held.Y), &(Pad.Newpress.Y), &(Pad.Released.Y), &(Pad.HeldTime.Y));
	updatePadState(SDLK_a, &(Pad.Held.L), &(Pad.Newpress.L), &(Pad.Released.L), &(Pad.HeldTime.L));
	updatePadState(SDLK_s, &(Pad.Held.R), &(Pad.Newpress.R), &(Pad.Released.R), &(Pad.HeldTime.R));
	updatePadState(SDLK_d, &(Pad.Held.Start), &(Pad.Newpress.Start), &(Pad.Released.Start), &(Pad.HeldTime.Start));
	updatePadState(SDLK_f, &(Pad.Held.Select), &(Pad.Newpress.Select), &(Pad.Released.Select), &(Pad.HeldTime.Select));

	// Exit (assigned as Esc on keyboard)
	if (keyState[SDLK_ESCAPE]) exit(0);
	
	// Update other stylus properties
	Stylus.Downtime *= !Stylus.Newpress; // = 0 if newpress
	Stylus.Downtime += Stylus.Held;

	Stylus.Uptime *= !Stylus.Released; // = 0 when released
	Stylus.Uptime += !Stylus.Held;

	if (Stylus.Held) {

		Stylus.DblClick = Stylus.Newpress && (Stylus.Downtime + Stylus.Uptime < DOUBLE_CLICK_TIME);

		if (Stylus.Newpress) {
			Stylus.Vx = Stylus.oldVx = 0;
			Stylus.Vy = Stylus.oldVy = 0;
		} else {
			Stylus.oldVx = Stylus.Vx;
			Stylus.oldVy = Stylus.Vy;
			Stylus.Vx = mouseX - Stylus.X;
			Stylus.Vy = mouseY - Stylus.Y;
		}

		Stylus.X = mouseX;
		Stylus.Y = mouseY;
	}
}

void woopsiWaitVBL() {
	woopsiVblFunc();
}

int fatInitDefault() { return 1; }

void updatePadState(int sdlKey, u16* heldKey, u16* newpressKey, u16* releasedKey, u16* heldTimeKey) {
	if ((keyState[sdlKey]) && ((!*heldKey) && (!*newpressKey))) {
		*newpressKey = true;
	} else if ((!keyState[sdlKey]) && ((*heldKey) || (*newpressKey))) {
		*releasedKey = true;
		*heldKey = false;
		*newpressKey = false;
		*heldTimeKey = 0;
	}
}

#else

// Using libnds

WoopsiUI::FrameBuffer* frameBuffer[2];
WoopsiUI::GadgetStyle* defaultGadgetStyle;

_pads Pad;
_stylus Stylus;

void woopsiUpdateInput() {
	struct XYTscPos touch;
	
	// Update held timers
	if (Pad.Held.Left) Pad.HeldTime.Left++;
	if (Pad.Held.Right) Pad.HeldTime.Right++;
	if (Pad.Held.Up) Pad.HeldTime.Up++;
	if (Pad.Held.Down) Pad.HeldTime.Down++;
	if (Pad.Held.A) Pad.HeldTime.A++;
	if (Pad.Held.B) Pad.HeldTime.B++;
	if (Pad.Held.X) Pad.HeldTime.X++;
	if (Pad.Held.Y) Pad.HeldTime.Y++;
	if (Pad.Held.Start) Pad.HeldTime.Start++;
	if (Pad.Held.Select) Pad.HeldTime.Select++;
	if (Pad.Held.L) Pad.HeldTime.L++;
	if (Pad.Held.R) Pad.HeldTime.R++;
	
	// Get the state of the keys
	scanKeys();
	Pad.Newpress.AllKeys = keysDown();
	Pad.Held.AllKeys = keysHeld();
	Pad.Released.AllKeys = keysUp();
	
	// Update held timers
	if (!Pad.Held.Left) Pad.HeldTime.Left = 0;
	if (!Pad.Held.Right) Pad.HeldTime.Right = 0;
	if (!Pad.Held.Up) Pad.HeldTime.Up = 0;
	if (!Pad.Held.Down) Pad.HeldTime.Down = 0;
	if (!Pad.Held.A) Pad.HeldTime.A = 0;
	if (!Pad.Held.B) Pad.HeldTime.B = 0;
	if (!Pad.Held.X) Pad.HeldTime.X = 0;
	if (!Pad.Held.Y) Pad.HeldTime.Y = 0;
	if (!Pad.Held.Start) Pad.HeldTime.Start = 0;
	if (!Pad.Held.Select) Pad.HeldTime.Select = 0;
	if (!Pad.Held.L) Pad.HeldTime.L = 0;
	if (!Pad.Held.R) Pad.HeldTime.R = 0;

	// Deal with the Stylus.
	XYReadScrPosUser(&touch);
	
	Stylus.Newpress = Pad.Newpress.Touch;
	Stylus.Held     = Pad.Held.Touch;
	Stylus.Released = Pad.Released.Touch;

	Stylus.DblClick = Stylus.Newpress && (Stylus.Downtime + Stylus.Uptime < DOUBLE_CLICK_TIME);
	Stylus.Downtime *= !Stylus.Newpress; // = 0 if newpress
	Stylus.Downtime += Stylus.Held;

	Stylus.Uptime *= !Stylus.Released; // = 0 when released
	Stylus.Uptime += !Stylus.Held;

	if (Stylus.Held) {
		if (Stylus.Newpress) {
			Stylus.Vx = Stylus.oldVx = 0;
			Stylus.Vy = Stylus.oldVy = 0;
		} else{
			Stylus.oldVx = Stylus.Vx;
			Stylus.oldVy = Stylus.Vy;
			Stylus.Vx = touch.touchXpx - Stylus.X;
			Stylus.Vy = touch.touchYpx - Stylus.Y;
		}
		
		Stylus.X = touch.touchXpx;
		Stylus.Y = touch.touchYpx;
	}
}

void woopsiVblFunc() {
	woopsiUpdateInput();
}

void initWoopsiGfxMode() {
	setBacklight(POWMAN_BACKLIGHT_TOP_BIT | POWMAN_BACKLIGHT_BOTTOM_BIT);
	
	//TGDS Projects require this handler: irqSet(IRQ_VBLANK, woopsiVblFunc);

	SETDISPCNT_MAIN(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	SETDISPCNT_SUB(MODE_5_2D | DISPLAY_BG3_ACTIVE);

	VRAMBLOCK_SETBANK_A((u8)(1));	//VRAM_A_MAIN_BG
	VRAMBLOCK_SETBANK_C((u8)(4));	//VRAM_C_SUB_BG

	// Initialise backgrounds
	//bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	//bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

	#define BG_256x256       (1<<14)
	#define BG_15BITCOLOR    (1<<7)
	#define BG_CBB1          (1<<2)
	
	REG_BG3CNT = BG_256x256 | BG_15BITCOLOR | BG_CBB1;	
	REG_BG3CNT_SUB = BG_256x256 | BG_15BITCOLOR | BG_CBB1;	
	
	frameBuffer[1] = new WoopsiUI::FrameBuffer((u16*)BG_BMP_RAM(0), SCREEN_WIDTH, SCREEN_HEIGHT);
	frameBuffer[0] = new WoopsiUI::FrameBuffer((u16*)BG_BMP_RAM_SUB(0), SCREEN_WIDTH, SCREEN_HEIGHT);

	woopsiInitDefaultGadgetStyle();
	
	memset( &Stylus, 0, sizeof(_stylus) );
}

__attribute__((section(".itcm")))
void woopsiWaitVBL() {
	handleARM9SVC();	/* Do not remove, handles TGDS services */
	IRQWait(IRQ_HBLANK);
}

#endif

void woopsiInitDefaultGadgetStyle() {
	defaultGadgetStyle = new WoopsiUI::GadgetStyle();

	defaultGadgetStyle->colours.back = woopsiRGB(20, 20, 20);
	defaultGadgetStyle->colours.shine = woopsiRGB(31, 31, 31);
	defaultGadgetStyle->colours.highlight = woopsiRGB(12, 17, 23);
	defaultGadgetStyle->colours.shadow = woopsiRGB(0, 0, 0);
	defaultGadgetStyle->colours.fill = woopsiRGB(24, 24, 24);
	defaultGadgetStyle->colours.dark = woopsiRGB(15, 15, 15);
	defaultGadgetStyle->font = new WoopsiUI::NewTopaz();
	defaultGadgetStyle->glyphFont = new WoopsiUI::GlyphFont();
}

void woopsiFreeDefaultGadgetStyle() {
	delete defaultGadgetStyle->font;
	delete defaultGadgetStyle->glyphFont;
	delete defaultGadgetStyle;
}

void woopsiFreeFrameBuffers() {

	// Delete the framebuffers
	delete frameBuffer[0];
	delete frameBuffer[1];
}
