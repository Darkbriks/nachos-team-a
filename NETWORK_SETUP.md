# NachOS TCP/IP Network Setup Guide

This guide explains how to enable NachOS machines to communicate across different physical computers on the same network.

## Overview

NachOS now supports TCP/IP networking in addition to the original Unix domain sockets. This allows NachOS instances running on different physical machines to communicate over a real network.

## Configuration

### 1. Network Mode Selection

In `code/machine/sysdep.cc`, line 106:

```c
#define USE_TCPIP_NETWORK 1  // 1 = TCP/IP, 0 = Unix sockets (local only)
```

- Set to `1` for TCP/IP networking (cross-machine communication)
- Set to `0` for Unix domain sockets (local machine only, original behavior)

### 2. Port Configuration

Base port for NachOS network (line 108):

```c
#define BASE_PORT 9000
```

Each machine uses port `BASE_PORT + machine_id`:
- Machine 0: port 9000
- Machine 1: port 9001
- Machine 2: port 9002
- etc.

### 3. Machine Address Mapping

Edit the `machineHosts` array in `code/machine/sysdep.cc` (lines 114-125) to map machine IDs to IP addresses:

```c
static const char* machineHosts[MAX_MACHINES] = {
    "192.168.1.100",  // Machine 0
    "192.168.1.101",  // Machine 1
    "192.168.1.102",  // Machine 2
    // ... etc
};
```

## Local Testing (Same Physical Machine)

Default configuration uses localhost for all machines:

```bash
# Terminal 1 - Machine 0
cd code/build
./nachos-step6 -m 0 -l 1.0 -R 1

# Terminal 2 - Machine 1
cd code/build
./nachos-step6 -m 1 -l 1.0 -R 0
```

## Cross-Machine Setup

### Example: Two Physical Computers

**Computer A (192.168.1.100):**
1. Edit `code/machine/sysdep.cc`:
   ```c
   static const char* machineHosts[MAX_MACHINES] = {
       "192.168.1.100",  // Machine 0 (this computer)
       "192.168.1.101",  // Machine 1 (other computer)
       // ... etc
   };
   ```

2. Rebuild:
   ```bash
   cd code/build
   make clean && make
   ```

3. Run as Machine 0:
   ```bash
   ./nachos-step6 -m 0 -l 1.0 -R 1
   ```

**Computer B (192.168.1.101):**
1. Edit `code/machine/sysdep.cc` with **same** IP addresses as Computer A
   ```c
   static const char* machineHosts[MAX_MACHINES] = {
       "192.168.1.100",  // Machine 0 (other computer)
       "192.168.1.101",  // Machine 1 (this computer)
       // ... etc
   };
   ```

2. Rebuild:
   ```bash
   cd code/build
   make clean && make
   ```

3. Run as Machine 1:
   ```bash
   ./nachos-step6 -m 1 -l 1.0 -R 0
   ```

## Firewall Configuration

Make sure UDP ports 9000-9009 (or your BASE_PORT range) are open:

### Linux (ufw):
```bash
sudo ufw allow 9000:9009/udp
```

### Linux (iptables):
```bash
sudo iptables -A INPUT -p udp --dport 9000:9009 -j ACCEPT
```

### Windows Firewall:
Create inbound rule for UDP ports 9000-9009

## Testing Cross-Machine Communication

### 1. Test Network Connectivity

From Computer A:
```bash
nc -u -z 192.168.1.101 9001
```

From Computer B:
```bash
nc -u -z 192.168.1.100 9000
```

### 2. Run Reliable Network Test

On both computers simultaneously:

**Computer A:**
```bash
./nachos-step6 -m 0 -l 1.0 -R 1
```

**Computer B:**
```bash
./nachos-step6 -m 1 -l 1.0 -R 0
```

You should see:
- "Got 'Hello reliable!' from X, box 1" messages
- "Message X ACKed!" confirmations
- "ReliableMailTest: Test completed!"

## Troubleshooting

### Connection Refused / No Response

1. Check firewall settings on both machines
2. Verify IP addresses in `machineHosts` array match actual IPs
3. Ensure both machines are on the same network
4. Check that ports aren't already in use: `netstat -an | grep 9000`

### Permission Denied

Ports below 1024 require root. Use ports >= 1024 (default 9000 is fine).

### Rebuild Required

After changing IP addresses in `sysdep.cc`, you **must** rebuild:
```bash
make clean && make
```

## Network Protocol

- **Transport**: UDP (unreliable, for simulating packet loss)
- **Reliability**: Implemented at application layer (ReliablePost)
- **Features**: ACK/timeout/retransmission, sequence numbers, event-driven

## Architecture

The TCP/IP implementation modifies these functions in `sysdep.cc`:

1. **OpenSocket()**: Creates UDP socket with SO_REUSEADDR
2. **AssignNameToSocket()**: Binds to `BASE_PORT + machineID`
3. **SendToSocket()**: Resolves machine ID to IP:port and sends
4. **ReadFromSocket()**: Receives from any source
5. **DeAssignNameToSocket()**: No-op for TCP/IP

All changes are conditional on `USE_TCPIP_NETWORK`, so you can switch back to Unix sockets by setting it to 0.

## Advanced Configuration

### Dynamic IP Configuration

For more flexibility, you can modify the code to read IP addresses from a configuration file:

1. Create `/etc/nachos/network.conf`:
   ```
   0 192.168.1.100
   1 192.168.1.101
   2 192.168.1.102
   ```

2. Modify `sysdep.cc` to read this file at startup

### Multiple Networks

To run multiple independent NachOS networks on the same physical network, use different `BASE_PORT` values for each network.

## Performance Notes

- TCP/IP adds minimal overhead compared to Unix sockets
- Network latency depends on physical network quality
- The `-l` parameter controls simulated packet loss rate (0.0 to 1.0)
- Event-driven architecture ensures efficient ACK processing

## Security Considerations

- No authentication or encryption
- All machines on the network can potentially communicate
- Intended for educational/lab environments only
- Do not use on untrusted networks
