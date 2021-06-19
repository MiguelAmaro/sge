/* date = January 5th 2021 8:02 pm */

#ifndef FLIGHTCONTROL_FILEIO_H
#define FLIGHTCONTROL_FILEIO_H

#include "LAL.h"

u32 
FileIO_GetFileSize(readonly u8 *path);

void 
BuildPrefixSuffixTable(u32* table, u32 table_size,readonly u8 *SearchTerm);

// TODO(MIGUEL): Take out file and use u8 * instead
u32 
StringMatchKMP(readonly u8 *Text, readonly u32 BytesToRead, readonly u8 *SearchTerm);

#endif //FLIGHTCONTROL_FILEIO_H
