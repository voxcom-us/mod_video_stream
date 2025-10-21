# mod_video_stream

A FreeSWITCH module that streams L16 audio from a channel to a websocket endpoint. If websocket sends back responses (eg. JSON) it can be effectively used with ASR engines such as IBM Watson etc., or any other purpose you find applicable.

***This module supports bi-directional audio streaming (see python example below).***

## About

- The purpose of `mod_video_stream` was to provide a simple, low-dependency yet effective module for streaming audio and receiving responses from a websocket server.

## Installation

### Dependencies

It requires `libfreeswitch-dev`, `libssl-dev`, `zlib1g-dev`, `libevent-dev` and `libspeexdsp-dev` on Debian/Ubuntu which are regular packages for Freeswitch installation.

### Building

After cloning please execute: **git submodule init** and **git submodule update** to initialize the submodule.

#### Custom path

If you built FreeSWITCH from source, eq. install dir is /usr/local/freeswitch, add path to pkgconfig:

```shell
export PKG_CONFIG_PATH=/usr/local/freeswitch/lib/pkgconfig
```

To build the module, from the cloned repository:

```shell
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

**TLS** is `OFF` by default. To build with TLS support add `-DUSE_TLS=ON` to cmake line.

#### DEB Package

To build DEB package after making the module:

```shell
cpack -G DEB
```

Debian package will be placed in root directory `_packages` folder.

## Scripted Build & Installation

```shell
sudo apt-get -y install git \
    && cd /usr/src/ \
    && git clone https://github.com/amigniter/mod_video_stream.git \
    && cd mod_video_stream \
    && sudo bash ./build-mod-video-stream.sh
```

### Channel variables

The following channel variables can be used to fine tune websocket connection and also configure mod_video_stream logging:

| Variable                               | Description                                             | Default |
| -------------------------------------- | ------------------------------------------------------- | ------- |
| STREAM_MESSAGE_DEFLATE                 | true or 1, disables per message deflate                 | off     |
| STREAM_HEART_BEAT                      | number of seconds, interval to send the heart beat      | off     |
| STREAM_SUPPRESS_LOG                    | true or 1, suppresses printing to log                   | off     |
| STREAM_BUFFER_SIZE                     | buffer duration in milliseconds, divisible by 20        | 20      |
| STREAM_EXTRA_HEADERS                   | JSON object for additional headers in string format     | none    |
| ~~STREAM_NO_RECONNECT~~                    | true or 1, disables automatic websocket reconnection    | off     |
| STREAM_TLS_CA_FILE                     | CA cert or bundle, or the special values SYSTEM or NONE | SYSTEM  |
| STREAM_TLS_KEY_FILE                    | optional client key for WSS connections                 | none    |
| STREAM_TLS_CERT_FILE                   | optional client cert for WSS connections                | none    |
| STREAM_TLS_DISABLE_HOSTNAME_VALIDATION | true or 1 disable hostname check in WSS connections     | false   |

- Per message deflate compression option is enabled by default. It can lead to a very nice bandwidth savings. To disable it set the channel var to `true|1`.
- Heart beat, sent every xx seconds when there is no traffic to make sure that load balancers do not kill an idle connection.
- Suppress parameter is omitted by default(false). All the responses from websocket server will be printed to the log. Not to flood the log you can suppress it by setting the value to `true|1`. Events are fired still, it only affects printing to the log.
- `Buffer Size` actually represents a duration of audio chunk sent to websocket. If you want to send e.g. 100ms audio packets to your ws endpoint
you would set this variable to 100. If ommited, default packet size of 20ms will be sent as grabbed from the audio channel (which is default FreeSWITCH frame size)
- Extra headers should be a JSON object with key-value pairs representing additional HTTP headers. Each key should be a header name, and its corresponding value should be a string.

  ```json
  {
      "Header1": "Value1",
      "Header2": "Value2",
      "Header3": "Value3"
  }
- ~~Websocket automatic reconnection is on by default. To disable it set this channel variable to true or 1.~~

  - libwsc does not support automatic reconnection.
- TLS (for WSS) options can be fine tuned with the `STREAM_TLS_*` channel variables:
  - `STREAM_TLS_CA_FILE` the ca certificate (or certificate bundle) file. By default is `SYSTEM` which means use the system defaults.
