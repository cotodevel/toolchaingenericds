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

//Changelog:
//24 Dec. 2024: Add TGDS Thread System 1.0 (Linux)
//25 Dec. 2024: Port TGDS Thread System 1.0 (NDS)

#include "typedefsTGDS.h"
#include "TGDS_threads.h"
#include "timerTGDS.h"
#include "exceptionTGDS.h"
#include "debugNocash.h"

#ifdef ARM9
__attribute__((section(".dtcm")))
#endif
struct task_Context threadQueue;

#ifdef ARM9
__attribute__((section(".itcm")))
#endif
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void initThreadSystem(struct task_Context * taskCtx){
    memset((u8*)taskCtx, 0, sizeof(struct task_Context));
    int i = 0;
    for(i = 0; i < MAX_THREADS_AVAILABLE; i++){
        taskCtx->tasksList[i].taskStatus = INVAL_THREAD;
        taskCtx->tasksList[i].timeQty = 0;
        taskCtx->tasksList[i].fn_task = NULL;
        taskCtx->tasksList[i].fn_args = NULL;
		taskCtx->tasksList[i].fn_taskOnOverflow = NULL;
    }
}

//Registers a thread to run
//Returns: 
//  Thread index if thread was registered successfully
//  THREAD_OVERFLOW if there's no room for more threads
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int registerThread(struct task_Context * taskCtx, TaskFn incomingTask, u32 * taskArgs, int threadTimeInMS, TaskFn OnOverflowExceptionIncomingTask){
    int i = 0;
    for(i = 0; i < MAX_THREADS_AVAILABLE; i++){
        if(taskCtx->tasksList[i].taskStatus == INVAL_THREAD){
            taskCtx->tasksList[i].taskStatus = THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE;
            taskCtx->tasksList[i].timeQty   = threadTimeInMS;
			taskCtx->tasksList[i].fn_task   = incomingTask;
            taskCtx->tasksList[i].fn_args   = taskArgs;
            taskCtx->tasksList[i].fn_taskOnOverflow = OnOverflowExceptionIncomingTask;
			break;
        }
    }
    if(i > (MAX_THREADS_AVAILABLE-1) ){
        return THREAD_OVERFLOW;
    }
    return i;
}

