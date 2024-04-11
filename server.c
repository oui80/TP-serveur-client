#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>

int main()
{
    // Création de la socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(12345); // Port d'écoute

    // Liaison de la socket à l'adresse et au port spécifiés
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Erreur lors de la liaison de la socket");
        exit(EXIT_FAILURE);
    }

    // Début de l'écoute des connexions entrantes
    if (listen(server_socket, 5) < 0)
    {
        perror("Erreur lors de l'écoute des connexions entrantes");
        exit(EXIT_FAILURE);
    }

    printf("Le serveur est en attente de connexions...\n");

    // Accepter les connexions entrantes
    int client_socket;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Boucle pour accepter les connexions en boucle
    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        printf("Nouvelle connexion acceptée\n");

        // Traiter la connexion (lecture/écriture)

        char buffer[1024] = {0};
        read(client_socket, buffer, 1024);
        if (strcmp(buffer, "ls") == 0)
        {
            // Ouvrir le répertoire ./data_serveur
            DIR *dir = opendir("./data_serveur");
            if (dir == NULL)
            {
                perror("Erreur lors de l'ouverture du répertoire");
            }

            // Lire les entrées du répertoire
            struct dirent *entry;
            char liste[1024] = {0};
            while ((entry = readdir(dir)) != NULL)
            {
                // Ignorer les entrées spéciales . et ..
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                {
                    continue;
                }

                // Afficher le nom du fichier
                strcat(liste, entry->d_name);
                strcat(liste, "\n");
            }
            write(client_socket, liste, strlen(liste));
            // Fermer le répertoire
            closedir(dir);

            printf("Message du client : %s\n", buffer);
        }

        int l = 3;

        char commande[l + 1];
        strncpy(commande, buffer, l);
        commande[l] = '\0';

        // nom du fichier
        int len = strlen(buffer);
        char filename[len - l];

        strncpy(filename, buffer + l + 1, len - l - 1);
        filename[len - l - 1] = '\0';
        char path[1024] = "./data_serveur/";
        strcat(path, filename);
        

        if (strcmp(commande, "put") == 0)
        {
            // on envoie ready au client
            write(client_socket, "ready", 5);


            // création du fichier
            FILE *file = fopen(path, "w");
            if (file == NULL)
            {
                perror("Erreur lors de la création du fichier");
                exit(EXIT_FAILURE);
            }

            // écriture du buffer dans le fichier
            size_t read_bytes;
            while ((read_bytes = read(client_socket, buffer, 1024)) > 0)
            {
                fwrite(buffer, 1, read_bytes, file);
            }

            // Fermer le fichier
            fclose(file);

            printf("Message du client : %s\n", commande);
        }
        else if (!strcmp(commande, "get"))
        {
            // Ouvrir le fichier demandé

            FILE *file = fopen(path, "r");
            if (file == NULL)
            {
                write(client_socket, "Fichier introuvable", 20);
            }
            else
            {
                char buffer[1024];

                // Lire et envoyer successivement les paquets de données du fichier
                size_t bytes_read;
                do
                {
                    bytes_read = fread(buffer, 1, 1024, file); // Lire un paquet de données du fichier
                    if (bytes_read < 1024 && ferror(file))
                    {
                        perror("Erreur lors de la lecture du fichier");
                        fclose(file);
                        return 1;
                    }
                    // Envoyer le paquet de données au client via le socket
                    if (write(client_socket, buffer, bytes_read) < 0)
                    {
                        perror("Erreur lors de l'envoi des données au client");
                        fclose(file);
                        return 1;
                    }
                } while (bytes_read == 1024);
                fclose(file);
                printf("Message du client : %s\n", commande);
            }
        }

        if (strcmp(buffer, "exit") == 0)
        {
            printf("Fermeture de la connexion\n");
            close(client_socket);
            printf("Message du client : %s\n", buffer);
            break;
        }

        // Fermer la socket du client
        close(client_socket);
        printf("Connexion fermée\n");
    }

    // Fermer la socket du serveur
    close(server_socket);
    printf("Serveur arrêté\n");

    return 0;
}
