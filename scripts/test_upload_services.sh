#!/bin/bash
echo "test audio data" > /tmp/test_upload.wav

echo "=== Test 0x0.st ==="
R=$(/usr/bin/curl -s -F "file=@/tmp/test_upload.wav" https://0x0.st --connect-timeout 10 --max-time 15 2>&1)
echo "Result: $R"

echo "=== Test file.io ==="
R=$(/usr/bin/curl -s -F "file=@/tmp/test_upload.wav" https://file.io --connect-timeout 10 --max-time 15 2>&1)
echo "Result: $R"

echo "=== Test transfer.sh ==="
R=$(/usr/bin/curl -s --upload-file /tmp/test_upload.wav https://transfer.sh/test.wav --connect-timeout 10 --max-time 15 2>&1)
echo "Result: $R"

echo "=== Test tmpfiles.org ==="
R=$(/usr/bin/curl -s -F "file=@/tmp/test_upload.wav" https://tmpfiles.org/api/v1/upload --connect-timeout 10 --max-time 15 2>&1)
echo "Result: $R"