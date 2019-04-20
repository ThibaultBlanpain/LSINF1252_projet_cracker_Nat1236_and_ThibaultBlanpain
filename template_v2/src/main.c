#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

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

  /* etape 0: lecture des arguments */
  int nthread = 1;
  bool consonne = false;
  char *fichier = NULL;
  int opt;
  while ((opt = getopt(argc, argv, "t:co:")) != -1)
  {
    switch(opt)
    {
      case "t":
       nthread = optarg;
       break;
      case "c":
       consonne = true;
       break;
      case "o":
       fichier = optarg;
       break;
      case "?":
       if (optopt == 't')
         printf (stderr, "Option -%c requiert un argument.\n", optopt);
       if (optopt == 'o')
         printf (stderr, "Option -%c requiert un argument.\n", optopt);
       else
         printf(stderr, "Option -`\\x%x' inconnu"\n", optopt);
       break;
      default:
       
    }
  }


  return EXIT_SUCCESS;
}
