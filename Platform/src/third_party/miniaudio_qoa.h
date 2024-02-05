/*
This implements a data source that decodes qoa streams via the reference implementation

This object can be plugged into any `ma_data_source_*()` API and can also be used as a custom
decoding backend. See the custom_decoder example.

You need to include this file after miniaudio.h.
*/
#ifndef miniaudio_qoa_h
#define miniaudio_qoa_h

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(MA_NO_QOA)
#define QOA_IMPLEMENTATION
#include "qoa.h"
#endif

    typedef struct
    {
        ma_data_source_base ds; /* The qoa decoder can be used independently as a data source. */
        ma_read_proc onRead;
        ma_seek_proc onSeek;
        ma_tell_proc onTell;
        void *pReadSeekTellUserData;
        ma_format format; /* Will be s16. */
#if !defined(MA_NO_QOA)
        qoa_desc info;
        ma_uint64 first_frame_pos;
        ma_uint64 sample_pos;
        size_t buffer_len;
        ma_uint8 *buffer;
        ma_uint64 sample_data_pos;
        ma_uint64 sample_data_pos_seek;
        size_t sample_data_len;
        ma_int16 *sample_data;
#endif
    } ma_qoa;

    MA_API ma_result ma_qoa_init(ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void *pReadSeekTellUserData, const ma_decoding_backend_config *pConfig, const ma_allocation_callbacks *pAllocationCallbacks, ma_qoa *pQOA);
    // MA_API ma_result ma_qoa_init_file(const char* pFilePath, const ma_decoding_backend_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_qoa* pQOA);
    MA_API void ma_qoa_uninit(ma_qoa *pQOA, const ma_allocation_callbacks *pAllocationCallbacks);
    MA_API ma_result ma_qoa_read_pcm_frames(ma_qoa *pQOA, void *pFramesOut, ma_uint64 frameCount, ma_uint64 *pFramesRead);
    MA_API ma_result ma_qoa_seek_to_pcm_frame(ma_qoa *pQOA, ma_uint64 frameIndex);
    MA_API ma_result ma_qoa_get_data_format(ma_qoa *pQOA, ma_format *pFormat, ma_uint32 *pChannels, ma_uint32 *pSampleRate, ma_channel *pChannelMap, size_t channelMapCap);
    MA_API ma_result ma_qoa_get_cursor_in_pcm_frames(ma_qoa *pQOA, ma_uint64 *pCursor);
    MA_API ma_result ma_qoa_get_length_in_pcm_frames(ma_qoa *pQOA, ma_uint64 *pLength);

#ifdef __cplusplus
}
#endif
#endif

#if defined(MINIAUDIO_IMPLEMENTATION) || defined(MA_IMPLEMENTATION)

static ma_result ma_qoa_ds_read(ma_data_source *pDataSource, void *pFramesOut, ma_uint64 frameCount, ma_uint64 *pFramesRead)
{
    return ma_qoa_read_pcm_frames((ma_qoa *)pDataSource, pFramesOut, frameCount, pFramesRead);
}

static ma_result ma_qoa_ds_seek(ma_data_source *pDataSource, ma_uint64 frameIndex)
{
    return ma_qoa_seek_to_pcm_frame((ma_qoa *)pDataSource, frameIndex);
}

static ma_result ma_qoa_ds_get_data_format(ma_data_source *pDataSource, ma_format *pFormat, ma_uint32 *pChannels, ma_uint32 *pSampleRate, ma_channel *pChannelMap, size_t channelMapCap)
{
    return ma_qoa_get_data_format((ma_qoa *)pDataSource, pFormat, pChannels, pSampleRate, pChannelMap, channelMapCap);
}

static ma_result ma_qoa_ds_get_cursor(ma_data_source *pDataSource, ma_uint64 *pCursor)
{
    return ma_qoa_get_cursor_in_pcm_frames((ma_qoa *)pDataSource, pCursor);
}

