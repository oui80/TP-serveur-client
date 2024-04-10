#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
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

    printf("Connecté au serveur\n");

    // Envoi d'un message au serveur
    char message[] = "Bonjour, serveur!";
    if (send(client_socket, message, strlen(message), 0) < 0)
    {
        perror("Échec de l'envoi du message");
        exit(EXIT_FAILURE);
    }
    printf("Message envoyé au serveur : %s\n", message);

    // Réception de la réponse du serveur
    char buffer[1024] = {0};
    if (recv(client_socket, buffer, 1024, 0) < 0)
    {
        perror("Échec de la réception de la réponse");
        exit(EXIT_FAILURE);
    }
    printf("Réponse du serveur : %s\n", buffer);

    // Fermeture de la socket du client
    close(client_socket);

    return 0;
}