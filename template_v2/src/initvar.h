#ifndef _INITVAR_H
#define _INITVAR_H

/*
fonction d'initialisation des semaphores
@name: le nom du semaphore.
@val: la valeur initiale du semaphore, positive ou nulle.
@return: un semaphore de nom name a ete cree, dont la valeur initiale est val.


void *init_sem(char * name, int val);
*/


/*
fonction de lecture associee au thread de lecture
@fichiers: un tbleau contenant le noms de tous les fichiers devant etre lus.
@return: ne renvoie rien
*/

void *lecture(void *fichiers);


/*
fonction de reversehash associee au thread de reversehash
@return: ne renvoie rien
*/

void *reverseHashFunc();

/*
fonction triant les candidats en selon l'occurence des consonnes ou voyelles
@return: 1 quand fini.
*/
int trieur(*bufReverseHash);

#endif
