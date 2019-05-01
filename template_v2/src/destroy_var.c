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

int TAILLEFICHIERLIRE;
uint8_t ** buf;
int bufSize;
char ** bufReverseHash;
char * candidat[16];

//Il faut faire attention à détruire proprement les sémaphores utilisés dès qu'ils ne sont plus nécessaires


void *dest_sem(char *name, int val) //peut-être inutile, puisque n'a pa sde condition
{
  sem_t name;
  sem_destroy(&name);
}
