// socket.cpp
#include "socket.hpp"

Socket::Socket() {
    
}

Socket::Socket(SOCKET _currentSocket, SOCKADDR* _socketAddr, SOCKADDR_IN* _socketAddrIn, socklen_t* _socketSize = 0)
    :currentSocket(_currentSocket),socketAddr(_socketAddr),socketAddrIn(_socketAddrIn),socketSize(_socketSize)
{
    
}

Socket::~Socket() {

}