#!/bin/bash
# Test NachOS TCP/IP network connectivity

echo "=========================================="
echo "NachOS Network Connectivity Test"
echo "=========================================="
echo ""

# Check if netcat is available
if ! command -v nc &> /dev/null; then
    echo "WARNING: netcat (nc) not found. Installing..."
    sudo apt-get install -y netcat 2>/dev/null || echo "Please install netcat manually"
    echo ""
fi

# Get configuration from sysdep.cc
SYSDEP_FILE="code/machine/sysdep.cc"

if [ ! -f "$SYSDEP_FILE" ]; then
    echo "ERROR: Cannot find $SYSDEP_FILE"
    exit 1
fi

echo "Current machine IP configuration:"
echo ""
grep -A 10 "static const char\* machineHosts" $SYSDEP_FILE | grep "Machine" | head -10
echo ""
echo "=========================================="
echo ""

# Extract IP addresses
declare -a ips
for i in {0..9}; do
    ips[$i]=$(grep "// Machine $i" $SYSDEP_FILE | grep -oP '"[^"]*"' | tr -d '"')
done

# Get local IP
LOCAL_IP=$(hostname -I | awk '{print $1}')
echo "Your local IP address: $LOCAL_IP"
echo ""

# Ask which machines to test
echo "Which machines do you want to test connectivity with?"
read -p "Enter machine IDs (space-separated, e.g., '0 1 2'): " machines

echo ""
echo "Testing connectivity..."
echo ""

for machine in $machines; do
    if [ $machine -lt 0 ] || [ $machine -ge 10 ]; then
        echo "ERROR: Invalid machine ID: $machine"
        continue
    fi

    ip=${ips[$machine]}
    port=$((9000 + machine))

    echo "Testing Machine $machine ($ip:$port)..."

    # Test if port is reachable
    if timeout 2 bash -c "echo > /dev/udp/$ip/$port" 2>/dev/null; then
        echo "  ✓ UDP port $port is reachable on $ip"
    else
        echo "  ✗ Cannot reach UDP port $port on $ip"
        echo "    Possible issues:"
        echo "    - Machine $machine not running NachOS"
        echo "    - Firewall blocking port $port"
        echo "    - IP address incorrect"
    fi

    # Check if it's localhost
    if [ "$ip" = "127.0.0.1" ] || [ "$ip" = "localhost" ]; then
        echo "  ℹ This is localhost - ensure both machines run on same computer"
    elif [ "$ip" = "$LOCAL_IP" ]; then
        echo "  ℹ This is your local IP - this machine"
    fi

    echo ""
done

echo "=========================================="
echo ""
echo "Firewall check:"
echo ""
echo "Make sure these UDP ports are open: 9000-9009"
echo ""
echo "Linux (ufw):     sudo ufw allow 9000:9009/udp"
echo "Linux (iptables): sudo iptables -A INPUT -p udp --dport 9000:9009 -j ACCEPT"
echo ""
echo "Check listening ports: netstat -an | grep 900"
echo ""
