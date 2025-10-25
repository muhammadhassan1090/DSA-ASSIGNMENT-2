# DSA-ASSIGNMENT-2
---

## 📋 Quick Overview

This Network Packet Monitor captures, analyzes, filters, and replays network packets using custom Stack and Queue data structures. It operates on Linux using raw sockets and parses 5 protocol types: Ethernet, IPv4, IPv6, TCP, and UDP.

---

## ⚙️ System Requirements

### Mandatory Requirements
- **Operating System:** Linux (Ubuntu 20.04+, Fedora, Debian, or similar)
- **Privileges:** Root/sudo access (required for raw sockets)
- **Compiler:** g++ with C++11 support or higher
- **Network:** Active network interface (eth0, wlan0, ens33, etc.)

### Check Your System

```bash
# Check OS
uname -s                    # Should show: Linux

# Check compiler
g++ --version               # Should be 4.8 or higher

# Check network interfaces
ip link show                # Lists all network interfaces
# OR
ifconfig -a

# Check if you have sudo
sudo whoami                 # Should show: root
```

---

## 🚀 How to Run the Program

### Step 1: Download/Clone the Project

```bash
# If from GitHub
git clone [your-repository-url]
cd NetworkMonitor

# OR if you have the files
mkdir NetworkMonitor
cd NetworkMonitor
# Copy all .h files, main.cpp, and Makefile here
```

### Step 2: Compile the Program

```bash
# Using Makefile (recommended)
make

# OR compile manually
g++ -std=c++11 -pthread -o network_monitor main.cpp -Wall -Wextra
```

**Expected Output:**
```
g++ -std=c++11 -Wall -Wextra -pthread -O2 -c main.cpp -o main.o
g++ -std=c++11 -Wall -Wextra -pthread -O2 -o network_monitor main.o
================================================
✅ Build complete!
Run with: sudo ./network_monitor
================================================
```

### Step 3: Find Your Network Interface

```bash
# List all interfaces
ip link show

# Common interface names:
# eth0     - Wired Ethernet
# wlan0    - Wireless (WiFi)
# ens33    - VMware virtual interface
# enp0s3   - VirtualBox interface
# lo       - Loopback (don't use this)
```

**Example Output:**
```
1: lo: <LOOPBACK,UP,LOWER_UP> ...
2: wlan0: <BROADCAST,MULTICAST,UP,LOWER_UP> ...  ← Use this one
3: eth0: <NO-CARRIER,BROADCAST,MULTICAST,UP> ...
```

### Step 4: Run the Program (WITH SUDO!)

```bash
# MUST use sudo for raw socket access
sudo ./network_monitor
```

**⚠️ CRITICAL:** If you see "Permission denied" or "Socket creation failed", you forgot `sudo`!

### Step 5: Enter Network Interface

```
Enter network interface name (e.g., eth0, wlan0, ens33): wlan0
```

**Press Enter** - You should see:
```
✅ Network Monitor initialized on interface: wlan0
✅ Raw socket created successfully
```

---

## 🎮 Using the Program - Menu Options

You'll see this menu:

```
╔════════════════════════════════════════════╗
║     NETWORK PACKET MONITOR SYSTEM          ║
╠════════════════════════════════════════════╣
║  1. Capture Packets (Continuous)           ║
║  2. Display Captured Packets               ║
║  3. Display Packet Details (Layers)        ║
║  4. Filter Packets by IP                   ║
║  5. Display Filtered Packets               ║
║  6. Replay Filtered Packets                ║
║  7. Retry Backup Packets                   ║
║  8. Display Statistics                     ║
║  9. Clear Processed Packets                ║
║  10. RUN COMPLETE DEMO (ALL REQUIREMENTS)  ║
║  0. Exit                                   ║
╚════════════════════════════════════════════╝
```

---

## 📖 Step-by-Step Usage Guide

### Basic Workflow: Capture → Display → Filter → Replay

#### 1️⃣ Capture Packets

```
Enter your choice: 1
Enter capture duration in seconds (minimum 60 for full demo): 60
```

**What happens:**
- Program captures packets for 60 seconds
- Each packet is dissected automatically
- Progress shown every 10 packets
- All packets stored in queue

**Expected output:**
```
🔍 CONTINUOUS PACKET CAPTURE for 60 seconds
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📦 Captured and dissected 10 packets...
Packet 1 Dissection: Ethernet → IPv4 → TCP
Packet 2 Dissection: Ethernet → IPv4 → UDP
...
✅ Continuous capture complete. Total: 247 packets
```

#### 2️⃣ Display Captured Packets

```
Enter your choice: 2
```

**What you see:**
```
📋 CURRENT PACKET LIST:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
ID    Source IP        Destination IP      Protocol  Size  Timestamp
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
1     192.168.1.105    142.250.183.206    TCP       1200  1699123456.123456
2     192.168.1.105    8.8.8.8            UDP       548   1699123456.234567
...
Total packets in queue: 247
```

**💡 TIP:** Note down some IP addresses for filtering in step 4!

#### 3️⃣ Display Packet Details (Dissected Layers)

```
Enter your choice: 3
Enter Packet ID to view dissection: 1
```

