#ifdef TWLMODE

#include "utils.twl.h"
#include "utilsTGDS.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

//#include <serial.h>
//#include "touch.h"
#include "memory.h"

//!	Checks whether the application is running in DSi mode.
bool isDSiMode() {
	return __dsimode;
}

bool cdcIsAvailable(void) {
	return isDSiMode() && (__DSiHeader->appflags & 0x01);
}

#ifdef ARM9
/*
	Array used to fill secure area, marked weak to allow nds files to be
	built with no secure area.

	To disable this add 'const int __secure_area__ = 0;'

	Value and type are unimportant the symbol only needs to exist
	elsewhere to prevent this one being linked.

*/

__attribute__((section(".secure")))
__attribute__((weak))
const char __secure_area__[2048];
#endif

#endif
