#!/bin/bash
# Try to create a public tunnel - no installation needed
# Kill any existing tunnels
kill %1 2>/dev/null

# Try serveo.net first (free, no account)
echo "=== Trying serveo.net ==="
timeout 5 ssh -o StrictHostKeyChecking=no -o ServerAliveInterval=5 -R 80:localhost:8080 serveo.net 2>&1 &
sleep 3
echo "serveo started (check output above)"

# Get the public URL
PUBLIC_URL=$(curl -s http://localhost:4040/api/tunnels 2>/dev/null || echo "no_local_api")
echo "Public URL info: $PUBLIC_URL"