#!/bin/bash
echo "=== /tmp/subforge ==="
ls -la /tmp/subforge/ 2>/dev/null
echo "---"
for d in /tmp/subforge/*/; do
  echo "DIR: $d"
  ls -la "$d" 2>/dev/null
  echo ""
done