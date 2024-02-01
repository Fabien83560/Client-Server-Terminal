#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include <fcntl.h>
#include <unordered_map>

#include "serveur.hpp"
#include "client.hpp"

#define PORT 5656

using namespace std;

/* Socket et contexte d'adressage du serveur */
SOCKADDR_IN* serveurIN  = new SOCKADDR_IN;
SOCKET serveurSOCKET = 0;
socklen_t serveurSIZE = sizeof(&serveurIN);

Serveur serveur(serveurSOCKET, reinterpret_cast<SOCKADDR*>(serveurIN), serveurIN, &serveurSIZE);

// Variable global
vector<Client> connectedClients;

// Mutex Liste de Clients
pthread_mutex_t connectedClientsMutex;

// Table de hashage pour les mutex des fichiers ouvert
unordered_map<string, pthread_mutex_t*> mutexHashMap;

// Fonction pour savoir si un fichier existe dans le répertoire ServeurFiles
bool fileExist(string filename)
{
    system("ls ServeurFiles/ >> .tmp.txt");

    string tmpPath = ".tmp.txt";
    ifstream file(tmpPath);

    if (file.is_open())
    {
        string line;

        while (getline(file, line))
        {
            if (line == filename) {
                system("rm .tmp.txt");
                return true;
            }
        }
        file.close();
    }
    else
        cerr << "Erreur : Impossible d'ouvrir le fichier." << endl;
    system("rm .tmp.txt");
    return false;
}

void closeServeur()
{
    pthread_mutex_unlock(&connectedClientsMutex);
    // Libération de la mémoire des mutex lié au fichiers
    for (auto& pair : mutexHashMap)
    {
        pthread_mutex_destroy(pair.second);
        delete pair.second;
    }
    mutexHashMap.clear();

    cout << "Fermeture du socket serveur" << endl;
    closesocket(serveur.getSOCKET());
    cout << "Fermeture du serveur terminée" << endl;
    exit(0);
}

