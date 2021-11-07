/* Contributed by Steven
 * 20071213
 * - Minor modifications and SDL code by Ant
 */

#ifndef _WOOPSIFUNCS_H_
#define _WOOPSIFUNCS_H_

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include "framebuffer.h"
#include "bitmapwrapper.h"
#include "gadgetstyle.h"
#include "newtopaz.h"
#include "glyphfont.h"
#include "defines.h"

/**
 * Structure to hold the status of the DS' control buttons.
 */
typedef struct PAD {
	union {
		struct {
			u16 A:1;			/**< A button */
			u16 B:1;			/**< B button */
			u16 Select:1;		/**< Select button */
			u16 Start:1;		/**< Start button */
			u16 Right:1;		/**< Right d-pad */
			u16 Left:1;			/**< Left d-pad */
			u16 Up:1;			/**< Up d-pad */
			u16 Down:1;			/**< Down d-pad */
			u16 R:1;			/**< R button */
			u16 L:1;			/**< L button */
			u16 X:1;			/**< X button */
			u16 Y:1;			/**< Y button */
			u16 Touch:1;		/**< Stylus touch */
			u16 Lid:1;			/**< Lid state */
		};
		u16 AllKeys;			/**< Bitmask representing entire control */
	};
} _pad;

/**
 * Struct containing the length of time that each button has been held down.
 */
typedef struct PadHeldTimeStruct {
	u32 A;			/**< A button */
	u32 B;			/**< B button */
	u32 Select;		/**< Select button */
	u32 Start;		/**< Start button */
	u32 Right;		/**< Right button */
	u32 Left;		/**< Left button */
	u32 Up;			/**< Up button */
	u32 Down;		/**< Down button */
	u32 R;			/**< R button */
	u32 L;			/**< L button */
	u32 X;			/**< X button */
	u32 Y;			/**< Y button */
} PadHeldTime;

/**
 * Structure holding a pad struct for each possible button state.
 */
typedef struct PADS {
   _pad Held;					/**< Each value set to 1 represents a held button */
   _pad Released;				/**< Inverse of held */
   _pad Newpress;				/**< Each value set to 1 represents a newly pressed button */
	PadHeldTime HeldTime;		/**< Length of time each button has been held */
} _pads;

/**
 * Struture holding the status of the stylus.
 */
typedef struct STYLUS {
	u8 Held:1;					/**< Stylus is held down */
	u8 Released:1;				/**< Inverse of held */
	u8 Newpress:1;				/**< Stylus has been newly pressed */
	u8 unused:5;				/**< Padding bits */
	s16 X;						/**< X co-ord of the stylus */
	s16 Y;						/**< Y co-ord of the stylus */
	s16 Vx;						/**< X distance the stylus has moved in the last vblank */
	s16 Vy;						/**< Y distance the stylus has moved in the last vblank */
	s16 oldVx;					/**< X co-ord of the stylus at the previous vblank */
	s16 oldVy;					/**< Y co-ord of the stylus at the previous vblank */
	s16 Downtime;				/**< Number of vblanks stylus has been held */
	s16 Uptime;					/**< Number of vblanks stylus has been released */
	s16 DblClick;				/**< Indicates a double click */
} _stylus;

/**
 * Instance of the _pads struct allows access to the pad state.
 */
extern _pads Pad;

/**
 * Instance of the _stylus struct allows access to the stylus state.
 */
extern _stylus Stylus;

#ifdef USING_SDL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void updatePadState(int sdlKey, u16* heldKey, u16* newpressKey, u16* releasedKey, u16* heldTimeKey);

#else

#include <unistd.h>
#include "biosTGDS.h"

#undef BgType_Text8bpp
#undef BgType_Text4bpp
#undef BgType_Rotation
#undef BgType_ExRotation
#undef BgType_Bmp8
#undef BgType_Bmp16

//! Allowed background types, used in bgInit and bgInitSub.
typedef enum
{
	BgType_Text8bpp, 	//!< 8bpp Tiled background with 16 bit tile indexes and no allowed rotation or scaling
	BgType_Text4bpp, 	//!< 4bpp Tiled background with 16 bit tile indexes and no allowed rotation or scaling
	BgType_Rotation, 	//!< Tiled background with 8 bit tile indexes Can be scaled and rotated
	BgType_ExRotation, 	//!< Tiled background with 16 bit tile indexes Can be scaled and rotated
	BgType_Bmp8, 		//!< Bitmap background with 8 bit color values which index into a 256 color palette
	BgType_Bmp16 		//!< Bitmap background with 16 bit color values of the form aBBBBBGGGGGRRRRR (if 'a' is set the pixel will be rendered...if not the pixel will be transparent)
}BgType;

#undef BgSize_R_128x128
#undef BgSize_R_256x256
#undef BgSize_R_512x512
#undef BgSize_R_1024x1024

#undef BgSize_T_256x256
#undef BgSize_T_512x256
#undef BgSize_T_256x512
#undef BgSize_T_512x512

