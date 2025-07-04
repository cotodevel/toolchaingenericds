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

#ifndef __TGDS_threads_h__
#define __TGDS_threads_h__

#ifndef ARM9
//Linux timer code
#include <unistd.h>
#include <sys/time.h>
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
#endif

#include <stdio.h>
#include <string.h>
#include "InterruptsARMCores_h.h"
#include "timerTGDS.h"

#define MAX_THREADS_AVAILABLE ((int)10)

//Thread Status
#define INVAL_THREAD ((int)-1)
#define THREAD_OVERFLOW ((int)-2)
#define THREAD_EXECUTE_OK_WAIT_FOR_SLEEP ((int)-3)
#define THREAD_EXECUTE_OK_WAKEUP_FROM_SLEEP_GO_IDLE ((int)-4)
#define THREAD_EXECUTION_PAUSED ((int)-5)

#define MAX_THREAD_OVERFLOW_CAPACITY_TIME_MILLISECONDS ((int)90) //This is the maximum capacity in milliseconds a thread can take before overflows on hardware (100% CPU usage). This value is not accurate on emulators.

typedef void (*TaskFn)(u32 *);



struct task_def{
    int taskStatus;
    int timeQty;	//User defined thread waiting time in milliseconds
	int remainingThreadTime;	//Internal remaining thread time
	TaskFn fn_task;
    u32 * fn_args; //format: TaskFn
	TaskFn fn_taskOnOverflow;
    struct task_Context * parentTaskCtx;
	enum timerUnits timerFormat;
	int elapsedThreadTime;
}  __attribute__ ((aligned(4)));

struct task_Context{
    struct task_def tasksList[MAX_THREADS_AVAILABLE];
    int tasksCount;
}  __attribute__ ((aligned(4)));

//Microseconds on NDS timers cause 100% CPU usage when timer interrupts are enabled.
//Wrap around timer objects to solve that, by preventing timer interrupts on microseconds.
extern int timerDest;
extern int currentTimerUnits;

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
static inline void thread_StartTimerCounter(enum timerUnits units, u32 timerIRQ){
    switch(timerIRQ){
		case(IRQ_TIMER0):{
			timerDest = 0;
		}break;
		case(IRQ_TIMER1):{
			timerDest = 1;
		}break;
		case(IRQ_TIMER2):{
			timerDest = 2;
		}break;
		case(IRQ_TIMER3):{
			timerDest = 3;
		}break;
		default:{
			return;
		}break;
	}
	currentTimerUnits=(int)units;
	TIMERXDATA(timerDest) = TIMER_FREQ(currentTimerUnits);
	TIMERXCNT(timerDest) = TIMER_DIV_1 | TIMER_ENABLE;
	irqDisable(timerIRQ);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
static inline unsigned int thread_GetTimerCounter(){
    return (TIMERXDATA(timerDest) / (int)currentTimerUnits);
}

#if (defined(__GNUC__) && !defined(__clang__))
__attribute__((optimize("O0")))
#endif
#if (!defined(__GNUC__) && defined(__clang__))
__attribute__ ((optnone))
#endif
static inline void thread_StopTimerCounter(){
    TIMERXCNT(timerDest) = 0;
    TIMERXDATA(timerDest) = 0;
    currentTimerUnits = 0;
}


#endif


#ifdef __cplusplus
extern "C" {
#endif

extern struct task_Context threadQueue;
extern struct task_Context * getTGDSThreadSystem();
extern void initThreadSystem(struct task_Context * taskCtx);
extern int registerThread(struct task_Context * taskCtx, TaskFn incomingTask, u32 * taskArgs, int threadTimeInMS, TaskFn OnOverflowExceptionIncomingTask, enum timerUnits timerMethod);
extern bool pauseThread(struct task_Context * taskCtx, TaskFn fn_taskToPause);
extern bool resumeThread(struct task_Context * taskCtx, TaskFn fn_taskToResume);
extern bool removeThread(struct task_Context * taskCtx, TaskFn fn_taskToRemove);
extern int worker_thread(struct task_def * curTask);
extern int runThreads(struct task_Context * taskCtx, bool waitForVblk);

#ifdef __cplusplus
}
#endif
