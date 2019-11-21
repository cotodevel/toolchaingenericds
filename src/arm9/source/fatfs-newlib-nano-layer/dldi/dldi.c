#include "dsregs.h"
#include "dldi.h"
#include "busTGDS.h"

const uint32  DLDI_MAGIC_NUMBER = 
	0xBF8DA5ED;	
	
// Stored backwards to prevent it being picked up by DLDI patchers
const sint8 DLDI_MAGIC_STRING_BACKWARDS [DLDI_MAGIC_STRING_LEN] =
	{'\0', 'm', 'h', 's', 'i', 'h', 'C', ' '} ;

// The only built in driver
const DLDI_INTERFACE* io_dldi_data = &_io_dldi_stub;

const struct DISC_INTERFACE_STRUCT* dldiGetInternal (void) {
	if (_io_dldi_stub.ioInterface.features & FEATURE_SLOT_GBA) {
		SetBusSLOT1ARM7SLOT2ARM9();
	}
	if (_io_dldi_stub.ioInterface.features & FEATURE_SLOT_NDS) {
		SetBusSLOT1ARM9SLOT2ARM7();
	}
	
	return &dldiGet()->ioInterface;
}

struct DLDI_INTERFACE* dldiGet(void) {
	return &_io_dldi_stub;
}
