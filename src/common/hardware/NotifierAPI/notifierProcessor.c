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

#include <string.h>
#include "notifierProcessor.h"
#include "memoryHandleTGDS.h"
#include "dmaTGDS.h"
#include "ipcfifoTGDS.h"

#ifdef ARM9
#include "nds_cp15_misc.h"
#endif

//user wrapper
struct notifierProcessorHandlerQueued RunFunctionExtProcessor(u32 arg0, u32 arg1, u32 arg2, u32 arg3, u32 * Callback, int CallbackSize){
	struct notifierDescriptorFunction notifierDescriptorFunctionInst;
	int argCount = 0;
	if(arg0 > 0){
		argCount++;
	}
	if(arg1 > 0){
		argCount++;
	}
	if(arg2 > 0){
		argCount++;
	}
	if(arg3 > 0){
		argCount++;
	}
	notifierDescriptorFunctionInst.arg0 = arg0;
	notifierDescriptorFunctionInst.arg1 = arg1;
	notifierDescriptorFunctionInst.arg2 = arg2;
	notifierDescriptorFunctionInst.arg3 = arg3;
	notifierDescriptorFunctionInst.argsSize = argCount;
	notifierDescriptorFunctionInst.handlerFN = Callback;
	notifierDescriptorFunctionInst.handlerSize = CallbackSize;
	return processRunSync(notifierDescriptorFunctionInst);
}

//Coto: The notifierProcessor library allows ARM9 to send threads to ARM7 directly, execute them and return the result if any. It can send threads Async (FIFO irq) or Sync (slower)
// notifierProcessor library depends on hardware FIFO interrupts, making them FAST.

struct notifierProcessorHandlerQueued processRunSync(struct notifierDescriptorFunction notifierDescriptorFunctionInst){
	struct notifierProcessorHandlerQueued notifierProcessorHandlerQueuedRet;
	notifierProcessorHandlerQueuedRet.notifierStatus = NotifierProcessorTaskAborted;
	int newIndex = getnotifierProcessorNewInstance();
	if(newIndex != notifierProcessorInstanceInvalid){
		struct notifierDescriptor notifierDescriptorInst;
		//populate the Data
		//notifierDescriptorInst.HandlerPtrInSharedMem = (u32 *)notifierDescriptorFunctionInst.handlerFN;	//no, the HandlerPtrInSharedMem is according to Cluster Format as shown in notifierProcessor.h
		notifierDescriptorInst.argsSize = notifierDescriptorFunctionInst.argsSize;
		notifierDescriptorInst.args[0] = notifierDescriptorFunctionInst.arg0;
		notifierDescriptorInst.args[1] = notifierDescriptorFunctionInst.arg1;
		notifierDescriptorInst.args[2] = notifierDescriptorFunctionInst.arg2;
		notifierDescriptorInst.args[3] = notifierDescriptorFunctionInst.arg3;
		
		int topFnSize = 0;
		if(notifierDescriptorFunctionInst.handlerSize > notifierHandlerFunctionSizeTop ){
			topFnSize = notifierHandlerFunctionSizeTop;
		}
		else{
			topFnSize = notifierDescriptorFunctionInst.handlerSize;
		}
		notifierDescriptorInst.HandlerSize = topFnSize;
		int notifierDescriptorCluster = (sizeof(struct notifierDescriptor) + notifierHandlerFunctionSizeTop);
		notifierDescriptorInst.indexNotifierDescriptor = newIndex;
		
		notifierDescriptorInst.notifierProcessorHandlerQueuedOut.notifierStatus = NotifierProcessorTaskNew;
		
		//allocate function in SharedWram linear memory
		uint32 * FnHeap = (uint32*)(uint16*)vramHeapAlloc(HeapBlock, 0x027FF000, notifierDescriptorCluster);
		notifierDescriptorInst.HandlerPtrInSharedMem = (uint8*)(FnHeap) + (sizeof(struct notifierDescriptor));
		
		//clean struct notifierProcessorHandlerQueued
		memset((uint8*)&notifierDescriptorInst.notifierProcessorHandlerQueuedOut, 0, sizeof(struct notifierProcessorHandlerQueued));
		
		//Cluster [
		//copy [struct notifierDescriptor] into it
		memcpy((uint8*)FnHeap, (uint8*)&notifierDescriptorInst,sizeof(struct notifierDescriptor));
		
		//copy [Function Handler] into it
		memcpy((uint8*)notifierDescriptorInst.HandlerPtrInSharedMem, (uint8*)notifierDescriptorFunctionInst.handlerFN,notifierDescriptorInst.HandlerSize);
		// ]
		
		//0 cmd: 1: index, 2: (u32)struct notifierDescriptor * getNotifierDescriptorByIndex(index)
		SendFIFOWords(notifierProcessorRunThread, (uint32)notifierDescriptorInst.indexNotifierDescriptor, (uint32)getNotifierDescriptorByIndex(notifierDescriptorInst.indexNotifierDescriptor), NULL);
		
		//printf("notifierProcessorRunThread");
		//printf("sent: %x index: %d", (uint32)FnHeap,notifierDescriptorInst.indexNotifierDescriptor);
		//while(1==1);	//ok so far
		
		struct notifierDescriptor * notifierDescriptorPtr = getNotifierDescriptorByIndex(notifierDescriptorInst.indexNotifierDescriptor);
		while(getNotifierMessage(getSafenotifierDescriptor(notifierDescriptorPtr)) == NotifierProcessorTaskNew){
			//idle loop
			//printf("at least once we wait for NPT");
			//while(1==1);	//ok so far
		}
		
		//printf("ok so far :%d",getSafenotifierDescriptor(notifierDescriptorPtr)->notifierStatus);
		//while(1==1);	//ok so far
		return getSafenotifierDescriptor(notifierDescriptorPtr)->notifierProcessorHandlerQueuedOut;
	}
	return notifierProcessorHandlerQueuedRet;
}

