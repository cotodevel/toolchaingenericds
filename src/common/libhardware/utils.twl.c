
#include "utils.twl.h"
#include "utilsTGDS.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

//#include <serial.h>
//#include "touch.h"
//#include "memory.h"

bool cdcIsAvailable(void) {
	return isDSiMode();
}
