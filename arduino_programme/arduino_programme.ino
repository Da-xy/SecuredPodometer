#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <Wire.h>
#include <ADXL345.h>

ADXL345 adxl; //variable adxl is an instance of the ADXL345 library

boolean AQC = true; //Détection de l'acquisition des données pour les tests
int PERIODE_AQC = 10; //Intervalle de temps entre chaque acquisition en millisecondes
double xyz[3]; //Tableau dans lequel sont stockées les valeurs retournées par l'acceleromètre
char clavier; //Detection des touches du clavier sur le moniteur serie
double ValueRef = 0.9; //Valeur de référence de la norme des accélérations sur x, y, z pour compter les pas
const int nmax = 19;
int h = nmax;
uint8_t nb = nmax;
double bbb[nmax];
uint8_t na = nmax;
double aaa[nmax];
double www[nmax];
double historique[nmax];
int bug = 0;
bool flagdetectpas = false;

uint16_t compteurTemps;
uint8_t compteurMessages;
uint16_t nbPas = 0;
uint32_t * key;
uint32_t * v;

void setup() {
  int i;

  randomSeed(analogRead(0)); // Initialise la graine pour le generateur aléatoire

  compteurTemps = 0;
  compteurMessages = 0;

  v = (uint32_t *)calloc(2, sizeof(uint32_t)); // Tableaux contenant les messages à chiffrés
  key = (uint32_t *)calloc(4, sizeof(uint32_t)); // Tableaux contenant la clé qui servira à chiffrer


  for (int i = 0; i < 2; ++i) { // Initialisation de la clé
    key[i] = 0x12345678;
  }
  for (int i = 2; i < 4; ++i) {
    key[i] = 0x87654321;
  }

  Serial.begin(9600);
  adxl.powerOn();
  SetFilter();
  SetHistorique();
  bug = 0;
  while (!Serial) {};

  if (!SigFox.begin()) {
    Serial.println("Shield error or not present!");
    return;
  }

  delay(100);

  // Send the module to the deepest sleep
  SigFox.end();
  SigFox.debug();


  String s;
  Serial.println("Entre \'A\' pour démarrer");
  while (!Serial.available());
  while (Serial.available()) {
    s += (char)Serial.read();
  }
  Serial.println(s.charAt(0));
  if (s.charAt(0) == 'A') {
    Serial.println("Envoi..");
    //sendPas(nbPas);
  }
  else {
    Serial.println("Mauvaise entrée");
  }
  while (!changeKey()) { // Changement de la clé au premier lancement si, échec attente 10 minutes
    delay(600000);
  }
  sendChangeOK(); // Renvoie vérification

  Serial.println("Lancement de l'aquisition");
  
}

void loop() // Boucle principale
{
  float valeur = 0;
  compteurTemps++; 
  
  if (compteurTemps > 60000) { // Si temps = 10 minutes on envoie le nombre de pas et on réinitialise
    Serial.println("Envoi des données");
    compteurTemps = 0;
    compteurMessages++;
    sendPas(nbPas);
    nbPas = 0;
  }

  if (compteurMessages > 140) { // Si message > 140 alors on change de clé 
    while (changeKey());
    sendChangeOK();
    compteurMessages = 0;
  }

  if (AQC) {
    valeur = adc_filtrage_float_FD2();
    if (valeur > 500 && !flagdetectpas) {
      nbPas++;
      flagdetectpas = true;
      delay(500);
      Serial.println(nbPas);
    }
    else if (valeur < 10 && flagdetectpas) {
      flagdetectpas = false;
    }
    
    delay(PERIODE_AQC);
  }
  delay(10);
}

//Fonction qui permet de mettre le nombre de pas dans le tableau d'entiers 32 bits
void stepsInV(uint16_t steps, uint32_t * v, uint8_t pos) {
  if (pos <= 16) {
    v[0] = steps;
    v[0] = v[0] << 16;
    v[0] = v[0] >> pos;
  }
  else {
    Serial.println("Erreur décalage trop important");
  }
}

// Fonction qui permet de tranformer un tableau d'entiers 32 bits en un tableau d'entiers 8 bits
uint8_t * uint_32ToByteTab(uint32_t *v, int taille) {
  uint8_t * tab;
  uint8_t indice = 0;
  tab = (uint8_t *)calloc(taille * 4, sizeof(uint8_t));
  for (int i = 0; i < taille; ++i) {
    for (int j = 31; j > -1; --j) {
      tab[4 * i + (3 - j / 8)] += (v[i] & 1 << j) >> (j / 8) * 8;
    }
  }
  return tab;
}
// Envoi le nombre de pas chiffré avec la clé en vigueur
void sendPas(uint16_t pas) {
  byte * buff;

  cleanBuffer32(v, 2);

  stepsInV(pas, v, 0);
  encipher(64, v, key);
  buff = uint_32ToByteTab(v, 2);
  SigFox.begin();
  delay(100);
  SigFox.beginPacket();
  SigFox.write(buff, 8);

  int ret = SigFox.endPacket();
  if (ret > 0 ) {
    Serial.println("OK");
  }
  else {
    Serial.println("KO");
  }

  SigFox.end();

  free(buff);
}