void InitializeThreads(){
	int i = 0;
	struct notifierDescriptor notifierDescriptorInitialized;
	notifierDescriptorInitialized.HandlerPtrInSharedMem = notifierDescriptorHandlerPtrInSharedMemDefault;
	notifierDescriptorInitialized.HandlerSize = notifierDescriptorHandlerSizeDefault;
	notifierDescriptorInitialized.argsSize = notifierDescriptorargsSizeDefault;
	for(i = 0; i < notifierArgsTopSize; i++){
		notifierDescriptorInitialized.args[i] = notifierDescriptorargsDefault;
	}
	notifierDescriptorInitialized.indexNotifierDescriptor = notifierProcessorInstanceInvalid;
	notifierDescriptorInitialized.notifierProcessorHandlerQueuedOut.notifierStatus = NotifierProcessorTaskInvalid;
	memset((uint8*)&notifierDescriptorInitialized.notifierProcessorHandlerQueuedOut, 0, sizeof(struct notifierProcessorHandlerQueued));
	
	i = 0;
	for(i = 0; i < notifierProcessorInstancesTop; i++){
		struct notifierDescriptor * notifierDescriptorPtr = getNotifierDescriptorByIndex(i);
		if(notifierDescriptorPtr != NULL){
			memcpy((uint8*)notifierDescriptorPtr, (uint8*)&notifierDescriptorInitialized, sizeof(struct notifierDescriptor));
		}
	}
}

void processRunAsync(struct notifierDescriptorFunction notifierDescriptorFunctionInst){
	//todo
}


//it is always linear alloc [n * notifierDescriptorCluster] thus the binary search is linear as well
struct notifierDescriptor * getNotifierDescriptorByIndex(int index){
	if(index < notifierProcessorInstancesTop){
		return (struct notifierDescriptor *)(0x027FF000 + (index * (sizeof(struct notifierDescriptor) + notifierHandlerFunctionSizeTop)));
	}
	return NULL;
}

struct notifierProcessorHandlerQueued RunNotifierProcessorThread(struct notifierDescriptor * notifierDescriptorInst){
	//run thread here
	typedef u32 (*RunnableNotifierProcessorThread)(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
	RunnableNotifierProcessorThread	thread;
	thread = (RunnableNotifierProcessorThread)notifierDescriptorInst->HandlerPtrInSharedMem;
	u32 FnRetValue = thread(notifierDescriptorInst->args[0], notifierDescriptorInst->args[1], notifierDescriptorInst->args[2], notifierDescriptorInst->args[3]);
	//result
	notifierDescriptorInst->notifierProcessorHandlerQueuedOut.notifierProcessorHandlerQueuedRetValue = (u32)FnRetValue;
	notifierDescriptorInst->notifierProcessorHandlerQueuedOut.notifierStatus = NotifierProcessorTaskFinished;
	return notifierDescriptorInst->notifierProcessorHandlerQueuedOut;
}

void setNotifierProcessorMessage(struct notifierProcessorHandlerQueued * notifierProcessorHandlerQueuedOutInst, struct notifierDescriptor * notifierDescriptorInst){
	memcpy((uint8*)&notifierDescriptorInst->notifierProcessorHandlerQueuedOut,(uint8*)notifierProcessorHandlerQueuedOutInst,sizeof(struct notifierProcessorHandlerQueued));
}


void setNotifierMessage(enum notifierProcessor notifierStatusInst, struct notifierDescriptor * notifierDescriptorInst){
	notifierDescriptorInst->notifierProcessorHandlerQueuedOut.notifierStatus = notifierStatusInst;
}

enum notifierProcessor getNotifierMessage(struct notifierDescriptor * notifierDescriptorInst){
	return notifierDescriptorInst->notifierProcessorHandlerQueuedOut.notifierStatus;
}

struct notifierDescriptor * getSafenotifierDescriptor(struct notifierDescriptor * notifierDescriptorInst){
	#ifdef ARM9
	//Prevent Cache problems.
	coherent_user_range_by_size((uint32)notifierDescriptorInst, (int)sizeof(struct notifierDescriptor));
	#endif
	return notifierDescriptorInst;
}