**What you see:**
```
═══════════════════════════════════════════
  Packet ID: 1
  Timestamp: 1699123456.123456
  Size: 1200 bytes
  Source IP: 192.168.1.105
  Destination IP: 142.250.183.206
  Protocol: TCP
  Estimated Delay: 1 ms
═══════════════════════════════════════════
```

#### 4️⃣ Filter Packets by IP

```
Enter your choice: 4
Enter Source IP: 192.168.1.105
Enter Destination IP: 142.250.183.206
```

**What happens:**
- Program searches all captured packets
- Matches source AND destination IPs
- Skips oversized packets (>1500 bytes) if threshold exceeded

**Expected output:**
```
🔎 FILTERING PACKETS: 192.168.1.105 → 142.250.183.206
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✓ Matched packet 1 | Size: 1200 | Protocol: TCP
✓ Matched packet 15 | Size: 548 | Protocol: UDP
...
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Checked 247 packets
✅ Filtered 43 matching packets
```

#### 5️⃣ Display Filtered Packets (with Delay)

```
Enter your choice: 5
```

**What you see:**
```
📋 FILTERED PACKETS WITH DELAY:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
ID    Source IP        Dest IP             Size  Delay(ms)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
1     192.168.1.105    142.250.183.206    1200  1
15    192.168.1.105    142.250.183.206    548   0
...
Total filtered packets: 43
```

**Note:** Delay = Packet Size / 1000 milliseconds

#### 6️⃣ Replay Filtered Packets

```
Enter your choice: 6
```

**What happens:**
- Each packet is replayed with calculated delay
- Success/failure reported for each packet
- Failed packets moved to backup queue

**Expected output:**
```
▶️  REPLAYING FILTERED PACKETS CONTINUOUSLY
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
⏳ Packet 1: Delay 1ms... ✅ SUCCESS (1200 bytes)
⏳ Packet 15: Delay 0ms... ❌ FAILED - Operation not permitted
...
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Replay Summary: ✅ Success: 38 | ❌ Failed: 5
📦 5 packets moved to backup queue for retry
```

**⚠️ Note:** Replay often fails due to network permissions - this is normal!

#### 7️⃣ Retry Failed Packets (Up to 2 Times)

```
Enter your choice: 7
```

**What happens:**
- Attempts to resend failed packets
- Maximum 2 retry attempts per packet
- Skips oversized packets (>1500 bytes)

**Expected output:**
```
🔄 RETRYING BACKUP PACKETS (Max 2 retries per packet)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🔄 Retry #1 for packet 15... ✅ SUCCESS
🔄 Retry #1 for packet 25... ❌ FAILED
🔄 Retry #2 for packet 25... ✅ SUCCESS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Retry Results: ✅ 4 | ❌ 1
Remaining in backup: 1
```

#### 8️⃣ Display Statistics

```
Enter your choice: 8
```

**What you see:**
```
📊 Network Monitor Statistics:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  Total Captured Packets: 247
  Filtered Packets (replay list): 43
  Backup Queue (failed): 1
  Interface: wlan0
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

#### 9️⃣ Clear Processed Packets

```
Enter your choice: 9
```

**What happens:**
- Removes all packets from main queue
- Useful for starting fresh capture

**Expected output:**
```
✅ Removed 247 processed packets from queue
```

#### 🔟 Run Complete Demo (ALL Requirements)

```
Enter your choice: 10
```

**What happens:**
This option runs ALL test cases automatically:
1. Captures for 60 seconds
2. Displays packet list
3. Shows dissected layers (you enter packet ID)
4. Filters by IP (you enter source/dest IPs)
5. Displays filtered packets with delays
6. Replays packets
7. Retries failed packets
8. Shows final statistics

**Perfect for demonstrating all requirements at once!**

---

## 🎯 Quick Start - Complete Example Session

```bash
# 1. Compile
make

# 2. Run with sudo
sudo ./network_monitor

# 3. Enter interface
Enter network interface name: wlan0

# 4. Capture packets
Enter your choice: 1
Enter capture duration in seconds: 60
[Wait 60 seconds while browsing web to generate traffic]

# 5. View captured packets
Enter your choice: 2
[Note some IP addresses]

# 6. Filter packets
Enter your choice: 4
Enter Source IP: 192.168.1.105
Enter Destination IP: 142.250.183.206

# 7. View filtered packets
Enter your choice: 5

# 8. Replay packets
Enter your choice: 6

# 9. Retry failed
Enter your choice: 7

# 10. Exit
Enter your choice: 0
```
---


## 🎓 Learning Objectives Demonstrated

When you run this program, you'll see:

1. **Stack (LIFO)** in action - Protocol layers parsed bottom-up, displayed top-down
2. **Queue (FIFO)** in action - Packets processed in order of arrival
3. **Custom data structures** - No STL, all manual implementation
4. **Network programming** - Raw sockets, packet capture
5. **Protocol parsing** - Ethernet, IPv4, IPv6, TCP, UDP
6. **Error handling** - Retry mechanism with backup queue
7. **Algorithm design** - Filtering, replay, oversized handling

---
