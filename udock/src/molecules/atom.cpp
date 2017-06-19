#include "atom.h"

bool Atome::_InitOk = false;
std::string Atome::_TypesStr[NB_ATOM_UDOCK_ID]; //Pour faire le lien avec chaque atome et son type sybyl
double Atome::_AtomsRadius[NB_ATOM_UDOCK_ID]; //Radius de chaque atome
double Atome::_AtomsEpsilons[NB_ATOM_UDOCK_ID]; //Voir cornell 1995
double Atome::_AtomsEpsilonsSquared[NB_ATOM_UDOCK_ID][NB_ATOM_UDOCK_ID]; //Voir cornell 1995, pour le calcul de Aij Bij
