// serveur.cpp
#include <iostream>
#include <arpa/inet.h>

#include "serveur.hpp"

Serveur::Serveur(SOCKET _currentSocket, SOCKADDR* _socketAddr, SOCKADDR_IN* _socketAddrIn, socklen_t* _socketSize, int _sock_err)
    : Socket(_currentSocket, _socketAddr, _socketAddrIn, _socketSize), sock_err(_sock_err)
{
    setSOCKET(socket(AF_INET, SOCK_STREAM, 0));
    getSOCKADDR_IN()->sin_addr.s_addr = htonl(INADDR_ANY);
    getSOCKADDR_IN()->sin_family = AF_INET;
    getSOCKADDR_IN()->sin_port = htons(PORT);
    sock_err = bind(getSOCKET(), reinterpret_cast<SOCKADDR*>(getSOCKADDR_IN()), sizeof(*(getSOCKADDR_IN())));
}

Serveur::~Serveur() {

}
