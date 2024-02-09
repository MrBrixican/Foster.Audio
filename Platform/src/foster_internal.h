#ifndef FOSTER_INTERNAL_H
#define FOSTER_INTERNAL_H

#include "foster_platform.h"
#include "third_party/miniaudio.h"

// foster global state
typedef struct
{
	FosterBool running;
	FosterDesc desc;
	ma_engine* audioEngine;
} FosterState;

FosterState* FosterGetState();

void FosterLogInfo(const char* fmt, ...);

void FosterLogWarn(const char* fmt, ...);

void FosterLogError(const char* fmt, ...);

#endif
