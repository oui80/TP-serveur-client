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

                printf("Nom du fichier : %s\n", filename);
                FILE *file = fopen(filename, "r");
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
                char buffer[1024] = {0};
                receive_message(client_socket, buffer);
                if (!strcmp(buffer, "Fichier introuvable"))
                {
                    printf("Fichier introuvable\n");
                }
                else
                {
                    FILE *file = fopen(filename, "w");
                    fprintf(file, "%s", buffer);
                    fclose(file);
                    printf("Fichier %s téléchargé\n", filename);
                }
                close(client_socket);
                free(commande);
            }
        }
    }
    return 0;
}
// Fermeture de la socket du client
