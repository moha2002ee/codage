#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "FichierClient.h" // contient les fonctions pour le fichier client
#include "protocole.h" // contient la cle et la structure d'un message

void handlerSIGINT(int sig);
void copieChaine(const char* aCopier, char* endroitCopie); //Va permettre de copier les chaines envoyer dans la fonction constructeurRequete et controle les tailles

MESSAGE constructeurRequete(int nbElem, long type, int expediteur, int typeRequete, int data1, const char* data2, const char* data3, const char* data4, float data5);
/*Permet de construire une requete en envoyent d'abord le Nombre d'élements que contiendra la requete suivie des champs correspondant dans la structure
!!! ATTENTION Les champs non utulisées doivent etre a NULL ou nullptr (pour les char*)*/

void utilisationTableConnexions(MESSAGE *pM,MESSAGE* pReponse);
/*On utulisera ici une fonction qui traitera les requete afin de ne pas faire de boucle dans chaque CASE de la fonction main on passera donc l'adresse du message *pM
et de la reponse *pReponse*/

void login(MESSAGE* message, MESSAGE* reponse); //fonction login va gerer la demande de LOGIN du client et faire les vérifications
void envoiRequete(MESSAGE* pReponse); //fonction qui gere l'envoi de toute les requetes
void envoiSignal(int pid, int typeSignal); //fonction qui gere l'envoi d'un signal a un processus
void reponseLOGIN(MESSAGE reponse, int typeClient, int typeRequete, const char* rep); //initialise le type de reponse a renvoyer au client pour le LOGIN

int idQ,idShm,idSem;
int fdPipe[2];
TAB_CONNEXIONS *tab;

void afficheTab();

int main()
{
  // Armement des signaux
  // TO DO
  fprintf(stderr,"Armement du signal SIGINT\n");
  struct sigaction A;
  A.sa_handler = handlerSIGINT;
  A.sa_flags = 0;
  sigemptyset(&A.sa_mask);
  sigaction(SIGINT,&A,NULL);

  // Creation des ressources
  // Creation de la file de message
  fprintf(stderr,"(SERVEUR %d) Creation de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,IPC_CREAT | IPC_EXCL | 0600)) == -1)  // CLE definie dans protocole.h
  {
    perror("(SERVEUR) Erreur de msgget");
    exit(1);
  }

  // TO BE CONTINUED

  // Creation du pipe
  // TO DO

  // Initialisation du tableau de connexions
  tab = (TAB_CONNEXIONS*) malloc(sizeof(TAB_CONNEXIONS)); 

  for (int i=0 ; i<6 ; i++)
  {
    tab->connexions[i].pidFenetre = 0;
    strcpy(tab->connexions[i].nom,"");
    tab->connexions[i].pidCaddie = 0;
  }
  tab->pidServeur = getpid();
  tab->pidPublicite = 0;

  afficheTab();

  // Creation du processus Publicite (étape 2)
  // TO DO

  // Creation du processus AccesBD (étape 4)
  // TO DO

  MESSAGE m;
  MESSAGE reponse;

  while(1)
  {
  	fprintf(stderr,"(SERVEUR %d) Attente d'une requete...\n",getpid());
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),1,0) == -1)
    {
      perror("(SERVEUR) Erreur de msgrcv");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }

    switch(m.requete)
    {
      case CONNECT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONNECT reçue de %d\n",getpid(),m.expediteur);

                      utilisationTableConnexions(&m, &reponse); //On va traiter la requete dans la boucle de la fonction 
                      break;

      case DECONNECT : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete DECONNECT reçue de %d\n",getpid(),m.expediteur);

                      utilisationTableConnexions(&m, &reponse);
                      break;

      case LOGIN :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete LOGIN reçue de %d : --%d--%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2,m.data3);

                      //gestion du login
                      login(&m, &reponse);

                      //envoi de la reponse au client
                      envoiRequete(&reponse);

                      //envoi du signal SIGUSR1 pour faire afficher le message dans data4
                      envoiSignal(reponse.type, SIGUSR1);
                      break; 

      case LOGOUT :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);

                      utilisationTableConnexions(&m, &reponse);
                      break;

      case UPDATE_PUB :  // TO DO
                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case PAYER : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);
                      break;

      case NEW_PUB :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);
                      break;
    }
    afficheTab();
  }
}

void afficheTab()
{
  fprintf(stderr,"Pid Serveur   : %d\n",tab->pidServeur);
  fprintf(stderr,"Pid Publicite : %d\n",tab->pidPublicite);
  fprintf(stderr,"Pid AccesBD   : %d\n",tab->pidAccesBD);
  for (int i=0 ; i<6 ; i++)
    fprintf(stderr,"%6d -%20s- %6d\n",tab->connexions[i].pidFenetre,
                                                      tab->connexions[i].nom,
                                                      tab->connexions[i].pidCaddie);
  fprintf(stderr,"\n");
}

