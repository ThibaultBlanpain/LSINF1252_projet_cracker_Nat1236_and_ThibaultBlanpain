#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>

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
  char *fichierout = NULL;
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
       fichierout = optarg;
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

  /* petite section de test de verification des options */
  printf("Nombres de threads: %d \n", nthread);
  printf("Tri par consonne? %s \n", consonne ? "true" : "false");
  if (fichierout != NULL)
    printf("Le fichier de sortie a ete specifie comme: %s \n", fichierout);
  /* fin de la petite section de test des options */

  /* etape 1: lecture des fichiers de hash
  il faut des threads (un par type d entree)
  */
  /* on cherche tous les fichier .bin (a lire) */
  int i ;
  int placeFich = 0;
  char fichs[argc];
  for(i = 0; i < argc; i++)
  {
    char *argTestBin = argv[i];
    int lengthArg = strlen(argTestBin);
    if(argTestBin[lengthArg - 1] == 'n' && argTestBin[lengthArg - 2] == 'i' &&
  argTestBin[lengthArg - 3] == 'b' && argTestBin[lengthArg - 4] == '.')
      {
        fichs[placeFich] = argTestBin ;
        placeFich = placeFich + 1;
      }
  }
  /* maintenant que les fichiers a lire (.bin) sont stockes dans un tableau, il
  faut les differencier, selon leur provenance et les lire */



  return EXIT_SUCCESS;
}
