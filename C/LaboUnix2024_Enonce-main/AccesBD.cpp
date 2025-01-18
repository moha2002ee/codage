#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <mysql.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ;

int main(int argc, char *argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask, SIGINT);
  sigprocmask(SIG_SETMASK, &mask, NULL);

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr, "(ACCESBD %d) Recuperation de l'id de la file de messages\n", getpid());
  if ((idQ = msgget(CLE, 0)) == -1)
  {
    perror("(ACCESBD) Erreur de msgget");
    exit(1);
  }

  // Récupération descripteur lecture du pipe
  int fdRpipe = atoi(argv[1]);
  MYSQL *connexion;
  char requete[200];
  // Connexion à la base de donnée
  // TO DO
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion, "localhost", "Student", "PassStudent_1", "PourStudent", 0, 0, 0))
  {
    perror("(Acceebd) il y a un probleme avec la connexion a la bd ");
    exit(1);
  }

  MESSAGE m;
     MYSQL_ROW ligne;  
  ARTICLE reponse;
  MYSQL_RES *resultat;

  while (1)
  {
    // Lecture d'une requete sur le pipe
    // TO DO
    int lectureDeLaPipe = read(fdRpipe, &m, sizeof(MESSAGE));
    if (lectureDeLaPipe == 0)
    {
      fprintf(stderr, "c'est la fin de la communaction");
      mysql_close(connexion);
      exit(1);
    }
    switch (m.requete)
    {
    case CONSULT: // TO DO
      fprintf(stderr, "(ACCESBD %d) Requete CONSULT reçue de %d\n", getpid(), m.expediteur);
      // Acces BD
      fprintf(stderr, "(CADDIE %d) Requete CONSULT reçue de %d\n", getpid(), m.expediteur);

      // Construction et exécution d'une requête SQL
      char table[11];
      strcpy(table, "UNIX_FINAL");
      sprintf(requete, "select * from %s where id = %d;", table, m.data1);

      if (mysql_query(connexion, requete) != 0) // Si l'exécution échoue
      {
        fprintf(stderr, "Erreur de mysql_query: %s\n", mysql_error(connexion));
        exit(1);
      }

      printf("Requete SELECT réussie.\n");

      // Récupération des résultats de la requête
      if ((resultat = mysql_store_result(connexion)) == NULL)
      {
        fprintf(stderr, "Erreur de mysql_store_result: %s\n", mysql_error(connexion));
        mysql_free_result(resultat);
        reponse.stock = -1;
        exit(1);
      }

      if ((ligne = mysql_fetch_row(resultat)) != NULL) // Si une ligne est trouvée
      {
        // Remplit le message de réponse avec les données trouvées
        reponse.id = atoi(ligne[0]);
        strncpy(reponse.intitule, ligne[1], sizeof(reponse.intitule) - 1);
        reponse.intitule[sizeof(reponse.intitule) - 1] = '\0'; // Sécurise la chaîne
        reponse.prix = atof(ligne[2]);
        reponse.stock = atoi(ligne[3]);
        strncpy(reponse.image, ligne[4], sizeof(reponse.image) - 1);
        reponse.image[sizeof(reponse.image) - 1] = '\0'; // Sécurise la chaîne

        // Envoie le message de réponse
        if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
        {
          perror("Erreur de msgsnd");
          exit(1);
        }

      }
      // Preparation de la reponse

      // Envoi de la reponse au bon caddie
      break;

    case ACHAT: // TO DO
      fprintf(stderr, "(ACCESBD %d) Requete ACHAT reçue de %d\n", getpid(), m.expediteur);
      // Acces BD

      // Finalisation et envoi de la reponse
      break;

    case CANCEL: // TO DO
      fprintf(stderr, "(ACCESBD %d) Requete CANCEL reçue de %d\n", getpid(), m.expediteur);
      // Acces BD

      // Mise à jour du stock en BD
      break;
    }
  }
}
