
/*****************************
 * ima-adpcm decoder by Discostew
 *****************************/

.global decode_mono_ima_adpcm
.global decode_stereo_ima_adpcm

.arch	armv4t
.cpu	arm7tdmi
.text
.arm
.align

#define cur		r0
#define da1		r4
#define st1		r5
#define sa1		r6
#define da2		r7
#define st2		r8
#define sa2		r9

#define	ta		r10
#define	tb		r11
#define tc		r12
#define	td		r0

#define istab	r14

#define src		r1
#define dest	r2
#define iter	r3


/*	
	SampXCode = Stream_CurSamples & 0xF;
	Stream_CurSamples >>= 4;
	Diff = (((( SampXCode & 0x7 ) << 1 ) + 1 ) * STREAM_StepTab[ Stream_StepIndex ]) >> 3;
	Stream_StepIndex += STREAM_IndexTab[ SampXCode ];
	Stream_LastSample += ( SampXCode & 0x8 ? -Diff : Diff );
	
	if( Stream_StepIndex > 88 )			Stream_StepIndex = 88;
	if( Stream_StepIndex < 0 )		Stream_StepIndex = 0;
	if( Stream_LastSample > 32767 )		Stream_LastSample = 32767;
	if( Stream_LastSample <= -32768 )	Stream_LastSample = -32768;
	*target++ = Stream_LastSample;
	
 */
 
/*************************************************************
 *                                                           *
 * decode_mono_ima_adpcm( data, source, target, iterations ) *
 *                                                           *
 *************************************************************/

decode_mono_ima_adpcm:

	push	{r0,r4-r11,lr}
	ldmia	r0, {cur,da1,st1,sa1}
	adr		istab, Index_Step_Table
	orr		iter, iter, cur, lsl#28
	mov		td, #1
	lsl		sa1, #16

.examine_mono:
	ands	tc, iter, #0xF0000000
	beq		.reload_mono
.decode_mono_loop:

