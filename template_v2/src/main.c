#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "reverse.h"
#include "sha256.h"

int TAILLEFICHIERLIRE;
void *lecture(void *fichiers)
{ //fonction de lecture
  int argc = TAILLEFICHIERLIRE;
  char ** fichs = (char **) fichiers;
  int i;
  for(i = 0; i < argc && fichs[i] != NULL; i++)
  {
    int fd = open(fichs[i], O_RDONLY);
    if(fd == -1)
      pthread_exit(NULL); //fails to open ok
    int size = sizeof(int);
    int buf;
    int rd = read(fd, &buf, size);
    if( rd < 0)
    {
      int err;
      err = close(fd);
      if(err==-1)
        pthread_exit(NULL);
      pthread_exit(NULL); //fails to read ok
    }
  }
  pthread_exit(NULL);
}

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
         fprintf (stderr, "Caractere d'option inconnu `\\x%x'.\n", optopt);
       return -1;
    }
  }
  if (optind == argc) {
        fprintf (stderr, "Il est necessaire de specifier au moins un argument.\n");
        return 1;
    }

  /* petite section de test de verification des options */
  printf("Nombres de threads: %ld \n", nthread);
  printf("Tri par consonne? %s \n", consonne ? "true" : "false");
  if (fichierout != NULL)
    printf("Le fichier de sortie a ete specifie comme: %s \n", fichierout);
  /* fin de la petite section de test des options */

  /* etape 1: lecture des fichiers de hash
  il faut des threads (un par type d entree)
  */
  /* on cherche tous les fichier .bin (a lire) */


  /* maintenant que les fichiers a lire (.bin) sont stockes dans un tableau, il
  faut les differencier, selon leur provenance et les lire */

  int i ;
  int placeFich = 0;
  TAILLEFICHIERLIRE = argc;
  char *fichs[argc];
  for(i = 0; i < argc; i++)
  {
    char *argTestBin = argv[i];
    int lengthArg = strlen(argTestBin);
    if(argTestBin[lengthArg - 1] == 'n' && argTestBin[lengthArg - 2] == 'i' &&
  argTestBin[lengthArg - 3] == 'b' && argTestBin[lengthArg - 4] == '.') 
      {
        fichs[placeFich] = argTestBin;
        placeFich = placeFich + 1;
      }
  }

  /* creer la structure comprenant les arguments de lecture*/



  pthread_t thread_lectureEasy ;
  if (pthread_create(&thread_lectureEasy, NULL, lecture, &fichs) == -1) {
    perror("pthread_create");
    return EXIT_FAILURE ;
  }

  pthread_join(thread_lectureEasy, NULL);

  return EXIT_SUCCESS;
} //fin de la main()
