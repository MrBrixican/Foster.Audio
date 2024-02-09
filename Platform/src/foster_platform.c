#ifdef _MSC_VER
//#pragma warning(disable: C2220)
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "third_party/miniaudio.h"
#include "third_party/miniaudio_libvorbis.h"
#include "third_party/miniaudio_qoa.h"

#ifdef _MSC_VER
//#pragma warning(default: C2220)
#endif

#include "foster_platform.h"
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

static Vector3 vec3f_to_Vector3(ma_vec3f v)
{
	Vector3 value = {v.x, v.y, v.z};
	return value;
}

// begin Vorbis

static ma_result ma_decoding_backend_init__libvorbis(void* pUserData, ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void* pReadSeekTellUserData, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend)
{
    ma_result result;
    ma_libvorbis* pVorbis;

    (void)pUserData;

    pVorbis = (ma_libvorbis*)ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
    if (pVorbis == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    result = ma_libvorbis_init(onRead, onSeek, onTell, pReadSeekTellUserData, pConfig, pAllocationCallbacks, pVorbis);
    if (result != MA_SUCCESS) {
        ma_free(pVorbis, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pVorbis;

    return MA_SUCCESS;
}

static ma_result ma_decoding_backend_init_file__libvorbis(void* pUserData, const char* pFilePath, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend)
{
    ma_result result;
    ma_libvorbis* pVorbis;

    (void)pUserData;

    pVorbis = (ma_libvorbis*)ma_malloc(sizeof(*pVorbis), pAllocationCallbacks);
    if (pVorbis == NULL) {
        return MA_OUT_OF_MEMORY;
    }

    result = ma_libvorbis_init_file(pFilePath, pConfig, pAllocationCallbacks, pVorbis);
    if (result != MA_SUCCESS) {
        ma_free(pVorbis, pAllocationCallbacks);
        return result;
    }

    *ppBackend = pVorbis;

    return MA_SUCCESS;
}

static void ma_decoding_backend_uninit__libvorbis(void* pUserData, ma_data_source* pBackend, const ma_allocation_callbacks* pAllocationCallbacks)
{
    ma_libvorbis* pVorbis = (ma_libvorbis*)pBackend;

    (void)pUserData;

    ma_libvorbis_uninit(pVorbis, pAllocationCallbacks);
    ma_free(pVorbis, pAllocationCallbacks);
}

static ma_result ma_decoding_backend_get_channel_map__libvorbis(void* pUserData, ma_data_source* pBackend, ma_channel* pChannelMap, size_t channelMapCap)
{
    ma_libvorbis* pVorbis = (ma_libvorbis*)pBackend;

    (void)pUserData;

    return ma_libvorbis_get_data_format(pVorbis, NULL, NULL, NULL, pChannelMap, channelMapCap);
}

static ma_decoding_backend_vtable g_ma_decoding_backend_vtable_libvorbis =
{
    ma_decoding_backend_init__libvorbis,
    ma_decoding_backend_init_file__libvorbis,
    NULL, /* onInitFileW() */
    NULL, /* onInitMemory() */
    ma_decoding_backend_uninit__libvorbis
};

// end Vorbis

// begin QOA

static ma_result ma_decoding_backend_init__qoa(void* pUserData, ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void* pReadSeekTellUserData, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_data_source** ppBackend)
{
	ma_result result;
	ma_qoa* pQoa;

	(void)pUserData;

	pQoa = (ma_qoa*)ma_malloc(sizeof(*pQoa), pAllocationCallbacks);
	if (pQoa == NULL) {
		return MA_OUT_OF_MEMORY;
	}

	result = ma_qoa_init(onRead, onSeek, onTell, pReadSeekTellUserData, pConfig, pAllocationCallbacks, pQoa);
	if (result != MA_SUCCESS) {
		ma_free(pQoa, pAllocationCallbacks);
		return result;
	}

	*ppBackend = pQoa;

	return MA_SUCCESS;
}

static void ma_decoding_backend_uninit__qoa(void* pUserData, ma_data_source* pBackend, const ma_allocation_callbacks* pAllocationCallbacks)
{
	ma_qoa* pQoa = (ma_qoa*)pBackend;

	(void)pUserData;

	ma_qoa_uninit(pQoa, pAllocationCallbacks);
	ma_free(pQoa, pAllocationCallbacks);
}

static ma_result ma_decoding_backend_get_channel_map__qoa(void* pUserData, ma_data_source* pBackend, ma_channel* pChannelMap, size_t channelMapCap)
{
	ma_qoa* pQoa = (ma_qoa*)pBackend;

	(void)pUserData;

	return ma_qoa_get_data_format(pQoa, NULL, NULL, NULL, pChannelMap, channelMapCap);
}

static ma_decoding_backend_vtable g_ma_decoding_backend_vtable_qoa =
{
	ma_decoding_backend_init__qoa,
	NULL, /* onInitFile() */
	NULL, /* onInitFileW() */
	NULL, /* onInitMemory() */
	ma_decoding_backend_uninit__qoa
};

// end QOA

// begin Audio

/*
Add your custom backend vtables here. The order in the array defines the order of priority. The
vtables will be passed in to the resource manager config.
*/
static ma_decoding_backend_vtable* pCustomBackendVTables[] =
{
	&g_ma_decoding_backend_vtable_qoa,
	&g_ma_decoding_backend_vtable_libvorbis,
};

void FosterAudioStartup(FosterDesc desc)
{
	fstate.desc = desc;
	fstate.running = false;

	fstate.audioEngine = ma_malloc(sizeof(ma_engine), NULL);

	ma_resource_manager_config resourceManagerConfig;
	ma_resource_manager* resourceManager = ma_malloc(sizeof(ma_resource_manager), NULL);
	ma_engine_config engineConfig;

	/* Using custom decoding backends requires a resource manager. */
	resourceManagerConfig = ma_resource_manager_config_init();
	resourceManagerConfig.ppCustomDecodingBackendVTables = pCustomBackendVTables;
	resourceManagerConfig.customDecodingBackendCount = sizeof(pCustomBackendVTables) / sizeof(pCustomBackendVTables[0]);
	resourceManagerConfig.pCustomDecodingBackendUserData = NULL;  /* <-- This will be passed in to the pUserData parameter of each function in the decoding backend vtables. */

	if (MA_SUCCESS != ma_resource_manager_init(&resourceManagerConfig, resourceManager)) {
		FosterLogError("Unable to create Audio Engine (Resource Manager)");
		ma_free(fstate.audioEngine, NULL);
		ma_free(resourceManager, NULL);
		return;
	}

	/* Once we have a resource manager we can create the engine. */
	engineConfig = ma_engine_config_init();
	engineConfig.pResourceManager = resourceManager;

	if (MA_SUCCESS != ma_engine_init(&engineConfig, fstate.audioEngine))
	{
		FosterLogError("Unable to create Audio Engine");
		ma_free(fstate.audioEngine, NULL);
		ma_free(resourceManager, NULL);
		return;
	}

	fstate.running = true;
}

void FosterAudioShutdown()
{
	if (!fstate.running)
		return;

	ma_engine_uninit(fstate.audioEngine);
	ma_free(fstate.audioEngine, NULL);

	fstate.running = false;
}

float FosterAudioGetVolume()
{
	return ma_engine_get_volume(fstate.audioEngine);
}

void FosterAudioSetVolume(int index, float value)
{
	ma_engine_set_volume(fstate.audioEngine, value);
}

int FosterAudioGetChannels()
{
	return ma_engine_get_channels(fstate.audioEngine);
}

int FosterAudioGetSampleRate()
{
	return ma_engine_get_sample_rate(fstate.audioEngine);
}

uint64_t FosterAudioGetTimePcmFrames()
{
	return ma_engine_get_time_in_pcm_frames(fstate.audioEngine);
}

void FosterAudioSetTimePcmFrames(int index, uint64_t value)
{
	ma_engine_set_time_in_pcm_frames(fstate.audioEngine, value);
}

int FosterAudioGetListenerCount()
{
	return ma_engine_get_listener_count(fstate.audioEngine);
}

void *FosterAudioDecode(void *data, int length, FosterAudioFormat *format, int *channels, int *sampleRate, uint64_t *decodedFrameCount)
{
	void *frames = NULL;
	ma_decoder_config config = ma_decoder_config_init(*format, *channels, *sampleRate);
	config.pCustomBackendUserData = NULL;  /* In this example our backend objects are contained within a ma_decoder_ex object to avoid a malloc. Our vtables need to know about this. */
	config.ppCustomBackendVTables = pCustomBackendVTables;
	config.customBackendCount = sizeof(pCustomBackendVTables) / sizeof(pCustomBackendVTables[0]);

	ma_decode_memory(data, length, &config, decodedFrameCount, &frames);
	*format = config.format;
	*channels = config.channels;
	*sampleRate = config.sampleRate;
	return frames;
}

void *FosterAudioEncodeWAV(void* data, uint64_t frameCount, FosterAudioFormat format, int channels, int sampleRate)
{
	return NULL; // TODO
}

void* FosterAudioEncodeQOA(void* data, uint64_t frameCount, FosterAudioFormat format, int channels, int sampleRate)
{
	return NULL; // TODO
}

void FosterAudioFree(void *data)
{
	ma_free(data, NULL);
}

void FosterAudioRegisterEncodedData(const char *name, void *data, int length)
{
	ma_resource_manager *manager = ma_engine_get_resource_manager(fstate.audioEngine);
	ma_resource_manager_register_encoded_data(manager, name, data, length);
}

void FosterAudioRegisterDecodedData(const char *name, const void *data, uint64_t frameCount, FosterAudioFormat format, int channels, int sampleRate)
{
	ma_resource_manager *manager = ma_engine_get_resource_manager(fstate.audioEngine);
	ma_resource_manager_register_decoded_data(manager, name, data, frameCount, format, channels, sampleRate);
}

void FosterAudioUnregisterData(const char *name)
{
	ma_resource_manager *manager = ma_engine_get_resource_manager(fstate.audioEngine);
	ma_resource_manager_unregister_data(manager, name);
}

// end Audio

// begin AudioListener

FosterBool FosterAudioListenerGetEnabled(int index)
{
	return ma_engine_listener_is_enabled(fstate.audioEngine, index);
}

void FosterAudioListenerSetEnabled(int index, FosterBool value)
{
	ma_engine_listener_set_enabled(fstate.audioEngine, index, value);
}

Vector3 FosterAudioListenerGetPosition(int index)
{
	return vec3f_to_Vector3(ma_engine_listener_get_position(fstate.audioEngine, index));
}

void FosterAudioListenerSetPosition(int index, Vector3 value)
{
	ma_engine_listener_set_position(fstate.audioEngine, index, value.x, value.y, value.z);
}

Vector3 FosterAudioListenerGetVelocity(int index)
{
	return vec3f_to_Vector3(ma_engine_listener_get_velocity(fstate.audioEngine, index));
}

void FosterAudioListenerSetVelocity(int index, Vector3 value)
{
	ma_engine_listener_set_velocity(fstate.audioEngine, index, value.x, value.y, value.z);
}

Vector3 FosterAudioListenerGetDirection(int index)
{
	return vec3f_to_Vector3(ma_engine_listener_get_direction(fstate.audioEngine, index));
}

void FosterAudioListenerSetDirection(int index, Vector3 value)
{
	ma_engine_listener_set_direction(fstate.audioEngine, index, value.x, value.y, value.z);
}

FosterSoundCone FosterAudioListenerGetCone(int index)
{
	FosterSoundCone value;
	ma_engine_listener_get_cone(fstate.audioEngine, index, &value.innerAngleInRadians, &value.outerAngleInRadians, &value.outerGain);
	return value;
}

void FosterAudioListenerSetCone(int index, FosterSoundCone value)
{
	ma_engine_listener_set_cone(fstate.audioEngine, index, value.innerAngleInRadians, value.outerAngleInRadians, value.outerGain);
}

Vector3 FosterAudioListenerGetWorldUp(int index)
{
	return vec3f_to_Vector3(ma_engine_listener_get_world_up(fstate.audioEngine, index));
}

void FosterAudioListenerSetWorldUp(int index, Vector3 value)
{
	ma_engine_listener_set_world_up(fstate.audioEngine, index, value.x, value.y, value.z);
}

// end AudioListener

// begin Sound

FosterSound *FosterSoundCreate(const char *path, FosterSoundFlags flags, FosterSoundGroup *soundGroup)
{
	FosterState *state = FosterGetState();
	ma_sound *sound = ma_malloc(sizeof(ma_sound), NULL);

	if (MA_SUCCESS != ma_sound_init_from_file(state->audioEngine, path, flags, (ma_sound_group *)soundGroup, NULL, sound))
	{
		FosterLogError("Unable to create Sound from file");
		ma_free(sound, NULL);
		return NULL;
	}

	return (FosterSound *)sound;
}

void FosterSoundPlay(FosterSound *sound)
{
	ma_sound_start((ma_sound *)sound);
}

void FosterSoundStop(FosterSound *sound)
{
	ma_sound_stop((ma_sound *)sound);
}

void FosterSoundDestroy(FosterSound *sound)
{
	ma_sound_uninit((ma_sound *)sound);
	ma_free((ma_sound *)sound, NULL);
}

float FosterSoundGetVolume(FosterSound *sound)
{
	return ma_sound_get_volume((ma_sound *)sound);
}

void FosterSoundSetVolume(FosterSound *sound, float value)
{
	ma_sound_set_volume((ma_sound *)sound, value);
}

float FosterSoundGetPitch(FosterSound *sound)
{
	return ma_sound_get_pitch((ma_sound *)sound);
}

void FosterSoundSetPitch(FosterSound *sound, float value)
{
	ma_sound_set_pitch((ma_sound *)sound, value);
}

float FosterSoundGetPan(FosterSound *sound)
{
	return ma_sound_get_pan((ma_sound *)sound);
}

void FosterSoundSetPan(FosterSound *sound, float value)
{
	ma_sound_set_pan((ma_sound *)sound, value);
}

FosterBool FosterSoundGetPlaying(FosterSound *sound)
{
	return ma_sound_is_playing((ma_sound *)sound);
}

FosterBool FosterSoundGetFinished(FosterSound *sound)
{
	return ma_sound_at_end((ma_sound *)sound);
}

void FosterSoundGetDataFormat(FosterSound *sound, FosterAudioFormat *format, int *channels, int *sampleRate)
{
	ma_sound_get_data_format((ma_sound*)sound, format, channels, sampleRate, NULL, 0);
}

uint64_t FosterSoundGetLengthPcmFrames(FosterSound *sound)
{
	uint64_t value = 0;
	ma_sound_get_length_in_pcm_frames((ma_sound *)sound, &value);
	return value;
}

uint64_t FosterSoundGetCursorPcmFrames(FosterSound *sound)
{
	uint64_t value = 0;
	ma_sound_get_cursor_in_pcm_frames((ma_sound *)sound, &value);
	return value;
}

void FosterSoundSetCursorPcmFrames(FosterSound *sound, uint64_t value)
{
	ma_sound_seek_to_pcm_frame((ma_sound *)sound, value);
}

FosterBool FosterSoundGetLooping(FosterSound *sound)
{
	return ma_sound_is_looping((ma_sound *)sound);
}

void FosterSoundSetLooping(FosterSound *sound, FosterBool value)
{
	ma_sound_set_looping((ma_sound *)sound, value);
}

uint64_t FosterSoundGetLoopBeginPcmFrames(FosterSound *sound)
{
	uint64_t value = 0;
	ma_data_source *source = ma_sound_get_data_source((ma_sound *)sound);
	ma_data_source_get_loop_point_in_pcm_frames(source, &value, NULL);
	return value;
}

void FosterSoundSetLoopBeginPcmFrames(FosterSound *sound, uint64_t value)
{
	ma_data_source *source = ma_sound_get_data_source((ma_sound *)sound);
	ma_data_source_set_loop_point_in_pcm_frames(source, value, FosterSoundGetLoopEndPcmFrames(sound));
}

uint64_t FosterSoundGetLoopEndPcmFrames(FosterSound *sound)
{
	uint64_t value = 0;
	ma_data_source *source = ma_sound_get_data_source((ma_sound *)sound);
	ma_data_source_get_loop_point_in_pcm_frames(source, NULL, &value);
	return value;
}

void FosterSoundSetLoopEndPcmFrames(FosterSound *sound, uint64_t value)
{
	ma_data_source *source = ma_sound_get_data_source((ma_sound *)sound);
	ma_data_source_set_loop_point_in_pcm_frames(source, FosterSoundGetLoopBeginPcmFrames(sound), value);
}

FosterBool FosterSoundGetSpatialized(FosterSound *sound)
{
	return ma_sound_group_is_spatialization_enabled((ma_sound *)sound);
}

void FosterSoundSetSpatialized(FosterSound *sound, FosterBool value)
{
	ma_sound_group_set_spatialization_enabled((ma_sound *)sound, value);
}

Vector3 FosterSoundGetPosition(FosterSound *sound)
{
	return vec3f_to_Vector3(ma_sound_get_position((ma_sound *)sound));
}

void FosterSoundSetPosition(FosterSound *sound, Vector3 value)
{
	ma_sound_set_position((ma_sound *)sound, value.x, value.y, value.z);
}

Vector3 FosterSoundGetVelocity(FosterSound *sound)
{
	return vec3f_to_Vector3(ma_sound_get_velocity((ma_sound *)sound));
}

void FosterSoundSetVelocity(FosterSound *sound, Vector3 value)
{
	ma_sound_set_velocity((ma_sound *)sound, value.x, value.y, value.z);
}

Vector3 FosterSoundGetDirection(FosterSound *sound)
{
	return vec3f_to_Vector3(ma_sound_get_direction((ma_sound *)sound));
}

void FosterSoundSetDirection(FosterSound *sound, Vector3 value)
{
	ma_sound_set_direction((ma_sound *)sound, value.x, value.y, value.z);
}

FosterSoundPositioning FosterSoundGetPositioning(FosterSound *sound)
{
	return ma_sound_get_positioning((ma_sound *)sound);
}

void FosterSoundSetPositioning(FosterSound *sound, FosterSoundPositioning value)
{
	ma_sound_set_positioning((ma_sound *)sound, value);
}

int FosterSoundGetPinnedListenerIndex(FosterSound *sound)
{
	return ma_sound_get_pinned_listener_index((ma_sound *)sound);
}

void FosterSoundSetPinnedListenerIndex(FosterSound *sound, int value)
{
	ma_sound_set_pinned_listener_index((ma_sound *)sound, value);
}

FosterSoundAttenuationModel FosterSoundGetAttenuationModel(FosterSound *sound)
{
	return ma_sound_get_attenuation_model((ma_sound *)sound);
}

void FosterSoundSetAttenuationModel(FosterSound *sound, FosterSoundAttenuationModel value)
{
	ma_sound_set_attenuation_model((ma_sound *)sound, value);
}

float FosterSoundGetRolloff(FosterSound *sound)
{
	return ma_sound_get_rolloff((ma_sound *)sound);
}

void FosterSoundSetRolloff(FosterSound *sound, float value)
{
	ma_sound_set_rolloff((ma_sound *)sound, value);
}

float FosterSoundGetMinGain(FosterSound *sound)
{
	return ma_sound_get_min_gain((ma_sound *)sound);
}

void FosterSoundSetMinGain(FosterSound *sound, float value)
{
	ma_sound_set_min_gain((ma_sound *)sound, value);
}

float FosterSoundGetMaxGain(FosterSound *sound)
{
	return ma_sound_get_max_gain((ma_sound *)sound);
}

void FosterSoundSetMaxGain(FosterSound *sound, float value)
{
	ma_sound_set_max_gain((ma_sound *)sound, value);
}

float FosterSoundGetMinDistance(FosterSound *sound)
{
	return ma_sound_get_min_distance((ma_sound *)sound);
}

void FosterSoundSetMinDistance(FosterSound *sound, float value)
{
	ma_sound_set_min_distance((ma_sound *)sound, value);
}

float FosterSoundGetMaxDistance(FosterSound *sound)
{
	return ma_sound_get_max_distance((ma_sound *)sound);
}

void FosterSoundSetMaxDistance(FosterSound *sound, float value)
{
	ma_sound_set_max_distance((ma_sound *)sound, value);
}

FosterSoundCone FosterSoundGetCone(FosterSound *sound)
{
	FosterSoundCone value;
	ma_sound_get_cone((ma_sound *)sound, &value.innerAngleInRadians, &value.outerAngleInRadians, &value.outerGain);
	return value;
}

void FosterSoundSetCone(FosterSound *sound, FosterSoundCone value)
{
	ma_sound_set_cone((ma_sound *)sound, value.innerAngleInRadians, value.outerAngleInRadians, value.outerGain);
}

float FosterSoundGetDirectionalAttenuationFactor(FosterSound *sound)
{
	return ma_sound_get_directional_attenuation_factor((ma_sound *)sound);
}

void FosterSoundSetDirectionalAttenuationFactor(FosterSound *sound, float value)
{
	ma_sound_set_directional_attenuation_factor((ma_sound *)sound, value);
}

float FosterSoundGetDopplerFactor(FosterSound *sound)
{
	return ma_sound_get_doppler_factor((ma_sound *)sound);
}

void FosterSoundSetDopplerFactor(FosterSound *sound, float value)
{
	ma_sound_set_doppler_factor((ma_sound *)sound, value);
}

// end Sound

// begin SoundGroup

FosterSoundGroup *FosterSoundGroupCreate(FosterSoundGroup* parent)
{
	FosterState *state = FosterGetState();
	ma_sound_group *soundGroup = ma_malloc(sizeof(ma_sound_group), NULL);

	if (MA_SUCCESS != ma_sound_group_init(state->audioEngine, 0, (ma_sound_group*)parent, soundGroup))
	{
		FosterLogError("Unable to create SoundGroup");
		ma_free(soundGroup, NULL);
		return NULL;
	}

	return (FosterSoundGroup *)soundGroup;
}

void FosterSoundGroupDestroy(FosterSoundGroup *soundGroup)
{
	ma_sound_group_uninit((ma_sound_group *)soundGroup);
	ma_free((ma_sound_group *)soundGroup, NULL);
}

float FosterSoundGroupGetVolume(FosterSoundGroup *soundGroup)
{
	return ma_sound_group_get_volume((ma_sound_group *)soundGroup);
}

void FosterSoundGroupSetVolume(FosterSoundGroup *soundGroup, float value)
{
	ma_sound_group_set_volume((ma_sound_group *)soundGroup, value);
}

float FosterSoundGroupGetPitch(FosterSoundGroup* soundGroup)
{
	return ma_sound_group_get_pitch((ma_sound_group*)soundGroup);
}

void FosterSoundGroupSetPitch(FosterSoundGroup* soundGroup, float value)
{
	ma_sound_group_set_pitch((ma_sound_group*)soundGroup, value);
}

// end SoundGroup

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