Can be `NONE` which result in no peer verification.
  - `STREAM_TLS_CERT_FILE` optional client tls certificate file sent to the server.
  - `STREAM_TLS_KEY_FILE` optional client tls key file for the given certificate.
  - `STREAM_TLS_DISABLE_HOSTNAME_VALIDATION` if `true`, disables the check of the hostname against the peer server certificate.
Defaults to `false`, which enforces hostname match with the peer certificate.

## API

### Commands

The freeswitch module exposes the following API commands:

```shell
uuid_video_stream <uuid> start <wss-url> <mix-type> <sampling-rate> <metadata>
```

Attaches a media bug and starts streaming audio (in L16 format) to the websocket server. FS default is 8k. If sampling-rate is other than 8k it will be resampled.

- `uuid` - Freeswitch channel unique id
- `wss-url` - websocket url `ws://` or `wss://`
- `mix-type` - choice of
  - "mono" - single channel containing caller's audio
  - "mixed" - single channel containing both caller and callee audio
  - "stereo" - two channels with caller audio in one and callee audio in the other.
- `sampling-rate` - choice of
  - "8k" = 8000 Hz sample rate will be generated
  - "16k" = 16000 Hz sample rate will be generated
- `metadata` - (optional) a valid `utf-8` text to send. It will be sent the first before audio streaming starts.

```shell
uuid_video_stream <uuid> send_text <metadata>
```

Sends a text to the websocket server. Requires a valid `utf-8` text.

```shell
uuid_video_stream <uuid> stop <metadata>
```

Stops audio stream and closes websocket connection. If _metadata_ is provided it will be sent before the connection is closed.

```shell
uuid_video_stream <uuid> pause
```

Pauses audio stream

```shell
uuid_video_stream <uuid> resume
```

Resumes audio stream

## Events

Module will generate the following event types:

- `mod_video_stream::json`
- `mod_video_stream::connect`
- `mod_video_stream::disconnect`
- `mod_video_stream::error`
- `mod_video_stream::play`

### response

Message received from websocket endpoint. Json expected, but it contains whatever the websocket server's response is.

#### Freeswitch event generated

**Name**: mod_video_stream::json
**Body**: WebSocket server response

### connect

Successfully connected to websocket server.

#### Freeswitch event generated

**Name**: mod_video_stream::connect
**Body**: JSON

```json
{
 "status": "connected"
}
```

### disconnect

Disconnected from websocket server.

#### Freeswitch event generated

**Name**: mod_video_stream::disconnect
**Body**: JSON

```json
{
 "status": "disconnected",
 "message": {
  "code": 1000,
  "reason": "Normal closure"
 }
}
```

- code: `<int>`
- reason: `<string>`

### error

There is an error with the connection. Multiple fields will be available on the event to describe the error.

#### Freeswitch event generated

**Name**: mod_video_stream::error
**Body**: JSON

```json
{
 "status": "error",
 "message": {
  "code": 1,
  "error": "String explaining the error"
 }
}
```

- code: `<int>`
- error: `<string>`

#### Possible `code` values

| Code | Enum Name             | Meaning                                              |
|:----:|:----------------------|:-----------------------------------------------------|
| 1    | `IO`                  | I/O error when reading/writing sockets               |
| 2    | `INVALID_HEADER`      | Server sent a malformed WebSocket header             |
| 3    | `SERVER_MASKED`       | Server frames were masked (not allowed by spec)      |
| 4    | `NOT_SUPPORTED`       | Requested feature (e.g. extension) not supported     |
| 5    | `PING_TIMEOUT`        | No PONG received within timeout                      |
| 6    | `CONNECT_FAILED`      | TCP connection or DNS lookup failed                  |
| 7    | `TLS_INIT_FAILED`     | Couldn't initialize SSL/TLS context                  |
| 8    | `SSL_HANDSHAKE_FAILED`| SSL/TLS handshake with server failed                 |
| 9    | `SSL_ERROR`           | Generic OpenSSL error (certificate, cipher, etc.)    |

### play

**Name**: mod_video_stream::play
**Body**: JSON

Websocket server may return JSON object containing base64 encoded audio to be played by the user. To use this feature, response must follow the format:

```json
{
  "type": "streamAudio",
  "data": {
    "audioDataType": "raw",
    "sampleRate": 8000,
    "audioData": "base64 encoded audio"
  }
}
```

