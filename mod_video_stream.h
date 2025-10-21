#ifndef MOD_VIDEO_STREAM_H
#define MOD_VIDEO_STREAM_H

#include <switch.h>
#include <speex/speex_resampler.h>

#define MY_BUG_NAME "video_stream"
#define MAX_SESSION_ID (256)
#define MAX_WS_URI (4096)
#define MAX_METADATA_LEN (8192)

#define EVENT_CONNECT "mod_video_stream::connect"
#define EVENT_DISCONNECT "mod_video_stream::disconnect"
#define EVENT_ERROR "mod_video_stream::error"
#define EVENT_JSON "mod_video_stream::json"
#define EVENT_PLAY "mod_video_stream::play"

typedef void (*responseHandler_t)(switch_core_session_t *session, const char *eventName, const char *json);

struct private_data
{
    switch_mutex_t *mutex;
    char sessionId[MAX_SESSION_ID];
    SpeexResamplerState *read_resampler;
    SpeexResamplerState *write_resampler;
    responseHandler_t responseHandler;
    void *pVideoStreamer;
    char ws_uri[MAX_WS_URI];
    int sampling;
    int wsSampling;
    int channels;
    int audio_paused : 1;
    int close_requested : 1;
    char initialMetadata[8192];
    switch_buffer_t *read_sbuffer;
    switch_buffer_t *write_sbuffer;
    switch_mutex_t *write_mutex;
    switch_thread_t *write_thread;
    int rtp_packets;
};

typedef struct private_data private_t;

enum notifyEvent_t
{
    CONNECT_SUCCESS,
    CONNECT_ERROR,
    CONNECTION_DROPPED,
    MESSAGE
};

#endif // MOD_VIDEO_STREAM_H
