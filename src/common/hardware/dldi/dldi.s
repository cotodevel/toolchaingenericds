@ARM7 DLDI is custom implemented

#ifdef ARM9

@---------------------------------------------------------------------------------
	.section    .dldiSection, "awx", %progbits
	.align	4
	.arm
	.global _io_dldi_stub
@---------------------------------------------------------------------------------
.equ FEATURE_MEDIUM_CANREAD,		0x00000001
.equ FEATURE_MEDIUM_CANWRITE,		0x00000002
.equ FEATURE_SLOT_GBA,			0x00000010
.equ FEATURE_SLOT_NDS,			0x00000020

.equ DLDI_ALLOCATED_SPACE,		32768

_io_dldi_stub:

dldi_start:

@---------------------------------------------------------------------------------
@ Driver patch file standard header -- 16 bytes
	.word	0xBF8DA5ED		@ Magic number to identify this region
	.asciz	" Chishm"		@ Identifying Magic string (8 bytes with null terminator)
	.byte	0x01			@ Version number
	.byte	0x0F	@32KiB	@ Log [base-2] of the size of this driver in bytes.
	.byte	0x00			@ Sections to fix
	.byte 	0x0F	@32KiB	@ Log [base-2] of the allocated space in bytes.
	
@---------------------------------------------------------------------------------
@ Text identifier - can be anything up to 47 chars + terminating null -- 16 bytes
	.align	4
	.asciz "TGDS RAMDISK DLDI Driver @0x08000000 32MB"

@---------------------------------------------------------------------------------
@ Offsets to important sections within the data	-- 32 bytes
	.align	6
	.word   dldi_start		@ data start
	.word   dldi_end		@ data end
	.word	0x00000000		@ Interworking glue start	-- Needs address fixing
	.word	0x00000000		@ Interworking glue end
	.word   0x00000000		@ GOT start					-- Needs address fixing
	.word   0x00000000		@ GOT end
	.word   0x00000000		@ bss start					-- Needs setting to zero
	.word   0x00000000		@ bss end

@---------------------------------------------------------------------------------
@ IO_INTERFACE data -- 32 bytes
	.ascii	"DLDI"					@ ioType
	.word	(FEATURE_MEDIUM_CANREAD | FEATURE_SLOT_GBA  | FEATURE_MEDIUM_CANWRITE)		@ Features
	.word	_DLDI_startup			@ 
	.word	_DLDI_isInserted		@ 
	.word	_DLDI_readSectors		@   Function pointers to standard device driver functions
	.word	_DLDI_writeSectors		@ 
	.word	_DLDI_clearStatus		@ 
	.word	_DLDI_shutdown			@ 

@---------------------------------------------------------------------------------
	.align
	.pool

.global _DLDI_startup
_DLDI_startup:
	mov r0,#1	@bool datatype: true == 1 
	bx lr

.global _DLDI_isInserted
_DLDI_isInserted:
	mov r0,#1	@bool datatype: true == 1 
	bx lr

.global _DLDI_clearStatus
_DLDI_clearStatus:
	mov r0,#1	@bool datatype: true == 1 
	bx lr

.global _DLDI_shutdown
_DLDI_shutdown:
	mov r0,#1	@bool datatype: true == 1 
	bx lr

@extern bool _DLDI_readSectors(unsigned int sector, unsigned int sectorCount, unsigned char* buffer);
.global _DLDI_readSectors
_DLDI_readSectors:
	push	{fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #32
	str	r0, [fp, #-24]
	str	r1, [fp, #-28]
	str	r2, [fp, #-32]
	mov	r3, #512	@0x200
	str	r3, [fp, #-12]
	mov	r3, #0
	str	r3, [fp, #-8]
	b	read_loop
read_copy:
	ldr	r3, [fp, #-8]
	ldr	r2, [fp, #-12]
	mul	r3, r2, r3
	ldr	r2, [fp, #-32]
	add	r3, r2, r3
	str	r3, [fp, #-16]
	ldr	r2, [fp, #-8]
	ldr	r3, [fp, #-24]
	add	r3, r2, r3
	ldr	r2, [fp, #-12]
	mul	r3, r2, r3
	add	r1, r3, #134217728	@ 0x8000000
	ldr	r2, [fp, #-16]
	ldr	r0, [fp, #-16]
	ldr	r3, [fp, #-12]
	add	r3, r0, r3
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	bl	copyMem
	ldr	r3, [fp, #-8]
	add	r3, r3, #1
	str	r3, [fp, #-8]
	ldr	r3, [fp, #-28]
	sub	r3, r3, #1
	str	r3, [fp, #-28]
read_loop:
	ldr	r3, [fp, #-28]
	cmp	r3, #0
	bne	read_copy
	mov	r3, #1
	mov	r0, r3
	sub	sp, fp, #4
	pop	{fp, lr}
	bx	lr


@extern bool _DLDI_writeSectors(unsigned int sector, unsigned int sectorCount, const unsigned char* buffer);
.global _DLDI_writeSectors
_DLDI_writeSectors:
	push	{fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #32
	str	r0, [fp, #-24]
	str	r1, [fp, #-28]
	str	r2, [fp, #-32]
	mov	r3, #512	@0x200
	str	r3, [fp, #-12]
	mov	r3, #0
	str	r3, [fp, #-8]
	b	write_loop
write_copy:
	ldr	r2, [fp, #-8]
	ldr	r3, [fp, #-24]
	add	r3, r2, r3
	ldr	r2, [fp, #-12]
	mul	r3, r2, r3
	add	r3, r3, #134217728	@0x8000000
	str	r3, [fp, #-16]
	ldr	r3, [fp, #-8]
	ldr	r2, [fp, #-12]
	mul	r3, r2, r3
	ldr	r2, [fp, #-32]
	add	r1, r2, r3
	ldr	r2, [fp, #-16]
	ldr	r0, [fp, #-16]
	ldr	r3, [fp, #-12]
	add	r3, r0, r3
	mov	r0, r1
	mov	r1, r2
	mov	r2, r3
	bl	copyMem
	ldr	r3, [fp, #-8]
	add	r3, r3, #1
	str	r3, [fp, #-8]
	ldr	r3, [fp, #-28]
	sub	r3, r3, #1
	str	r3, [fp, #-28]
write_loop:
	ldr	r3, [fp, #-28]
	cmp	r3, #0
	bne	write_copy 
	mov	r3, #1
	mov	r0, r3
	sub	sp, fp, #4
	pop	{fp, lr}
	bx	lr


@format:	r0 = src_vma_section, r1 = dest_lma_start, r2 = dest_lma_end	(where both lma are a whole physical memory region from start to end range address)
.global copyMem
copyMem:
	push {r0-r3, lr}
	mov r3,r2
	mov r2,r1
	mov r1,r0
	bl	copy
	pop {r0-r3, lr}
	bx                  lr                          /* return to caller */
.pool

copy:               
	cmp                 r2,r3                       /* check if we've reached the end */
	ldrlo               r0,[r1],#4                  /* if end not reached, get word and advance source pointer */
	strlo               r0,[r2],#4                  /* if end not reached, store word and advance destination pointer */
	blo                 copy                        /* if end not reached, branch back to loop */
	bx                  lr   

.pool
@---------------------------------------------------------------------------------
dldi_data_end:
@ Pad to end of allocated space
.space (DLDI_ALLOCATED_SPACE - (dldi_data_end - dldi_start))
dldi_end:
	.end
@---------------------------------------------------------------------------------
#endif