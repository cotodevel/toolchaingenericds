#ifndef __keyboard9_h__
#define __keyboard9_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

#include "ipcfifoTGDS.h"

#define KB_NORMAL 0
#define KB_CAPS   1
#define KB_SHIFT  2

#define ECHO_ON	 0
#define ECHO_OFF 1

#define BSP	0x8		// Backspace
#define CAP	0x2		// Caps
#define RET	'\n'	// Enter
#define SHF	0x4		// Shift

#define PEN_DOWN (~ ( ((struct sIPCSharedTGDS *)TGDSIPCStartAddress)->buttons7) & (1 << 6))


#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const unsigned char keyboard_Hit[12][32];
extern const unsigned char keyboard_Hit_Shift[12][32];

extern void setTile(uint16 *map, int x, int y, int pal);
extern void initKeyboard();
extern char processKeyboard(char* str, unsigned int max, unsigned int echo);

#ifdef __cplusplus
}
#endif
