#ifndef _INIT_VAR.H
#define _INIT_VAR.H


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

#endif
