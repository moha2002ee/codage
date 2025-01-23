#include "windowclient.h"
#include "ui_windowclient.h"
#include <QMessageBox>
#include <string>
using namespace std;

#include "protocole.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>

extern WindowClient *w;

int idQ, idShm;
bool logged;
char* pShm;
ARTICLE articleEnCours;
float totalCaddie = 0.0;

void handlerSIGUSR1(int sig);
void handlerSIGUSR2(int sig);

#define REPERTOIRE_IMAGES "images/"

WindowClient::WindowClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowClient)
{
    ui->setupUi(this);

    // Configuration de la table du panier (ne pas modifer)
    ui->tableWidgetPanier->setColumnCount(3);
    ui->tableWidgetPanier->setRowCount(0);
    QStringList labelsTablePanier;
    labelsTablePanier << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetPanier->setHorizontalHeaderLabels(labelsTablePanier);
    ui->tableWidgetPanier->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetPanier->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetPanier->horizontalHeader()->setVisible(true);
    ui->tableWidgetPanier->horizontalHeader()->setDefaultSectionSize(160);
    ui->tableWidgetPanier->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetPanier->verticalHeader()->setVisible(false);
    ui->tableWidgetPanier->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Recuperation de l'identifiant de la file de messages
    //fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la file de messages\n",getpid());
    // TO DO
    if ((idQ = msgget(CLE,0)) == -1)
    {
        perror("Erreur de msgget");
        exit(1);
    }

    // Recuperation de l'identifiant de la mémoire partagée
    fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la mémoire partagée\n",getpid());
    // TO DO
    if ((idShm = shmget(CLE,0,0)) == -1)
    {
        perror("Erreur de shmget");
        exit(1);
    }

    // Attachement à la mémoire partagée
    // TO DO
    if ((pShm = (char*)shmat(idShm,NULL,0)) == (char*)-1)
    {
        perror("Erreur de shmat");
        exit(1);
    }

    setPublicite(pShm); //pour mettre le publicite quand le programme il demarre est ne pas attendre une seconde pour recevoir le pub

    // Armement des signaux
    // TO DO
    //fprintf(stderr, "ARMEMENT SIGUSR1\n");
    struct sigaction B;
    B.sa_handler = handlerSIGUSR1;
    B.sa_flags = 0;
    sigemptyset(&B.sa_mask);
    sigaction(SIGUSR1,&B,NULL);

    struct sigaction A;
    A.sa_handler = handlerSIGUSR2;
    A.sa_flags = 0;
    sigemptyset(&A.sa_mask);
    sigaction(SIGUSR2,&A,NULL);

    // Envoi d'une requete de connexion au serveur
    // TO DO
    MESSAGE m;

    m.type = 1;
    m.expediteur = getpid();
    m.requete = CONNECT;

    if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
        perror("Erreur de msgsnd");
        exit(1);
    }

    //fprintf(stderr,"(Client %d) connection envoyer...\n",getpid());

    // Exemples à supprimer
    //setPublicite("Promotions sur les concombres !!!");
    //setArticle("pommes",5.53,18,"pommes.jpg");
    //ajouteArticleTablePanier("cerises",8.96,2);
}

