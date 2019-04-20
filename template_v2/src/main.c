#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "reverse.h"
#include "sha256.h"

int main(int argc, char **argv){
  /*
  les etapes du programme:
  0- lire les arguments
  1- lire les fichiers
  2- utiliser la fonction reversehash(const uint8_t *hash, char *res, size_t len);
  3- trier
  4- display ligne par ligne
  */

  /* etape 0: lecture des options */
  /* penser a implementer de la programmation defensive (sur les options, qui ne
seront pas d office des int ou char*) */
  long int nthread = 1;
  bool consonne = false;
  char *fichier = NULL;
  int opt;
  while ((opt = getopt(argc, argv, "t:co:")) != -1)
  {
    switch(opt)
    {
      case 't':
       nthread = atol(optarg);
       break;
      case 'c':
       consonne = true;
       break;
      case 'o':
       fichier = optarg;
       break;
      case '?':
       if (optopt == 't')
         fprintf (stderr, "Option -%c requiert un argument.\n", optopt);
       if (optopt == 'o')
         fprintf (stderr, "Option -%c requiert un argument.\n", optopt);
       else
         fprintf (stderr, "Caractere d option inconnu `\\x%x'.\n", optopt);
       return -1;
    }

  }

  /* etape 1 :  */
  int fd = open();


  return EXIT_SUCCESS;
}