static ma_result ma_qoa_ds_get_length(ma_data_source *pDataSource, ma_uint64 *pLength)
{
    return ma_qoa_get_length_in_pcm_frames((ma_qoa *)pDataSource, pLength);
}

static ma_data_source_vtable g_ma_qoa_ds_vtable =
    {
        ma_qoa_ds_read,
        ma_qoa_ds_seek,
        ma_qoa_ds_get_data_format,
        ma_qoa_ds_get_cursor,
        ma_qoa_ds_get_length};

static ma_result ma_qoa_init_internal(const ma_decoding_backend_config *pConfig, ma_qoa *pQOA)
{
    ma_result result;
    ma_data_source_config dataSourceConfig;

    if (pQOA == NULL)
    {
        return MA_INVALID_ARGS;
    }

    MA_ZERO_OBJECT(pQOA);
    pQOA->format = ma_format_s16; /* s16 by default. */

    if (pConfig != NULL && (pConfig->preferredFormat == ma_format_s16))
    {
        pQOA->format = pConfig->preferredFormat;
    }
    else
    {
        /* Getting here means something other than s16 was specified. Just leave this unset to use the default format. */
    }

    dataSourceConfig = ma_data_source_config_init();
    dataSourceConfig.vtable = &g_ma_qoa_ds_vtable;

    result = ma_data_source_init(&dataSourceConfig, &pQOA->ds);
    if (result != MA_SUCCESS)
    {
        return result; /* Failed to initialize the base data source. */
    }

    return MA_SUCCESS;
}

MA_API ma_result ma_qoa_init(ma_read_proc onRead, ma_seek_proc onSeek, ma_tell_proc onTell, void *pReadSeekTellUserData, const ma_decoding_backend_config *pConfig, const ma_allocation_callbacks *pAllocationCallbacks, ma_qoa *pQOA)
{
    ma_result result;

    (void)pAllocationCallbacks; /* Can't seem to find a way to configure memory allocations in qoa. */

    result = ma_qoa_init_internal(pConfig, pQOA);
    if (result != MA_SUCCESS)
    {
        return result;
    }

    if (onRead == NULL || onSeek == NULL)
    {
        return MA_INVALID_ARGS; /* onRead and onSeek are mandatory. */
    }

    pQOA->onRead = onRead;
    pQOA->onSeek = onSeek;
    pQOA->onTell = onTell;
    pQOA->pReadSeekTellUserData = pReadSeekTellUserData;

#if !defined(MA_NO_QOA)
    {
        /* Read and decode the file header */

        ma_uint8 header[QOA_MIN_FILESIZE];
        size_t read = 0;

        if (onRead(pReadSeekTellUserData, header, QOA_MIN_FILESIZE, &read) != MA_SUCCESS)
        {
            return MA_IO_ERROR;
        }

        pQOA->first_frame_pos = qoa_decode_header(header, QOA_MIN_FILESIZE, &pQOA->info);

        if (!pQOA->first_frame_pos)
        {
            return MA_INVALID_FILE;
        }

        /* Rewind the file back to beginning of the first frame */

        if (onSeek(pReadSeekTellUserData, pQOA->first_frame_pos, ma_seek_origin_start) != MA_SUCCESS)
        {
            return MA_BAD_SEEK;
        }

        /* Allocate memory for the sample data for one frame and a buffer to hold one frame of encoded data. */

        pQOA->sample_data = (ma_int16 *)malloc(pQOA->info.channels * QOA_FRAME_LEN * sizeof(short) * 2);
        if (!pQOA->sample_data)
        {
            return MA_OUT_OF_MEMORY;
        }

        pQOA->buffer = (ma_uint8 *)malloc(qoa_max_frame_size(&pQOA->info));
        if (!pQOA->buffer)
        {
            free(pQOA->sample_data);
            pQOA->sample_data = NULL;
            return MA_OUT_OF_MEMORY;
        }

        return MA_SUCCESS;
    }
#else
    {
        /* qoa is disabled. */
        return MA_NOT_IMPLEMENTED;
    }
#endif
}