WindowClient::~WindowClient()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNom()
{
  strcpy(nom,ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setImage(const char* image)
{
  // Met à jour l'image
  char cheminComplet[80];
  sprintf(cheminComplet,"%s%s",REPERTOIRE_IMAGES,image);
  QLabel* label = new QLabel();
  label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  label->setScaledContents(true);
  QPixmap *pixmap_img = new QPixmap(cheminComplet);
  label->setPixmap(*pixmap_img);
  label->resize(label->pixmap()->size());
  ui->scrollArea->setWidget(label);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::isNouveauClientChecked()
{
  if (ui->checkBoxNouveauClient->isChecked()) return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setArticle(const char* intitule,float prix,int stock,const char* image)
{
  ui->lineEditArticle->setText(intitule);
  if (prix >= 0.0)
  {
    char Prix[20];
    sprintf(Prix,"%.2f",prix);
    ui->lineEditPrixUnitaire->setText(Prix);
  }
  else ui->lineEditPrixUnitaire->clear();
  if (stock >= 0)
  {
    char Stock[20];
    sprintf(Stock,"%d",stock);
    ui->lineEditStock->setText(Stock);
  }
  else ui->lineEditStock->clear();
  setImage(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getQuantite()
{
  return ui->spinBoxQuantite->value();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setTotal(float total)
{
  if (total >= 0.0)
  {
    char Total[20];
    sprintf(Total,"%.2f",total);
    ui->lineEditTotal->setText(Total);
  }
  else ui->lineEditTotal->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::loginOK()
{
  ui->pushButtonLogin->setEnabled(false);
  ui->pushButtonLogout->setEnabled(true);
  ui->lineEditNom->setReadOnly(true);
  ui->lineEditMotDePasse->setReadOnly(true);
  ui->checkBoxNouveauClient->setEnabled(false);

  ui->spinBoxQuantite->setEnabled(true);
  ui->pushButtonPrecedent->setEnabled(true);
  ui->pushButtonSuivant->setEnabled(true);
  ui->pushButtonAcheter->setEnabled(true);
  ui->pushButtonSupprimer->setEnabled(true);
  ui->pushButtonViderPanier->setEnabled(true);
  ui->pushButtonPayer->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::logoutOK()
{
  ui->pushButtonLogin->setEnabled(true);
  ui->pushButtonLogout->setEnabled(false);
  ui->lineEditNom->setReadOnly(false);
  ui->lineEditMotDePasse->setReadOnly(false);
  ui->checkBoxNouveauClient->setEnabled(true);

  ui->spinBoxQuantite->setEnabled(false);
  ui->pushButtonPrecedent->setEnabled(false);
  ui->pushButtonSuivant->setEnabled(false);
  ui->pushButtonAcheter->setEnabled(false);
  ui->pushButtonSupprimer->setEnabled(false);
  ui->pushButtonViderPanier->setEnabled(false);
  ui->pushButtonPayer->setEnabled(false);

  setNom("");
  setMotDePasse("");
  ui->checkBoxNouveauClient->setCheckState(Qt::CheckState::Unchecked);

  setArticle("",-1.0,-1,"");

  w->videTablePanier();
  totalCaddie = 0.0;
  w->setTotal(-1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du panier (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteArticleTablePanier(const char* article,float prix,int quantite)
{
    char Prix[20],Quantite[20];

    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetPanier->rowCount();
    nbLignes++;
    ui->tableWidgetPanier->setRowCount(nbLignes);
    ui->tableWidgetPanier->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetPanier->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetPanier->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetPanier->setItem(nbLignes-1,2,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::videTablePanier()
{
    ui->tableWidgetPanier->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetPanier->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{
    // TO DO (étape 1)
    MESSAGE m;

    // envoi d'un logout si logged
    if(logged == true)
    {
        m.type = 1;
        m.expediteur = getpid();
        m.requete = LOGOUT;

        if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
        {
            perror("Erreur de msgsnd");
            exit(1);
        }
    }

    // Envoi d'une requete DECONNECT au serveur
    m.type = 1;
    m.expediteur = getpid();
    m.requete = DECONNECT;

    if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
        perror("Erreur de msgsnd");
        exit(1);
    }

    // Envoi d'une requete de deconnexion au serveur

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogin_clicked()
{
    // Envoi d'une requete de login au serveur
    // TO DO
    MESSAGE m;

    m.type = 1;
    m.expediteur = getpid();
    m.requete = LOGIN;
    strcpy(m.data2, getNom());
    strcpy(m.data3, getMotDePasse());
    m.data1 = isNouveauClientChecked();

    if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
        perror("Erreur de msgsnd");
        exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogout_clicked()
{
    // Envoi d'une requete CANCEL_ALL au serveur (au cas où le panier n'est pas vide)
    // TO DO

    // Envoi d'une requete de logout au serveur
    // TO DO

    MESSAGE m;

    m.type = 1;
    m.expediteur = getpid();
    m.requete = LOGOUT;

    if (msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
        perror("Erreur de msgsnd");
        exit(1);
    }

    logoutOK();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSuivant_clicked()
{
    // TO DO (étape 3)
    // Envoi d'une requete CONSULT au serveur
    MESSAGE reponse;

    reponse.type = 1;
    reponse.expediteur = getpid();
    reponse.data1 = articleEnCours.id + 1;
    reponse.requete = CONSULT;

    if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
        perror("Erreur de msgsnd");
        exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPrecedent_clicked()
{
    // TO DO (étape 3)
    // Envoi d'une requete CONSULT au serveur
    MESSAGE reponse;

    reponse.type = 1;
    reponse.expediteur = getpid();
    reponse.data1 = articleEnCours.id - 1;
    reponse.requete = CONSULT;

    if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
        perror("Erreur de msgsnd");
        exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonAcheter_clicked()
{
    // TO DO (étape 5)
    // Envoi d'une requete ACHAT au serveur

    MESSAGE reponse;
    char quant[20];

    sprintf(quant, "%d", getQuantite());

    reponse.type = 1;
    reponse.expediteur = getpid();
    reponse.data1 = articleEnCours.id;
    strcpy(reponse.data2, quant);
    reponse.requete = ACHAT;

    if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
        perror("Erreur de msgsnd");
        exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSupprimer_clicked()
{
    // TO DO (étape 6)
    // Envoi d'une requete CANCEL au serveur

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonViderPanier_clicked()
{
    // TO DO (étape 6)
    // Envoi d'une requete CANCEL_ALL au serveur

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPayer_clicked()
{
    // TO DO (étape 7)
    // Envoi d'une requete PAYER au serveur

    char tmp[100];
    sprintf(tmp,"Merci pour votre paiement de %.2f ! Votre commande sera livrée tout prochainement.",totalCaddie);
    dialogueMessage("Payer...",tmp);

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handlerSIGUSR1(int sig)
{
    MESSAGE m, reponse;
    
    //fprintf(stderr,"(CLIENT %d) MESSAGE RECU...\n",getpid());

    //if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) != -1)  // !!! a modifier en temps voulu !!!
    while (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(), IPC_NOWAIT) != -1) 
    {
      switch(m.requete)
      {
        case LOGIN :
                    if(m.data1 == 0)
                    {
                        w->dialogueMessage("LOGIN", m.data4);
                        w->loginOK();
                        logged = true;

                        reponse.type = 1;
                        reponse.expediteur = getpid();
                        reponse.data1 = 1;
                        reponse.requete = CONSULT;

                        if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                        {
                            perror("Erreur de msgsnd");
                            exit(1);
                        }
                    }
                    else w->dialogueErreur("LOGIN", m.data4);

                    break;

        case CONSULT : // TO DO (étape 3)
                    //fprintf(stderr,"(CLIENT %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);

                    articleEnCours.id = m.data1;
                    strcpy(articleEnCours.intitule, m.data2);
                    articleEnCours.stock = atoi(m.data3);
                    strcpy(articleEnCours.image, m.data4);
                    articleEnCours.prix = m.data5;

                    w->setArticle(articleEnCours.intitule, articleEnCours.prix, articleEnCours.stock , articleEnCours.image);

                    break;

        case ACHAT : // TO DO (étape 5)
                    if(atoi(m.data3) != 0)
                    {
                        char msg[35];

                        strcpy(msg, m.data3);

                        strcat(msg, " unité(s) de ");

                        strcat(msg, m.data2);

                        strcat(msg, " achetées avec succès");

                        w->dialogueMessage("ACHAT", msg);

                        reponse.type = 1;
                        reponse.expediteur = getpid();
                        reponse.requete = CADDIE;

                        if (msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0) == -1)
                        {
                            perror("Erreur de msgsnd");
                            exit(1);
                        }

                        w->videTablePanier();
                        totalCaddie = 0.0;
                    }
                    else w->dialogueErreur("ACHAT", "Stock insuffisant !");

                    break;

         case CADDIE : // TO DO (étape 5)
                    fprintf(stderr,"(CLIENT %d) Caddie recu...\n",getpid());

                    w->ajouteArticleTablePanier(m.data2, m.data5, atoi(m.data3));

                    totalCaddie = totalCaddie + (m.data5 * atoi(m.data3));

                    w->setTotal(totalCaddie);

                    break;

         case TIME_OUT : // TO DO (étape 6)
                    break;

         case BUSY : // TO DO (étape 7)
                    break;

         default :
                    break;
      }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void handlerSIGUSR2(int sig)
{
    //fprintf(stderr,"(CLIENT %d) PUBLICITE RECU...\n",getpid());

    //printf("Contenu de la memoire partagee : --%s--\n",pShm);

    w->setPublicite(pShm);
}