// Chiffrement XTea
void encipher(unsigned int num_rounds, unsigned long* v, unsigned long* k) {
  unsigned long v0 = v[0], v1 = v[1], i;
  unsigned long sum = 0, delta = 0x9E3779B9;
  for (i = 0; i < num_rounds; i++) {
    v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
    sum += delta;
    v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum >> 11 & 3]);
  }
  v[0] = v0; v[1] = v1;
}

//Dechiffrement XTEA
void decipher(unsigned int num_rounds, unsigned long* v, unsigned long* k) {
  unsigned long v0 = v[0], v1 = v[1], i;
  unsigned long delta = 0x9E3779B9, sum = delta * num_rounds;
  for (i = 0; i < num_rounds; i++) {
    v1 -= ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum >> 11 & 3]);
    sum -= delta;
    v0 -= ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
  }
  v[0] = v0; v[1] = v1;
}

//Debuggage Affichage en binaire des tableaux entiers 32 ou 8 bits
void afficheTabByte(uint8_t * tab, uint8_t nbVectors) {
  if (Serial) {
    for (uint8_t i = 0; i < nbVectors; ++i) {
      for (int j = 7; j > -1; j--) {
        if (tab[i] & (1 << j)) {
          Serial.print ("1");
        }
        else {
          Serial.print ("0");
        }
      }
      Serial.print(" ");
    }
    Serial.println();
  }
}

void afficheTab32(uint32_t * tab, uint8_t nbVectors) {
  if (Serial) {
    for (uint8_t i = 0; i < nbVectors; ++i) {
      for (int j = 31; j > -1; j--) {
        if (tab[i] & (1 << j)) {
          Serial.print ("1");
        }
        else {
          Serial.print ("0");
        }
      }
      Serial.print(" ");
      Serial.println(tab[i], HEX);
    }
    Serial.println();
  }
}


// Fonction de filtrage des données
float adc_filtrage_float_FD2() {
  double accum, valeur;
  uint8_t i, k;
  int signe = 0;
  accum = AcqDonnee();
  i = 2 * nmax - 1;
  for (k = 1; k < na; k++) {
    accum -= www[i % nmax] * aaa[k];
    i--;
  }
  www[h % nmax] = accum;
  accum = 0;
  i = h % nmax;
  for (k = 0; k < nb; k++) {
    accum += www[i % nmax] * bbb[k];

    i--;
  }
  if (accum < 0) {
    accum = accum * -1;
  }
  h++;
  return accum;
}

// Initialisation du filtre
void SetFilter() {
  aaa[0] = 3.82E-013;
  aaa[1] = 6.87E-012;
  aaa[2] = 5.84E-011;
  aaa[3] = 3.11E-010;
  aaa[4] = 1.17E-009;
  aaa[5] = 3.27E-009;
  aaa[6] = 7.09E-009;
  aaa[7] = 1.21E-008;
  aaa[8] = 1.67E-008;
  aaa[9] = 1.86E-008;
  aaa[10] = 1.67E-008;
  aaa[11] = 1.21E-008;
  aaa[12] = 7.09E-009;
  aaa[13] = 3.27E-009;
  aaa[14] = 1.17E-009;
  aaa[15] = 3.11E-010;
  aaa[16] = 5.84E-011;
  aaa[17] = 6.87E-012;
  aaa[18] = 3.82E-013;

  bbb[0] = 1;
  bbb[1] = -12.6532725882;
  bbb[2] = 76.206148953;
  bbb[3] = -290.1235233153;
  bbb[4] = 782.0734426798;
  bbb[5] = -1584.5712385922;
  bbb[6] = 2499.9646185133;
  bbb[7] = -3139.3250557579;
  bbb[8] = 3179.9863418273;
  bbb[9] = -2617.1256446536;
  bbb[10] = 1753.7280666607;
  bbb[11] = -954.3638325853;
  bbb[12] = 418.5597791403;
  bbb[13] = -145.9095860481;
  bbb[14] = 39.5331100557;
  bbb[15] = -8.0317790871;
  bbb[16] = 1.1520638936;
  bbb[17] = -0.1040977694;
  bbb[18] = 0.0044587735;
}

