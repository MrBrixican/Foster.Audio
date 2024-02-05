#include "foster_platform.h"
#include "foster_audio.h"
#include "foster_internal.h"

#include <stdio.h>
#include <stdarg.h>

#define FOSTER_MAX_MESSAGE_SIZE 1024

#define FOSTER_CHECK(flags, flag) \
	(((flags) & (flag)) != 0)

#define FOSTER_ASSERT_RUNNING_RET(func, ret) \
	do { if (!fstate.running) { FosterLogError("Failed '%s', Foster is not running", #func); return ret; } } while(0)

#define FOSTER_ASSERT_RUNNING(func) \
	do { if (!fstate.running) { FosterLogError("Failed '%s', Foster is not running", #func); return; } } while(0)

static FosterState fstate;

FosterState* FosterGetState()
{
	return &fstate;
}

void FosterStartup(FosterDesc desc)
{
	fstate.desc = desc;
	fstate.running = false;

	// initialize audio
	if (!FosterAudioStartup())
	{
		FosterLogError("Foster Failed to start Audio");
		return;
	}

	fstate.running = true;
}

void FosterShutdown()
{
	if (!fstate.running)
		return;
	if (fstate.audioEngine != NULL)
		FosterAudioShutdown();
	fstate.running = false;
}

void FosterLogInfo(const char* fmt, ...)
{
	if (fstate.desc.logging == FOSTER_LOGGING_NONE ||
		fstate.desc.onLogInfo == NULL)
		return;
		
	char msg[FOSTER_MAX_MESSAGE_SIZE];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	fstate.desc.onLogInfo(msg);
}

void FosterLogWarn(const char* fmt, ...)
{
	if (fstate.desc.logging == FOSTER_LOGGING_NONE ||
		fstate.desc.onLogWarn == NULL)
		return;

	char msg[FOSTER_MAX_MESSAGE_SIZE];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	fstate.desc.onLogWarn(msg);
}

void FosterLogError(const char* fmt, ...)
{
	if (fstate.desc.logging == FOSTER_LOGGING_NONE ||
		fstate.desc.onLogError == NULL)
		return;

	char msg[FOSTER_MAX_MESSAGE_SIZE];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	fstate.desc.onLogError(msg);
}
