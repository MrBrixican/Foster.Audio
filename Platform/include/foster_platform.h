#ifndef FOSTER_H
#define FOSTER_H

#include <stdbool.h>
#include <stdint.h>

// Export API
#if defined _WIN32 || defined __CYGWIN__
	#define FOSTER_API __declspec(dllexport)
	#define FOSTER_CALL __cdecl
#elif __GNUC__
	#define FOSTER_API __attribute__((__visibility__("default")))
	#define FOSTER_CALL
#else
	#define FOSTER_API
	#define FOSTER_CALL
#endif

typedef uint8_t FosterBool;

typedef enum FosterLogging
{
	FOSTER_LOGGING_DEFAULT,
	FOSTER_LOGGING_ALL,
	FOSTER_LOGGING_NONE
} FosterLogging;

typedef enum FosterSoundFlags
{
    FOSTER_SOUND_FLAG_STREAM                = 0x00000001,
    FOSTER_SOUND_FLAG_DECODE                = 0x00000002,
    FOSTER_SOUND_FLAG_NO_SPATIALIZATION     = 0x00004000
} FosterSoundFlags;

typedef enum FosterAudioFormat
{
	FOSTER_AUDIO_FORMAT_UNKNOWN = 0,
	FOSTER_AUDIO_FORMAT_U8 = 1,
	FOSTER_AUDIO_FORMAT_S16 = 2,
	FOSTER_AUDIO_FORMAT_S24 = 3,
	FOSTER_AUDIO_FORMAT_S32 = 4,
	FOSTER_AUDIO_FORMAT_F32 = 5
} FosterAudioFormat;

typedef enum FosterSoundPositioning
{
	FOSTER_SOUND_POSITIONING_ABSOLUTE,
	FOSTER_SOUND_POSITIONING_RELATIVE
} FosterSoundPositioning;

typedef enum FosterSoundAttenuationModel
{
	FOSTER_SOUND_ATTENUATION_MODEL_NONE,
	FOSTER_SOUND_ATTENUATION_MODEL_INVERSE,
	FOSTER_SOUND_ATTENUATION_MODEL_LINEAR,
	FOSTER_SOUND_ATTENUATION_MODEL_EXPONENTIAL
} FosterSoundAttenuationModel;

typedef void (FOSTER_CALL * FosterLogFn)(const char *msg);
typedef void (FOSTER_CALL * FosterWriteFn)(void *context, void *data, int size);

typedef struct FosterSound FosterSound;
typedef struct FosterSoundGroup FosterSoundGroup;

typedef struct FosterDesc
{
	FosterLogFn onLogInfo;
	FosterLogFn onLogWarn;
	FosterLogFn onLogError;
	FosterLogging logging;
} FosterDesc;

typedef struct Vector3
{
	float x, y, z;
} Vector3;

typedef struct FosterSoundCone
{
	float innerAngleInRadians, outerAngleInRadians, outerGain;
} FosterSoundCone;