void* fonctionThead(void*_arg) {
    class Client* client = (class Client*) _arg;

    cout << "Un client se connecte avec le socket " << client->getSOCKET() << " de " << inet_ntoa(client->getSOCKADDR_IN()->sin_addr)
                << " sur le port " << htons(client->getSOCKADDR_IN()->sin_port) << endl;
    char message[256] = "";
    while (strcmp(message, "q\n") != 0)
    {
        char demande[256] = "$ ";
        write(client->getSOCKET(), demande, 256);

        if (read(client->getSOCKET(), message, 256) == 0)
            break;
        else
        {
            if(strcmp(message, "q\n") == 0)
                break;
            else
            {
                // Séparation de la commande et du reste
                istringstream iss(message);
                string command;
                iss >> command;

                string filename;
                iss >> ws;
                getline(iss, filename);

                // Commande creer
                if(strcmp(strtok(message," "),"creer") == 0)
                {
                    char result[256];
                    if(! (filename.empty()))
                    {
                        if( ! (fileExist(filename)))
                        {
                            string output = "touch ServeurFiles/" + filename;
                            system(output.c_str());
                            string stringResult = "Création de " + filename + ".";
                            strcpy(result,stringResult.c_str());
                        }
                        else
                        {
                            string stringResult = "Le fichier " + filename + " existe déja !";
                            strcpy(result,stringResult.c_str());
                        }
                    }
                    else
                    {
                        string stringResult = "Nom de fichier manquant.";
                        strcpy(result,stringResult.c_str());
                    }
                    write(client->getSOCKET(), result, 256);
                }
                // Commande effacer
                else if(strcmp(strtok(message," "),"effacer") == 0)
                {
                    char result[256];
                    if(! (filename.empty()))
                    {
                        if(fileExist(filename))
                        {
                            string output = "rm ServeurFiles/" + filename;
                            system(output.c_str());
                            string stringResult = "Suppression de " + filename + ".";
                            strcpy(result,stringResult.c_str());
                        }
                        else
                        {
                            string stringResult = "effacer : impossible de supprimer '" + filename + "': Aucun fichier ou dossier de ce type.";
                            strcpy(result,stringResult.c_str());
                        }
                    }
                    else
                    {
                        string stringResult = "Nom de fichier manquant.";
                        strcpy(result,stringResult.c_str());
                    }
                    write(client->getSOCKET(), result, 256);
                }
                // Commande editer
                else if(strcmp(strtok(message," "),"editer") == 0)
                {
                    string result;
                    if (!filename.empty())
                    {
                        if (fileExist(filename))
                        {
                            auto it = mutexHashMap.find(filename);

                            // Si un mutex existe déja pour se fichier l'utilisé sinon en créer un nouveau
                            pthread_mutex_t *myMutex = nullptr;
                            if (it != mutexHashMap.end())
                                myMutex = it->second;
                            else
                            {
                                myMutex = new pthread_mutex_t;
                                pthread_mutex_init(myMutex, NULL);
                            }
                            // Essayé de bloquer le fichier afin de gérer les cas 
                            int mutex = pthread_mutex_trylock(myMutex);

                            // Si le mutex n'est pas bloquer
                            if(mutex == 0)
                            {
                                mutexHashMap[filename] = myMutex;
                                char buffer[256] = "";
                                string line;
                                ostringstream content;
                                
                                result = "Début de l'éditage du fichier " + filename + ".";
                                write(client->getSOCKET(), result.c_str(), 256);

                                while (true)
                                {
                                    if (read(client->getSOCKET(), buffer, 256) == 0)
                                        break;

                                    line = buffer;

                                    if (line == "finir\n")
                                        break;

                                    content << line;
                                }

                                ofstream outFile("ServeurFiles/" + filename, ios::app);
                                if (outFile.is_open())
                                {
                                    outFile << content.str();
                                    outFile.close();

                                    result = "Édition du fichier " + filename + " terminée avec succès.";
                                }
                                else
                                    result = "Erreur lors de l'ouverture du fichier " + filename + " pour édition.";
                                
                                // Déblocage du fichier
                                pthread_mutex_unlock(myMutex);
                                // Actualisation du mutex correspondant au fichier
                                mutexHashMap[filename] = myMutex;
                            }
                            else if(mutex == EBUSY)
                                result = "Un autre utilisateur edite actuellement ce fichier veuillez re essayer ultérieurement.";
                            else
                                result = "Une erreur c'est produit lors du blockage du fichier " + filename + ".";
                        }
                        else
                            result = "editer : impossible d'éditer '" + filename + "': Aucun fichier ou dossier de ce type.";
                    }
                    else
                        result = "Nom de fichier manquant.";

                    write(client->getSOCKET(), result.c_str(), 256);
                }
                // Commande ecrire
                else if (strcmp(strtok(message, " "), "ecrire") == 0)
                {
                    string result;
                    if (!(filename.empty()))
                    {
                        istringstream iss(filename);

                        string file;
                        int line;
                        string content;

                        iss >> file >> line >> ws;
                        getline(iss, content);
                        if(fileExist(file))
                        {
                            string output = "wc -l ServeurFiles/" + file + " > .tmp.txt";
                            system(output.c_str());

                            ifstream fichierTemp(".tmp.txt");
                            int totalLines;
                            if (fichierTemp.is_open())
                            {
                                fichierTemp >> totalLines;
                                fichierTemp.close();
                            }
                            output = "rm .tmp.txt";
                            system(output.c_str());
                            
                            if(line !=  0)
                            {
                                if(totalLines > line)
                                {
                                    if( ! content.empty())
                                    {
                                        string commande1 = "head -n " + to_string(line - 1) + " ServeurFiles/" + file + " > .tmp.txt";
                                        system(commande1.c_str());

                                        string commande2 = "echo \"" + content + "\" >> .tmp.txt";
                                        system(commande2.c_str());

                                        string commande3 = "tail -n +" + to_string(line) + " ServeurFiles/" + file + " >> .tmp.txt && mv .tmp.txt ServeurFiles/" + file;
                                        system(commande3.c_str());

                                        result = "La ligne a été rajouter avec succès.";
                                    }
                                    else
                                        result = "Merci de renseigner le contenu de la ligne a rajouter.";
                                }
                                else
                                    result = "Merci de renseigner un numéro de ligne valide.";
                            }
                            else
                                result = "Numéro de ligne a rajouter manquant.";
                        }
                        else
                            result = "ecrire : impossible de rajouter la ligne " + to_string(line) + " du fichier '" + file + "': Aucun fichier ou dossier de ce type.";
                    }
                    else
                        result = "ecrire : ecrire exemple.txt 5 contenu a rajouter.";

                    write(client->getSOCKET(), result.c_str(), 256);
                }
                // Commande effacerl
                else if(strcmp(strtok(message," "),"effacerl") == 0)
                {
                    string result;
                    if(! (filename.empty()))
                    {
                        istringstream iss(filename);

                        string file;
                        int line;

                        iss >> file >> line;
                        if(line != 0)
                        {
                            if(fileExist(file))
                            {
                                string output = "wc -l ServeurFiles/" + file + " > .tmp.txt";
                                system(output.c_str());

                                ifstream fichierTemp(".tmp.txt");
                                int nombreDeLignes;
                                if (fichierTemp.is_open())
                                {
                                    fichierTemp >> nombreDeLignes;
                                    fichierTemp.close();
                                }
                                output = "rm .tmp.txt";
                                system(output.c_str());
                                if (nombreDeLignes > line)
                                {
                                    string commande = "head -n " + to_string(line - 1) + " ServeurFiles/" + file + " > .tmp.txt && tail -n +" + 
                                                            to_string(line + 1) + " ServeurFiles/" + file + " >> .tmp.txt && mv .tmp.txt ServeurFiles/" + file;

                                    int res = system(commande.c_str());
                                    if (res == 0)
                                        result = "La ligne " + to_string(line) + " a été supprimée avec succès.";
                                }
                                else
                                    result = "Impossible de supprimé la ligne " + to_string(line) + " car le fichier n'a pas assez de ligne.";
                            }
                            else
                                result = "effacerl : impossible de supprimer la ligne " + to_string(line) + " du fichier '" + file + "': Aucun fichier ou dossier de ce type.";
                        }
                        else
                            result = "Impossible de supprimé cette ligne.";
                    }
                    else
                        result = "Nom de fichier manquant";
                    write(client->getSOCKET(), result.c_str(), 256);
                }
                // Commande lire
                else if (strcmp(strtok(message, " "), "lire") == 0)
                {
                    if (!filename.empty())
                    {
                        stringstream ssCommande;
                        ssCommande << "less ServeurFiles/" + filename << "> .tmp";

                        system(ssCommande.str().c_str());

                        struct stat st;
                        const char fichier[5] = ".tmp";
                        stat(fichier, &st);
                        size_t size = st.st_size;

                        int tmpFd = open(".tmp", O_RDONLY);
                        if (tmpFd > 0)
                        {
                            vector<char> buffer(size);
                            read(tmpFd, buffer.data(), size - 1);
                            stringstream ssTaille;
                            ssTaille << size;
                            write(client->getSOCKET(), &size, sizeof(size_t));
                            write(client->getSOCKET(), buffer.data(), size);
                            system("rm .tmp");
                            buffer.clear();
                        }
                        else
                        {
                            char result[256];
                            string stringResult = "Nom de fichier manquant.";
                            strcpy(result, stringResult.c_str());
                            write(client->getSOCKET(), result, 256);
                        }
                    }
                }
                // Commande obtenir
                else if(strcmp(strtok(message," "),"obtenir") == 0)
                {
                    char result[256];
                    if (!filename.empty())
                    {
                        if (fileExist(filename))
                        {
                            ifstream file("ServeurFiles/" + filename, ios::in);

                            if (file.is_open())
                            {
                                string output = "cp ServeurFiles/" + filename + " ClientFiles/" + filename;
                                system(output.c_str());
                                string result = "Le contenu du fichier " + filename + " à été téléchargé !";
                                write(client->getSOCKET(), result.c_str(), 256);
                            }
                            else
                            {
                                string stringResult = "Erreur lors de l'ouverture du fichier " + filename + ".";
                                strcpy(result, stringResult.c_str());
                                write(client->getSOCKET(), result, 256);
                            }
                        }
                        else
                        {
                            string stringResult = "obtenir : impossible d'obtenir '" + filename + "': Aucun fichier ou dossier de ce type.";
                            strcpy(result, stringResult.c_str());
                            write(client->getSOCKET(), result, 256);
                        }
                    }
                    else
                    {
                        string stringResult = "Nom de fichier manquant.";
                        strcpy(result, stringResult.c_str());
                        write(client->getSOCKET(), result, 256);
                    }
                }
                // Commande Inconnue
                else
                {
                    string stringRes = command + " : Commande inconnue !";
                    write(client->getSOCKET(), stringRes.c_str(), 256);
                }
            }
        }
    }

    // Fermeture de la connection
    shutdown(client->getSOCKET(), 2);

    pthread_mutex_lock(&connectedClientsMutex);

    // Suppression du Client dans la liste de Client connectées
    auto it = connectedClients.begin();
    while (it != connectedClients.end())
    {
        if (*it == *client)
        {
            it = connectedClients.erase(it);
            cout << "Un client viens de ce déconecté."<<endl;
            cout<<"Client Connectées : " << connectedClients.size() - 1 <<endl;
        }
        else
            ++it;
    }

    // Dans le cas ou il n'y a plus de Client connectées fermer le serveur
    if(connectedClients.size() == 1)
        closeServeur();
    
    pthread_mutex_unlock(&connectedClientsMutex);

    return (NULL);
}

