@are these regs writable from ARM7??

.text	
.arm
.code 32


.global SetSlot1Slot2ARM7
SetSlot1Slot2ARM7:	
	push {r0-r3,lr}
	ldr r0,= 0x4000210
	mov r1, #1
	str r1, [r0]
	//map the gba cartridge to the arm7 and nds card too
	ldr r0,= 0x4000204
	ldrh r1, [r0]
	orr r1, #0x80
	orr r1, #0x800 //card to arm7
	bic r1, #0x8000	//set memory priority to arm9
	strh r1, [r0]
	pop {r0-r3,lr}
	bx lr


.global SetSlot1Slot2ARM9
SetSlot1Slot2ARM9:
	push {r0-r3,lr}
	ldr r0,= 0x4000210
	mov r1, #1
	str r1, [r0]
	//map the gba cartridge to the arm7 and nds card too
	ldr r0,= 0x4000204
	ldrh r1, [r0]
	bic r1, #0x80
	bic r1, #0x800 //card to arm9
	bic r1, #0x8000	//set memory priority to arm9
	strh r1, [r0]
	pop {r0-r3,lr}
	bx lr

.pool
