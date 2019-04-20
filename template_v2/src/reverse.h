#ifndef _REVERSE_H
#define _REVERSE_H

#include <stdint.h>
#include <stdbool.h>

/* reversehash
 * Calcule l'inverse d'un hash SHA-256 par bruteforce.
 * L'espace de recherche est limité aux lettres minuscules.
 *
 * @hash: le hash SHA-256 (32 bytes) à inverser
 * @res: adresse où sera écrit l'inverse du hash
 * @len: longueur maximale de l'inverse
 * @return: true si un inverse a été trouvé, false sinon
 */
bool reversehash(const uint8_t *hash, char *res, size_t len);

#endif
