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
/* declaration et initialisation*/
int idQ, idShm;
bool logged;
char *pShm;
ARTICLE articleEnCours;
float totalCaddie = 0.0;
MESSAGE requete;

/* fonction */
void handlerSIGUSR1(int sig);
void handlerSIGUSR2(int sig);
MESSAGE constructeurRequete(int nbElem, long type, int expediteur, int typeRequete, int data1, const char *data2, const char *data3, const char *data4, float data5);
void copyChaine(char *destinitation, const char *source);

/* define */
#define REPERTOIRE_IMAGES "images/"

WindowClient::WindowClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowClient)
{
  ui->setupUi(this);

  // Configuration de la table du panier (ne pas modifer)
  ui->tableWidgetPanier->setColumnCount(3);
  ui->tableWidgetPanier->setRowCount(0);
  QStringList labelsTablePanier;
  labelsTablePanier << "Article"
                    << "Prix à l'unité"
                    << "Quantité";
  ui->tableWidgetPanier->setHorizontalHeaderLabels(labelsTablePanier);
  ui->tableWidgetPanier->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->tableWidgetPanier->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableWidgetPanier->horizontalHeader()->setVisible(true);
  ui->tableWidgetPanier->horizontalHeader()->setDefaultSectionSize(160);
  ui->tableWidgetPanier->horizontalHeader()->setStretchLastSection(true);
  ui->tableWidgetPanier->verticalHeader()->setVisible(false);
  ui->tableWidgetPanier->horizontalHeader()->setStyleSheet("background-color: lightyellow");

  // Recuperation de l'identifiant de la file de messages // Etape 1
  fprintf(stderr, "(CLIENT %d) Recuperation de l'id de la file de messages\n", getpid());
  // TO DO
  if ((idQ = msgget(CLE, 0)) == -1)
  {
    perror("(CLIENT ) Recuperation de l'id de la file de messages (Errreur) \n");
  }

  // Recuperation de l'identifiant de la mémoire partagée
  //fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la mémoire partagée\n",getpid());
  // TO DO

  // Attachement à la mémoire partagée
  // TO DO
  /*
  if ((idShm = shmget(CLE, 0, 0)) == -1)
  {
    perror("Erreur de shmget");
    exit(1);
  }
  printf("idShm = %d\n", idShm);
  if ((pShm = (char *)shmat(idShm, NULL, SHM_RDONLY)) == (char *)-1)
  {
    perror("Erreur de shmat");
    exit(1);
  }
  printf("pShm = %s\n", pShm);*/
  // Armement des signaux
  // TO DO
  struct sigaction A;
  A.sa_handler = handlerSIGUSR1;
  sigemptyset(&A.sa_mask);
  A.sa_flags = 0;

  if (sigaction(SIGUSR1, &A, NULL) == -1)
  {
    perror("(CLIENT)Erreur de sigaction SIGUSR1");
  }

  struct sigaction B;
  B.sa_handler = handlerSIGUSR2;
  sigemptyset(&B.sa_mask);
  B.sa_flags = 0;

  if (sigaction(SIGUSR2, &B, NULL) == -1)
  {
    perror("(CLIENT)Erreur de sigaction SIGUSR2");
  }

  // Envoi d'une requete de connexion au serveur
  // TO DO
  requete = constructeurRequete(3, 1, getpid(), CONNECT, 0, nullptr, nullptr, nullptr, 0.0);
  if (msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long), 0) == -1)
  {
    perror("(CLIENT)il ya une erreur avec l'envoie de la requete CONNECT\n");
  }

  // Exemples à supprimer
  setPublicite("Promotions sur les concombres !!!");
  setArticle("pommes", 5.53, 18, "pommes.jpg");
  ajouteArticleTablePanier("cerises", 8.96, 2);
}

