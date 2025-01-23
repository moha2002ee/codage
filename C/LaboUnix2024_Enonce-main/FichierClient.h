#ifndef FICHIER_CLIENT_H
#define FICHIER_CLIENT_H

#define FICHIER_CLIENTS "clients.dat"

// Structure pour le client
typedef struct
{
  char  nom[20];  // Nom du client
  int   hash;     // Hash du mot de passe
} CLIENT;

// Déclaration des fonctions
int estPresent(const char* nom); 
// retourne -1 en cas d'erreur, 0 si non trouvé, position dans le fichier si trouvé

int hash(const char* motDePasse);
// Calcule le hash du mot de passe = (somme pondérée des codes ASCII) % 97

void ajouteClient(const char* nom, const char* motDePasse);
// Ajoute un nouveau client à la fin du fichier

int verifieMotDePasse(int pos, const char* motDePasse);
// Vérifie si le mot de passe est correct pour un client à une position donnée

int listeClients(CLIENT *vecteur);
// Liste les clients du fichier dans le vecteur passé en argument

#endif
