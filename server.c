#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

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
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len)))
    {
        printf("Nouvelle connexion acceptée\n");

        // Traiter la connexion (lecture/écriture)
        char buffer[1024] = {0};
        read(client_socket, buffer, 1024);
        printf("Message du client : %s\n", buffer);
        write(client_socket, "Message reçu par le serveur", strlen("Message reçu par le serveur"));

        // Fermer la socket du client
        close(client_socket);
    }

    // Fermer la socket du serveur
    close(server_socket);

    return 0;
}
