#include <iostream>
#include <string.h>
#include <vector>

#include "client.hpp"

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define PORT 5656

using namespace std;

int main(void)
{
    int erreur = 0;

    // Socket et contexte d'adressage du client
    SOCKADDR_IN* clientIN = new SOCKADDR_IN;
    SOCKET clientSOCKET = 0;
    socklen_t clientSIZE = sizeof(clientIN);

    Client client(clientSOCKET, reinterpret_cast<SOCKADDR*>(clientIN), clientIN, &clientSIZE);

    if (!erreur)
    {
        // Si le client a réussit a établir une connexion avec le serveur
        if (connect(client.getSOCKET(), reinterpret_cast<SOCKADDR*>(client.getSOCKADDR_IN()), sizeof(*(client.getSOCKADDR_IN()))) != SOCKET_ERROR)
        {
            cout << "Connexion à " << inet_ntoa(client.getSOCKADDR_IN()->sin_addr) << " sur le port "
                      << htons(client.getSOCKADDR_IN()->sin_port) << endl;

            cout<<"Liste des commandes : "<<endl;
            cout<<"creer : creer <nom du fichier>"<<endl;
            cout<<"effacer : effacer <nom du fichier>"<<endl;
            cout<<"editer : editer <nom du fichier>"<<endl;
            cout<<"ecrire : ecrire <nom du fichier> <numéro de la ligne> <ligne à ajouter>"<<endl;
            cout<<"effacerl : effacerl <nom du fichier> <ligne à supprimer>"<<endl;
            cout<<"lire : lire <nom du fichier>"<<endl;
            cout<<"obtenir : obtenir <nom du fichier>"<<endl;
            cout<<"Pour quitter la connexion avec le serveur : q"<<endl;



            char buffer[256] = "";
            while (strcmp(buffer, "q\n") != 0)
            {
                if (read(client.getSOCKET(), buffer, 256) == 0)
                    break;
                cout << buffer;

                if (strcmp(buffer, "$ ") == 0)
                {
                    char commande[256];
                    if (fgets(commande, 256, stdin) == NULL)
                        break;

                    write(client.getSOCKET(), commande, 256);

                    if (strcmp(commande, "q\n") != 0)
                    {
                        // Commande lire
                        if (strncmp(commande, "lire", 4) == 0)
                        {
                            size_t size;
                            if (read(client.getSOCKET(), &size, sizeof(size_t)) > 0)
                            {
                                vector<char> fichier(size + 1, '\0');
                                read(client.getSOCKET(), fichier.data(), size);
                                for (const auto& element : fichier)
                                    cout << element;
                                cout<<endl; 
                                cout<<endl;
                            }
                        }
                        // Commande editer
                        else if (strncmp(commande, "editer", 6) == 0)
                        {
                            if (read(client.getSOCKET(), buffer, 256) == 0)
                                break;
                            cout << buffer << endl;
                            string message = buffer;
                            
                            if (message.find("Début") == 0)
                            {
                                cout<<"Pour arréter de rajouter des lignes taper 'finir'."<<endl;
                                string line;
                                while (true)
                                {
                                    cout<<"ligne : ";
                                    getline(cin, line);
                                    line += "\n";
                                    write(client.getSOCKET(), line.c_str(), line.size() + 1);

                                    if (line == "finir\n")
                                        break;
                                }
                                if (read(client.getSOCKET(), buffer, 256) == 0)
                                break;
                                cout << buffer << endl;
                            }
                            cout<<endl;
                        }
                        // Autre commandes
                        else
                        {
                            if (read(client.getSOCKET(), buffer, 256) == 0)
                                break;
                            cout << buffer << endl;
                            cout<<endl;
                        }
                    }
                }
            }
        }
        else
            cout << "Impossible de se connecter" << endl;
        closesocket(client.getSOCKET());
    }

    return EXIT_SUCCESS;
}