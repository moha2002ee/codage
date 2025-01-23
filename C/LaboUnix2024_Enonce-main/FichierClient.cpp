#include "FichierClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

int estPresent(const char* nom)
{
    int fd, pos = 0, rc;
    CLIENT buffer;

    // Ouverture du fichier
    if ((fd = open(FICHIER_CLIENTS, O_RDONLY)) == -1)
    {
        // perror("Erreur de open() dans estPresent");
        return -1;  // Fichier introuvable ou erreur
    }

    // Vérifier si le fichier est vide
    if ((rc = lseek(fd, 0, SEEK_END)) == -1)
    {
        perror("Erreur de lseek");
        close(fd);
        return -1;
    }

    if (rc == 0) // Si le fichier est vide
    {
        close(fd);
        return -1;  // Aucun client présent
    }

    // Revenir au début du fichier
    if ((rc = lseek(fd, 0, SEEK_SET)) == -1)
    {
        perror("Erreur de lseek");
        close(fd);
        return -1;
    }

    // Parcourir le fichier pour trouver le client
    while ((rc = read(fd, &buffer, sizeof(CLIENT))) > 0)
    {
        if (strcmp(buffer.nom, nom) == 0) // Si le nom correspond
        {
            close(fd);
            return pos;  // Retourner la position zéro-indexée
        }
        pos++;
    }

    // Si aucun client n'est trouvé
    close(fd);
    return -1;
}



////////////////////////////////////////////////////////////////////////////////////

int hash(const char* motDePasse)
{
    int s = 0, j = 1, i = 0;

    while (motDePasse[i] != '\0')
    {
        s += (j * motDePasse[i]);
        i++;
        j++;
    }

    return s % 97;  // Retourne un hash entre 0 et 96
}

////////////////////////////////////////////////////////////////////////////////////

void ajouteClient(const char* nom, const char* motDePasse)
{
    int fd;
    CLIENT buffer;

    // Ouverture du fichier en mode ajout
    fd = open(FICHIER_CLIENTS, O_WRONLY | O_CREAT | O_APPEND, 0664);
    if (fd == -1)
    {
        perror("Erreur d'ouverture du fichier dans ajouteClient");
        return;
    }

    // Initialiser la structure CLIENT
    strncpy(buffer.nom, nom, sizeof(buffer.nom) - 1);
    buffer.nom[sizeof(buffer.nom) - 1] = '\0';  // Sécuriser la chaîne
    buffer.hash = hash(motDePasse);

    // Écriture du client dans le fichier
    if (write(fd, &buffer, sizeof(CLIENT)) != sizeof(CLIENT))
    {
        perror("Erreur d'écriture dans le fichier");
    }

    // Fermeture du fichier
    if (close(fd) == -1)
    {
        perror("Erreur de fermeture du fichier");
    }
}

////////////////////////////////////////////////////////////////////////////////////

int verifieMotDePasse(int pos, const char* motDePasse)
{
    int fd, rc, mdp;
    CLIENT buffer;

    // Ouverture du fichier en lecture
    if ((fd = open(FICHIER_CLIENTS, O_RDONLY)) == -1)
    {
        perror("Erreur d'ouverture du fichier dans verifieMotDePasse");
        return -1;
    }

    pos--;  // Ajustement de la position (indexation à partir de 1)

    // Se déplacer à la position correcte dans le fichier
    if ((rc = lseek(fd, pos * sizeof(CLIENT), SEEK_SET)) == -1)
    {
        perror("Erreur de lseek");
        close(fd);
        return -1;
    }

    // Calcul du hash du mot de passe
    mdp = hash(motDePasse);

    // Lecture du client à cette position
    if ((rc = read(fd, &buffer, sizeof(CLIENT))) == -1)
    {
        perror("Erreur de lecture");
        close(fd);
        return -1;
    }

    // Vérification du mot de passe
    int veri = (mdp == buffer.hash) ? 1 : 0;

    // Fermeture du fichier
    if (close(fd) == -1)
    {
        perror("Erreur de fermeture du fichier");
    }

    return veri;
}

////////////////////////////////////////////////////////////////////////////////////

int listeClients(CLIENT *vecteur)
{
    int fd, rc, nbr = 0;
    CLIENT buffer;

    // Ouverture du fichier en lecture
    if ((fd = open(FICHIER_CLIENTS, O_RDONLY)) == -1)
    {
        perror("Erreur d'ouverture du fichier dans listeClients");
        return -1;
    }

    // Lecture du fichier et remplissage du tableau
    while ((rc = read(fd, &buffer, sizeof(CLIENT))) > 0 && nbr < 20)
    {
        vecteur[nbr] = buffer;  // Copier le client dans le vecteur
        nbr++;
    }

    // Fermeture du fichier
    if (close(fd) == -1)
    {
        perror("Erreur de fermeture du fichier");
    }

    return nbr;
}
