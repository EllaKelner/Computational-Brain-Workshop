#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <mutex>

class UDPReceiver {
public:
    UDPReceiver();
    ~UDPReceiver();

    bool Init(unsigned short port);
    std::string Receive();

    void StartListening();
    void StopListening();
    std::string GetLatestPacket();

//private:
    SOCKET socket_;
    sockaddr_in serverAddr_;
    bool initialized_;
    bool listening_;
    std::thread listenThread_;
    std::mutex packetMutex_;
    std::string lastReceivedUDPPacket_;

    void ListenForData();
};

#endif // UDP_RECEIVER_H