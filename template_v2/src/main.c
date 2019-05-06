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
#include <semaphore.h>

#include "reverse.h"
#include "sha256.h"
#include "initvar.h"
#include "destroy_var.h"

/*
variables globales
*/

int TAILLEFICHIERLIRE;
int HashBufSize;
size_t sizeReverseMdp = strlen("abcdabcdabcdabcd");
int index;
pthread_mutex_t mutexIndex;
sem_t semHashBufEmpty;
sem_t semHashBufFull;



sem_init(&semHashBufEmpty,0,1);
sem_init(&semHashBufFull, 0, 0);

/*
les threads:
0- lecture
1- ecriture
2- thread
*/

/*
0- thread de lecture: prend un tableau de noms de fichiers .bin en argument, lit
ces fichiers et stocke les hashs lus dans un buffer nomme HashBuf.
*/
void *lecture(void *fichiers)
{
  int argc = TAILLEFICHIERLIRE;
  char ** fichs = (char **) fichiers;
  int i;
  for(i = 0; i < argc && fichs[i] != NULL; i++)
  {
    printf("Préparation de l'ouverture du fichier %s\n", fichs[i]);
    int fd = open(fichs[i], O_RDONLY);
    if(fd == -1)
    {
      printf("Echec de l'ouverture du fichier %s\n", fichs[i]);
      pthread_exit(NULL); //fails to open ok
    }
    printf("Lecture du fichier numero %d\n", i);
    int size = sizeof(uint8_t)*32;
    buf = (uint8_t) malloc(size);
    int rd = read(fd, &buf, size);
    printf("Fichier numéro %d lu\n", i);
    while(rd > 0)
    {
      sem_wait(&semHashBufEmpty);
      if(index >= HashBufSize)
      {
        sem_wait(&semHashBufEmpty);
      }
      pthread_mutex_lock(&mutexIndex);
      *HashBuf[index] = buf;
      index += 1;
      rd = read(fd, &buf, size);
      sem_post(&semHashBufFull); /* et oui, on a ajoute un element au tableau */
      sem_post(&semHashBufEmpty);
      pthread_mutex_unlock(&mutexIndex);
    }

    if( rd < 0)
    {
      int err;
      err = close(fd);
      if(err==-1)
      {
        printf("Echec de la fermeture du fichier %s\n", fichs[i]);
        pthread_exit(NULL);
      }
      printf("Echec de la lecture du fichier %s. Ce fichier a pu etre ferme\n", fichs[i]);
      pthread_exit(NULL); //fails to read ok
    }
    printf("Le fichier %s a été ouvert, lu et fermé\n", fichs[i]);
  }
  printf("Tous les fichiers ont été lus, ouverts et fermés\n");
  sem_destroy(&semHashBufEmpty);
  pthread_exit(NULL);
}

/*
1- thread d'ecriture: se nourrit directement dans le tableau HashBuf, reversehash
ses elements un par un et stock les reversehash dans un tableau Reversed.
*/
void *reverseHashFunc()
{
  uint8_t *localHash;
  while(true)
  {
    sem_wait(&semHashBufFull);
    pthread_mutex_lock(&mutexIndex);
    localHash = *HashBuf[index];
    *HashBuf[index] = NULL;
    sem_post(&semHashBufEmpty); /* et oui, une place vient de se liberer */
    pthread_mutex_unlock(&mutexIndex);
  }
  /* si on trouve un reversehash, on remplit le tableau des candidats */
  if(reversehash(buf[i], localHash, sizeReverseMdp))
  {
    //sem sem lock unlock 
    *candidatsTab[indexCandide] = localHash;
  }
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
  extern bool consonne = false;
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
         fprintf (stderr, "Caractère d'option inconnu `\\x%x'.\n", optopt);
       return -1;
    }
  }
  if (optind == argc) {
        fprintf (stderr, "Il est nécessaire de spécifier au moins un argument.\n");
        return 1;
    }

  /* petite section de test de verification des options */
  printf("Nombre de threads: %ld \n", nthread);
  printf("Tri par consonne? %s \n", consonne ? "true" : "false");
  if (fichierout != NULL)
    printf("Le fichier de sortie a été spécifié comme: %s \n", fichierout);
  printf("argc: %d, optind: %d \n", argc, optind);
  /* fin de la petite section de test des options */

  /* etape 1: lecture des fichiers de hash
  il faut des threads (un par type d entree)
  */
  /* on cherche tous les fichier .bin (a lire) */


  /* A faire : maintenant que les fichiers a lire (.bin) sont stockes dans un tableau, il
  faut les differencier, selon leur provenance et les lire */

  int i ;
  int placeFich = 0;
  HashBufSize = sizeof(uint8_t)*32*nthread;
  **HashBuf = (char *) malloc(HashBufSize);
  TAILLEFICHIERLIRE = argc-optind;
  char *fichs[TAILLEFICHIERLIRE];
  for(i = optind; i < argc; i++)
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

  pthread_t thread_lectureEasy;
  if (pthread_create(&thread_lectureEasy, NULL, lecture, (void*)&(*fichs)) == -1) {
    perror("pthread_create");
    return EXIT_FAILURE ;
  }
  printf("Le thread de lecture basic a été créé\n");

  /* on lit les fichiers .bin */


  pthread_t thread_reverseHash;
  for(i = 0; i < nthread; i++)
  {
    if(pthread_create(&thread_reverseHash, NULL, reverseHashFunc, NULL) == -1)
    {
      perror("pthread_create");
      return EXIT_FAILURE;
    }
  }


  pthread_join(thread_lectureEasy, NULL);
  return EXIT_SUCCESS;
} //fin de la main()
