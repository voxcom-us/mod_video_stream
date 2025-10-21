#ifndef VIDEO_STREAMER_GLUE_H
#define VIDEO_STREAMER_GLUE_H
#include "mod_video_stream.h"

int validate_ws_uri(const char *url, char *wsUri);
switch_status_t is_valid_utf8(const char *str);
switch_status_t stream_session_send_text(switch_core_session_t *session, char *text);
switch_status_t stream_session_pauseresume(switch_core_session_t *session, int pause);
switch_status_t stream_session_init(switch_core_session_t *session, responseHandler_t responseHandler, uint32_t samples_per_second, char *wsUri, int wsSampling, int channels, char *metadata, void **ppUserData);
switch_status_t stream_session_write_thread_init(switch_core_session_t *session, void *pUserData);
switch_bool_t stream_frame(switch_media_bug_t *bug);
switch_status_t stream_session_cleanup(switch_core_session_t *session, char *text, int channelIsClosing);

#endif // VIDEO_STREAMER_GLUE_H
