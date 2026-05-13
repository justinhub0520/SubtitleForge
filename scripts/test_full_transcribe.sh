#!/bin/bash
cd /home/project01/SubForge
API_KEY=$(grep DASHSCOPE_API_KEY .env | cut -d= -f2)

# Step 1: upload file
echo "test audio data" > /tmp/test_audio.wav
RESP=$(/usr/bin/curl -s -w '\nHTTP_CODE:%{http_code}' \
  -H "Authorization: Bearer ${API_KEY}" \
  -F "file=@/tmp/test_audio.wav" \
  --connect-timeout 30 --max-time 60 \
  https://dashscope.aliyuncs.com/api/v1/files 2>&1)
echo "=== UPLOAD RESP ==="
echo "$RESP"

# Step 2: submit task with file_ids
FILE_ID=$(echo "$RESP" | head -1 | python3 -c "import sys,json; d=json.load(sys.stdin); print(d['data']['uploaded_files'][0]['file_id'])" 2>/dev/null)
echo "=== FILE_ID ==="
echo "$FILE_ID"

BODY="{\"model\":\"fun-asr\",\"input\":{\"file_ids\":[\"${FILE_ID}\"]},\"parameters\":{\"language_hints\":[\"zh\"]}}"
echo "=== BODY ==="
echo "$BODY"

RESP2=$(/usr/bin/curl -s -w '\nHTTP_CODE:%{http_code}' \
  -H "Authorization: Bearer ${API_KEY}" \
  -H "Content-Type: application/json" \
  -H "X-DashScope-Async: enable" \
  -d "$BODY" \
  --connect-timeout 30 --max-time 60 \
  https://dashscope.aliyuncs.com/api/v1/services/audio/asr/transcription 2>&1)
echo "=== SUBMIT RESP ==="
echo "$RESP2"