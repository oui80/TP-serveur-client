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
            printf("Envoi de la commande ls au serveur\n");
            send_message(client_socket, "ls");
            receive_message(client_socket, buffer);
            close(client_socket);
            printf("fichier présent : \n%s\n", buffer);
        }
        else
        {
            printf("Message envoyé au serveur : %s\n", message);
            char *filename;
            char *commande = malloc(1024);
            strcpy(commande, message);
            // Utilisation de strtok() pour extraire les deux mots
            filename = strtok(commande, " "); // Divise la chaîne par les espaces
            filename = strtok(NULL, " ");
            // Affichage des deux mots extraits
            printf("Commande : %s\n", message);
            printf("Nom du fichier : %s\n", filename);
            if (!strcmp(commande, "get"))
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
            else if (!strcmp(message, "put"))
            {
                char filename[100];
                FILE *file = fopen(filename, "r");
                if (file == NULL)
                {
                    printf("Fichier introuvable\n");
                    continue;
                }
                client_socket = connexion();
                send_message(client_socket, message);
                char c;
                while ((c = fgetc(file)) != EOF)
                {
                    send(client_socket, &c, 1, 0);
                }
                close(client_socket);
                fclose(file);
                printf("Fichier %s envoyé\n", filename);
            }
        }
    }
    // Fermeture de la socket du client

    return 0;
}