MA_API void ma_qoa_uninit(ma_qoa *pQOA, const ma_allocation_callbacks *pAllocationCallbacks)
{
    if (pQOA == NULL)
    {
        return;
    }

    (void)pAllocationCallbacks;

#if !defined(MA_NO_QOA)
    {
        free(pQOA->buffer);
        free(pQOA->sample_data);
    }
#else
    {
        /* qoa is disabled. Should never hit this since initialization would have failed. */
        MA_ASSERT(MA_FALSE);
    }
#endif

    ma_data_source_uninit(&pQOA->ds);
}

MA_API ma_result ma_qoa_read_pcm_frames(ma_qoa *pQOA, void *pFramesOut, ma_uint64 frameCount, ma_uint64 *pFramesRead)
{
    if (pFramesRead != NULL)
    {
        *pFramesRead = 0;
    }

    if (frameCount == 0)
    {
        return MA_INVALID_ARGS;
    }

    if (pQOA == NULL)
    {
        return MA_INVALID_ARGS;
    }

#if !defined(MA_NO_QOA)
    {
        ma_result result = MA_SUCCESS; /* Must be initialized to MA_SUCCESS. */

        ma_uint64 src_index = pQOA->sample_data_pos * pQOA->info.channels;
        ma_uint64 dst_index = 0, totalFramesRead = 0;

        for (ma_uint64 i = 0; i < frameCount; i++)
        {
            /* Do we have to decode more samples? */
            if (pQOA->sample_data_len - pQOA->sample_data_pos == 0)
            {
                pQOA->buffer_len = 0;
                pQOA->onRead(pQOA->pReadSeekTellUserData, pQOA->buffer, qoa_max_frame_size(&pQOA->info), &pQOA->buffer_len);

                unsigned int frame_len;
                qoa_decode_frame(pQOA->buffer, pQOA->buffer_len, &pQOA->info, pQOA->sample_data, &frame_len);
                pQOA->sample_data_pos = pQOA->sample_data_pos_seek;
                pQOA->sample_data_pos = 0;
                pQOA->sample_data_len = frame_len;

                if (!frame_len)
                {
                    result = MA_AT_END;
                    break;
                }

                src_index = pQOA->sample_data_pos * pQOA->info.channels;
            }

            ma_int16 *sample_data = (ma_int16 *)(pFramesOut);

            for (unsigned int c = 0; c < pQOA->info.channels; c++)
                sample_data[dst_index++] = pQOA->sample_data[src_index++];

            pQOA->sample_data_pos++;
            pQOA->sample_pos++;

            ++totalFramesRead;
        }

        if (pFramesRead != NULL)
        {
            *pFramesRead = totalFramesRead;
        }

        return result;
    }
#else
    {
        /* qoa is disabled. Should never hit this since initialization would have failed. */
        MA_ASSERT(MA_FALSE);

        (void)pFramesOut;
        (void)frameCount;
        (void)pFramesRead;

        return MA_NOT_IMPLEMENTED;
    }
#endif
}

MA_API ma_result ma_qoa_seek_to_pcm_frame(ma_qoa *pQOA, ma_uint64 frameIndex)
{
    if (pQOA == NULL)
    {
        return MA_INVALID_ARGS;
    }

#if !defined(MA_NO_QOA)
    {
        if (frameIndex > pQOA->info.samples)
        {
            return MA_INVALID_ARGS;
        }

        ma_uint64 qoaFrame = frameIndex / QOA_FRAME_LEN;
        ma_uint64 maxQoaFrame = pQOA->info.samples / QOA_FRAME_LEN;

        if (qoaFrame > maxQoaFrame)
        {
            qoaFrame = maxQoaFrame;
        }

        pQOA->sample_pos = frameIndex;
        pQOA->sample_data_len = 0;
        pQOA->sample_data_pos = 0;
        pQOA->sample_data_pos_seek = frameIndex % QOA_FRAME_LEN;

        ma_uint64 offset = pQOA->first_frame_pos + qoaFrame * qoa_max_frame_size(&pQOA->info);
        if (pQOA->onSeek(pQOA->pReadSeekTellUserData, offset, ma_seek_origin_start) != MA_SUCCESS)
        {
            return MA_BAD_SEEK;
        }

        // TODO: frame accurate seek

        return MA_SUCCESS;
    }
#else
    {
        /* qoa is disabled. Should never hit this since initialization would have failed. */
        MA_ASSERT(MA_FALSE);

        (void)frameIndex;

        return MA_NOT_IMPLEMENTED;
    }
#endif
}

