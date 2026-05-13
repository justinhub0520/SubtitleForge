#!/bin/bash
cd /home/project01/SubForge
API_KEY=$(grep DASHSCOPE_API_KEY .env | cut -d= -f2)

echo test > /tmp/t.wav
RESP=$(/usr/bin/curl -s \
  -H "Authorization: Bearer ${API_KEY}" \
  -F "file=@/tmp/t.wav" \
  https://dashscope.aliyuncs.com/api/v1/files)

FILE_ID=$(echo "$RESP" | grep -o '"file_id":"[^"]*"' | head -1 | cut -d'"' -f4)
echo "FILE_ID=$FILE_ID"

DOWNLOAD_URL="https://dashscope.aliyuncs.com/api/v1/files/${FILE_ID}/download"
echo "=== DOWNLOAD TEST ==="
echo "URL: $DOWNLOAD_URL"
/usr/bin/curl -s -o /dev/null -w "HTTP:%{http_code}" "$DOWNLOAD_URL" -H "Authorization: Bearer ${API_KEY}"
echo ""

echo "=== SUBMIT TASK ==="
BODY="{\"model\":\"fun-asr\",\"input\":{\"file_urls\":[\"${DOWNLOAD_URL}\"]},\"parameters\":{\"language_hints\":[\"zh\"]}}"
echo "Body: $BODY"
/usr/bin/curl -s -w "\nHTTP:%{http_code}" \
  -H "Authorization: Bearer ${API_KEY}" \
  -H "Content-Type: application/json" \
  -H "X-DashScope-Async: enable" \
  -d "$BODY" \
  https://dashscope.aliyuncs.com/api/v1/services/audio/asr/transcription