//Runs a thread once then CPU goes to sleep
//Returns: Various Thread Execution Results
//  INVAL_THREAD if not assigned
//  THREAD_OVERFLOW time qty was exceeded
//	THREAD_EXECUTE_OK_WAIT_FOR_SLEEP if thread was executed, but needs to concurrently wait for QtyTime to go idle meanwhile (CPU sleep)
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int worker_thread(struct task_def * curTask){
    if( curTask != NULL ){
        
		//Execute current thread here if available & ready to run
		if(curTask->taskStatus == THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE){
			//1) If threadQueue took less than curTask->timeQty, override timer value with (curTask->timeQty - timeQtyExecuted ), go idle, then run next thread

			//2) If threadQueue took more than curTask->timeQty, write timeQtyExecutedMS into curTask->timeQty, then return THREAD_OVERFLOW
			#ifndef ARM9
			struct timeval startTask_t;
			struct timeval endTask_t;
			gettimeofday(&startTask_t, 0);
			int time_in_millStart = (startTask_t.tv_sec) * 1000 + (startTask_t.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond
			#endif
			#ifdef ARM9
			startTimerCounter(tUnitsMilliseconds, 1, IRQ_TIMER2); //tUnitsMilliseconds equals 1 millisecond/unit. A single unit (1) is the default value for normal timer count-up scenarios.  //timerTicks explanation: reset internal getTimerCounter() counter. Each timerTicks unit equals to 1 (one) millisecond, same as waitTotalMS as well curTask->timeQty.
			int time_in_millStart = (int)getTimerCounter();
			#endif
			
			curTask->fn_task(curTask->fn_args);

			#ifndef ARM9
			gettimeofday(&endTask_t, 0);
			int time_in_millEnd = (endTask_t.tv_sec) * 1000 + (endTask_t.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond
			#endif
			#ifdef ARM9
			int time_in_millEnd = (int)getTimerCounter();
			#endif
			
			int timeQtyExecutedMS = (time_in_millEnd - time_in_millStart);
			int waitTotalMS = curTask->timeQty - timeQtyExecutedMS;
			if((waitTotalMS >= 0)	&& (curTask->timeQty < MAX_THREAD_OVERFLOW_CAPACITY_TIME_MILLISECONDS) ){
				curTask->internalRemainingThreadTime = waitTotalMS;
				curTask->taskStatus = THREAD_EXECUTE_OK_WAIT_FOR_SLEEP;	//printf("Task: [Took: %d ms]-[Slept: %d ms]", timeQtyExecutedMS, waitTotalMS);
			}
			else{
				curTask->internalRemainingThreadTime = -waitTotalMS;
				curTask->taskStatus = THREAD_OVERFLOW;
			}
		}
    }
	
    return curTask->taskStatus;
}

//Runs registered threads through a Time slots system (every n milliseconds, while CPU goes to sleep). 
//	Returns: Total of threads ran successfully. (Neither INVAL_THREAD or THREAD_OVERFLOW counts)
#ifdef ARM9
__attribute__((section(".itcm")))
#endif
#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
int runThreads(struct task_Context * taskCtx){
    int threadRunOK = 0;
    int i = 0;
    for(i = 0; i < MAX_THREADS_AVAILABLE; i++){
        struct task_def * curTask = &taskCtx->tasksList[i];

        if(taskCtx->tasksList[i].taskStatus != INVAL_THREAD){
            int retCode = worker_thread(curTask);
            if((retCode != INVAL_THREAD) && !(retCode == THREAD_OVERFLOW) && (retCode == THREAD_EXECUTE_OK_WAIT_FOR_SLEEP) ){
                
				//Thread's idling here (CPU sleep). Until available later
				if(curTask->internalRemainingThreadTime > 0){
					curTask->internalRemainingThreadTime--; //Each unit equals to 1ms
					#ifndef ARM9
					usleep(1 * 1000); //usleep() takes microseconds, so you will have to multiply the input by 1000 in order to sleep in milliseconds.
					#endif
					
					#ifdef ARM9
					HaltUntilIRQ();	//IRQWait(0, IRQ_TIMER3); //allow ARM9 to rely on Timer (1 ms) + other interrupts, so it wastes less cycles idling.
					#endif
				}
				else{
					curTask->taskStatus = THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE;
					threadRunOK++;
				}
            }
            else if(retCode == THREAD_OVERFLOW){
				if(curTask->fn_taskOnOverflow != NULL){
					curTask->fn_taskOnOverflow((u32*)curTask);
				}
				curTask->internalRemainingThreadTime = 0;
				curTask->taskStatus = THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE;
				HaltUntilIRQ(); //After thread overflow, CPU goes to sleep.
			}
			
        }
		else{
			HaltUntilIRQ(); //If threads aren't assigned, CPU goes to sleep.
		}
    }
    return threadRunOK;
}

/*
//////////////////////////////////////////////////////// Threading User code start : TGDS Project specific ////////////////////////////////////////////////////////
//User callback when Task Overflows. Intended for debugging purposes only, as normal user code tasks won't overflow if a task is implemented properly.
//	u32 * args = This Task context
void onThreadOverflowUserCode(u32 * args){
	struct task_def * thisTask = (struct task_def *)args;
	
	char threadStatus[64];
	switch(thisTask->taskStatus){
		case(INVAL_THREAD):{
			strcpy(threadStatus, "INVAL_THREAD");
		}break;
		
		case(THREAD_OVERFLOW):{
			strcpy(threadStatus, "THREAD_OVERFLOW");
		}break;
		
		case(THREAD_EXECUTE_OK_WAIT_FOR_SLEEP):{
			strcpy(threadStatus, "THREAD_EXECUTE_OK_WAIT_FOR_SLEEP");
		}break;
		
		case(THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE):{
			strcpy(threadStatus, "THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE");
		}break;
	}
	
	char debOut2[256];
	if( thisTask->taskStatus == THREAD_OVERFLOW){
		sprintf(debOut2, "[%s]. Thread requires at least (%d) ms. ", threadStatus, thisTask->internalRemainingThreadTime);
	}
	else{
		sprintf(debOut2, "[%s]. ", threadStatus);
	}
	
	int TGDSDebuggerStage = 10;
	u8 fwNo = *(u8*)(0x027FF000 + 0x5D);
	handleDSInitOutputMessage((char*)debOut2);
	handleDSInitError(TGDSDebuggerStage, (u32)fwNo);
	
	while(1==1){
		HaltUntilIRQ();
	}
}

void taskA(u32 * args){
    
	int i = 0;
	int addRes = 0;
    for(i = 0; i < 100000; i++){ //1000000 overflows on ARM9 hardware (equals to MAX_THREAD_OVERFLOW_CAPACITY_TIME_MILLISECONDS or higher)
        addRes += (i + 1 + i);
    }
	
	char debOut2[256];
	sprintf(debOut2, "Task A -- %s -- has run. (%d) %d", (char*)args, addRes, rand() % 0xFF);
	printf(debOut2);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void taskB(u32 * args){
    
	int i = 0;
	int addRes = 0;
    for(i = 0; i < 1000 ; i++){
        addRes += (i + 1 + i);
    }
	
	char debOut2[256];
	sprintf(debOut2, "Task B -- %s -- has run. (%d) %d", (char*)args, addRes, rand() % 0xFF);
	printf(debOut2);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void taskC(u32 * args){
    
	int i = 0;
	int addRes = 0;
    for(i = 0; i < 1000 ; i++){
        addRes += (i + 1 + i);
    }
	
	char debOut2[256];
	sprintf(debOut2, "Task C -- %s -- has run. (%d) %d", (char*)args, addRes, rand() % 0xFF);
	printf(debOut2);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif

#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
void runTests(){
	printf("TGDS Thread System 1.0.");

    int taskATimeMS = 15; //Task execution in milliseconds. This thread requires at least 15ms or thread overflow occurs
    char taskAarg[256];
    strcpy(taskAarg, "*Task A's arguments*");

    int taskBTimeMS = 8; //Task execution in milliseconds.  This thread requires at least 8ms or thread overflow occurs
    char taskBarg[256];
    strcpy(taskBarg, "*Task B's arguments*");
	
	int taskCTimeMS = 8; //Task execution in milliseconds.  This thread requires at least 8ms or thread overflow occurs
    char taskCarg[256];
    strcpy(taskCarg, "*Task C's arguments*");
	
    //Init + Register 2 tasks to run + run them
    initThreadSystem(&threadQueue);

    if(registerThread(&threadQueue, (TaskFn)&taskA, (u32*)&taskAarg, taskATimeMS, (TaskFn)&onThreadOverflowUserCode) != THREAD_OVERFLOW){
        
    } 

    if(registerThread(&threadQueue, (TaskFn)&taskB, (u32*)&taskBarg, taskBTimeMS, (TaskFn)&onThreadOverflowUserCode) != THREAD_OVERFLOW){
        
    }
	
	if(registerThread(&threadQueue, (TaskFn)&taskC, (u32*)&taskCarg, taskCTimeMS, (TaskFn)&onThreadOverflowUserCode) != THREAD_OVERFLOW){
        
    }
	
	while(1==1){
		int threadsRan = runThreads(&threadQueue);
		
		clrscr();
		printf(" ---- ");
		printf(" ---- ");
		printf(" ---- ");
		printf("%d tasks have ran successfuly. ", threadsRan);
		
		
		printf("Press (A) to continue. >%d", TGDSPrintfColor_Yellow);
		while(1==1){
			scanKeys();
			if(keysDown()&KEY_A){
				scanKeys();
				while(keysDown() & KEY_A){
					scanKeys();
				}
				break;
			}
		}
		
	}
}

#ifndef ARM9
int main()
{
	runTests();
    return 0;
}
#endif

//////////////////////////////////////////////////////////////////////// Threading User code end /////////////////////////////////////////////////////////////////////////////
*/