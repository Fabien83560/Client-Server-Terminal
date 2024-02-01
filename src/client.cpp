// client.cpp
#include "client.hpp"
#include <iostream>

Client::Client() {

}

Client::Client(SOCKET _currentSocket, SOCKADDR* _socketAddr, SOCKADDR_IN* _socketAddrIn, socklen_t* _socketSize = 0)
    :Socket(_currentSocket,_socketAddr,_socketAddrIn,_socketSize)
{
    setSOCKET(socket(AF_INET, SOCK_STREAM, 0));
    getSOCKADDR_IN()->sin_family = AF_INET;
    getSOCKADDR_IN()->sin_addr.s_addr = inet_addr("127.0.0.1");
    getSOCKADDR_IN()->sin_port = htons(PORT);
}

Client::~Client() {

}

void Client::setSocket(Socket& newSocket) {
    setSOCKET(newSocket.getSOCKET());
}

bool Client::operator==(Client& other){
    return (getSOCKET() == other.getSOCKET());
}