#ifndef NETWORK_MONITOR_H
#define NETWORK_MONITOR_H

#include "Packet.h"
#include "Queue.h"
#include "PacketAnalyzer.h"
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstring>

class NetworkMonitor {
private:
    int sock;
    std::string interface;
    
    Queue<Packet> packetQueue;           
    Queue<Packet> filteredQueue;         
    Queue<Packet> backupQueue;           
    
    PacketAnalyzer analyzer;
    std::atomic<bool> capturing;
    int oversizedThreshold;
    int oversizedCount;
    
public:
    NetworkMonitor(const std::string& iface) 
        : interface(iface), capturing(false), oversizedThreshold(5), oversizedCount(0) {
        
        sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (sock < 0) {
            perror("Socket creation failed. Run with sudo/root privileges");
            exit(1);
        }
        
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
        
        if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
            perror("Binding to interface failed");
            close(sock);
            exit(1);
        }
        
        std::cout << "✅ Network Monitor initialized on interface: " << interface << std::endl;
        std::cout << "✅ Raw socket created successfully\n";
    }
    
    ~NetworkMonitor() { 
        capturing = false;
        close(sock); 
    }

    void capturePacketsContinuous(int duration = 60) {
        unsigned char buffer[65536];
        int id = 1;
        capturing = true;
        
        std::cout << "\n🔍 CONTINUOUS PACKET CAPTURE for " << duration << " seconds\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        auto startTime = std::chrono::steady_clock::now();
        auto endTime = startTime + std::chrono::seconds(duration);
        
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        while (capturing && std::chrono::steady_clock::now() < endTime) {
            ssize_t size = recvfrom(sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
            
            if (size > 0) {
                Packet p(id++, buffer, size);
   
                analyzer.dissect(p);
                
                packetQueue.enqueue(p);
                
                if (id % 10 == 0) {
                    std::cout << "📦 Captured and dissected " << id - 1 << " packets...\r" << std::flush;
                }
            }
        }
        
        capturing = false;
        std::cout << "\n✅ Continuous capture complete. Total: " << (id - 1) << " packets\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    }

    void displayPackets() {
        if (packetQueue.isEmpty()) {
            std::cout << "\n⚠️  No packets captured yet.\n";
            return;
        }
        
        std::cout << "\n📋 CURRENT PACKET LIST:\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "ID\tSource IP\t\tDestination IP\t\tProtocol\tSize\tTimestamp\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        Queue<Packet> temp = packetQueue;
        int count = 0;
        int maxDisplay = 50;
        
        while (!temp.isEmpty() && count < maxDisplay) {
            Packet p = temp.front();
            temp.dequeue();
            
            std::cout << p.id << "\t"
                      << p.srcIP;

            if (p.srcIP.length() < 16) std::cout << "\t";
            std::cout << "\t" << p.dstIP;
            if (p.dstIP.length() < 16) std::cout << "\t";
            
            std::cout << "\t" << p.protocol << "\t\t"
                      << p.size << "\t"
                      << p.getTimestampStr() << std::endl;
            count++;
        }
        
        if (!temp.isEmpty()) {
            std::cout << "... and " << (packetQueue.size() - maxDisplay) << " more packets\n";
        }
        
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "Total packets in queue: " << packetQueue.size() << "\n";
    }

    void displayPacketDetails(int packetId) {
        if (packetQueue.isEmpty()) {
            std::cout << "\n⚠️  No packets in queue.\n";
            return;
        }
        
        Queue<Packet> temp = packetQueue;
        bool found = false;
        
        while (!temp.isEmpty()) {
            Packet p = temp.front();
            temp.dequeue();
            
            if (p.id == packetId) {
                analyzer.displayPacketLayers(p);
                found = true;
                break;
            }
        }
        
        if (!found) {
            std::cout << "\n⚠️  Packet with ID " << packetId << " not found.\n";
            std::cout << "Available packet IDs: ";

            Queue<Packet> temp2 = packetQueue;
            int showCount = 0;
            while (!temp2.isEmpty() && showCount < 10) {
                std::cout << temp2.front().id;
                temp2.dequeue();
                showCount++;
                if (!temp2.isEmpty() && showCount < 10) std::cout << ", ";
            }
            if (packetQueue.size() > 10) {
                std::cout << "... (total: " << packetQueue.size() << " packets)";
            }
            std::cout << "\n";
        }
    }

    void filterPackets(const std::string& src, const std::string& dst) {
        if (packetQueue.isEmpty()) {
            std::cout << "\n⚠️  No packets to filter. Please capture packets first.\n";
            return;
        }
        
        std::cout << "\n🔎 FILTERING PACKETS: " << src << " → " << dst << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        Queue<Packet> temp = packetQueue;
        int matchCount = 0;
        int skippedOversized = 0;
        int checkedCount = 0;
        oversizedCount = 0;
      
        filteredQueue.clear();
   
        while (!temp.isEmpty()) {
            Packet p = temp.front();
            temp.dequeue();
            checkedCount++;

            if (p.srcIP == src && p.dstIP == dst) {
                if (p.size > 1500) {
                    oversizedCount++;
                    if (oversizedCount > oversizedThreshold) {
                        skippedOversized++;
                        std::cout << "⚠️  Skipping oversized packet " << p.id 
                                  << " (Size: " << p.size << " bytes)\n";
                        continue;
                    }
                }

                filteredQueue.enqueue(p);
                matchCount++;
                std::cout << "✓ Matched packet " << p.id 
                          << " | Size: " << p.size 
                          << " | Protocol: " << p.protocol << "\n";
            }
        }
        
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "Checked " << checkedCount << " packets\n";
        std::cout << "✅ Filtered " << matchCount << " matching packets\n";
        if (skippedOversized > 0) {
            std::cout << "⚠️  Skipped " << skippedOversized << " oversized packets\n";
        }
        if (matchCount == 0) {
            std::cout << "\n💡 TIP: Use option 2 to see available IPs in captured packets\n";
        }
    }

    void displayFilteredPackets() {
        if (filteredQueue.isEmpty()) {
            std::cout << "\n⚠️  No filtered packets available.\n";
            return;
        }
        
        std::cout << "\n📋 FILTERED PACKETS WITH DELAY:\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "ID\tSource IP\t\tDest IP\t\t\tSize\tDelay(ms)\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        Queue<Packet> temp = filteredQueue;
        while (!temp.isEmpty()) {
            Packet p = temp.front();
            temp.dequeue();
 
            std::cout << p.id << "\t"
                      << p.srcIP << "\t"
                      << p.dstIP << "\t"
                      << p.size << "\t"
                      << p.getEstimatedDelay() << std::endl;
        }
        
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "Total filtered packets: " << filteredQueue.size() << "\n";
    }

    void replayPackets() {
        if (filteredQueue.isEmpty()) {
            std::cout << "\n⚠️  No filtered packets to replay.\n";
            return;
        }
        
        std::cout << "\n▶️  REPLAYING FILTERED PACKETS CONTINUOUSLY\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        int successCount = 0;
        int failureCount = 0;
        
        while (!filteredQueue.isEmpty()) {
            Packet p = filteredQueue.front();
            filteredQueue.dequeue();
            
            int delay = p.size / 1000;
            std::cout << "⏳ Packet " << p.id << ": Delay " << delay << "ms... ";
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));

            ssize_t sent = send(sock, p.data.data(), p.size, 0);
            
            if (sent < 0 || sent != static_cast<ssize_t>(p.size)) {

                std::cout << "❌ FAILED - " << strerror(errno) << "\n";

                backupQueue.enqueue(p);
                failureCount++;
            } else {
                std::cout << "✅ SUCCESS (" << sent << " bytes)\n";
                successCount++;
            }
        }
        
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "Replay Summary: ✅ Success: " << successCount 
                  << " | ❌ Failed: " << failureCount << "\n";
        
        if (failureCount > 0) {
            std::cout << "📦 " << failureCount << " packets moved to backup for retry\n";
        }
    }

    void retryBackupPackets() {
        if (backupQueue.isEmpty()) {
            std::cout << "\n✅ No packets in backup queue.\n";
            return;
        }
        
        std::cout << "\n🔄 RETRYING BACKUP PACKETS (Max 2 retries per packet)\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        
        Queue<Packet> tempBackup;
        int retrySuccess = 0;
        int retryFailed = 0;
        
        while (!backupQueue.isEmpty()) {
            Packet p = backupQueue.front();
            backupQueue.dequeue();

            if (!p.canRetry()) {
                std::cout << "❌ Packet " << p.id << " exceeded max retries (2). Discarding.\n";
                continue;
            }
            
            p.incrementRetry();
            std::cout << "🔄 Retry #" << p.retryCount << " for packet " << p.id;

            if (p.size > 1500) {
                std::cout << " - ⚠️  Skipped (oversized: " << p.size << " bytes)\n";
                continue;
            }
            
            std::cout << "... ";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            ssize_t sent = send(sock, p.data.data(), p.size, 0);
            
            if (sent < 0 || sent != static_cast<ssize_t>(p.size)) {
                std::cout << "❌ FAILED\n";
                tempBackup.enqueue(p);
                retryFailed++;
            } else {
                std::cout << "✅ SUCCESS\n";
                retrySuccess++;
            }
        }
        
        backupQueue = tempBackup;
        
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "Retry Results: ✅ " << retrySuccess << " | ❌ " << retryFailed << "\n";
        std::cout << "Remaining in backup: " << backupQueue.size() << "\n";
    }

    void clearProcessedPackets() {
        int count = packetQueue.size();
        packetQueue.clear();
        std::cout << "✅ Removed " << count << " processed packets from queue\n";
    }
    
    void displayStatistics() {
        std::cout << "\n📊 Network Monitor Statistics:\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "  Total Captured Packets: " << packetQueue.size() << "\n";
        std::cout << "  Filtered Packets (replay list): " << filteredQueue.size() << "\n";
        std::cout << "  Backup Queue (failed): " << backupQueue.size() << "\n";
        std::cout << "  Interface: " << interface << "\n";
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    }
};

#endif