#undef BgSize_ER_128x128 
#undef BgSize_ER_256x256 
#undef BgSize_ER_512x512 
#undef BgSize_ER_1024x1024 
#undef BgSize_B8_128x128 
#undef BgSize_B8_256x256 
#undef BgSize_B8_512x256 
#undef BgSize_B8_512x512 
#undef BgSize_B8_1024x512 
#undef BgSize_B8_512x1024 

#undef BgSize_B16_128x128 
#undef BgSize_B16_256x256 
#undef BgSize_B16_512x256 
#undef BgSize_B16_512x512 
	
/**
 * \brief Allowed background Sizes
 * The lower 16 bits of these defines can be used directly to set the background control register bits
 * \ingroup api_group
 */
typedef enum
{
	BgSize_R_128x128 =   (0 << 14), /*!< 128 x 128 pixel rotation background */
	BgSize_R_256x256 =   (1 << 14), /*!< 256 x 256 pixel rotation background */
	BgSize_R_512x512 =   (2 << 14), /*!< 512 x 512 pixel rotation background */
	BgSize_R_1024x1024 = (3 << 14), /*!< 1024 x 1024 pixel rotation background */

	BgSize_T_256x256 = (0 << 14) | (1 << 16), /*!< 256 x 256 pixel text background */
	BgSize_T_512x256 = (1 << 14) | (1 << 16), /*!< 512 x 256 pixel text background */
	BgSize_T_256x512 = (2 << 14) | (1 << 16), /*!< 256 x 512 pixel text background */
	BgSize_T_512x512 = (3 << 14) | (1 << 16), /*!< 512 x 512 pixel text background */

	BgSize_ER_128x128 = (0 << 14) | (2 << 16), /*!< 128 x 128 pixel extended rotation background */
	BgSize_ER_256x256 = (1 << 14) | (2 << 16), /*!< 256 x 256 pixel extended rotation background */
	BgSize_ER_512x512 = (2 << 14) | (2 << 16), /*!< 512 x 512 pixel extended rotation background */
	BgSize_ER_1024x1024 = (3 << 14) | (2 << 16),/*!< 1024 x 1024 extended pixel rotation background */

	BgSize_B8_128x128 =  ((0 << 14) | (1 << 7) | (3 << 16)),  /*!< 128 x 128 pixel 8 bit bitmap background */
	BgSize_B8_256x256 =  ((1 << 14) | (1 << 7) | (3 << 16)),  /*!< 256 x 256 pixel 8 bit bitmap background */
	BgSize_B8_512x256 =  ((2 << 14) | (1 << 7) | (3 << 16)),  /*!< 512 x 256 pixel 8 bit bitmap background */
	BgSize_B8_512x512 =  ((3 << 14) | (1 << 7) | (3 << 16)),  /*!< 512 x 512 pixel 8 bit bitmap background */
	BgSize_B8_1024x512 = (1 << 14) |  (3 << 16),		    	/*!< 1024 x 512 pixel 8 bit bitmap background */
	BgSize_B8_512x1024 = (0) | (3 << 16),					/*!< 512 x 1024 pixel 8 bit bitmap background */

	BgSize_B16_128x128 = ((0 << 14) | BIT(7) | BIT(2) | (4 << 16)),  /*!< 128 x 128 pixel 16 bit bitmap background */
	BgSize_B16_256x256 = ((1 << 14) | BIT(7) | BIT(2) | (4 << 16)),  /*!< 256 x 256 pixel 16 bit bitmap background */
	BgSize_B16_512x256 = ((2 << 14) | BIT(7) | BIT(2) | (4 << 16)),  /*!< 512 x 512 pixel 16 bit bitmap background */
	BgSize_B16_512x512 = ((3 << 14) | BIT(7) | BIT(2) | (4 << 16)),  /*!< 1024 x 1024 pixel 16 bit bitmap background */
}BgSize;

#endif

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __cplusplus
/**
 * Pointers to the DS' framebuffers.
 */
extern WoopsiUI::FrameBuffer* frameBuffer[2];

/**
 * Pointer to the default gadget style.
 */
extern WoopsiUI::GadgetStyle* defaultGadgetStyle;
#endif

/**
 * Initialise the DS' screens into framebuffer mode.  Also sets up some other subsystems
 * and IRQs.
 */
extern void initWoopsiGfxMode();

/**
 * Initialise the default gadget style.
 */
extern void woopsiInitDefaultGadgetStyle();

/**
 * Delete the default gadget style.
 */
extern void woopsiFreeDefaultGadgetStyle();

/**
 * Delete the framebuffer objects.
 */
extern void woopsiFreeFrameBuffers();

/**
 * Wait for a VBL.  Switches into a wait state if the lid is closed.
 */
extern void woopsiWaitVBL();


extern void woopsiVblFunc();

/**
 * Update the pad and stylus structs with the latest physical status.  Called
 * every frame by the VBL function.
 * @see woopsiWaitVBL().
 */
extern void woopsiUpdateInput();

#ifdef __cplusplus
}
#endif

#endif
