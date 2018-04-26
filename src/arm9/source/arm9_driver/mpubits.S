#include "dsregs_asm.h"

.global	mpuEnable
.type	mpuEnable STT_FUNC
mpuEnable:
mrc	p15, 0, r0, c1, c0, 0
orr	r0,r0,#CR_M
mcr	p15, 0, r0, c1, c0, 0
bx lr

.global	mpuDisable
.type	mpuDisable STT_FUNC
mpuDisable:
mrc	p15, 0, r0, c1, c0, 0
bic	r0,r0,#CR_M
mcr	p15, 0, r0, c1, c0, 0
bx lr