/*************** single channel *********************/

	and		tb, da1, #7
	ands	ta, da1, #8
	add		tb, td, tb, lsl#1
	sub 	ta, istab, tb, LSL #2
	ldrsh	ta, [ta]
	add 	tc, istab, st1, LSL #2
	ldrsh	tc, [tc]
	add		st1, ta
	lsl		tb,	tb, #16
	lsr		tb,	tb, #16
	mul		ta, tb, tc
	lsr		da1, #4
	lsl		ta, #13
	subne	sa1, sa1, ta
	sub		iter, #0x10000001
	
	push {r0-r3,lr}
	mov r0, sa1
	moveq r1, ta
	bleq sat_adds32
		@addeq	sa1, sa1, ta
	mov sa1, r0
	pop {r0-r3,lr}

	cmp		st1, #89
	ldrge	st1, [istab, #-68]
	lsr		tb, sa1, #16
	cmp		st1, #0
	ldrlt	st1, [istab, #-72]
	strh	tb, [dest], #2
	
	ands	tb, iter, #0x0FFFFFFF
	bne		.examine_mono
	
	lsr		sa1, #16
	mov		cur, iter, lsr#28
	
	pop		{ta}
	stmia	ta, {cur,da1,st1,sa1}

	pop		{r4-r11,pc}
		
/*************************************************************
 *                                                           *
 * decode_stereo_ima_adpcm( data, source, target, iterations ) *
 *                                                           *
 *************************************************************/

decode_stereo_ima_adpcm:

	push	{r0,r4-r11,lr}
	ldmia	r0, {cur,da1,st1,sa1,da2,st2,sa2}
	adr		istab, Index_Step_Table
	orr		iter, iter, cur, lsl#28
	mov		td, #1
	lsl		sa1, #16
	lsl		sa2, #16
	

.examine_stereo:
	ands	tc, iter, #0xF0000000
	beq		.reload_stereo
.decode_stereo_loop:

	and		tb, da1, #7
	ands	ta, da1, #8
	add		tb, td, tb, lsl#1
	sub 	ta, istab, tb, LSL #2
	ldrsh	ta, [ta]
	add 	tc, istab, st1, LSL #2
	ldrsh	tc, [tc]
	add		st1, ta
	smull 	ta, tc, tb, tc
	
	lsr		da1, #4
	lsl		ta, #13
	subne 	sa1, sa1, ta
	and		tb, da2, #7
	
	push {r0-r3,lr}
	mov r0, sa1
	moveq r1, ta
	bleq sat_adds32
		@addeq	sa1, sa1, ta
	mov sa1, r0
	pop {r0-r3,lr}
	ands	ta, da2, #8
	add		tb, td, tb, lsl#1
	sub 	ta, istab, tb, LSL #2
	ldrsh	ta, [ta]
	add 	tc, istab, st2, LSL #2
	ldrsh	tc, [tc]
	add		st2, ta
	smull 	ta, tc, tb, tc
	sub		iter, #0x10000001
	lsl		ta, #13
	subne	sa2, sa2, ta
	lsr		da2, #4
	push {r0-r3,lr}
	mov r0, sa2
	moveq r1, ta
	bleq sat_adds32
		@addeq	sa2, sa2, ta
	mov sa2, r0
	pop {r0-r3,lr}	
	lsr		tb, sa1, #16
	lsr		ta, sa2, #16
	
	cmp		st1, #89
	cmplt	st2, #89
	bge		.satihi
1:
	cmp		st1, #0
	cmpge	st2, #0
	blt		.satilo

2:	
	strh	tb, [dest], #2
	strh	ta, [dest], #2
	
	ands	tb, iter, #0x0FFFFFFF
	bne		.examine_stereo
	
	lsr		sa1, #16
	lsr		sa2, #16
	mov		cur, iter, lsr#28
	
	pop		{ta}
	stmia	ta, {cur,da1,st1,sa1,da2,st2,sa2}

	pop		{r4-r11,pc}
	
	
.reload_mono:
	orr		iter, #0x80000000
	ldr		da1, [src], #4
	b		.decode_mono_loop

.reload_stereo:
	orr		iter, #0x80000000
	ldr		da1, [src], #4
	ldr		da2, [src], #4
	b		.decode_stereo_loop


.satihi:
	cmp		st1, #89
	ldrge	st1, [istab, #-68]
	cmp		st2, #89
	ldrge	st2, [istab, #-68]
	b		1b

.satilo:
	cmp		st1, #0
	ldrlt	st1, [istab, #-72]
	cmp		st2, #0
	ldrlt	st2, [istab, #-72]
	b		2b

.word	0
.word	88
.word	8, 8, 6, 6, 4, 4, 2, 2
.word	-1, -1, -1, -1, -1, -1, -1, -1
Index_Step_Table:
.word	7, 8, 9, 10, 11, 12, 13, 14
.word	16, 17, 19, 21, 23, 25, 28, 31
.word	34, 37, 41, 45, 50, 55, 60, 66
.word	73, 80, 88, 97, 107, 118, 130, 143
.word	157, 173, 190, 209, 230, 253, 279, 307
.word	337, 371, 408, 449, 494, 544, 598, 658
.word	724, 796, 876, 963, 1060, 1166, 1282, 1411
.word	1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024
.word	3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484
.word	7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899
.word	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794
.word	32767

.pool
@s32 sat_adds32(s32 x, s32 y);
sat_adds32:
	mvn	r2, #-2147483648	@0x80000000
	add	r2, r2, r0, lsr #31
	add	r0, r1, r0
	eor	r3, r2, r1
	eor	r1, r1, r0
	eor	r1, r1, #-2147483648	@0x80000000
	orr	r1, r3, r1
	cmn	r1, #1
	movgt	r0, r2
	bx	lr

@s32 sat_subs32(s32 x, s32 y)
sat_subs32:
	mvn	r3, #-2147483648	@0x80000000
	sub	r2, r0, r1
	add	r0, r3, r0, lsr #31
	eor	r3, r0, r2
	eor	r1, r0, r1
	and	r1, r1, r3
	cmp	r1, #0
	movlt	r2, r0
	mov	r0, r2
	bx	lr