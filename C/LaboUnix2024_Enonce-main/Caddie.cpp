#include <stdio.h>      // Bibliothèque standard pour les entrées/sorties
#include <stdlib.h>     // Bibliothèque standard pour des fonctions utilitaires (ex: exit)
#include <sys/types.h>  // Définitions des types de données pour IPC et autres
#include <sys/ipc.h>    // Interface pour les clés IPC
#include <sys/msg.h>    // Interface pour les files de messages
#include <sys/shm.h>    // Interface pour la mémoire partagée
#include <signal.h>     // Gestion des signaux
#include <string.h>     // Fonctions pour manipuler des chaînes de caractères
#include <sys/stat.h>   // Manipulation des permissions des fichiers
#include <fcntl.h>      // Contrôle des fichiers
#include <unistd.h>     // Fonctionnalités UNIX standard (ex: close, write)
#include <mysql.h>      // Interface pour interagir avec une base de données MySQL
#include "protocole.h"  // Fichier spécifique définissant les clés et structures pour le projet

int idQ;                // Identifiant de la file de messages
ARTICLE articles[10];   // Tableau pour stocker les articles
int nbArticles = 0;     // Nombre d'articles dans le panier

int fdWpipe;            // Descripteur pour le pipe d'écriture
int pidClient;          // PID du client associé

MYSQL* connexion;       // Pointeur pour gérer la connexion MySQL

void handlerSIGALRM(int sig); // Prototype pour le gestionnaire de signal SIGALRM

int main(int argc,char* argv[]) 
{
    // Masquage du signal SIGINT pour le programme
    sigset_t mask;      
    sigaddset(&mask,SIGINT);
    sigprocmask(SIG_SETMASK,&mask,NULL);

    // Préparer l'armement des signaux (pas encore implémenté)
    // TO DO

    // Récupération de l'identifiant de la file de messages
    fprintf(stderr,"(CADDIE %d) Recuperation de l'id de la file de messages\n", getpid());

    if ((idQ = msgget(CLE,0)) == -1) // Si la récupération échoue
    {
        perror("(CADDIE) Erreur de msgget"); // Affiche un message d'erreur
        exit(1); // Termine le programme avec un code d'erreur
    }

    // Connexion à la base de données MySQL
    connexion = mysql_init(NULL); // Initialise le gestionnaire MySQL
    if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
    {
        fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
        exit(1); // Quitte si la connexion échoue
    }

    MESSAGE m;             // Variable pour recevoir des messages
    MESSAGE reponse;       // Variable pour envoyer des réponses

    char requete[200];     // Buffer pour les requêtes SQL
    char newUser[20];      // Buffer pour le nom d'un utilisateur
    MYSQL_RES* resultat;   // Résultat de requêtes MySQL
    MYSQL_ROW Tuple;       // Ligne d'un résultat MySQL
    int fdWpipe;

    fdWpipe = atoi(argv[1]);

    while(1) // Boucle principale pour traiter les requêtes
    {
        fprintf(stderr,"(CADDIE %d) attente de requete\n",getpid());

        // Réception d'un message depuis la file de messages
        if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
        {
            perror("(CADDIE) Erreur de msgrcv");
            exit(1);
        }

        // Gestion des requêtes reçues
        switch(m.requete)
        {
            case LOGIN: // Requête pour se connecter
                fprintf(stderr,"(CADDIE %d) Requete LOGIN reçue de %d\n",getpid(),m.expediteur);
                pidClient = m.expediteur;
                break;

            case LOGOUT: // Requête pour se déconnecter
                fprintf(stderr,"(CADDIE %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                mysql_close(connexion); // Fermeture de la connexion MySQL
                exit(5); // Quitte le programme
                break;

            case CONSULT: // Requête pour consulter des données
                fprintf(stderr,"(CADDIE %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);

                // Construction et exécution d'une requête SQL
                char table[11];
                strcpy(table, "UNIX_FINAL");
                sprintf(requete,"select * from %s where id = %d;", table, m.data1);

                if (mysql_query(connexion,requete) != 0) // Si l'exécution échoue
                {
                    fprintf(stderr, "Erreur de mysql_query: %s\n",mysql_error(connexion));
                    exit(1);
                }

                printf("Requete SELECT réussie.\n");

                // Récupération des résultats de la requête
                if ((resultat = mysql_store_result(connexion)) == NULL)
                {
                    fprintf(stderr, "Erreur de mysql_store_result: %s\n",mysql_error(connexion));
                    exit(1);
                }

                if((Tuple = mysql_fetch_row(resultat)) != NULL) // Si une ligne est trouvée
                {
                    // Remplit le message de réponse avec les données trouvées
                    reponse.type = m.expediteur;
                    reponse.expediteur = getpid();
                    reponse.requete = CONSULT;
                    reponse.data1 = atoi(Tuple[0]);
                    strcpy(reponse.data2, Tuple[1]);
                    strcpy(reponse.data3, Tuple[3]);
                    strcpy(reponse.data4, Tuple[4]);
                    reponse.data5 = atof(Tuple[2]);

                    // Envoie le message de réponse
                    if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                    {
                        perror("Erreur de msgsnd");
                        exit(1);
                    }

                    kill(reponse.type, SIGUSR1); // Envoie un signal au client
                }

                break;

            case ACHAT: // Requête pour acheter un article
                fprintf(stderr,"(CADDIE %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                break;

            case CADDIE: // Requête pour afficher le caddie
                fprintf(stderr,"(CADDIE %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                break;

            case CANCEL: // Requête pour annuler un article
                fprintf(stderr,"(CADDIE %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                break;

            case CANCEL_ALL: // Requête pour annuler tous les articles
                fprintf(stderr,"(CADDIE %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                break;

            case PAYER: // Requête pour payer le caddie
                fprintf(stderr,"(CADDIE %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);
                break;
        }
    }
}

void handlerSIGALRM(int sig) // Gestionnaire pour le signal SIGALRM
{
    fprintf(stderr,"(CADDIE %d) Time Out !!!\n",getpid());
    exit(0); // Termine le programme
}
