#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "data.h"
#include <fcntl.h>
#include <unistd.h>

void AProposServeurHV(char *Version,char *Nom1,char* Nom2) ;

int RechercheHV(char* NomFichier,int Reference ,struct VehiculeHV *UnRecord) ;