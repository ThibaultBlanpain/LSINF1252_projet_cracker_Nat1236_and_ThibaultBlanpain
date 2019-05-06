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


uint8_t ** buf;
char ** bufReverseHash;
char * candidat[16];

/*
void *init_sem(char * name, int val)  //pourquoi cette fonction ?
{
  if(val < 0)
  {
    printf("Le semaphore doit etre initialise avec une valeur >= 0");
    exit(42);
  }
  sem_t name;
  sem_init(&name, 0, val);
}*/


void *lecture(void *fichiers)
{ //fonction de lecture
  extern int TAILLEFICHIERLIRE;
  int argc = TAILLEFICHIERLIRE;
  char ** fichs = (char **) fichiers;
  int i;
  for(i = 0; i < argc && fichs[i] != NULL; i++)
  {
    printf("Préparation de l'ouverture du fichier %s\n", fichs[i]);
    int fd = open(fichs[i], O_RDONLY);
    printf("Fichier numéro %d ouvert\n", i);
    if(fd == -1)
    {
      printf("Echec de l'ouverture du fichier %s\n", fichs[i]);
      pthread_exit(NULL); //fails to open ok
    }
    printf("Reading file number %d\n", i);
    int size = sizeof(int);

    sem_t name;
    sem_init(&name,0,1);
    sem_wait(&name); //entrée dans la section critique, on protège le buffer
    extern int bufSize;
    buf = malloc(bufSize);
    int rd = read(fd, &buf, size);
    printf("Fichier numéro %d lu\n", i);
    if( rd < 0)
    {
      int err;
      err = close(fd);
      if(err==-1)
      {
        printf("Echec de la fermeture du fichier %s\n", fichs[i]);
        pthread_exit(NULL);
      }
      printf("Echec de la lecture du fichier %s\n", fichs[i]);
      printf("Le fichier %s a été fermé\n", fichs[i]);
      pthread_exit(NULL); //fails to read ok
    }
    printf("Le fichier %s a été ouvert, lu et fermé\n", fichs[i]);
  }
  printf("Tous les fichiers ont été lus, ouverts et fermés\n");
  pthread_exit(NULL);

  sem_post(&name);
  sem_destroy(&name); //libération du buffer pour un autre thread

  //quand est-ce qu'on free la mémoire utilisée par le buffer ?
}

//ou alors, il vaut mieux initialiser les sémaphores dans la main ?


/*
fonction de thread qui va reversehash les hashs presents dans buf
et stocker les mdp au clair dans le bufReverseHash
                         !!!!!!!!!!!!!!!!!!
ATTENTION: il faut maintenir le thread en vie tout le long du programme
                         !!!!!!!!!!!!!!!!!!
*/
void *reverseHashFunc()
{
  int i;
  extern int bufSize;
  for(i = 0; i < bufSize; i++)
  {
    char *res = malloc(sizeof(char)*16);
    size_t sizeReverseMdp = strlen("abcdabcdabcdabcd");

    sem_t *name;
    sem_init(&name,0,1);
    sem_wait(&name); //entrée dans la section critique, on protège le buffer

    if(buf[i] != NULL && reversehash(buf[i], res, sizeReverseMdp))
    {
      int j = 0;
      while(bufReverseHash[j] != NULL)
      {
        j++;
      }
      strcpy(bufReverseHash[j], res);
      printf(bufReverseHash[i]);
    }
  }
  pthread_exit(NULL);

  sem_post(&name);
  sem_destroy(&name); //libération du buffer pour un autre thread
}
