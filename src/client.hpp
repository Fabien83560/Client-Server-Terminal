// client.hpp
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "socket.hpp"


class Client : public Socket {
    public:
         Client();
        Client(SOCKET , SOCKADDR* , SOCKADDR_IN* , socklen_t*);
        ~Client();
        
        inline Socket& getSocket() {return *this;}
        void setSocket(Socket& newSocket);
        bool operator==(Client& other);
};

#endif // CLIENT_HPP