WindowClient::~WindowClient()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char *Text)
{
  if (strlen(Text) == 0)
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char *WindowClient::getNom()
{
  strcpy(nom, ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char *Text)
{
  if (strlen(Text) == 0)
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char *WindowClient::getMotDePasse()
{
  strcpy(motDePasse, ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char *Text)
{
  if (strlen(Text) == 0)
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setImage(const char *image)
{
  // Met à jour l'image
  char cheminComplet[80];
  sprintf(cheminComplet, "%s%s", REPERTOIRE_IMAGES, image);
  QLabel *label = new QLabel();
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
  if (ui->checkBoxNouveauClient->isChecked())
    return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setArticle(const char *intitule, float prix, int stock, const char *image)
{
  ui->lineEditArticle->setText(intitule);
  if (prix >= 0.0)
  {
    char Prix[20];
    sprintf(Prix, "%.2f", prix);
    ui->lineEditPrixUnitaire->setText(Prix);
  }
  else
    ui->lineEditPrixUnitaire->clear();
  if (stock >= 0)
  {
    char Stock[20];
    sprintf(Stock, "%d", stock);
    ui->lineEditStock->setText(Stock);
  }
  else
    ui->lineEditStock->clear();
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
    sprintf(Total, "%.2f", total);
    ui->lineEditTotal->setText(Total);
  }
  else
    ui->lineEditTotal->clear();
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

  setArticle("", -1.0, -1, "");

  w->videTablePanier();
  totalCaddie = 0.0;
  w->setTotal(-1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du panier (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteArticleTablePanier(const char *article, float prix, int quantite)
{
  char Prix[20], Quantite[20];

  sprintf(Prix, "%.2f", prix);
  sprintf(Quantite, "%d", quantite);

  // Ajout possible
  int nbLignes = ui->tableWidgetPanier->rowCount();
  nbLignes++;
  ui->tableWidgetPanier->setRowCount(nbLignes);
  ui->tableWidgetPanier->setRowHeight(nbLignes - 1, 10);

  QTableWidgetItem *item = new QTableWidgetItem;
  item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  item->setTextAlignment(Qt::AlignCenter);
  item->setText(article);
  ui->tableWidgetPanier->setItem(nbLignes - 1, 0, item);

  item = new QTableWidgetItem;
  item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  item->setTextAlignment(Qt::AlignCenter);
  item->setText(Prix);
  ui->tableWidgetPanier->setItem(nbLignes - 1, 1, item);

  item = new QTableWidgetItem;
  item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  item->setTextAlignment(Qt::AlignCenter);
  item->setText(Quantite);
  ui->tableWidgetPanier->setItem(nbLignes - 1, 2, item);
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
  if (liste.size() == 0)
    return -1;
  QModelIndex index = liste.at(0);
  int indice = index.row();
  return indice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char *titre, const char *message)
{
  QMessageBox::information(this, titre, message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char *titre, const char *message)
{
  QMessageBox::critical(this, titre, message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{
  // TO DO (étape 1)
  // envoi d'un logout si logged
  if (logged)
  {
    requete = constructeurRequete(3, 1, getpid(), LOGOUT, 0, nullptr, nullptr, nullptr, 0.0);
    if (msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long), 0) == -1)
    {
      perror("(CLIENT)il ya une erreur avec l'envoie de la requete Logout\n");
    }
  }

  // Envoi d'une requete DECONNECT au serveur
  requete = constructeurRequete(3, 1, getpid(), DECONNECT, 0, nullptr, nullptr, nullptr, 0.0);
  if (msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long), 0) == -1)
  {
    perror("(CLIENT)il ya une erreur avec l'envoie de la requete Deconnect\n");
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
  requete = constructeurRequete(6, 1, getpid(), LOGIN, isNouveauClientChecked(), getNom(), getMotDePasse(), nullptr, 0.0);

  if (msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long), 0) == -1)
  {
    perror("(Login)(CLIENT)Erreur lors de l'envoie de la requete LOGIN\n");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogout_clicked()
{
  // Envoi d'une requete CANCEL_ALL au serveur (au cas où le panier n'est pas vide)
  // TO DO

  // Envoi d'une requete de logout au serveur
  // TO DO
  requete = constructeurRequete(3, 1, getpid(), LOGOUT, 0, nullptr, nullptr, nullptr, 0.0);
  if (msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long), 0) == -1)
  {
    perror("(CLIENT)il ya une erreur avec l'envoie de la requete LOGOUT\n");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSuivant_clicked()
{
  // TO DO (étape 3)
  // Envoi d'une requete CONSULT au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPrecedent_clicked()
{
  // TO DO (étape 3)
  // Envoi d'une requete CONSULT au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonAcheter_clicked()
{
  // TO DO (étape 5)
  // Envoi d'une requete ACHAT au serveur
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
  sprintf(tmp, "Merci pour votre paiement de %.2f ! Votre commande sera livrée tout prochainement.", totalCaddie);
  dialogueMessage("Payer...", tmp);

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
  MESSAGE m;

  while (msgrcv(idQ, &m, sizeof(MESSAGE) - sizeof(long), getpid(), IPC_NOWAIT) != -1) // !!! a modifier en temps voulu !!!//(sizeof long ?) c'est pour qu'il prennet pas en compte le lonh => une normalisation
  {
    switch (m.requete)
    {
    case LOGIN: // etape 1
      if (m.data1 == 1)
      {
        w->dialogueMessage(m.data3, m.data4);
        w->loginOK();
        logged = true;
      }
      else if (m.data1 == 0)
      {
        w->dialogueErreur(m.data3, m.data4);
      }
      break;

    case CONSULT: // TO DO (étape 3)
      break;

    case ACHAT: // TO DO (étape 5)
      break;

    case CADDIE: // TO DO (étape 5)
      break;

    case TIME_OUT: // TO DO (étape 6)
      break;

    case BUSY: // TO DO (étape 7)
      break;

    default:
      break;
    }
  }
}
void handlerSIGUSR2(int sig)
{
  if (pShm != NULL)
  {
    // Lire le contenu de la mémoire partagée
    char publicite[51];

    strncpy(publicite, pShm, 50);
    publicite[51] = '\0';

    w->setPublicite(publicite);
  }
}
void copieChaine(const char *aCopier, char *endroitCopie)
{
  if (strlen(aCopier) <= sizeof(endroitCopie) - 1)
  {
    strcpy(endroitCopie, aCopier);
  }

  else
  {
    fprintf(stderr, "Erreur :dépasse la taille maximale (caractères autorisés).\n");
    w->dialogueErreur("ERREUR", "TROP LONG");
    exit(1);
  }
}
MESSAGE constructeurRequete(int nbElem, long type, int expediteur, int typeRequete, int data1, const char *data2, const char *data3, const char *data4, float data5)
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
