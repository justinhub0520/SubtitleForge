#!/bin/bash
set -e
cd /home/project01/SubForge
API_KEY=$(grep DASHSCOPE_API_KEY .env | cut -d= -f2)

AUDIO=$(ls /tmp/subforge/*/audio.wav 2>/dev/null | head -1)
if [ -z "$AUDIO" ]; then
  echo "No cached audio, creating test file"
  dd if=/dev/urandom of=/tmp/test_audio.wav bs=1024 count=100 2>/dev/null
  AUDIO=/tmp/test_audio.wav
fi
echo "Audio: $AUDIO ($(stat -c%s $AUDIO) bytes)"

# Step 1: Upload
echo "=== Step 1: Upload to DashScope Files ==="
RESP=$(/usr/bin/curl -s -H "Authorization: Bearer ${API_KEY}" -F "file=@${AUDIO}" https://dashscope.aliyuncs.com/api/v1/files)
echo "$RESP"
FILE_ID=$(echo "$RESP" | grep -o '"file_id":"[^"]*"' | head -1 | cut -d'"' -f4)
echo "FILE_ID=$FILE_ID"

# Step 2: Resolve OSS URL
echo "=== Step 2: Get OSS URL ==="
RESP=$(/usr/bin/curl -s -H "Authorization: Bearer ${API_KEY}" "https://dashscope.aliyuncs.com/api/v1/files/${FILE_ID}")
echo "$RESP"
FILE_URL=$(echo "$RESP" | grep -o '"url":"[^"]*"' | head -1 | cut -d'"' -f4)
echo "FILE_URL=$FILE_URL" | head -c 100

# Step 3: Submit transcription
echo "=== Step 3: Submit transcription ==="
BODY="{\"model\":\"fun-asr\",\"input\":{\"file_urls\":[\"${FILE_URL}\"]},\"parameters\":{\"language_hints\":[\"zh\"]}}"
/usr/bin/curl -s -w "\nHTTP:%{http_code}" \
  -H "Authorization: Bearer ${API_KEY}" \
  -H "Content-Type: application/json" \
  -H "X-DashScope-Async: enable" \
  -d "$BODY" \
  https://dashscope.aliyuncs.com/api/v1/services/audio/asr/transcription