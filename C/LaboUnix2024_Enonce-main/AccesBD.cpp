#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <mysql.h>
#include "protocole.h" // Contient la clé et la structure d'un message

int idQ;
MYSQL *connexion;

// Déclarations des fonctions
void traiterConsultation(MESSAGE message);
void traiterAchat(MESSAGE message);
void traiterAnnulation(MESSAGE message);

int main(int argc, char *argv[])
{
  // Masquage du signal SIGINT
  sigset_t mask;
  sigaddset(&mask, SIGINT);
  sigprocmask(SIG_SETMASK, &mask, NULL);

  // Récupération de l'identifiant de la file de messages
  fprintf(stderr, "(ACCESBD %d) Récupération de l'id de la file de messages\n", getpid());
  if ((idQ = msgget(CLE, 0)) == -1)
  {
    perror("(ACCESBD) Erreur de msgget");
    exit(1);
  }

  // Récupération du descripteur de lecture du pipe
  int fdRpipe = atoi(argv[1]);
  fprintf(stderr, "(ACCESBD %d) Descripteur de pipe reçu : %d\n", getpid(), fdRpipe);

  // Connexion à la base de données
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion, "localhost", "Student", "PassStudent1_", "PourStudent", 0, 0, 0) == NULL)
  {
    fprintf(stderr, "(SERVEUR) Erreur de connexion à la base de données...\n");
    exit(1);
  }

  MESSAGE message;
  int resultatLecture;

  while (1)
  {
    // Lecture d'une requête sur le pipe
    fprintf(stderr, "(ACCESBD %d) Attente d'une requête...\n", getpid());

    if ((resultatLecture = read(fdRpipe, &message, sizeof(MESSAGE))) < 0)
    {
      perror("(ACCESBD) Erreur de lecture");
      exit(1);
    }

    if (resultatLecture == 0)
    {
      // Fin de lecture
      mysql_close(connexion);
      close(fdRpipe);
      exit(1);
    }

    // Traitement de la requête
    switch (message.requete)
    {
    case CONSULT:
      traiterConsultation(message);
      break;
    case ACHAT:
      traiterAchat(message);
      break;
    case CANCEL:
      traiterAnnulation(message);
      break;
    default:
      fprintf(stderr, "(ACCESBD %d) Requête inconnue reçue\n", getpid());
      break;
    }
  }
}

// Fonction pour gérer la requête CONSULT
void traiterConsultation(MESSAGE message)
{
  fprintf(stderr, "(ACCESBD %d) Requête CONSULT reçue de %d\n", getpid(), message.expediteur);

  char requeteSQL[200];
  MYSQL_RES *resultatRequete;
  MYSQL_ROW ligneResultat;
  MESSAGE reponse;

  // Construction de la requête SQL
  sprintf(requeteSQL, "select * from UNIX_FINAL where id = %d;", message.data1);

  if (mysql_query(connexion, requeteSQL) != 0)
  {
    fprintf(stderr, "Erreur de mysql_query: %s\n", mysql_error(connexion));
    exit(1);
  }

  printf("Requête SELECT réussie.\n");

  // Récupération du résultat
  if ((resultatRequete = mysql_store_result(connexion)) == NULL)
  {
    fprintf(stderr, "Erreur de mysql_store_result: %s\n", mysql_error(connexion));
    exit(1);
  }

  // Préparation de la réponse
  if ((ligneResultat = mysql_fetch_row(resultatRequete)) != NULL)
  {
    reponse.type = message.expediteur;
    reponse.expediteur = getpid();
    reponse.requete = CONSULT;
    reponse.data1 = atoi(ligneResultat[0]);
    strcpy(reponse.data2, ligneResultat[1]);
    strcpy(reponse.data3, ligneResultat[3]);
    strcpy(reponse.data4, ligneResultat[4]);
    reponse.data5 = atof(ligneResultat[2]);
  }
  else
  {
    reponse.type = message.expediteur;
    reponse.expediteur = getpid();
    reponse.requete = CONSULT;
    reponse.data1 = -1;
  }

  // Envoi de la réponse
  if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
  {
    perror("Erreur de msgsnd");
    exit(1);
  }
}

// Fonction pour gérer la requête ACHAT
void traiterAchat(MESSAGE message)
{
  fprintf(stderr, "(ACCESBD %d) Requête ACHAT reçue de %d\n", getpid(), message.expediteur);

  char requeteSQL[200];
  MYSQL_RES *resultatRequete;
  MYSQL_ROW ligneResultat;
  MESSAGE reponse;

  // Construction de la requête SQL
  sprintf(requeteSQL, "select * from UNIX_FINAL where id = %d;", message.data1);

  if (mysql_query(connexion, requeteSQL) != 0)
  {
    fprintf(stderr, "Erreur de mysql_query: %s\n", mysql_error(connexion));
    exit(1);
  }

  printf("Requête SELECT réussie.\n");

  // Récupération du résultat
  if ((resultatRequete = mysql_store_result(connexion)) == NULL)
  {
    fprintf(stderr, "Erreur de mysql_store_result: %s\n", mysql_error(connexion));
    exit(1);
  }

  // Préparation de la réponse
  if ((ligneResultat = mysql_fetch_row(resultatRequete)) != NULL)
  {
    int quantiteStock = atoi(ligneResultat[3]), quantiteDemandee = atoi(message.data2);

    if (quantiteDemandee <= quantiteStock)
    {
      reponse.type = message.expediteur;
      reponse.expediteur = getpid();
      reponse.requete = ACHAT;
      reponse.data1 = atoi(ligneResultat[0]);
      strcpy(reponse.data2, ligneResultat[1]);
      strcpy(reponse.data3, message.data2);
      strcpy(reponse.data4, ligneResultat[4]);
      reponse.data5 = atof(ligneResultat[2]);

      // Mise à jour du stock
      int nouveauStock = quantiteStock - quantiteDemandee;
      sprintf(requeteSQL, "update UNIX_FINAL set stock = %d where id = %d;", nouveauStock, message.data1);

      if (mysql_query(connexion, requeteSQL) != 0)
      {
        fprintf(stderr, "Erreur de mysql_query: %s\n", mysql_error(connexion));
        exit(1);
      }

      printf("Requête UPDATE réussie.\n");
    }
    else
    {
      reponse.type = message.expediteur;
      reponse.expediteur = getpid();
      reponse.requete = ACHAT;
      reponse.data1 = atoi(ligneResultat[0]);
      strcpy(reponse.data2, ligneResultat[1]);
      strcpy(reponse.data3, "0");
      strcpy(reponse.data4, ligneResultat[4]);
      reponse.data5 = atof(ligneResultat[2]);
    }
  }

  // Envoi de la réponse
  if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
  {
    perror("Erreur de msgsnd");
    exit(1);
  }
}

// Fonction pour gérer la requête CANCEL
void traiterAnnulation(MESSAGE message)
{
  fprintf(stderr, "(ACCESBD %d) Requête CANCEL reçue de %d\n", getpid(), message.expediteur);

  // À implémenter : Mise à jour du stock en BD
}