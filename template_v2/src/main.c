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
#include <getopt.h>
#include <semaphore.h>

#include "reverse.h"
#include "sha256.h"

/////////////////////////////////////////////////////////////////////////////////////////
/*
Introduction au programme:
Ce programme a pour objectif de proposer une liste de mots de passe decrytptes,
a partir de fichiers comprenant les hashs de ces mots de passe.

Il est evident que ce programme n est pas fonctionnel. Cependant, en se basant
sur le rapport, il est possible de comprendre l idee derriere ce programme.

Afin de permettre au lecteur de se reperer dans ce programme qui n a pas ete decoupe
en plusieurs fichiers ".c" et ".h", voici la structure globale de celui-ci:

-lignes 45 a 79 : variables globales
-lignes 82 a 248 : fonctions utilitaires
-lignes 252 a 375 : thread de lecture suivi du thread de reversehash
-lignes 378 a 541 : fonction main

*/
/////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////
/*
Variables globales du programme
*/
/////////////////////////////////////////////////////////////////////////////////////////

int TAILLEFICHIERLIRE;
int HashBufSize;
bool consonne = false;
long int nthread = 1;
uint8_t ** HashBuf;

typedef struct Candidats
{
  struct Candidats * next;
  char codeclair[16];  //représente le mot de passe reverse-hashed
  int nbrOccurence;
} Candid_Node;

typedef struct list {
  struct Candidats * head;
  struct Candidats * last;
  int nbrOccMax;
} list_t;

size_t sizeReverseMdp = 16;
uint8_t indexG = 0;
list_t * ListCandidat;

/* varProd = variable indiquant que le thread de lecture continue à lire :
vaut 1 si le thread est en train de lire
vaut 0 si le thread de lecture a terminé*/
int varProd = 1;
/* variable indiquant quand les threads consommateurs ont fini de consommer les hashs*/
int varFinito = 0;

pthread_mutex_t mutexIndex;
pthread_mutex_t mutexTAILLEFICHIERLIRE;

sem_t semHashBufEmpty;
sem_t semHashBufFull;


/* Fonctions utilitaires */
/////////////////////////////////////////////////////////////////////////////////////////
/*
Fonction qui affiche sur la sortie standard les codes clairs de la liste ListCandidat
retourne 0 si les candidats ont été affichés
retourne -1 sinon
*/
/////////////////////////////////////////////////////////////////////////////////////////
int displayStd(list_t * ListCandidat)
{
  struct Candidats * runner = ListCandidat->head;
  while(runner != NULL)
  {
    char * tobeprinted = runner->codeclair;
    printf("%s\n", tobeprinted);
    struct Candidats *tmp = runner;
    runner = runner->next;
    free(tmp);
  }
  return 0;
  return -1;
}

