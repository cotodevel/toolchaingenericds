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

#endif
