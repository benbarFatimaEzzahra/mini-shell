/*
  Projet minishell - Licence 3 Info - PSI 2022
 
  Nom :
  Prénom :
  Num. étudiant :
  Groupe de projet :
  Date :
 
  Parsing de la ligne de commandes utilisateur (implémentation).
 
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "parser.h"
#include "processus.h"

int trim(char* str) {
assert(str != NULL);
  /*
    On commance par la fin de la chaîne,
    tant qu'il y a un espace/tabulation/retour à la ligne,
    on incrémente le nombre d'espaces jusqu'on arrivera à
    un caractère.
    On remplace la position où on arrêté et
    on l'affecte par '\0'
  */
  int debut = 0;
  int fin = strlen(str);
  for (int i = fin - 1; str[i] == ' ' || str[i] == '\t' || str[i] == '\n'; i++)
  {
    fin = fin - 1;
  }
  str[fin] = '\0';

  /*
    On accumule le nombre d'espaces depuis le début de la chaîne,
    puis on utilise la fonction memmove pour déplacer la chaîne
    depuis la position où on n'a pas trouvé un espace.
  */

  for (int i = 0; str[i] == ' ' || str[i] == '\t' || str[i] == '\n'; i++)
  {
    debut = debut + 1;
  }
  memmove(str, str + debut, 1 + strlen(str) - debut);
  return 0;
}

int clean(char* str) {
  assert(str != NULL);
  /*
    En utilisant le même principe de la fonction trim,
    mais cette fois, à chaque caractère, si il est un espace
    on boucle tant qu'il y a d'autres espaces après
    pour les éliminer
  */
  for (int i = 0; i < (int) strlen(str); i++)
  {
    int j = i;
    while (str[j] == ' ' || str[j] == '\t')
    {
      j++;
    }
    if (j > i + 1)
      memmove(str + (i + 1), str + j, 1 + strlen(str) - j);
  }
  return 0;
}

int separates_s(const char *tok)
{
  assert(tok != NULL);
  const char *reserved[] = {";", "&&", "||", "|", ">", "<", ">>", "2>", "2>>"};
  int size = sizeof reserved / sizeof *reserved;
  for (int i = 0; i < size; i++)
  {
    if (strcmp(tok, reserved[i]) == 0)
      return 1;
  }
  return 0;
}

int substenv(char* str, size_t max) {
 int i = 0;
    while (!str[i])
    {
        if (str[0] == '$')
        {
            char *var = &str[i];
            memmove(var, var + 1, strlen(var));
            //str[i] = getenv(var);
        }
        i++;
    }
    return 0;
}


int strcut(char* str, char sep, char** tokens, size_t max) {

}

// Optionnel
int strcut_s(char* str, char sep, char** tokens, size_t max) {

}