int displaySpec(list_t * ListCandidat)
{
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*
Fonction créant un noeud
@pre: prend un code clair (reversehash) en entrée
retourne un pointeur vers le noeud créé
retourne NULL si le noeud n'a pas pu etre créé
*/
/////////////////////////////////////////////////////////////////////////////////////////
Candid_Node *init_node(char * codeClair)
{
  Candid_Node *a=NULL;
  a = (Candid_Node *) malloc(sizeof(Candid_Node));
  if(!a)
    return NULL;

  strcpy(a->codeclair,codeClair);
  a->next = NULL;
  return a;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*
Fonction créant et puis ajoutant un noeud à la suite d'une liste
retourne 0 si le noeud a été ajouté
retourne -1 sinon
*/
/////////////////////////////////////////////////////////////////////////////////////////
int add_node(struct list * list, char * codeClair)
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
  list->last->next = a;
  list->last = a;
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
/*
Fonction de comparaison
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
Calcule le nombre d'occurence de voyelles/consonnes (selon le bool consonne) d'un candidat
et modifie dans la struct Candidats le paramètre nbrOccurence .
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
    if(localString[i] == 'a' || localString[i] == 'e' || localString[i] == 'y' || localString[i] == 'u' || localString[i] == 'i' || localString[i] == 'o')
    {
      nbrOccLocal += 1;
    }
  }
  /* si l'option consonne est activée, on doit trier selon le nombre de consonnes */
  if(consonne)
  {
    nbrOccLocal = (strlen(localString) - 1 - nbrOccLocal);
  }
  Node->nbrOccurence = nbrOccLocal;
  if(ListCandidat->nbrOccMax < Node->nbrOccurence)
  {
    ListCandidat->nbrOccMax = Node->nbrOccurence ;
  }
  return;
}


/////////////////////////////////////////////////////////////////////////////////////////
/*
Fonction dont le but est d'enlever les candidats dont le nombre nbrOccurence
n'est pas égal au nbrOccurence le plus grand
retourne 1 en cas d'exécution correcte,
retourne -1 en cas d'échec.
*/
/////////////////////////////////////////////////////////////////////////////////////////
int trieur(list_t *ListCandidat)
{
  /* le cas d'une liste vide est considéré comme une liste triée */
  if(ListCandidat->head == NULL)
  {
    return 1;
  }
  /* ce noeud parcourt la liste pour trouver les noeuds à supprimer */
  struct Candidats * runner = ListCandidat->head;
  while(runner != NULL)
  {
    /* il faut enlever le noeud suivant */
    if(compare(runner->next->nbrOccurence, ListCandidat->nbrOccMax) == -1)
    {
      struct Candidats * tmp = runner->next;
      runner->next = runner->next->next;
      free(tmp);
    }
    /* ListCandidat->nbrOccMax n est pas le plus grand nombre d occurence*/
    if(compare(runner->next->nbrOccurence, ListCandidat->nbrOccMax) == 1)
    {
      printf("La valeur de ListCandidat->nbrOccMax n'est pas valide\n");
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
*/
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
/*
0- Thread de lecture : prend un tableau de noms de fichiers .bin en argument, lit
ces fichiers et stocke les hashs lus dans un buffer nomme HashBuf.
*/
/////////////////////////////////////////////////////////////////////////////////////////
void *lecture(void *fichiers)
{
  pthread_mutex_lock(&mutexTAILLEFICHIERLIRE);
  int argc = TAILLEFICHIERLIRE;
  pthread_mutex_unlock(&mutexTAILLEFICHIERLIRE);
  char ** fichs = (char **) fichiers;
  int i;
  uint8_t * buf;
  for(i = 0; i < argc ; i++)
  {
    printf("Préparation de l ouverture du fichier %s\n", fichs[i]);
    int fd = open(fichs[i], O_RDONLY);
    if(fd == -1)
    {
      printf("Echec de l ouverture du fichier %s\n", fichs[i]);
      pthread_exit(NULL); //fails to open ok
    }
    printf("Lecture du fichier numero %d\n", i);
    int size = 32 * sizeof(uint8_t);
    buf = (uint8_t *) malloc(size);
    int rd = read(fd, (void *) buf, size);
    while(rd > 0)
    {
      sem_wait(&semHashBufEmpty);
      pthread_mutex_lock(&mutexIndex);
      HashBuf[indexG] = buf ;
      buf = (uint8_t *) malloc(size);
      indexG += 1;
      rd = read(fd,(void *) buf, size);
      sem_post(&semHashBufFull); /* et oui, on a ajoute un element au tableau */
      if(indexG < nthread-1)
      {
        sem_post(&semHashBufEmpty);
      }
      pthread_mutex_unlock(&mutexIndex);
    }
    if(rd == 0)
    {
      printf("Fermeture du fichier\n");
      int err;
      err = close(fd);
      if(err==-1)
      {
        printf("Echec de la fermeture du fichier\n");
        pthread_exit(NULL);
      }
    }
    if( rd < 0)
    {
      int err;
      err = close(fd);
      if(err==-1)
      {
        printf("Echec de la fermeture du fichier\n");
        pthread_exit(NULL);
      }
      printf("Echec de la lecture du fichier. Ce fichier a pu etre ferme\n");
      pthread_exit(NULL); //fails to read ok
    }
    printf("Le fichier a ete ouvert, lu et ferme\n");
  }
  varProd = 0; //Le thread de lecture se finit.
  printf("Tous les fichiers ont ete lus, ouverts et fermes\n");
  sem_destroy(&semHashBufEmpty);
  free(buf); //on free la mémoire allouée précédemment
  pthread_exit(NULL);
}


/////////////////////////////////////////////////////////////////////////////////////////
/*
1- Thread d'écriture : se nourrit directement dans le tableau HashBuf, reversehash
ses elements un par un et stocke les reversehash dans un tableau Reversed.
*/
/////////////////////////////////////////////////////////////////////////////////////////
void *reverseHashFunc()
{
  while(indexG >= 0 && varProd)
  {
    char * candid = malloc(17*sizeof(char));
    sem_wait(&semHashBufFull);
    pthread_mutex_lock(&mutexIndex);
    indexG -= 1;
    uint8_t * localHash = HashBuf[indexG];
    HashBuf[indexG] = NULL;
    pthread_mutex_unlock(&mutexIndex);
    sem_post(&semHashBufEmpty); /* et oui, une place vient de se liberer */
    bool err = reversehash(localHash, candid, 16);
    printf("MDP trouve : %s \n", candid);
    if(!err)
    {
      printf("Aucun inverse n a ete trouve\n");
    }
    if(err)
    {
      int ret = add_node(ListCandidat, candid);
      if(ret == -1)
      {
        printf("Erreur dans l ajout des noeuds a la liste des candidats");
      }
      return (void *) 0;
    }
    else
    {
      printf("Aucun candidat n a pu etre trouve pour au moins un hash");
    }
  } //Fin de la boucle while
  varFinito += 1;
  pthread_exit(NULL);
}


/*
les etapes du programme:
0- lire les arguments
1- lire les fichiers
2- utiliser la fonction reversehash(const uint8_t *hash, char *res, size_t len);
3- trier
4- display ligne par ligne

Fonction main :
retourne 0 si l execution a pu se terminer
retourne -1 sinon.
*/
int main(int argc, char **argv)
{
  sem_init(&semHashBufEmpty,0,1);
  sem_init(&semHashBufFull, 0, 0);
//////////////////////////////////////////////
  /* Etape 0: lecture des options */
//////////////////////////////////////////////

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
  if (optind == argc)
  {
        fprintf (stderr, "Il est necessaire de specifier au moins un argument.\n");
        return -1;
  }

  /* petite section de test de verification des options */
  printf("Nombre de threads: %ld \n", nthread);
  printf("Tri par consonne? %s \n", consonne ? "true" : "false");
  if (fichierout != NULL)
    printf("Le fichier de sortie a ete specifie comme: %s \n", fichierout);
  printf("argc: %d, optind: %d \n", argc, optind);
  /* fin de la petite section de test des options */

/////////////////////////////////////////////////////////////////////////////////////////
  /* Etape 1: lecture des fichiers de hash
  il faut des threads (un par type d entree)
  */
  /* on cherche tous les fichier .bin (a lire) */
/////////////////////////////////////////////////////////////////////////////////////////

  int i ;
  int placeFich = 0 ;
  HashBufSize = nthread;
  HashBuf = (uint8_t **) malloc(nthread * sizeof(uint8_t*));
  if(!HashBuf)
  {
    printf("not Hashbuf due to malloc\n");
  }
  if (HashBuf)
  {
    for (i=0; i < nthread; i++)
    {
      HashBuf[i] = NULL;
    }
  }

  pthread_mutex_lock(&mutexTAILLEFICHIERLIRE);
  TAILLEFICHIERLIRE = argc-optind;
  char *fichs[TAILLEFICHIERLIRE];
  pthread_mutex_unlock(&mutexTAILLEFICHIERLIRE);
  for (i = optind; i < argc; i++)
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
  /* Etape 2: reversehash
  Creation de tous les threads de reversehash
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
/////////////////////////////////////////////////////////////////////////////////////////
/* Etape 3: trieur
Appel a la fonction trieur qui supprime tous les mauvais candidats de la liste chainee
*/
/////////////////////////////////////////////////////////////////////////////////////////
  while(varFinito != nthread)
  {
    continue;
  }
  int retTri = trieur(ListCandidat);
  if (retTri == -1)
  {
    printf("La liste de candidats n a pas pu être triee\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  /* Etape 4: display ligne par ligne
  Appel a la fonction display qui ecrit sur soit la sortie standard, soit dans un fichier
  précisé, les bons candidats, un par ligne
  */
  /////////////////////////////////////////////////////////////////////////////////////////
  //affichage sur la stortie standard
  if(fichierout == NULL)
  {
    int retDisp = displayStd(ListCandidat);
    if(retDisp == -1)
    {
      printf("Les candidats n ont pas pu etre affiches\n");
    }
    return 0;
  }
  //remplit le fichier "fichierout" qui a ete specifie.
  //doit encore etre implemente.
  else
  {
    int retDispSpec = displaySpec(ListCandidat);
    if(retDispSpec == -1)
    {
      printf("Les candidats n ont pas pu etre affiches\n");
    }
    return 0;
  }
  free(HashBuf);
  return EXIT_SUCCESS;
} //fin de la main()