int main(int argc, char **argv)
{
    pthread_mutex_init(&connectedClientsMutex, NULL);

    int erreur = 0;

    pthread_mutex_lock(&connectedClientsMutex);
    connectedClients.push_back(Client());
    pthread_mutex_unlock(&connectedClientsMutex);

    if (!erreur)
    {
        if (serveur.getSOCKET() != INVALID_SOCKET)
        {
            cout << "Le socket " << serveur.getSOCKET() << " est maintenant ouvert en mode TCP/IP" << endl;

            if (serveur.getSockErr() != SOCKET_ERROR)
            {
                 // L'application commence à écouter du coté serveur
                serveur.setSockErr(listen(serveur.getSOCKET(), 10));
                cout << "Écoute du port " << PORT << endl;

                if (serveur.getSockErr() != SOCKET_ERROR)
                {
                    cout << "Patientez pendant que le client se connecte sur le port " << PORT << endl;
                    while(true)
                    {
                        pthread_mutex_lock(&connectedClientsMutex);
                        cout<<"Client Connectées : " << connectedClients.size() - 1 <<endl;
                        pthread_mutex_unlock(&connectedClientsMutex);
                        
                        // Création des informations client
                        SOCKADDR_IN* clientIN = new SOCKADDR_IN;
                        SOCKET clientSOCKET = 0;
                        socklen_t clientSIZE = sizeof(clientIN);

                        Client client(clientSOCKET, reinterpret_cast<SOCKADDR*>(clientIN), clientIN, &clientSIZE);

                        // Accepter la connexion
                        client.setSOCKET(accept(serveur.getSOCKET(), (SOCKADDR*)client.getSOCKADDR_IN(), client.getSocklen_t()));

                        if (client.getSOCKET() != INVALID_SOCKET)
                        {
                            // Ajouter le client a la liste de client connectées
                            pthread_mutex_lock(&connectedClientsMutex);
                            connectedClients.push_back(client);

                            // Création du thread permettant au client d'intéragir avec le serveur
                            pthread_t p;
                            pthread_create(&p, NULL, fonctionThead, &connectedClients.back());
                            pthread_mutex_unlock(&connectedClientsMutex);
                        }
                        else 
                            cerr << "Erreur lors de la connexion du client." << endl;
                    }
                }
                else
                    cerr << "listen";
            }
            else
                cerr << "Veuillez attendre la réouverture du port " << PORT << ", cela peut prendre plus d'une minute !"<<endl;
        }
        else
            cerr << "socket";
    }

    return EXIT_SUCCESS;
}
