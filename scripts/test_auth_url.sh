#!/bin/bash
cd /home/project01/SubForge
API_KEY=$(grep DASHSCOPE_API_KEY .env | cut -d= -f2)

echo test > /tmp/t.wav
RESP=$(/usr/bin/curl -s -H "Authorization: Bearer ${API_KEY}" -F "file=@/tmp/t.wav" https://dashscope.aliyuncs.com/api/v1/files)
FILE_ID=$(echo "$RESP" | grep -o '"file_id":"[^"]*"' | head -1 | cut -d'"' -f4)
echo "FILE_ID=$FILE_ID"

echo "=== TEST: api_key query param ==="
DL_URL="https://dashscope.aliyuncs.com/api/v1/files/${FILE_ID}/download?api_key=${API_KEY}"
HTTP=$(/usr/bin/curl -s -o /dev/null -w "%{http_code}" "$DL_URL")
echo "HTTP: $HTTP"
echo "URL: $DL_URL" | head -c 80
echo ""

echo "=== TEST: Bearer in URL param ==="
DL_URL2="https://dashscope.aliyuncs.com/api/v1/files/${FILE_ID}/download?token=${API_KEY}"
HTTP2=$(/usr/bin/curl -s -o /dev/null -w "%{http_code}" "$DL_URL2")
echo "HTTP: $HTTP2"

echo "=== TEST: submit with api_key in URL ==="
BODY="{\"model\":\"fun-asr\",\"input\":{\"file_urls\":[\"https://dashscope.aliyuncs.com/api/v1/files/${FILE_ID}/download?api_key=${API_KEY}\"]},\"parameters\":{\"language_hints\":[\"zh\"]}}"
/usr/bin/curl -s -w "\nHTTP:%{http_code}" -H "Authorization: Bearer ${API_KEY}" -H "Content-Type: application/json" -H "X-DashScope-Async: enable" -d "$BODY" https://dashscope.aliyuncs.com/api/v1/services/audio/asr/transcription