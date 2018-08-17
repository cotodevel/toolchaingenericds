/*

			Copyright (C) 2018  Coto
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

//Coto: The notifierProcessor library allows ARM9 to send threads to ARM7 directly, execute them and return the result if any. It can send threads Async (FIFO irq) or Sync (slower)

#ifndef __notifierProcessor_h__
#define __notifierProcessor_h__

#include "dsregs.h"
#include "dsregs_asm.h"
#include "typedefsTGDS.h"

#define notifierProcessorInstanceInvalid (int)(-1)	//how many instances of notifierProcessor (threads) can be run at the same time.
#define notifierArgsTopSize (int)(4)	//4 == fast on ARM Cores
#define notifierHandlerBinarySize (int)(2 * 1024)	//Cluster(s) / (1024 * 2)
#define notifierHandlerFunctionSizeTop (512 - sizeof(struct notifierDescriptor))	//512 / 4 = 128 ARM opcodes :( oh well
#define notifierProcessorInstancesTop (int)(notifierHandlerBinarySize/(notifierHandlerFunctionSizeTop + sizeof(sizeof(struct notifierDescriptor))))	//how many instances of notifierProcessor (threads) can be run at the same time.

enum notifierProcessor
{
  NotifierProcessorTaskIdle = 0,
  NotifierProcessorTaskBusy = 1,	//assignment here
  NotifierProcessorTaskNew = 2,
  NotifierProcessorTaskAborted = 3,
  NotifierProcessorTaskFinished = 4,
  NotifierProcessorTaskInvalid = notifierProcessorInstanceInvalid
};

#define notifierDescriptorHandlerPtrInSharedMemDefault (int)(-100)
#define notifierDescriptorHandlerSizeDefault (int)(-99)
#define notifierDescriptorargsSizeDefault (int)(-98)
#define notifierDescriptorargsDefault (int)(-97)
//indexNotifierDescriptorDefault == notifierProcessorInstanceInvalid
//notifierStatus == NotifierProcessorTaskInvalid
//notifierProcessorHandlerQueuedOut must be zeroed

//each cluster is 	[struct notifierDescriptor]
//					[Function Handler]

struct notifierProcessorHandlerQueued {
	u32 notifierProcessorHandlerQueuedRetValue;
	enum notifierProcessor notifierStatus;
}; 

struct notifierDescriptorFunction{
	u32 arg0;
	u32 arg1;
	u32 arg2;
	u32 arg3;
	int argsSize;
	u32 * handlerFN;
	int handlerSize;
};

struct notifierDescriptor{
	u32 * HandlerPtrInSharedMem;									//ok
	int HandlerSize;												//ok
	int argsSize;	//will be between 1 ~ notifierArgsTopSize		//ok
	u32 args[notifierArgsTopSize];									//ok
	int indexNotifierDescriptor;									//ok
	struct notifierProcessorHandlerQueued notifierProcessorHandlerQueuedOut;						//ok	//todo
};

#endif


#ifdef __cplusplus
extern "C"{
#endif

extern struct notifierProcessorHandlerQueued processRunSync(struct notifierDescriptorFunction notifierDescriptorFunctionInst);
extern void processRunAsync(struct notifierDescriptorFunction notifierDescriptorFunctionInst);
extern struct notifierDescriptor * getNotifierDescriptorByIndex(int index);
extern struct notifierProcessorHandlerQueued RunNotifierProcessorThread(struct notifierDescriptor * notifierDescriptorInst);
extern void setNotifierProcessorMessage(struct notifierProcessorHandlerQueued * notifierProcessorHandlerQueuedOutInst, struct notifierDescriptor * notifierDescriptorInst);
extern struct notifierProcessorHandlerQueued RunFunctionExtProcessor(u32 arg0, u32 arg1, u32 arg2, u32 arg3, u32 * Callback, int CallbackSize);	//user code
extern void InitializeThreads();
extern struct notifierDescriptor * getSafenotifierDescriptor(struct notifierDescriptor * notifierDescriptorInst);
extern void setNotifierMessage(enum notifierProcessor notifierStatusInst, struct notifierDescriptor * notifierDescriptorInst);
extern enum notifierProcessor getNotifierMessage(struct notifierDescriptor * notifierDescriptorInst);

#ifdef __cplusplus
}
#endif