void utilisationTableConnexions(MESSAGE *pM,MESSAGE* pReponse)
{
  for(int i = 0; i < 6; i++){
    switch(pM->requete)
    {
      case CONNECT :  
                      if(tab->connexions[i].pidFenetre == 0)
                      {
                        tab->connexions[i].pidFenetre = pM->expediteur;  //On attribue le pid de l'expediteur (client) a une ligne de la table de connexions

                        i = 10;  //On casse la boucle
                      }

      case DECONNECT :
                      if(tab->connexions[i].pidFenetre == pM->expediteur)
                      {
                        tab->connexions[i].pidFenetre = 0;  // 0 car si la valeur = 0 le champs = vide

                        i = 10;
                      }   
                      break;

      case LOGIN :    
                    if(tab->connexions[i].pidFenetre == pM->expediteur)
                    {
                      if(pReponse->data1 == 1)
                      {
                        strcpy(tab->connexions[i].nom,pM->data2);
                        pReponse->expediteur = pM->expediteur;
                        envoiRequete(pReponse);
                      }
  
                      pReponse->type = pM->expediteur;
  
                      i = 10;
                    }
                    break; 

      case LOGOUT :   
                    if(tab->connexions[i].pidFenetre == pM->expediteur)
                    {
                      pReponse->requete = LOGOUT;
                      if(pReponse->type != 0)
                      envoiRequete(pReponse);
                      strcpy(tab->connexions[i].nom,"");
                      
                      i = 10;
                    }
                      break;

      case UPDATE_PUB :  // TO DO
                      break;

      case CONSULT :  // TO DO
                      break;

      case ACHAT :    // TO DO
                      break;

      case CADDIE :   // TO DO
                      break;

      case CANCEL :   // TO DO
                      break;

      case CANCEL_ALL : // TO DO
                      break;

      case PAYER : // TO DO
                      break;

      case NEW_PUB :  // TO DO
                      break;
    }
    
  }
}

void login(MESSAGE *pM, MESSAGE *pReponse)
{
  if(pM->data1 == 1){
    if(estPresent(pM->data2))
    {
      reponseLOGIN(pReponse, 0, LOGIN, "---Client deja existant !---");
      utilisationTableConnexions(pM, pReponse);
    }
    else
    { 
      ajouteClient(pM->data2, pM->data3);
      reponseLOGIN(pReponse, 1, LOGIN, "---Nouvel client cree: bienvenue !---");
      utilisationTableConnexions(pM, pReponse);

    }
  }
  else
  {
    int position = estPresent(pM->data2);
    if(position)
    {
      if(verifieMotDePasse(position, pM->data3))
      {
        reponseLOGIN(pReponse, 1, LOGIN, "---Re-bonjour cher client !---");
        utilisationTableConnexions(pM, pReponse);

      }
      else
      {
        reponseLOGIN(pReponse, 0, LOGIN, "---Mot de passe incorrect---");
        utilisationTableConnexions(pM, pReponse);

      }
        
    }
    else
    {
      reponseLOGIN(pReponse, 0, LOGIN, "---Utilisateur inconnu---");
      utilisationTableConnexions(pM, pReponse);

    }
      
  }
  
  
}

void reponseLOGIN(MESSAGE* pReponse, int typeClient, int typeRequete, const char* rep)
{
  pReponse->requete = typeRequete;
  pReponse->data1 = typeClient;
  strcpy(pReponse->data4, rep);
}

void envoiRequete(MESSAGE* pReponse)
{
  
  if (msgsnd(idQ, pReponse, sizeof(MESSAGE) - sizeof(long), 0) == -1) 
  {
    perror("(SERVEUR)Erreur envoi reponse\n");
    exit(1);
  }
}

void envoiSignal(int pid, int typeSignal)
{
  if(kill(pid, typeSignal) == -1) 
  {
    perror("erreur KILL");
    exit(1);
  }
}

void copieChaine(const char* aCopier, char* endroitCopie)
{
        if (strlen(aCopier) <= sizeof(endroitCopie) - 1) 
        { 
            strcpy(endroitCopie, aCopier);
        } 

        else 
        {
            fprintf(stderr, "Erreur :dépasse la taille maximale (caractères autorisés).\n");
        }
}

MESSAGE constructeurRequete(int nbElem, long type, int expediteur, int typeRequete, int data1, const char* data2, const char* data3, const char* data4, float data5)
{
    MESSAGE tmp;

    if (nbElem >= 3) 
    {
        tmp.type = type;
        tmp.expediteur = expediteur;
        tmp.requete = typeRequete;
    }

    if (nbElem >= 4) 
    {
        tmp.data1 = data1;
    }

    if (nbElem >= 5) 
    {
        copieChaine(data2, tmp.data2);
    }

    if (nbElem >= 6) 
    {
        copieChaine(data3, tmp.data3);
    }

    if (nbElem >= 7) 
    {
        copieChaine(data4, tmp.data4);
    }

    if (nbElem >= 8) 
    {
        tmp.data5 = data5;
    }

    return tmp;
}

void handlerSIGINT(int sig)
{
  fprintf(stderr,"\nSuppression de la file de message (%d)\n", idQ);

  kill(tab->pidPublicite, SIGINT);
  if (msgctl(idQ,IPC_RMID,NULL) ==-1)
  {
   perror("(SERVEUR)Erreur de msgctl(2), File de message non supprimé\n");
   exit(1);
  }
  
  /*PARTIE A DECOMMENTER PLUS TARD*/
  /*
  fprintf(stderr, "Supression de la mémoire partagée\n");

  if (shmctl(idShm, IPC_RMID, NULL) == -1)
  {
    perror("(SERVEUR)Erreur de suppresion de la mémoire partagée\n");
    exit(1);
  }

  // Fermeture du pipe
  if (close(fdPipe[0]) ==-1)
  {
    perror("Erreur fermeture sortie du pipe");
    exit(1);
  } 

  if (close(fdPipe[1]) ==-1)
  {
    perror("Erreur fermeture entree du pipe");
    exit(1);
  }
  */

}