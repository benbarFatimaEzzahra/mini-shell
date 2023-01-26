/*
  Projet minishell - Licence 3 Info - PSI 2022
 
  Nom :
  Prénom :
  Num. étudiant :
  Groupe de projet :
  Date :
 
  Gestion des processus (implémentation).
 
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include "processus.h"
#include "builtin.h"
#include "parser.h"


int exec_process(processus_t* p) {

    assert(p != NULL);
    /*
        Les variables déclarées ci-dessous, sont des
        variables qui dupliquent les sorties standard et
        d'erreurs ainsi que de l'entrée standard afin que
        chaque processus les utilise seulement à l'exécution.
    */
    int tmp_stdin, tmp_stdout, tmp_stderr;
    do
    {
        tmp_stdin = dup(0);
        tmp_stdout = dup(1);
        tmp_stderr = dup(2);

        /*
            On vérifie si la commande à exécutée contient des
            variables d'environnement dans ses arguments.
        */
       

        /*
            Si la commande est builtin, on gére
            la prochaine commande.
        */
        if (is_builtin(p->argv[0]) == 1)
        {
            int done = builtin(p);
            if (done != 0)
            {
                perror(p->argv[1]);
            }
            if (p->next != NULL)
            {
                p = p->next;
                continue;
            }

            if (p->next_success != NULL)
            {
                if (p->status == 0)
                {
                    p = p->next_success;
                    continue;
                }
                else
                {
                    p = NULL;
                }
            }

            if (p->next_failure != NULL)
            {
                if (p->status != 0)
                {
                    p = p->next_failure;
                    continue;
                }
                else
                {
                    p = NULL;
                }
            }
            p = NULL;
        }
        else
        {
            /*
                Sinon, on crée un processus fils
                pour exécuter la commande, et le
                processu père va attendre le status
                de la commande.
            */
            p->status = 0;
            p->pid = fork();
            if (p->pid == 0)
            {
                if (p->stdin > 0 && p->stdout > 1)
                {
                    // S'il s'agit d'une commande |
                    close(p->fdclose[0]);
                    dup2(p->fdclose[1], 1);
                    close(p->fdclose[1]);
                }
                if (p->stdin > 0)
                {
                    // S'il s'agit d'une commande <
                    dup2(p->stdin, 0);
                    close(p->stdin);
                }
                if (p->stdout > 1)
                {
                    // S'il s'agit d'une commande > ou >>
                    dup2(p->stdout, 1);
                    close(p->stdout);
                    close(p->stdin);
                }
                if (p->stderr > 2)
                {
                    // S'il s'agit d'une commande 2> ou 2>>
                    dup2(p->stderr, 2);
                    close(p->stderr);
                }
                execvp(*p->argv, p->argv);
                perror(*p->argv);
                exit(1);
            }
            else
            {
                if (p->stdin > 0 && p->stdout > 1)
                {
                    close(p->fdclose[1]);
                    dup2(p->fdclose[0], 0);
                    close(p->fdclose[0]);
                    p++;
                    execvp(*p->argv, p->argv);
                    perror("pipe error");
                    exit(1);
                }
                wait(&p->status);
                if (p->stdin > 0)
                {
                    close(p->stdin);
                }
                if (p->stdout > 1)
                {
                    close(p->stdout);
                }
                if (p->stderr > 2)
                {
                    close(p->stderr);
                }
                // Revenir à l'état standard des sorties standard et d'erreurs
                dup2(tmp_stdin, 0);
                close(tmp_stdin);
                dup2(tmp_stdout, 1);
                close(tmp_stdout);
                dup2(tmp_stderr, 2);
                close(tmp_stderr);

                // S'il s'agit d'une commande ;
                if (p->next != NULL)
                {
                    p = p->next;
                    continue;
                }

                // S'il s'agit d'une commande &&
                if (p->next_success != NULL)
                {
                    if (p->status == 0)
                    {
                        p = p->next_success;
                        continue;
                    }
                    else
                    {
                        p = NULL;
                    }
                }

                // S'il s'agit d'une commande ||
                if (p->next_failure != NULL)
                {
                    if (p->status != 0)
                    {
                        p = p->next_failure;
                        continue;
                    }
                    else
                    {
                        p = NULL;
                    }
                }

                p = NULL;
            }
        }
    } while (p != NULL);
    return 0;
}

int init_process(processus_t* p) {
    assert(p != NULL);
    do
    {
        p->path = NULL;
        memset(p->argv,0,MAX_CMD_SIZE);
        p->stdin = 0;
        p->stdout = 1;
        p->stderr = 2;
        p->next = NULL;
        p->next_success = NULL;
        p->next_failure = NULL;
        p = p->next;
    } while (p != NULL);
    return 0;
}



