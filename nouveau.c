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

/////////////////////////////////////////////////////////////////////////////////////////
/*
variables globales
*/
/////////////////////////////////////////////////////////////////////////////////////////

int TAILLEFICHIERLIRE;
int HashBufSize;
bool consonne = false;
char**HashBuf;
typedef struct Candidats
{
  struct Candidats *next;
  char * codeclair;  //représente le mot de passe reverse-hashed
  int nbrOccurence;
} Candid_Node;

typedef struct list {
  struct node *head;
  struct node *last;
  int nbrOccMax;
} list_t;

size_t sizeReverseMdp = strlen("abcdabcdabcdabcd");
int index;
list_t ListCandidat;
pthread_mutex_t mutexIndex;
sem_t semHashBufEmpty;
sem_t semHashBufFull;



sem_init(&semHashBufEmpty,0,1);
sem_init(&semHashBufFull, 0, 0);

/* fonctions utilitaires */

/////////////////////////////////////////////////////////////////////////////////////////
/*
fonction creant un noeud
@pre: prend un code clair (reversehash) en entree
retourne un pointeur vers le noeud cree
retourne NULL si le noeud n a pas pu etre cree
*/
/////////////////////////////////////////////////////////////////////////////////////////
Candid_Node *init_node(char * codeClair)
{
  Candid_Node *a=NULL;
  a = (Candid_Node *) malloc(sizeof(node_t));
  if(!a)
    return NULL;
  a->codeclair = codeClair;
  a->next = NULL;
  return a;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*
fonction ajoutant un noeud a la suite d une liste
retourne 0 si le noeud a ete ajoute
retourne -1 sinon
*/
/////////////////////////////////////////////////////////////////////////////////////////
int add_node(list_t *list, char *codeClair)
{
  if(!list)
    return -1;
  Candid_Node *a = init_node(codeClair);
  if(!a)
    return -1;
  if(list->head == NULL)
  {
    list->head = a;
    list->last = a;
    return 0;
  }
  list->head->next = a;
  list->head = a;
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
/*
fonction de comparaison
retourne -1 si a < b,
retourne 0 si a == b,
retourne 1 si a > b;
*/
/////////////////////////////////////////////////////////////////////////////////////////
int compare(int a, int b)
{
  if (a==b)
    return 0;
  if (b > a)
    return -1;
  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*
calcule le nombre d occurence de voyelles/consonnes (selon le bool consonne)  ATTENTION, CE SERAIT DANS UNE PROPOSITION DE MDP ??
et modifie dans la struct Candidats le parametre nbrOccurence (pour garder une trace du plus grand nombre de voyelles et update le meilleur candidat ??).
*/
/////////////////////////////////////////////////////////////////////////////////////////
void calculNbrOccu(Candid_Node * Node)
{
  char *localString = Node->codeclair;
  int i;
  int nbrOccLocal;
  for(i=0; i<strlen(localString);i++)
  {
    //
    if(localString == NULL)
    {
      nbrOccLocal = 0;
      Node->nbrOccurence = nbrOccLocal;
      return;
    }
    else(localString[i] == 'a' | localString[i] == 'e' | localString[i] == 'y' | localString[i] == 'u' | localString[i] == 'i' | localString[i] == 'o')
    {
      nbrOccLocal += 1;
    }
  }
  /* si l'option consonne est activée, on doit trier selon le nombre de consonnes */
  if(consonne)
  {
    nbrOccLocal = (strlen(localString) - 1 - nbrOccu);
  }
  Node->nbrOccurence = nbrOccLocal;
  return;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*
fonction dont le but est d enlever les candidats dont le nombre nbrOccurence
n est pas egal au nbrOccurence le plus grand
retourne 1 en cas d execution correcte,
retourne -1 en cas d echec.
il faut encore gerer le cas ou on doit enlever le premier noeud : POURQUOI ??
*/
/////////////////////////////////////////////////////////////////////////////////////////
int *trieur(list_t ListCandidat)
{
  /* le cas d une liste vide est considere comme une liste triee */
  if(head == NULL)
  {
    return 1;
  }
  /* ce noeud parcourt la liste pour trouver les noeuds a supprimer */
  struct Candid_Node * runner = ListCandidat->head;
  while(runner != NULL)
  {
    /* il faut enlever le noeud suivant */
    if(compare(runner->next->nbrOccurence, ListCandidat->nbrOccMax) == -1)
    {
      runner->next = runner->next->next;
    }
    /* ListCandidat->nbrOccMax n est pas le plus grand nombre d occurence*/
    if(compare(runner->next->nbrOccurence, ListCandidat->nbrOccMax) == 1)
    {
      printf("La valeur de ListCandidat->nbrOccMax n est pas valide\n");
      return -1;
    }
    runner = runner->next;
  }
  return 1;
}





/////////////////////////////////////////////////////////////////////////////////////////
/*
les threads:
0- lecture
1- ecriture
2- thread
*/
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
/*
0- thread de lecture: prend un tableau de noms de fichiers .bin en argument, lit
ces fichiers et stocke les hashs lus dans un buffer nomme HashBuf.
*/
/////////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////
/*
1- thread d'ecriture: se nourrit directement dans le tableau HashBuf, reversehash
ses elements un par un et stock les reversehash dans un tableau Reversed.
*/
/////////////////////////////////////////////////////////////////////////////////////////
void *reverseHashFunc()
{
  uint8_t *localHash;
  while(index >= 0)
  {
    char * candid;
    sem_wait(&semHashBufFull);
    localHash = *HashBuf[index];
    *HashBuf[index] = NULL;
    sem_post(&semHashBufEmpty); /* et oui, une place vient de se liberer */
  }
  /* si on trouve un reversehash, on remplit le tableau des candidats */

  /* il faut ajouter un semaphore */
  if(reversehash(localHash, candid, sizeReverseMdp))
  {
    //sem sem lock unlock
    //il faut stocker (fonction add) dans la liste chainee
  //  NewNode;
//    add();
    int ret = add_node(*ListCandidat, localHash);
    if(ret == -1)
    {
      printf("erreur dans l ajout des noeuds a la liste des candidats"):
    }
  }
}





  /*
  les etapes du programme:
  0- lire les arguments
  1- lire les fichiers
  2- utiliser la fonction reversehash(const uint8_t *hash, char *res, size_t len);
  3- trier
  4- display ligne par ligne
  */

int main(int argc, char **argv){
  /////////////////////////////////////////////////////////////////////////////////////////
  /* etape 0: lecture des options */
  /* penser a implementer de la programmation defensive (sur les options, qui ne
seront pas d office des int ou char*) */
/////////////////////////////////////////////////////////////////////////////////////////
  long int nthread = 1;
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

/////////////////////////////////////////////////////////////////////////////////////////
  /* etape 1: lecture des fichiers de hash
  il faut des threads (un par type d entree)
  */
  /* on cherche tous les fichier .bin (a lire) */

  /* A faire : maintenant que les fichiers a lire (.bin) sont stockes dans un tableau, il
  faut les differencier, selon leur provenance et les lire */
  /////////////////////////////////////////////////////////////////////////////////////////

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


/////////////////////////////////////////////////////////////////////////////////////////
  /* etape 2: reversehash
  creation de tous les thread de reversehash
  ATTENTION: il faut leur dire quand s arreter!!!
  */
/////////////////////////////////////////////////////////////////////////////////////////
  pthread_t thread_reverseHash;
  for(i = 0; i < nthread; i++)
  {
    if(pthread_create(&thread_reverseHash, NULL, reverseHashFunc, NULL) == -1)
    {
      perror("pthread_create");
      return EXIT_FAILURE;
    }
  }

  /* rajouter phtread_join non ? */

  return EXIT_SUCCESS;
} //fin de la main()
