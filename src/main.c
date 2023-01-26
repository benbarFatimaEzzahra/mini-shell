/*
  Projet minishell - Licence 3 Info - PSI 2022
 
  Nom :
  Prénom :
  Num. étudiant :
  Groupe de projet :
  Date :
 
  Interface du minishell.
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "processus.h"
#define MAX_LINE_SIZE 1024
#define MAX_CMD_SIZE 256

int main(int argc, char* argv[]) {
  char cmdline[MAX_LINE_SIZE]; // buffer des lignes de commandes
  char* cmdtoks[MAX_CMD_SIZE]; // "mots" de la ligne de commandes
  processus_t processes[MAX_CMD_SIZE];
  processus_t* current;
  int i;
  while (1) {

    // Effacer les contenus de cmdline, cmdtoks et processes
	memset( processes, 0, sizeof(processus_t)*MAX_LINE_SIZE );
	memset( cmdline, '\0', sizeof(char)*MAX_LINE_SIZE );
	memset( cmdtoks, 0, sizeof(char)*MAX_CMD_SIZE );
    // Initialiser les valeurs par défaut dans processes (stdin, stdout, stderr, ...)
    for(int i = 0;i<MAX_CMD_SIZE;i++)
    	init_process(&processes[i]);
    // Afficher un prompt
    printf("$ ");
    
    // Lire une ligne dans cmdline - Attention fgets enregistre le \n final
    if(fgets(cmdline, MAX_LINE_SIZE, stdin)==NULL)break;
    cmdline[strlen(cmdline)-1]='\0';
    // Traiter la ligne de commande
    //   - supprimer les espaces en début et en fin de ligne
    trim(cmdline);
    printf("after trim %s",cmdline);
    //   - ajouter d'éventuels espaces autour de ; ! || && & ...
    
    //   - supprimer les doublons d'espaces
    clean(cmdline);
    printf("after clean %s",cmdline);
    //   - traiter les variables d'environnement

    // Découper la ligne dans cmdtoks

    // Traduire la ligne en structures processus_t dans processes
    //parse_cmd(cmdtoks,processes);
    // Les commandes sont chaînées en fonction des séparateurs
    //   - next -> exécution inconditionnelle
    //   - next_success -> exécution si la commande précédente réussit
    //   - next_failure -> exécution si la commande précédente échoue

    // Exécuter les commandes dans l'ordre en fonction des opérateurs
    // de flux
    for (current=processes; current!=NULL; ) {
      exec_process(current);
      if (current->next!=NULL) {
	// Exécution inconditionnelle
	current=current->next;
	continue;
      }
      if (current->next_success!=NULL) {
	// Si la commande a réussi
	  current = current->next_success;
	continue;
      }
      if (current->next_failure!=NULL) {
	// Si la commande a échoué
	  current = current->next_failure;
      }
    }
  }
  
  fprintf(stderr, "\nGood bye!\n");
  return 0;
}
