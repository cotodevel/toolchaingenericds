#ifndef _DMA_FUNCS_H_
#define _DMA_FUNCS_H_

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"
#include <unistd.h>
#include "biosTGDS.h"

/**
 * Perform a DMA copy.
 * @param source Pointer to the source.
 * @param dest Pointer to the destination.
 * @param count The number of values to copy.
 */
void woopsiDmaCopy(const u16* source, u16* dest, u32 count);

/**
 * Fill region of memory with the same value using DMA.
 * @param fill The value to fill with.
 * @param dest Pointer to the destination.
 * @param count The number of values to write.
 */
void woopsiDmaFill(u16 fill, u16* dest, u32 count);

#endif

#ifdef __cplusplus
extern "C"{
#endif

extern void parseDirNameTGDS(char * dirName);
extern void parsefileNameTGDS(char * fileName);

#ifdef __cplusplus
}
#endif
