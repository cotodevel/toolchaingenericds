/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/

#include "typedefs.h"
#include "nds_cp15_misc.h"
#include "mem_handler_shared.h"

//Each region listed.
//Use one for MPU settings : vectors @0x00000000
//Use another for MPU settings : vectors @0xffff0000
T_mpuSetting mpuSetting[2];


uint32 EWRAMCached(uint32 address){
	uint32 bottom_mem = (address & nds_ewram_mask);
	return (uint32)(get_ewram_start() | bottom_mem);
}

uint32 EWRAMUncached(uint32 address){
	uint32 bottom_mem = (address & nds_ewram_mask);
	return (uint32)(((uint32)get_ewram_start() | (uint32)get_ewram_size()) | bottom_mem);
}


//these functions allow to change the NDS system MPU settings as whole, preventing you to keep track of each custom MPU setting.
// it is recomended that you use those functions for dealing with slower processes. For faster processes, just use the CP15 functions directly.

void updateMPUSetting(T_mpuSetting * mpuSetting_inst){
	
	//Disable only DCACHE & ICACHE / mpu
	CP15ControlRegisterDisable((CR_I | CR_C | CR_M));
	
	//DrainWrite
	DrainWriteBuffer();
	
	//reset caches
	flush_icache_all();
	flush_dcache_all();
	
	//Set Regions
	MProtectionRegionBase0(mpuSetting_inst->inst_regionSetting[0].regionsettings);
	MProtectionRegionBase1(mpuSetting_inst->inst_regionSetting[1].regionsettings);
	MProtectionRegionBase2(mpuSetting_inst->inst_regionSetting[2].regionsettings);
	MProtectionRegionBase3(mpuSetting_inst->inst_regionSetting[3].regionsettings);
	
	setitcm();
	MProtectionRegionBase4(mpuSetting_inst->inst_regionSetting[4].regionsettings);
	
	setdtcm();
	MProtectionRegionBase5(mpuSetting_inst->inst_regionSetting[5].regionsettings);
	
	MProtectionRegionBase6(mpuSetting_inst->inst_regionSetting[6].regionsettings);
	MProtectionRegionBase7(mpuSetting_inst->inst_regionSetting[7].regionsettings);
	
	//Writebuffer enable/disable for Regions
	WritebufferControl(mpuSetting_inst->WriteBufferAvailabilityForRegions);
	
	//DataCache enable/disable for Regions
	RegionDCacheEnable(mpuSetting_inst->DCacheAvailabilityForRegions);
	
	//InstructionCache enable/disable for Regions
	RegionICacheEnable(mpuSetting_inst->ICacheAvailabilityForRegions);
	
	//ITCM Access for Regions
	RegionIAccessPermission(mpuSetting_inst->ITCMAccessForRegions);
	
	//DTCM Access for Regions
	RegionDAccessPermission(mpuSetting_inst->DTCMAccessForRegions);
	
	//Enable only DCACHE & ICACHE
	CP15ControlRegisterEnable((CR_IT | CR_DT | CR_I | CR_C | CR_M));
	
}

#ifdef EXCEPTION_VECTORS_0xffff0000
//Sets default MPU Settings to use anytime. Uses vectors @ 0xffff0000.
//Region0: IO 
//Region1: SYSTEM ROM
//Region2: ALT VECTORS
//Region3: GBA Cart
//Region4: ITCM
//Region5: DTCM
//Region6: EWRAM Uncached
//Region7:
void set0xFFFF0000FastMPUSettings(){
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[0].regionsettings = (uint32)( PAGE_64M | 0x04000000 | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[1].regionsettings = (uint32)( PAGE_64K | 0xFFFF0000 | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[2].regionsettings = (uint32)( PAGE_4K | 0x00000000 | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[3].regionsettings = (uint32)( PAGE_128M | 0x08000000 | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[4].regionsettings = (uint32)((uint32)(&_itcm_start) | PAGE_32K | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[5].regionsettings = (uint32)((uint32)(&_dtcm_start) | PAGE_16K | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[6].regionsettings = (uint32)( PAGE_16M 	| 0x02000000 | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].inst_regionSetting[7].regionsettings = (uint32)( PAGE_4M 	| 0x02000000 | 1);
	mpuSetting[VECTORS_0xFFFF0000_MPU].WriteBufferAvailabilityForRegions = 0b10110000; //EWRAM, DTCM, ITCM
	
	mpuSetting[VECTORS_0xFFFF0000_MPU].DCacheAvailabilityForRegions = 0b10000010;	//DTCM & ITCM
	mpuSetting[VECTORS_0xFFFF0000_MPU].ICacheAvailabilityForRegions = 0b10000010;	//DTCM & ITCM
	
	mpuSetting[VECTORS_0xFFFF0000_MPU].ITCMAccessForRegions = 0x33333363;
	mpuSetting[VECTORS_0xFFFF0000_MPU].DTCMAccessForRegions = 0x33333363;
	
	updateMPUSetting((T_mpuSetting *)&mpuSetting[VECTORS_0xFFFF0000_MPU]);
}

#endif

#ifdef EXCEPTION_VECTORS_0x00000000
void set0x00000000FastMPUSettings(){
	//mpuSetting[VECTORS_0x00000000_MPU]
}
#endif