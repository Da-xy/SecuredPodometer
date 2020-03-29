#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void decipher(uint8_t num_rounds, uint32_t* v, uint32_t* k) {
  uint32_t v0 = v[0], v1 = v[1], i;
  uint32_t delta = 0x9E3779B9, sum = delta * num_rounds;
  for (i = 0; i < num_rounds; i++) {
    v1 -= ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum >> 11 & 3]);
    sum -= delta;
    v0 -= ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
  }
  v[0] = v0; v[1] = v1;
}

uint32_t * stringToLong(char * chaine, int * taille){
	int i;
	uint32_t * v = NULL;
	char buffer[9];

	*taille = (strlen(chaine) % 8 == 0) ? strlen(chaine) / 8 : strlen(chaine) / 8 + 1;
	if(chaine != NULL && taille > 0){
		v = (uint32_t *)malloc(*taille * sizeof(uint32_t));
		for(i=0; i<*taille; ++i){
			memcpy(buffer, &chaine[i*8], 8);
			buffer[8] = '\0';
			v[i] = strtoul(buffer, NULL, 16);
		}

	}
	return v;
}

void afficherTabUINT32(uint32_t* v, int taille){
	int i;
	for(i = 0; i < taille ; ++i){
		printf("%08x", v[i]);
	}
  printf("\n");
}

int main(int argc, char ** argv){
	uint32_t* v,*key;
	int taille1 = -1, taille2 = -1;

	if(argc >= 3){
		v = stringToLong(argv[1], &taille1);
		key = stringToLong(argv[2], &taille2);

		decipher(64, v, key);
		afficherTabUINT32(v, taille1);


		free(v);
		free(key);
	}
	return 0;
}