int parse_cmd(char* tokens[], processus_t* processes, size_t max) {
 assert(tokens != NULL);
  assert(processes != NULL);

  int position = 0;
  int commandNumber = 0;

  for (int i = 0; tokens[i] != NULL; i++)
  {
    /* Si le token actuel est un mot réservé
      on doit ajouter la commande avant avec leurs arguments.
      Lorsqu'on termine le traitement de chaque mot réservé,
      on incrémente le nombre de commandes et on change la
      position du dernière commande vers la nouvelle.
    */
    if (separates_s(tokens[i]) == 0)
    {
      processes[commandNumber].path = tokens[position];
      memset(processes[commandNumber].argv,0,MAX_CMD_SIZE);
    }
    if (strcmp(tokens[i], ";") == 0)
    {
      if (tokens[i + 1] != NULL) // Si jamais l’utilisateur a terminé la ligne avec un ;
        processes[commandNumber].next = &processes[commandNumber] + 1;
      tokens[i] = NULL;
      ++commandNumber;
      position = i + 1;
      continue;
    }
    if (strcmp(tokens[i], "<") == 0)
    {
      /* On utilise le descripteur de l'entrée standard
      pour lire les données entrantes.
      */
      processes[commandNumber].stdin = open(tokens[i + 1], O_RDONLY);
      if (processes[commandNumber].stdin == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("Erreur de stdin");
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], ">") == 0)
    {
      /* On utilise le descripteur de la sortie standard
        pour rediriger la sortie standard vers ce fichier.
      */
      processes[commandNumber].stdout = open(tokens[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0644);
      if (processes[commandNumber].stdout == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("Erreur de stdout");
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], ">>") == 0)
    {
      /* On utilise le descripteur de la sortie standard
        pour rediriger en ajoutant la sortie standard vers
        la fin du fichier sans écraser son contenu.
      */
      processes[commandNumber].stdout = open(tokens[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
      if (processes[commandNumber].stdout == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("Erreur stdout append");
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], "2>") == 0)
    {
      /* On utilise le descripteur de la sortie d'erreurs
        pour rediriger la sortie d'erreurs vers le fichier.
      */
      processes[commandNumber].stderr = open(tokens[i + 1], O_CREAT | O_TRUNC | O_WRONLY, 0644);
      if (processes[commandNumber].stderr == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("Erreur stderr");
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], "2>>") == 0)
    {
      /* On utilise le descripteur de la sortie d'erreurs
        pour rediriger en ajoutant la sortie d'erreurs vers
        la fin du fichier sans écraser son contenu.
      */
      processes[commandNumber].stderr = open(tokens[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0644);
      if (processes[commandNumber].stderr == -1)
      {
        // Traiter l’erreur => on fait simple, on affiche un message d’erreur et on arrête le traitement
        perror("Erreur stderr append");
        return 1;
      }
      tokens[i] = NULL;
      ++i;              // On a «consommé » un token de plus avec le nom de fichier
      tokens[i] = NULL; // La redirection ne fait pas partie des arguments
      continue;
    }
    if (strcmp(tokens[i], "|") == 0)
    {
      /* On commence à créer un tube pour les deux commandes,
        et on ouvert les descripteurs de la sortie standard de la première
        commandes, et l'entrée standard de la deuxième.
      */
      pipe(processes[commandNumber].fdclose);
      processes[commandNumber].stdout = processes[commandNumber].fdclose[1];
      processes[commandNumber + 1].stdin = processes[commandNumber].fdclose[0];
      processes[commandNumber].next = &processes[commandNumber] + 1;

      tokens[i] = NULL;
      ++commandNumber;
      position = i + 1;
      continue;
    }
    if (strcmp(tokens[i], "&&") == 0)
    {
      /* le mot réservé && permet d'exécuter la prochaine
        commande lorsque la commande actuelle est exécutée avec
        succès, alors cette fois on doit affecter la prochaine commande
        dans next_success parce qu'elle dépend de code de retour de la commande
        actuelle.
      */
      if (tokens[i + 1] != NULL)
        processes[commandNumber].next_success = &processes[commandNumber] + 1;
      tokens[i] = NULL;
      ++commandNumber;
      position = i + 1;
      continue;
    }

    if (strcmp(tokens[i], "||") == 0)
    {
      /* le mot réservé || permet d'exécuter la prochaine
        commande lorsque la commande actuelle n'est pas exécutée avec
        succès, alors cette fois on doit affecter la prochaine commande
        dans next_failure.
      */
      if (tokens[i + 1] != NULL)
        processes[commandNumber].next_failure = &processes[commandNumber] + 1;
      tokens[i] = NULL;
      ++commandNumber;
      position = i + 1;
      continue;
    }
  }
  return 0;
}
