#include "mainwindowex3.h"
#include "ui_mainwindowex3.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>
MainWindowEx3::MainWindowEx3(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindowEx3)
{
  ui->setupUi(this);
}

MainWindowEx3::~MainWindowEx3()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowEx3::setGroupe1(const char *Text)
{
  fprintf(stderr, "---%s---\n", Text);
  if (strlen(Text) == 0)
  {
    ui->lineEditGroupe1->clear();
    return;
  }
  ui->lineEditGroupe1->setText(Text);
}

void MainWindowEx3::setGroupe2(const char *Text)
{
  fprintf(stderr, "---%s---\n", Text);
  if (strlen(Text) == 0)
  {
    ui->lineEditGroupe2->clear();
    return;
  }
  ui->lineEditGroupe2->setText(Text);
}

void MainWindowEx3::setGroupe3(const char *Text)
{
  fprintf(stderr, "---%s---\n", Text);
  if (strlen(Text) == 0)
  {
    ui->lineEditGroupe3->clear();
    return;
  }
  ui->lineEditGroupe3->setText(Text);
}

void MainWindowEx3::setResultat1(int nb)
{
  char Text[20];
  sprintf(Text, "%d", nb);
  fprintf(stderr, "---%s---\n", Text);
  if (strlen(Text) == 0)
  {
    ui->lineEditResultat1->clear();
    return;
  }
  ui->lineEditResultat1->setText(Text);
}

void MainWindowEx3::setResultat2(int nb)
{
  char Text[20];
  sprintf(Text, "%d", nb);
  fprintf(stderr, "---%s---\n", Text);
  if (strlen(Text) == 0)
  {
    ui->lineEditResultat2->clear();
    return;
  }
  ui->lineEditResultat2->setText(Text);
}

void MainWindowEx3::setResultat3(int nb)
{
  char Text[20];
  sprintf(Text, "%d", nb);
  fprintf(stderr, "---%s---\n", Text);
  if (strlen(Text) == 0)
  {
    ui->lineEditResultat3->clear();
    return;
  }
  ui->lineEditResultat3->setText(Text);
}

bool MainWindowEx3::recherche1Selectionnee()
{
  return ui->checkBoxRecherche1->isChecked();
}

bool MainWindowEx3::recherche2Selectionnee()
{
  return ui->checkBoxRecherche2->isChecked();
}

bool MainWindowEx3::recherche3Selectionnee()
{
  return ui->checkBoxRecherche3->isChecked();
}

const char *MainWindowEx3::getGroupe1()
{
  if (ui->lineEditGroupe1->text().size())
  {
    strcpy(groupe1, ui->lineEditGroupe1->text().toStdString().c_str());
    return groupe1;
  }
  return NULL;
}

const char *MainWindowEx3::getGroupe2()
{
  if (ui->lineEditGroupe2->text().size())
  {
    strcpy(groupe2, ui->lineEditGroupe2->text().toStdString().c_str());
    return groupe2;
  }
  return NULL;
}

const char *MainWindowEx3::getGroupe3()
{
  if (ui->lineEditGroupe3->text().size())
  {
    strcpy(groupe3, ui->lineEditGroupe3->text().toStdString().c_str());
    return groupe3;
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowEx3::on_pushButtonLancerRecherche_clicked()
{
  fprintf(stderr, "Clic sur le bouton Lancer Recherche\n");
  // TO DO
  int idFils1, idFils2, idFils3;
  if (recherche1Selectionnee())
  {
    idFils1 = fork();
    if (idFils1 == 0)
    {
      //Le fils
      execl("./Lecture", "Lecture", getGroupe1(), NULL);
    }
  }
  if (recherche2Selectionnee())
  {
    idFils2 = fork();
    if (idFils2 == 0)
    {
      //Le fils
      execl("./Lecture", "Lecture", getGroupe2(), NULL);
    }
  }
  if (recherche3Selectionnee())
  {
    idFils3 = fork();
    if (idFils3 == 0)
    {
      //Le fils
      execl("./Lecture", "Lecture", getGroupe3(), NULL);
    }
  }

  //Le père
  int status;

  //1er wait
  int x = wait(&status);

  if (x == idFils1 && WIFEXITED(status) && recherche1Selectionnee())
  {
    setResultat1(WEXITSTATUS(status));
  }
  else if (x == idFils2 && WIFEXITED(status) && recherche2Selectionnee())
  {
    setResultat2(WEXITSTATUS(status));
  }
  else if (x == idFils3 && WIFEXITED(status) && recherche3Selectionnee())
  {
    setResultat3(WEXITSTATUS(status));
  }
  //2eme wait

  x = wait(&status);

  if (x == idFils1 && WIFEXITED(status) && recherche1Selectionnee())
  {
    setResultat1(WEXITSTATUS(status));
  }
  else if (x == idFils2 && WIFEXITED(status) && recherche2Selectionnee())
  {
    setResultat2(WEXITSTATUS(status));
  }
  else if (x == idFils3 && WIFEXITED(status) && recherche3Selectionnee())
  {
    setResultat3(WEXITSTATUS(status));
  }

  //3e wait
  x = wait(&status);

  if (x == idFils1 && WIFEXITED(status) && recherche1Selectionnee())
  {
    setResultat1(WEXITSTATUS(status));
  }
  else if (x == idFils2 && WIFEXITED(status) && recherche2Selectionnee())
  {
    setResultat2(WEXITSTATUS(status));
  }
  else if (x == idFils3 && WIFEXITED(status) && recherche3Selectionnee())
  {
    setResultat3(WEXITSTATUS(status));
  }
}

void MainWindowEx3::on_pushButtonVider_clicked()
{
  fprintf(stderr, "Clic sur le bouton Vider\n");
  setResultat1(0);
  setGroupe1("");
  setResultat2(0);
  setGroupe2("");
  setResultat3(0);
  setGroupe3("");
}

void MainWindowEx3::on_pushButtonQuitter_clicked()
{
  fprintf(stderr, "Clic sur le bouton Quitter\n");
  close();
}