MA_API ma_result ma_qoa_get_data_format(ma_qoa *pQOA, ma_format *pFormat, ma_uint32 *pChannels, ma_uint32 *pSampleRate, ma_channel *pChannelMap, size_t channelMapCap)
{
    /* Defaults for safety. */
    if (pFormat != NULL)
    {
        *pFormat = ma_format_unknown;
    }
    if (pChannels != NULL)
    {
        *pChannels = 0;
    }
    if (pSampleRate != NULL)
    {
        *pSampleRate = 0;
    }
    if (pChannelMap != NULL)
    {
        MA_ZERO_MEMORY(pChannelMap, sizeof(*pChannelMap) * channelMapCap);
    }

    if (pQOA == NULL)
    {
        return MA_INVALID_OPERATION;
    }

    if (pFormat != NULL)
    {
        *pFormat = pQOA->format;
    }

#if !defined(MA_NO_QOA)
    {
        if (pChannels != NULL)
        {
            *pChannels = pQOA->info.channels;
        }

        if (pSampleRate != NULL)
        {
            *pSampleRate = pQOA->info.samplerate;
        }

        if (pChannelMap != NULL)
        {
            /*
            1. Mono
            2. L, R
            3. L, R, C
            4. FL, FR, B/SL, B/SR
            5. FL, FR, C, B/SL, B/SR
            6. FL, FR, C, LFE, B/SL, B/SR
            7. FL, FR, C, LFE, B, SL, SR
            8. FL, FR, C, LFE, BL, BR, SL, SR

            Probably right?
            ma_channel_map_init_standard_channel_flac
            */
            ma_channel_map_init_standard(ma_standard_channel_map_flac, pChannelMap, channelMapCap, pQOA->info.channels);
        }

        return MA_SUCCESS;
    }
#else
    {
        /* qoa is disabled. Should never hit this since initialization would have failed. */
        MA_ASSERT(MA_FALSE);
        return MA_NOT_IMPLEMENTED;
    }
#endif
}

MA_API ma_result ma_qoa_get_cursor_in_pcm_frames(ma_qoa *pQOA, ma_uint64 *pCursor)
{
    if (pCursor == NULL)
    {
        return MA_INVALID_ARGS;
    }

    *pCursor = 0; /* Safety. */

    if (pQOA == NULL)
    {
        return MA_INVALID_ARGS;
    }

#if !defined(MA_NO_QOA)
    {
        *pCursor = pQOA->sample_pos;

        return MA_SUCCESS;
    }
#else
    {
        /* qoa is disabled. Should never hit this since initialization would have failed. */
        MA_ASSERT(MA_FALSE);
        return MA_NOT_IMPLEMENTED;
    }
#endif
}

MA_API ma_result ma_qoa_get_length_in_pcm_frames(ma_qoa *pQOA, ma_uint64 *pLength)
{
    if (pLength == NULL)
    {
        return MA_INVALID_ARGS;
    }

    *pLength = 0; /* Safety. */

    if (pQOA == NULL)
    {
        return MA_INVALID_ARGS;
    }

#if !defined(MA_NO_QOA)
    {
        *pLength = (ma_uint64)pQOA->info.samples;

        return MA_SUCCESS;
    }
#else
    {
        /* qoa is disabled. Should never hit this since initialization would have failed. */
        MA_ASSERT(MA_FALSE);
        return MA_NOT_IMPLEMENTED;
    }
#endif
}

#endif