// Remise à zéro de l'historique
void SetHistorique() {
  for (int l = 0; l < nmax; l++)
  {
    www[l] = 0;
    historique[l] = 0;
  }
}

// Acquisition des données de l'accéléromètre 
double AcqDonnee() {
  double valeur;
  adxl.getAcceleration(xyz);
  valeur = (sqrt(pow(xyz[0], 2) + pow(xyz[1], 2) + pow(xyz[2], 2)));
  return valeur;
}

// affichage de l'historique
void affHisto() {
  int i;
  for (i = 0; i < nmax; i++) {
    Serial.print(www[i]);
    Serial.print(" ; ");
  }
}

// Nettoyage du buffer avant réception
void cleanBuffer32(uint32_t * v, int taille) {
  for (int i = 0; i < taille; ++i) {
    bzero(v + i, 4);
  }
}

// Assignation d'un nombre aléatoire dans un tableau
void assignRandNumber32(uint32_t * v, int taille) {
  for (int i = 0; i < taille; ++i) {
    v[i] = random(0, 0x7FFFFFFF);
  }
}

// Fonction de changement de clé
bool changeKey() {
  uint8_t * buff;
  uint32_t * rand1 = (uint32_t *)malloc(2 * sizeof(uint32_t));
  uint32_t * randSave = (uint32_t *)malloc(2 * sizeof(uint32_t));
  uint32_t * reception = (uint32_t *)malloc(2 * sizeof(uint32_t));
  uint32_t * newKey;
  uint8_t i = 0;
  bool changement = false;
  assignRandNumber32(rand1, 2);
  memcpy(randSave, rand1, 8);
  
  newKey = generateKey(rand1, key);
  
  encipher(64, rand1, key);
  buff = uint_32ToByteTab(rand1, 2);
  
  // Start the module
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  SigFox.beginPacket();
  SigFox.write(buff, 8);

  int ret = SigFox.endPacket(true);  // send buffer to SIGFOX network and wait for a response
  if (ret > 0) {
    Serial.println("No transmission");
  } else {
    Serial.println("Transmission ok");
  }

  Serial.println(SigFox.status(SIGFOX));
  Serial.println(SigFox.status(ATMEL));

  if (SigFox.parsePacket()) {
    while (SigFox.available()) {
      buff[i] = SigFox.read();
      ++i;
    }
    
    byteInLong(reception, buff);
  } else {
    Serial.println("Could not get any response from the server");
    Serial.println("Check the SigFox coverage in your area");
    Serial.println("If you are indoor, check the 20dB coverage or move near a window");
  }


  SigFox.end();
  decipher(64, reception, newKey);

  if (reception[0] == randSave[0] && reception[1] == randSave[1]) {
    Serial.println("Changement de clé OK");
    memcpy(key, newKey, 16);
    changement = true;
  }
  else {
    Serial.println("Mauvais échange de clé, mettez la meme clé sur le serveur");
    Serial.println("Clé générée");
    afficheTab32(newKey, 4);
  }

  free(buff);
  free(rand1);
  free(reception);
  free(newKey);

  return changement;
}

// Tableau dans de byte dans un uint32
void byteInLong(uint32_t * v, uint8_t * b) {
  for (int i = 0; i < 2; ++i) {
    v[i] = 0;
    for (int j = 31; j > -1; --j) {
      if (b[(4 * i) + (3 - j / 8)] & (1 << j % 8)) {
        v[i] += (1 << j);
      }
    }
  }
}


// Génération de clés aléatoire
uint32_t * generateKey(uint32_t * number, uint32_t * key) {
  uint32_t * newKey = (uint32_t *)malloc(sizeof(uint32_t) * 4);
  for (int i = 0; i < 4; ++i) {
    newKey[i] = number[i % 2] ^ key[i];
  }
  encipher(64, newKey, key);
  encipher(64, newKey + 2, key);

  return newKey;
}

// Confirmation changement de clé
void sendChangeOK() {
  uint8_t * buff;
  uint32_t * change = (uint32_t *)malloc(sizeof(uint32_t)*2);
  change[0] = 0x4348414E;
  change[1] = 0x47454f4b;

  encipher(64, change, key);

  buff = uint_32ToByteTab(change, 2);

  // Start the module
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  SigFox.beginPacket();
  SigFox.write(buff, 8);

  int ret = SigFox.endPacket();  // send buffer to SIGFOX network and wait for a response
  if (ret > 0) {
    Serial.println("No transmission");
  } else {
    Serial.println("Transmission ok");
  }

  Serial.println(SigFox.status(SIGFOX));
  Serial.println(SigFox.status(ATMEL));
}




