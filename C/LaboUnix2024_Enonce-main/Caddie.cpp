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

ARTICLE articles[10];
int nbArticle = 0;

int fdWpipe;
int pidClient;

MYSQL *connexion;

void gestionnaireSIGALRM(int signal);

void traiterConnexion(MESSAGE message);
void traiterDeconnexion(MESSAGE message);
void traiterConsultation(MESSAGE message);
void traiterAchat(MESSAGE message);
void traiterAffichagearticles(MESSAGE message);
void traiterAnnulation(MESSAGE message);
void traiterAnnulationTout(MESSAGE message);
void traiterPaiement(MESSAGE message);

int main(int argc, char *argv[])
{
  // Masquage de SIGINT
  sigset_t masque;
  sigaddset(&masque, SIGINT);
  sigprocmask(SIG_SETMASK, &masque, NULL);

  // Armement des signaux
  // À implémenter

  // Récupération de l'identifiant de la file de messages
  fprintf(stderr, "(CADDIE %d) Récupération de l'id de la file de messages\n", getpid());

  if ((idQ = msgget(CLE, 0)) == -1)
  {
    perror("(CADDIE) Erreur de msgget");
    exit(1);
  }

  MESSAGE message;

  // Initialisation du articles avec des valeurs temporaires
  for (int i = 0; i < 10; i++)
  {
    articles[i].id = 0; // 0 utilisé pour indiquer une place libre
  }

  // Récupération du descripteur d'écriture du pipe
  fdWpipe = atoi(argv[1]);

  while (1)
  {
    fprintf(stderr, "(CADDIE %d) Attente de requête\n", getpid());

    if (msgrcv(idQ, &message, sizeof(MESSAGE) - sizeof(long), getpid(), 0) == -1)
    {
      perror("(CADDIE) Erreur de msgrcv");
      exit(1);
    }

    switch (message.requete)
    {
    case LOGIN:
      traiterConnexion(message);
      break;
    case LOGOUT:
      traiterDeconnexion(message);
      break;
    case CONSULT:
      traiterConsultation(message);
      break;
    case ACHAT:
      traiterAchat(message);
      break;
    case CADDIE:
      traiterAffichagearticles(message);
      break;
    case CANCEL:
      traiterAnnulation(message);
      break;
    case CANCEL_ALL:
      traiterAnnulationTout(message);
      break;
    case PAYER:
      traiterPaiement(message);
      break;
    }
  }
}

void gestionnaireSIGALRM(int signal)
{
  fprintf(stderr, "(CADDIE %d) Time Out !!!\n", getpid());

  // Annulation du articles et mise à jour de la base de données
  // On envoie à AccesBD autant de requêtes CANCEL qu'il y a d'articles dans le articles

  // Envoi d'un Time Out au client (s'il existe toujours)

  exit(0);
}

void traiterConnexion(MESSAGE message)
{
  fprintf(stderr, "(CADDIE %d) Requête LOGIN reçue de %d\n", getpid(), message.expediteur);
  // À implémenter
}

void traiterDeconnexion(MESSAGE message)
{
  fprintf(stderr, "(CADDIE %d) Requête LOGOUT reçue de %d\n", getpid(), message.expediteur);

  // mysql_close(connexion); // Déconnexion de la base de données

  close(fdWpipe);

  exit(5);
}

