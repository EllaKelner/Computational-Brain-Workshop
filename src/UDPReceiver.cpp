#include "UDPReceiver.h"
#include <cstdio>

// Link with Windows Socket library
#pragma comment(lib, "ws2_32.lib")

// Constructor: Initializes the UDPReceiver
UDPReceiver::UDPReceiver()
    : socket_(INVALID_SOCKET), initialized_(false), listening_(false)
{
    WSADATA wsaData;
    // Start Winsock 2.2
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        // Print error if WSAStartup fails
        printf("WSAStartup failed: %d\n", result);
        return;
    }
    // Mark as initialized if successful
    initialized_ = true;
}

// Destructor: Cleanup resources and stop listening
UDPReceiver::~UDPReceiver() {
    // Stop any ongoing listening operations
    StopListening();

    // Close the socket if valid
    if (socket_ != INVALID_SOCKET) {
        closesocket(socket_);
    }

    // Cleanup Winsock if initialized
    if (initialized_) {
        WSACleanup();
    }
}

// Initialize the socket and bind it to the specified port
bool UDPReceiver::Init(unsigned short port) {
    // Do not proceed if not initialized
    if (!initialized_) {
        return false;
    }

    // Create UDP socket
    socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_ == INVALID_SOCKET) {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        return false;
    }

    // Setup server address properties
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_port = htons(port);
    serverAddr_.sin_addr.S_un.S_addr = INADDR_ANY;

    // Bind the socket to the server address
    if (bind(socket_, (SOCKADDR*)&serverAddr_, sizeof(serverAddr_)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(socket_);
        return false;
    }

    return true;
}

// Receive data from the socket
std::string UDPReceiver::Receive() {
    if (socket_ == INVALID_SOCKET) {
        return "";
    }

    const int BUFFER_SIZE = 512;
    char recvBuffer[BUFFER_SIZE];
    int bytesReceived = recvfrom(socket_, recvBuffer, BUFFER_SIZE, 0, nullptr, nullptr);
    if (bytesReceived == SOCKET_ERROR) {
        printf("Recvfrom failed: %d\n", WSAGetLastError());
        return "";
    }

    // Return the received data as a string
    return std::string(recvBuffer, bytesReceived);
}

// Start listening for incoming data in a separate thread
void UDPReceiver::StartListening() {
    listening_ = true;
    listenThread_ = std::thread([this] { ListenForData(); });
}

// Stop listening and join the listening thread
void UDPReceiver::StopListening() {
    listening_ = false;
    if (listenThread_.joinable()) {
        listenThread_.join();
    }
}

// Continuously listen for data and update the latest received packet
void UDPReceiver::ListenForData() {
    while (listening_) {
        std::string data = Receive();
        if (!data.empty()) {
            std::lock_guard<std::mutex> lock(packetMutex_);
            lastReceivedUDPPacket_ = data;
        }
    }
}

// Get the most recent packet received
std::string UDPReceiver::GetLatestPacket() {
    std::lock_guard<std::mutex> lock(packetMutex_);
    return lastReceivedUDPPacket_;
}