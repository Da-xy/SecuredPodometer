#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

uint32_t * xorTabUINT32(uint32_t* tab1, uint32_t* tab2){
	  int i;
	 	uint32_t * res = (uint32_t *)malloc(sizeof(uint32_t)*4);
		for (int i = 0; i < 4; ++i) {
		    res[i] = tab1[i % 2] ^ tab2[i];
		}
	  return res;
}

int main(int argc, char ** argv){
		uint32_t* s1,*s2, *res;
		int taille1 = -1, taille2 = -1;

		if(argc >= 3){
			s1 = stringToLong(argv[1], &taille1);
			s2 = stringToLong(argv[2], &taille2);

	    res = xorTabUINT32(s1,s2);

			afficherTabUINT32(res, taille2);


			free(s1);
			free(s2);
	    free(res);

  	}
		return 0;
}