- audioDataType: `<raw|wav|mp3|ogg>`

Event generated by the module (subclass: _mod_video_stream::play_) will be the same as the `data` element with the **file** added to it representing filePath:

```json
{
  "audioDataType": "raw",
  "sampleRate": 8000,
  "file": "/path/to/the/file"
}
```

If printing to the log is not suppressed, `response` printed to the console will look the same as the event. The original response containing base64 encoded audio is replaced because it can be quite huge.

All the files generated by this feature will reside at the temp directory and will be deleted when the session is closed.

## Example (python)

This example will echo back media.

```python
import asyncio
import base64
import json
from datetime import datetime
from pathlib import Path
from typing import List

import numpy as np
from fastapi import FastAPI, WebSocket
from loguru import logger
from scipy.io.wavfile import write
from starlette.websockets import WebSocketDisconnect

SR = 16000  # sample rate (Hz)

app = FastAPI()


class API:
    def __init__(self, out_dir: Path = Path("./recordings")):
        self.out_dir = out_dir
        self.out_dir.mkdir(parents=True, exist_ok=True)

    async def forward(self, path: str, ingress: WebSocket):
        """
        Receive PCM16LE frames from client, stream the same audio bytes back
        to the client (loopback), and persist a single WAV file on disconnect.
        """
        chunks: List[np.ndarray] = []

        def _make_filename() -> Path:
            safe = "".join(c if c.isalnum() or c in ("-", "_") else "_" for c in (path or "session"))
            ts = datetime.now().strftime("%Y%m%d_%H%M%S")
            return self.out_dir / f"{safe}_{ts}.wav"

        try:
            while True:
                samples_bytes = await ingress.receive_bytes()
                b64 = base64.b64encode(samples_bytes).decode("ascii")
                payload = {
                    "type": "streamAudio",
                    "data": {
                        "audioDataType": "raw",
                        "sampleRate": SR,
                        "audioData": b64,
                    },
                }
                await ingress.send_text(json.dumps(payload, separators=(",", ":")))
                samples_int16 = np.frombuffer(samples_bytes, dtype=np.int16)
                chunks.append(samples_int16.copy())
        except WebSocketDisconnect as e:
            logger.info(f"websocket disconnected: code={e.code}, reason={getattr(e, 'reason', '')}")
        except Exception as e:
            logger.warning(f"websocket error: {e}")
        finally:
            if chunks:
                audio = np.concatenate(chunks)
                outfile = _make_filename()
                try:
                    write(str(outfile), SR, audio)  # writes int16 WAV
                    logger.info(f"saved recording: {outfile} (samples={audio.size}, duration={audio.size / SR:.2f}s)")
                except Exception as e:
                    logger.error(f"failed to write WAV file: {e}")
            else:
                logger.info("no audio chunks received; nothing to save.")


api = API()


@app.websocket("/live/{path}")
async def live_wss(path: str, ingress: WebSocket):
    logger.debug(f"connecting: path={path}")
    await ingress.accept()
    ws_ingress = asyncio.create_task(api.forward(path=path, ingress=ingress))
    await asyncio.gather(ws_ingress)


if __name__ == "__main__":
    import uvicorn
    uvicorn.run("live:app", host="0.0.0.0", port=8080, reload=False)
```

python environment setup

```shell
# setup environment with uv
uv venv --seed -p 3.12 ./.venv
source .venv/bin/activate

uv pip install loguru scipy fastapi 'uvicorn[standard]'
uv run python echo.py
```

lua dialplan.lua

```lua
api = freeswitch.API()

local channel_id = session:get_uuid()

session:execute("set", "playback_delimiter=!")
session:execute("answer")
local caller_id_number = session:getVariable("caller_id_number")
session:execute("set", "result=${uuid_video_stream ${uuid} start ws://localhost:8080/live/${uuid} mono 16000}")
freeswitch.consoleLog("INFO", "call answered with channel id [" .. channel_id .. "]")
session:execute("park")

```

freeswitch xml dialplan

```xml
<section name="dialplan" description="Regex/XML Dialplan">
  <context name="internal">
    <extension name="default">
      <condition field="destination_number" expression="^(.*)$">
        <action application="lua" data="/etc/freeswitch/scripts/dialplan.lua" />
      </condition>
    </extension>
  </context>
</section>
```
