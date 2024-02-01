// serveur.hpp
#ifndef SERVEUR_HPP
#define SERVEUR_HPP

#include <sys/socket.h>

#include "socket.hpp"

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

class Serveur : public Socket {
private:
    int sock_err;

public:
    Serveur(SOCKET, SOCKADDR*, SOCKADDR_IN*, socklen_t* = nullptr, int = 0);
    ~Serveur();

    inline int getSockErr() const { return sock_err; }
    inline void setSockErr(int err) { sock_err = err; }
};

#endif // SERVEUR_HPP
