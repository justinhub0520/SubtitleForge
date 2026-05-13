#!/bin/bash
cd /home/project01/SubForge
API_KEY=$(grep DASHSCOPE_API_KEY .env | cut -d= -f2)

AUDIO=$(ls /tmp/subforge/*/audio.wav 2>/dev/null | head -1)
if [ -z "$AUDIO" ]; then
  dd if=/dev/urandom of=/tmp/test_audio.wav bs=1024 count=50 2>/dev/null
  AUDIO=/tmp/test_audio.wav
fi
echo "Audio: $AUDIO ($(stat -c%s $AUDIO) bytes)"

echo "=== Test OpenAI-compatible transcription ==="
/usr/bin/curl -s -w "\nHTTP:%{http_code}" \
  -H "Authorization: Bearer ${API_KEY}" \
  -F "file=@${AUDIO};type=audio/wav" \
  -F "model=paraformer-v1" \
  --connect-timeout 30 --max-time 120 \
  https://dashscope.aliyuncs.com/compatible-mode/v1/audio/transcriptions