#if __cplusplus
extern "C" {
#endif

FOSTER_API void FosterAudioStartup(FosterDesc desc);

FOSTER_API void FosterAudioShutdown();

FOSTER_API float FosterAudioGetVolume();

FOSTER_API void FosterAudioSetVolume(int index, float value);

FOSTER_API int FosterAudioGetChannels();

FOSTER_API int FosterAudioGetSampleRate();

FOSTER_API uint64_t FosterAudioGetTimePcmFrames();

FOSTER_API void FosterAudioSetTimePcmFrames(int index, uint64_t value);

FOSTER_API int FosterAudioGetListenerCount();

FOSTER_API void* FosterAudioDecode(void* data, int length, FosterAudioFormat* format, int* channels, int* sampleRate, uint64_t* decodedFrameCount);

FOSTER_API void FosterAudioFree(void* data);

FOSTER_API void FosterAudioRegisterEncodedData(const char* name, void* data, int length);

FOSTER_API void FosterAudioRegisterDecodedData(const char* name, const void* data, uint64_t frameCount, FosterAudioFormat format, int channels, int sampleRate);

FOSTER_API void FosterAudioUnregisterData(const char* name);

FOSTER_API FosterBool FosterAudioListenerGetEnabled(int index);

FOSTER_API void FosterAudioListenerSetEnabled(int index, FosterBool value);

FOSTER_API Vector3 FosterAudioListenerGetPosition(int index);

FOSTER_API void FosterAudioListenerSetPosition(int index, Vector3 value);

FOSTER_API Vector3 FosterAudioListenerGetVelocity(int index);

FOSTER_API void FosterAudioListenerSetVelocity(int index, Vector3 value);

FOSTER_API Vector3 FosterAudioListenerGetDirection(int index);

FOSTER_API void FosterAudioListenerSetDirection(int index, Vector3 value);

FOSTER_API FosterSoundCone FosterAudioListenerGetCone(int index);

FOSTER_API void FosterAudioListenerSetCone(int index, FosterSoundCone value);

FOSTER_API Vector3 FosterAudioListenerGetWorldUp(int index);

FOSTER_API void FosterAudioListenerSetWorldUp(int index, Vector3 value);

FOSTER_API FosterSound* FosterSoundCreate(const char* path, FosterSoundFlags flags, FosterSoundGroup* soundGroup);

FOSTER_API void FosterSoundPlay(FosterSound* sound);

FOSTER_API void FosterSoundStop(FosterSound* sound);

FOSTER_API void FosterSoundDestroy(FosterSound* sound);

FOSTER_API float FosterSoundGetVolume(FosterSound* sound);

FOSTER_API void FosterSoundSetVolume(FosterSound* sound, float value);

FOSTER_API float FosterSoundGetPitch(FosterSound* sound);

FOSTER_API void FosterSoundSetPitch(FosterSound* sound, float value);

FOSTER_API float FosterSoundGetPan(FosterSound* sound);

FOSTER_API void FosterSoundSetPan(FosterSound* sound, float value);

FOSTER_API FosterBool FosterSoundGetPlaying(FosterSound* sound);

FOSTER_API FosterBool FosterSoundGetFinished(FosterSound* sound);

FOSTER_API void FosterSoundGetDataFormat(FosterSound* sound, FosterAudioFormat* format, int* channels, int* sampleRate);

FOSTER_API uint64_t FosterSoundGetLengthPcmFrames(FosterSound* sound);

FOSTER_API uint64_t FosterSoundGetCursorPcmFrames(FosterSound* sound);

FOSTER_API void FosterSoundSetCursorPcmFrames(FosterSound* sound, uint64_t value);

FOSTER_API FosterBool FosterSoundGetLooping(FosterSound* sound);

FOSTER_API void FosterSoundSetLooping(FosterSound* sound, FosterBool value);

FOSTER_API uint64_t FosterSoundGetLoopBeginPcmFrames(FosterSound* sound);

FOSTER_API void FosterSoundSetLoopBeginPcmFrames(FosterSound* sound, uint64_t value);

FOSTER_API uint64_t FosterSoundGetLoopEndPcmFrames(FosterSound* sound);

FOSTER_API void FosterSoundSetLoopEndPcmFrames(FosterSound* sound, uint64_t value);

FOSTER_API FosterBool FosterSoundGetSpatialized(FosterSound* sound);

FOSTER_API void FosterSoundSetSpatialized(FosterSound* sound, FosterBool value);

FOSTER_API Vector3 FosterSoundGetPosition(FosterSound* sound);

FOSTER_API void FosterSoundSetPosition(FosterSound* sound, Vector3 value);

FOSTER_API Vector3 FosterSoundGetVelocity(FosterSound* sound);

FOSTER_API void FosterSoundSetVelocity(FosterSound* sound, Vector3 value);

FOSTER_API Vector3 FosterSoundGetDirection(FosterSound* sound);

FOSTER_API void FosterSoundSetDirection(FosterSound* sound, Vector3 value);

FOSTER_API FosterSoundPositioning FosterSoundGetPositioning(FosterSound* sound);

FOSTER_API void FosterSoundSetPositioning(FosterSound* sound, FosterSoundPositioning value);

FOSTER_API int FosterSoundGetPinnedListenerIndex(FosterSound* sound);

FOSTER_API void FosterSoundSetPinnedListenerIndex(FosterSound* sound, int value);

FOSTER_API FosterSoundAttenuationModel FosterSoundGetAttenuationModel(FosterSound* sound);

FOSTER_API void FosterSoundSetAttenuationModel(FosterSound* sound, FosterSoundAttenuationModel value);

FOSTER_API float FosterSoundGetRolloff(FosterSound* sound);

FOSTER_API void FosterSoundSetRolloff(FosterSound* sound, float value);

FOSTER_API float FosterSoundGetMinGain(FosterSound* sound);

FOSTER_API void FosterSoundSetMinGain(FosterSound* sound, float value);

FOSTER_API float FosterSoundGetMaxGain(FosterSound* sound);

FOSTER_API void FosterSoundSetMaxGain(FosterSound* sound, float value);

FOSTER_API float FosterSoundGetMinDistance(FosterSound* sound);

FOSTER_API void FosterSoundSetMinDistance(FosterSound* sound, float value);

FOSTER_API float FosterSoundGetMaxDistance(FosterSound* sound);

FOSTER_API void FosterSoundSetMaxDistance(FosterSound* sound, float value);

FOSTER_API FosterSoundCone FosterSoundGetCone(FosterSound* sound);

FOSTER_API void FosterSoundSetCone(FosterSound* sound, FosterSoundCone value);

FOSTER_API float FosterSoundGetDirectionalAttenuationFactor(FosterSound* sound);

FOSTER_API void FosterSoundSetDirectionalAttenuationFactor(FosterSound* sound, float value);

FOSTER_API float FosterSoundGetDopplerFactor(FosterSound* sound);

FOSTER_API void FosterSoundSetDopplerFactor(FosterSound* sound, float value);

FOSTER_API FosterSoundGroup* FosterSoundGroupCreate(FosterSoundGroup* parent);

FOSTER_API void FosterSoundGroupDestroy(FosterSoundGroup* soundGroup);

FOSTER_API float FosterSoundGroupGetVolume(FosterSoundGroup* soundGroup);

FOSTER_API void FosterSoundGroupSetVolume(FosterSoundGroup* soundGroup, float value);

FOSTER_API float FosterSoundGroupGetPitch(FosterSoundGroup* soundGroup);

FOSTER_API void FosterSoundGroupSetPitch(FosterSoundGroup* soundGroup, float value);

#if __cplusplus
}
#endif

#endif
