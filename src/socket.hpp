// socket.hpp
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define PORT 5656

class Socket {
    private:
        SOCKET currentSocket;
        SOCKADDR* socketAddr;
        SOCKADDR_IN* socketAddrIn;
        socklen_t* socketSize;

    public:
        Socket();
        Socket(SOCKET _currentSocket, SOCKADDR* , SOCKADDR_IN* , socklen_t*);
        ~Socket();

        inline SOCKET& getSOCKET() {return currentSocket;}
        inline SOCKADDR* getSOCKADDR() {return socketAddr;}
        inline SOCKADDR_IN* getSOCKADDR_IN() {return socketAddrIn;}
        inline socklen_t* getSocklen_t() {return socketSize;}

        inline void setSOCKET(SOCKET s) {currentSocket = s;}
        inline void setSOCKADDR(SOCKADDR* s) {socketAddr = s;}
        inline void setSOCKADDR_IN(SOCKADDR_IN* s) {socketAddrIn = s;}
        inline void setSocklen_t(socklen_t* s) {socketSize = s;}
};

#endif // SOCKET_HPP
