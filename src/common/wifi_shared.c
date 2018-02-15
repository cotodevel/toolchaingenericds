
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "memoryHandleTGDS.h"

#include "timerTGDS.h"

#ifdef ARM7
#include <string.h>
#include "wifi_arm7.h"
#endif

#ifdef ARM9
#include <stdbool.h>

#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "wifi_arm9.h"

#endif


#include "wifi_shared.h"
//internal DSWIFI code
void DeInitWIFI(){
	#ifdef ARM7
	Wifi_Deinit();
	#endif
	#ifdef ARM9
	Wifi_DisconnectAP();
	Wifi_DisableWifi();
	DisableIrq(IRQ_TIMER3);
	Wifi_SetSyncHandler(NULL);
	sgIP_Hub_RemoveHardwareInterface(wifi_hw);
	TIMERXDATA(3) = 0;
	TIMERXCNT(3) = 0;
	SendMultipleWordACK(WIFI_DEINIT, 0, 0, NULL);
	#endif
}