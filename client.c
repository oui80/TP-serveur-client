#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int send_message(int client_socket, char *message)
{
    if (send(client_socket, message, strlen(message), 0) < 0)
    {
        perror("Échec de l'envoi du message");
        return -1;
    }
    printf("Message envoyé au serveur : %s\n", message);
    return 0;
}

int receive_message(int client_socket, char *buffer)
{

    if (recv(client_socket, buffer, 1024, 0) < 0)
    {
        perror("Échec de la réception de la réponse");
        return 0;
    }
    return 1;
}

void get_clavier(char *message)
{
    printf(">> ");
    fgets(message, 1024, stdin);
    message[strlen(message) - 1] = '\0';
}

int connexion()
{
    // Création de la socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345); // Port du serveur

    // Conversion de l'adresse IP en format binaire
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        perror("Adresse invalide/Adresse non supportée");
        exit(EXIT_FAILURE);
    }

    // Connexion au serveur
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Connexion échouée");
        exit(EXIT_FAILURE);
    }
    return client_socket;
}

#define BUFFER_SIZE 1024 // Taille du tampon de lecture/écriture

void receive_file(int client_socket, const char *filename)
{
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "./data_client/%s", filename);

    // Ouvrir le fichier en mode binaire pour éviter la transformation de fin de ligne
    FILE *file = fopen(filepath, "wb");
    if (file == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    char buffer[BUFFER_SIZE]; // Tampon de réception des données

    ssize_t bytes_received;
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0)
    {
        fwrite(buffer, 1, bytes_received, file); // Écrire les données dans le fichier
    }

    if (bytes_received < 0)
    {
        perror("Erreur lors de la réception des données");
    }

    fclose(file);

    if (bytes_received == 0)
    {
        printf("Fichier %s téléchargé avec succès\n", filename);
    }

    close(client_socket);
}

int main()
{
    int client_socket;
    // Envoi d'un message au serveur
    char message[1024];
    char buffer[1024] = {0};
    while (1)
    {
        get_clavier(message);
        if (!strcmp(message, "exit"))
        {
            client_socket = connexion();
            send_message(client_socket, "exit");
            close(client_socket);
            break;
        }

        if (!strcmp(message, "ls"))
        {

            client_socket = connexion();
            send_message(client_socket, "ls");
            receive_message(client_socket, buffer);
            close(client_socket);
            printf("fichier présent : \n%s\n", buffer);
        }
        else
        {
            int l = 3;

            char commande[l + 1];
            strncpy(commande, message, l);
            commande[l] = '\0';

            // nom du fichier
            int len = strlen(message);
            char filename[len - l + 1];

            strncpy(filename, message + l + 1, len - l - 1);
            filename[len - l - 1] = '\0';
            filename[len - l] = '\n';

            if (!strcmp(commande, "put"))
            {
                client_socket = connexion();
                send_message(client_socket, message);
                char filepath[256];
                snprintf(filepath, sizeof(filepath), "./data_client/%s", filename);

                printf("Nom du fichier : %s\n", filepath);

                // on recoit la réponse du serveur
                receive_message(client_socket, buffer);
                if (!strcmp(buffer, "ready"))
                {
                    printf("Envoi du fichier %s\n", filepath);
                }
                else
                {
                    printf("Erreur lors de l'envoi du fichier\n");
                    close(client_socket);
                    continue;
                }

                FILE *file = fopen(filepath, "r");
                if (file == NULL)
                {
                    perror("Erreur lors de l'ouverture du fichier");
                    exit(EXIT_FAILURE);
                }

                // Envoi du fichier
                char buffer[1024] = {0};
                size_t read_bytes;
                while ((read_bytes = fread(buffer, 1, 1024, file)) > 0)
                {
                    if (send(client_socket, buffer, read_bytes, 0) < 0)
                    {
                        perror("Échec de l'envoi du fichier");
                        exit(EXIT_FAILURE);
                    }
                }
                fclose(file);
                close(client_socket);
            }
            else if (!strcmp(commande, "get"))
            {
                client_socket = connexion();
                printf("Envoi de la commande get au serveur %s\n", message);
                send_message(client_socket, message);

                // recoit la réponse du serveur
                receive_message(client_socket, buffer);
                if (!strcmp(buffer, "Fichier introuvable"))
                {
                    printf("Fichier introuvable\n");
                }
                else
                {
                    printf("Fichier trouvé\n");

                    // création du fichier
                    char filepath[256];
                    snprintf(filepath, sizeof(filepath), "./data_client/%s", filename);

                    FILE *file = fopen(filepath, "w");
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
                    printf("Fichier %s téléchargé\n", filename);
                }
                close(client_socket);
            }
        }
    }
    return 0;
}
// Fermeture de la socket du client
