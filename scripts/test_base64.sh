#!/bin/bash
cd /home/project01/SubForge
API_KEY=$(grep DASHSCOPE_API_KEY .env | cut -d= -f2)

echo "=== Encode audio ==="
B64=$(base64 -w 0 /tmp/subforge/*/audio.wav 2>/dev/null | head -c 100)
echo "B64 length: $(echo $B64 | wc -c)"

AUDIO=$(ls /tmp/subforge/*/audio.wav 2>/dev/null | head -1)
if [ -z "$AUDIO" ]; then
  echo "No audio file found, using test file"
  echo "testdata" > /tmp/test.wav
  AUDIO=/tmp/test.wav
fi
echo "Audio: $AUDIO ($(stat -c%s $AUDIO) bytes)"

echo "=== Base64 encode ==="
B64=$(base64 -w 0 "$AUDIO")
echo "B64 length: ${#B64}"

echo "=== Build body and submit ==="
BODY=$(python3 -c "
import json, sys, base64
with open('$AUDIO', 'rb') as f:
    data = base64.b64encode(f.read()).decode()
d = {
    'model': 'fun-asr',
    'input': {'file_content': data},
    'parameters': {'language_hints': ['zh']}
}
print(json.dumps(d))
")

echo "$BODY" > /tmp/body.json
echo "Body size: $(stat -c%s /tmp/body.json)"

echo "=== Submit ==="
/usr/bin/curl -s -w "\nHTTP:%{http_code}" \
  -H "Authorization: Bearer ${API_KEY}" \
  -H "Content-Type: application/json" \
  -H "X-DashScope-Async: enable" \
  -d @/tmp/body.json \
  --connect-timeout 60 --max-time 120 \
  https://dashscope.aliyuncs.com/api/v1/services/audio/asr/transcription