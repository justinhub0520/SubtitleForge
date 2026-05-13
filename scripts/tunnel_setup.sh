#!/bin/bash
cd /tmp

# Download cloudflared
if [ ! -f cloudflared ]; then
  wget -q https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-amd64 -O cloudflared
  chmod +x cloudflared
fi

# Kill existing tunnels
killall cloudflared 2>/dev/null
sleep 1

# Start tunnel in background, capture URL
rm -f /tmp/cf_url.txt
./cloudflared tunnel --url http://localhost:8080 > /tmp/cf_log.txt 2>&1 &
CF_PID=$!
echo "cloudflared PID: $CF_PID"

# Wait for tunnel URL
for i in $(seq 1 15); do
  sleep 2
  URL=$(grep -o 'https://[^.]*\.trycloudflare\.com' /tmp/cf_log.txt | head -1)
  if [ -n "$URL" ]; then
    echo "TUNNEL_URL=$URL"
    echo "$URL" > /tmp/cf_url.txt
    break
  fi
done

cat /tmp/cf_log.txt