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

ARTICLE articles[10];
int nbArticles = 0;

int fdWpipe;
int pidClient;

MYSQL* connexion;

void handlerSIGALRM(int sig);

int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Armement des signaux
  // TO DO

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CADDIE %d) Recuperation de l'id de la file de messages\n", getpid());

  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(CADDIE) Erreur de msgget");
    exit(1);
  }

  MESSAGE m;
  MESSAGE reponse;
  int client, i, k;

  //initialiser vecteur articles avec des valeurs temporaire
  for(i = 0; i < 10; i++)
  {
    articles[i].id = 0; //0 utiliser pour indiquer une place libre
  }

  // Récupération descripteur écriture du pipe
  fdWpipe = atoi(argv[1]);

  while(1)
  {
    fprintf(stderr,"(CADDIE %d) attente de requete\n",getpid());

    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
    {
      perror("(CADDIE) Erreur de msgrcv");
      exit(1);
    }

    switch(m.requete)
    {
      case LOGIN :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGIN reçue de %d\n",getpid(),m.expediteur);
                      break;

      case LOGOUT :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);

                      //mysql_close(connexion); //Déconnexion de la base de données

                      close(fdWpipe);

                      exit(5);

                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);

                      // Construction et exécution de la requête
                      client = m.expediteur;

                      m.expediteur = getpid();

                      if (write(fdWpipe,&m,sizeof(MESSAGE)) != sizeof(MESSAGE)) return -1;

                      fprintf(stderr,"(CADDIE %d) attente pour le base de donnes\n",getpid());

                      if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
                      {
                        perror("(CADDIE) Erreur de msgrcv");
                        exit(1);
                      }

                      if(m.data1 != -1)
                      {
                        m.type = client;
                        m.expediteur = getpid();

                        if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                        {
                            perror("Erreur de msgsnd");
                            exit(1);
                        }

                        kill(m.type, SIGUSR1);
                      }
                      
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                      // on transfert la requete à AccesBD
                      client = m.expediteur;

                      m.expediteur = getpid();

                      if (write(fdWpipe,&m,sizeof(MESSAGE)) != sizeof(MESSAGE))
                      {
                        perror("(CADDIE) Erreur de read");
                        exit(1);
                      }
                      
                      // on attend la réponse venant de AccesBD

                      fprintf(stderr,"(CADDIE %d) attente pour le base de donnes\n",getpid());

                      if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
                      {
                        perror("(CADDIE) Erreur de msgrcv");
                        exit(1);
                      }
                        
                      // Envoi de la reponse au client

                      if(atoi(m.data3) != 0)
                      {
                        if(nbArticles < 10)
                        {
                          i = 0;

                          while(articles[i].id != 0 && i < 10) i++; //trouve le premier place libre

                          articles[i].id = m.data1;
                          strcpy(articles[i].intitule, m.data2);
                          articles[i].prix = m.data5;
                          articles[i].stock = atoi(m.data3);
                          strcpy(articles[i].image, m.data4);

                          nbArticles++;
                        }
                      }

                      m.type = client;
                      m.expediteur = getpid();

                      if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                      {
                          perror("Erreur de msgsnd");
                          exit(1);
                      }

                      kill(m.type, SIGUSR1);

                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);

                      m.type = m.expediteur;
                      m.expediteur = getpid();

                      k = 0;

                      for(i = 0; i < 10 && k < nbArticles; i++)
                      {
                        if(articles[i].id != 0)
                        {
                          m.data1 = articles[i].id;
                          strcpy(m.data2, articles[i].intitule);
                          m.data5 = articles[i].prix;
                          sprintf(m.data3, "%d", articles[i].stock);
                          strcpy(m.data4, articles[i].image);

                          if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                          {
                              perror("Erreur de msgsnd");
                              exit(1);
                          }

                          kill(m.type, SIGUSR1);

                          //sleep(1);

                          k++;

                          fprintf(stderr,"(CADDIE %d) Requete CADDIE envoyer au %d\n",getpid(),m.type);

                          printf("Voici le article: \n");
                          printf("%d ; %s ; %f ; %d\n", articles[i].id, articles[i].intitule, articles[i].prix, articles[i].stock);
                        }
                      }

                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);

                      // on transmet la requete à AccesBD

                      // Suppression de l'aricle du panier
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);

                      // On envoie a AccesBD autant de requeres CANCEL qu'il y a d'articles dans le panier

                      // On vide le panier
                      break;

      case PAYER :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);

                      // On vide le panier
                      break;
    }
  }
}

void handlerSIGALRM(int sig)
{
  fprintf(stderr,"(CADDIE %d) Time Out !!!\n",getpid());

  // Annulation du caddie et mise à jour de la BD
  // On envoie a AccesBD autant de requetes CANCEL qu'il y a d'articles dans le panier

  // Envoi d'un Time Out au client (s'il existe toujours)
         
  exit(0);
}