void traiterConsultation(MESSAGE message)
{
  fprintf(stderr, "(CADDIE %d) Requête CONSULT reçue de %d\n", getpid(), message.expediteur);

  // Construction et exécution de la requête
  int client = message.expediteur;

  message.expediteur = getpid();

  if (write(fdWpipe, &message, sizeof(MESSAGE)) != sizeof(MESSAGE))
    return;

  fprintf(stderr, "(CADDIE %d) Attente de la réponse de la base de données\n", getpid());

  if (msgrcv(idQ, &message, sizeof(MESSAGE) - sizeof(long), getpid(), 0) == -1)
  {
    perror("(CADDIE) Erreur de msgrcv");
    exit(1);
  }

  if (message.data1 != -1)
  {
    message.type = client;
    message.expediteur = getpid();

    if (msgsnd(idQ, &message, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
      perror("Erreur de msgsnd");
      exit(1);
    }

    kill(message.type, SIGUSR1);
  }
}

void traiterAchat(MESSAGE message)
{
  fprintf(stderr, "(CADDIE %d) Requête ACHAT reçue de %d\n", getpid(), message.expediteur);

  // On transfère la requête à AccesBD
  int client = message.expediteur;

  message.expediteur = getpid();

  if (write(fdWpipe, &message, sizeof(MESSAGE)) != sizeof(MESSAGE))
  {
    perror("(CADDIE) Erreur de read");
    exit(1);
  }

  // On attend la réponse venant de AccesBD

  fprintf(stderr, "(CADDIE %d) Attente de la réponse de la base de données\n", getpid());

  if (msgrcv(idQ, &message, sizeof(MESSAGE) - sizeof(long), getpid(), 0) == -1)
  {
    perror("(CADDIE) Erreur de msgrcv");
    exit(1);
  }

  // Envoi de la réponse au client

  if (atoi(message.data3) != 0)
  {
    if (nbArticle < 10)
    {
      int i = 0;

      while (articles[i].id != 0 && i < 10)
        i++; // Trouve la première place libre

      articles[i].id = message.data1;
      strcpy(articles[i].intitule, message.data2);
      articles[i].prix = message.data5;
      articles[i].stock = atoi(message.data3);
      strcpy(articles[i].image, message.data4);

      nbArticle++;
    }
  }

  message.type = client;
  message.expediteur = getpid();

  if (msgsnd(idQ, &message, sizeof(MESSAGE) - sizeof(long), 0) == -1)
  {
    perror("Erreur de msgsnd");
    exit(1);
  }

  kill(message.type, SIGUSR1);
}

void traiterAffichagearticles(MESSAGE message)
{
  fprintf(stderr, "(CADDIE %d) Requête CADDIE reçue de %d\n", getpid(), message.expediteur);

  message.type = message.expediteur;
  message.expediteur = getpid();

  int k = 0;

  for (int i = 0; i < 10 && k < nbArticle; i++)
  {
    if (articles[i].id != 0)
    {
      message.data1 = articles[i].id;
      strcpy(message.data2, articles[i].intitule);
      message.data5 = articles[i].prix;
      sprintf(message.data3, "%d", articles[i].stock);
      strcpy(message.data4, articles[i].image);

      if (msgsnd(idQ, &message, sizeof(MESSAGE) - sizeof(long), 0) == -1)
      {
        perror("Erreur de msgsnd");
        exit(1);
      }

      kill(message.type, SIGUSR1);

      k++;

      fprintf(stderr, "(CADDIE %d) Requête CADDIE envoyée à %d\n", getpid(), message.type);

      printf("Voici l'article : \n");
      printf("%d ; %s ; %f ; %d\n", articles[i].id, articles[i].intitule, articles[i].prix, articles[i].stock);
    }
  }
}

void traiterAnnulation(MESSAGE message)
{
  fprintf(stderr, "(CADDIE %d) Requête CANCEL reçue de %d\n", getpid(), message.expediteur);

  // On transmet la requête à AccesBD

  // Suppression de l'article du articles
}

void traiterAnnulationTout(MESSAGE message)
{
  fprintf(stderr, "(CADDIE %d) Requête CANCEL_ALL reçue de %d\n", getpid(), message.expediteur);

  // On envoie à AccesBD autant de requêtes CANCEL qu'il y a d'articles dans le articles

  // On vide le articles
}

void traiterPaiement(MESSAGE message)
{
  fprintf(stderr, "(CADDIE %d) Requête PAYER reçue de %d\n", getpid(), message.expediteur);

  // On vide le articles
}