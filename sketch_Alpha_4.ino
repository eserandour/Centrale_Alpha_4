//////////////////////////////////////////////////////////////////////////////////////////
/*
   Centrale Alpha 4
   Copyright 2013-2021 - Eric Sérandour
   http://3615.entropie.org
*/
const String VERSION = "2021.04.10";  // 22 h 25
/*   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
//////////////////////////////////////////////////////////////////////////////////////////

/*
  L'électronique :
  
  * Carte Arduino Mega 2560 R3.
  * Shield mémoire 2.0 de Snootlab (carte SD + horloge RTC).
  * Afficheur LCD 4x20 caractères DEM 20485 SYH-LY.
  * Clavier 12 touches matricielles KB12M.

  Le circuit :
  
    Les entrées analogiques :
  =============================
  * Entrée blanche => A0
  * Entrée bleue => A1
  * Entrée jaune => A2
 
    Les entrées / sorties numériques :
  ======================================
  * Afficheur LCD sur D2, D3, D4, D5, D6, D7 de la carte Arduino
    Brochage de l'afficheur compatible HD44780 :
    1  : Vss : GND (fil noir)
    2  : Vdd : Power Supply +5V (fil rouge)
    3  : V0 : Contrast Adjust. Point milieu du potentiomètre 1kohms (fil gris)
    4  : RS : Register Select Signal. => D2 de l'Arduino (fil bleu)
    5  : R/W : Data Read/Write. Relié à GND (fil noir)
    6  : E : Enable Signal. => D3 de l'Arduino (fil vert)
    7  : non connecté
    8  : non connecté
    9  : non connecté
    10 : non connecté
    11 : DB4 : Data Bus Line. => D4 de l'arduino (fil jaune)
    12 : DB5 : Data Bus Line. => D5 de l'arduino (fil orange)
    13 : DB6 : Data Bus Line. => D6 de l'arduino (fil rouge)
    14 : DB7 : Data Bus Line. => D7 de l'arduino (fil marron)
    15 : LED+ : Power supply for BKL(+). Relié à +5V (fil rouge)
    16 : LED- : Power supply for BKL(-). Relié à GND (fil noir)
  * LED verte, jaune, rouge sur D24, D26, D28 (chacune en série avec une résistance de 330 ohms)
  * Buzzer sur D8.        D8 -> R = 1 kohms -> Buzzer -> GND.
  * Entrées numérique sur D32 et D34. (Entrées vertes)
  * Clavier sur D42, D44, D46, D48 (pour les lignes du clavier).
                D36, D38, D40 (pour les colonnes du clavier).
    Brochage du clavier KB12M acheté chez Gotronic (vu de dessous) :
    R = Rangée. C = Colonne.
    1 (C3) => D36 de l'Arduino
    2 (C2) => D38 de l'Arduino
    3 (R1) => D48 de l'Arduino
    4 (C1) => D40 de l'Arduino
    5 (R2) => D46 de l'Arduino
    6 (C2) => Non connecté (car les broches 2 et 6 sont reliées en interne)
    7 (R3) => D44 de l'Arduino
    8 (C1) => D40 de l'Arduino (les broches 4 et 8 ne sont pas reliées en interne)
    9 (R4) => D42 de l'Arduino
  * La carte SD est reliée au bus SPI (bus série normalisé) de la manière suivante :
  ** SS - D10 du shield Snootlab -> D53. (voir la constante CHIP_SELECT)
  ** MOSI - D11 du shield Snootlab -> D51. (Master Out Slave In : Sortie de données séries)
  ** MISO - D12 du shield Snootlab -> D50. (Master In Slave Out : Entrée de données séries)
  ** SCK - D13 du shield Snootlab -> D52. (Serial ClocK : Pour synchroniser les échanges de données)
  Remarque : on peut accéder aux broches MOSI, MISO et SCK par l'intermédiaire du connecteur ICSP.
  L'intérêt est que cela ne demande pas un recablage lorsque que l'on passe de la Mega à la Uno
  et inversement. Reste le cas de SS.

    Les ports de communication :
  ======================================
  L'horloge RTC du shield mémoire est reliée à :
  * Bus I2C : SDA (C20 sur l'Arduino Mega, A4 sur l'Arduino Uno)
  * Bus I2C : SCL (C21 sur l'Arduino Mega, A5 sur l'Arduino Uno)
  Remarque : on peut accéder à ces broches par l'intermédiaire du connecteur à 10 points,
  après AREF : SDA pour la 9ème broche et SCL pour la 10ème broche. L'intéret est que cela
  ne demande pas un recablage lorsque que l'on passe de la Mega à la Uno et inversement.
  Pour le shield Snootlab mémoire 2.0, ceci est exploité au niveau de la carte, pas pour
  la version 1.0.
  
    Bilan :
  ===========
    Sont utilisées : A0, A1, A2,                                // EA1, EA2, EA3
                     D2, D3, D4, D5, D6, D7,                    // Afficheur LCD
                     D8,                                        // Buzzer
                     D50, D51, D52, D53, D10, D11, D12, D13,    // Carte SD (Bus SPI : SS/MOSI/MISO/SCK)
                     D24, D26, D28,                             // Leds
                     D32, D34,                                  // EN1, EN2
                     D36, D38, D40, D42, D44, D46, D48,         // Clavier
                     C20, C21.                                  // Horloge RTC (Bus I2C : SDA/SCL)
*/

//////////////////////////////////////////////////////////////////////////////////////////

// *** Afficheur LCD
// On importe la bibliothèque
#include <LiquidCrystal.h>
// Initialisation de la bibliothèque avec les numéros de broches utilisées
LiquidCrystal lcd(2,3,4,5,6,7);   // Correspond sur l'afficheur à RS,Enable,DB4,DB5,DB6,DB7
const byte NB_LIGNES_LCD = 4;     // Nombre de lignes de l'écran
const byte NB_COLONNES_LCD = 20;  // Nombre de colonnes de l'écran
byte ligneLCD = 0;                // Numéro de la ligne
byte colonneLCD = 0;              // Numéro de la colonne
char ligne[NB_COLONNES_LCD]; // Pour le formatage des nombres avec la fonction dtostrf()

//////////////////////////////////////////////////////////////////////////////////////////

// *** Buzzer
const byte BUZZER = 8;

//////////////////////////////////////////////////////////////////////////////////////////

// *** LED
const byte LED_ROUGE = 24;      // Fil orange
const byte LED_JAUNE = 26;      // Fil rouge
const byte LED_VERTE = 28;      // Fil marron

//////////////////////////////////////////////////////////////////////////////////////////

// *** Clavier
// On importe la bibliothèque
#include <Keypad.h>  // http://playground.arduino.cc/Code/Keypad
const byte ROWS = 4; // Nombre de rangées du clavier
const byte COLS = 3; // Nombre de colonnes du clavier
const char KEYS[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {48,46,44,42}; // Rangées R1,R2,R3,R4
byte colPins[COLS] = {40,38,36};    // Colonnes C1,C2,C3
Keypad keypad = Keypad(makeKeymap(KEYS), rowPins, colPins, ROWS, COLS);
// Supérieurs à 99 pour permettre de rentrer au clavier des nombres à 2 chiffres,
const byte TOUCHE_DIESE = 100;  // car le code ASCII de # est 35 
const byte TOUCHE_ETOILE = 101; // et celui de * est 42.
//const char NO_KEY = '\0'; // null ou code 0 en ASCII => intégré à la bibliothèque Keypad

//////////////////////////////////////////////////////////////////////////////////////////

// *** Horloge RTC
// On importe les bibliothèques
#include <Wire.h>
#include <RTClib.h>  // https://github.com/adafruit/RTClib
RTC_DS1307 RTC;
DateTime DateHeure;
String Date;
String Heure;

//////////////////////////////////////////////////////////////////////////////////////////

// *** Carte SD
// On importe la bibliothèque
#include <SPI.h>
#include <SD.h>
// On the Ethernet Shield, CHIP_SELECT (SS) is pin 4. Note that even if it's not
// used as the SS pin, the hardware SS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const byte CHIP_SELECT = 53; // Chip Select de la carte SD.
File dataFile;
String enteteFichier;
const String SEPARATEUR = ";"; // Séparateur de données pour le tableur
                              // Ne pas choisir l'espace à cause de Date Heure

//////////////////////////////////////////////////////////////////////////////////////////

// *** Ecrans
const byte TAG_ECRAN_ACCUEIL = 0;
const byte TAG_MENU_PRINCIPAL = 1;
const byte TAG_REGLAGE_HORLOGE = 2;
const byte TAG_MENU_ENREGISTREUR = 3;
const byte TAG_MENU_CADENCE = 4;
const byte TAG_MENU_MANUEL = 5;
const byte TAG_MENU_CAPTEURS = 6;
const byte TAG_ENREGISTRER_FICHIER = 7;
const byte TAG_TRANSFERER_FICHIER = 8;
const byte TAG_ENTREES = 9;
byte ecran = TAG_ECRAN_ACCUEIL;
int defilement = 0;

//////////////////////////////////////////////////////////////////////////////////////////

// *** Gestion du temps
const byte MODE_MANUEL = 0;
boolean declencheurExterne = false;
const unsigned long CADENCE[9] = {
MODE_MANUEL, // MODE MANUEL
100,         // 100 MS
1000,        // 1 S
5000,        // 5 S
15000,       // 15 S
60000,       // 1 MIN
300000,      // 5 MIN
900000,      // 15 MIN
3600000      // 1 H
};
unsigned long cadenceDefaut = CADENCE[2]; // Valeur par défaut.
unsigned long deltaMesures = cadenceDefaut; // Intervalle entre 2 mesures (en ms).
unsigned long time = 0;
unsigned long timeOffset = 0;

//////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////////

// *** LES CAPTEURS (début)

//////////////////////////////////////////////////////////////////////////////////////////

#define AUCUN_CAPTEUR -1

// *** Entrées analogiques en façade
const byte EA1 = 0;  // Entrée blanche. Entrée analogique A0.
const byte EA2 = 1;  // Entrée bleue. Entrée analogique A1.
const byte EA3 = 2;  // Entrée jaune. Entrée analogique A2.

// *** Entrées numériques en façade
const byte EN1 = 32;           // Entrée verte. Entrée numérique D32.
const byte EN2 = 34;           // Entrée verte. Entrée numérique D34.
const byte DECLENCHEUR = EN2;  // Pour déclencher un enregistrement en mode manuel

// *** Entrées disponibles
// Modifier en conséquence la procédure lectureCapteurs() juste ci-dessous
const byte NB_ENTREES_MAX = 5;
const byte ENTREE[NB_ENTREES_MAX] = {
  EA1,
  EA2,
  EA3,
  EN1,
  EN2
};
const String NOM_ENTREE[NB_ENTREES_MAX] = {
  "EA1 (BLANCHE)",
  "EA2 (BLEUE)",
  "EA3 (JAUNE)",
  "EN1 (VERTE)",
  "EN2 (VERTE)"
};
const String NOM_MESURE[NB_ENTREES_MAX] = {
  "EA1",
  "EA2",
  "EA3",
  "EN1",
  "EN2"
};

// *** Variables
boolean selectionCapteur[NB_ENTREES_MAX];
int nCapteurs;
int adresseCapteur[NB_ENTREES_MAX];
int mesureBrute[NB_ENTREES_MAX];

//////////////////////////////////////////////////////////////////////////////////////////

void initCapteurs()
{
  // Initialisation des entrées
  nCapteurs = 0;
  for (int i=0; i<NB_ENTREES_MAX; i++) {
    adresseCapteur[i] = AUCUN_CAPTEUR;
    selectionCapteur[i] = false;
  }
  // Initialisation de l'entete du fichier
  enteteFichier = "";
}

//////////////////////////////////////////////////////////////////////////////////////

void lectureCapteurs()
{
  int j=0;
  for (int i=0; i<nCapteurs ; i++) {
    if (adresseCapteur[i] == ENTREE[0]) {       // Entrée blanche (analogique)
      mesureBrute[j] = analogRead(ENTREE[0]);
    }
    else if (adresseCapteur[i] == ENTREE[1]) {  // Entrée bleue (analogique)
      mesureBrute[j] = analogRead(ENTREE[1]);
    }
    else if (adresseCapteur[i] == ENTREE[2]) {  // Entrée jaune (analogique)
      mesureBrute[j] = analogRead(ENTREE[2]);
    }
    else if (adresseCapteur[i] == ENTREE[3]) {  // Entrée verte 1 (numérique)
      mesureBrute[j] = digitalRead(ENTREE[3]);
    }
    else if (adresseCapteur[i] == ENTREE[4]) {  // Entrée verte 2 (numérique)
      mesureBrute[j] = digitalRead(ENTREE[4]);
    }
    j++;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////

// *** LES CAPTEURS (fin)

//////////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////////
/*
    *** SETUP
*/
//////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(9600);  // Setup serial. Transfert des données à 9600 bauds.
  
  // Afficheur LCD
  lcd.begin(20,4);
  lcd.clear();
  
  // Buzzer
  pinMode(BUZZER, OUTPUT);
  bipOK();
  
  // LED
  pinMode(LED_ROUGE, OUTPUT);
  pinMode(LED_JAUNE, OUTPUT);
  pinMode(LED_VERTE, OUTPUT);
  digitalWrite(LED_ROUGE, LOW);  // LED éteinte.
  digitalWrite(LED_JAUNE, LOW);  // LED éteinte.
  digitalWrite(LED_VERTE, LOW);  // LED éteinte.
 
  // Initialisation de l'interface I2C.
  Wire.begin();
  initHorloge();  // Horloge RTC DS 1307 (I2C)

  // Carte SD
  // Make sure that the default chip select pin is set to output, even if you don't use it.
  pinMode(CHIP_SELECT, OUTPUT); // Chip Select en sortie.
  // Les 4 broches 10,11,12,13 du shield mémoire sont reroutées vers le bus SPI de la carte
  // Arduino Mega par 4 fils. Afin que les broches D10,D11,D12,D13 de la carte Arduino ne
  // perturbent pas ce reroutage, on les définit comme des entrées.
  pinMode(10,INPUT);
  pinMode(11,INPUT);
  pinMode(12,INPUT);
  pinMode(13,INPUT);
  // See if the card is present and can be initialized.
  if (!SD.begin(CHIP_SELECT)) {
    bipErreur();
    lcd.setCursor(0,1);
    lcd.print("PROBLEME DE CARTE SD");    
    delay(3000);
    lcd.clear();
  } else {
    bipOK();
  }
  
  // Bornes numériques
  pinMode(EN1, INPUT);
  pinMode(EN2, INPUT);
}

//////////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////////
/*
    *** LOOP
*/
//////////////////////////////////////////////////////////////////////////////////////////

void loop()
{ 
  switch(ecran) {
    case TAG_ECRAN_ACCUEIL: afficherEcranAccueil(); break;
    case TAG_MENU_PRINCIPAL: afficherMenuPrincipal(); break;
    case TAG_REGLAGE_HORLOGE: afficherReglageHorloge(); break;
    case TAG_MENU_ENREGISTREUR: afficherMenuEnregistreur(); break;
    case TAG_MENU_CADENCE: afficherMenuCadence(); break;
    case TAG_MENU_MANUEL: afficherMenuManuel(); break;
    case TAG_MENU_CAPTEURS: afficherMenuCapteurs(); break;
    case TAG_ENREGISTRER_FICHIER: enregistrerFichier(); break;
    case TAG_TRANSFERER_FICHIER: transfererFichier(); break;
    case TAG_ENTREES: afficherEntrees(); break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////
/*
   *** ECRAN ACCUEIL
*/
//////////////////////////////////////////////////////////////////////////////////////

void afficherEcranAccueil()
{    
  // Affichage
  afficherHorloge();
  lcd.setCursor(0,0);
  lcd.print("* MENU");
  lcd.setCursor(11,0);
  lcd.print("# HORLOGE");

  // Leds de couleur
  digitalWrite(LED_ROUGE, LOW);  // LED rouge éteinte
  digitalWrite(LED_JAUNE, LOW);  // LED jaune éteinte
  digitalWrite(LED_VERTE, HIGH); // LED verte allumée
  
  // Gestion du clavier
  switch(keypad.getKey()) {      
    case '*': selectMenu(TAG_MENU_PRINCIPAL); break;
    case '#': selectMenu(TAG_REGLAGE_HORLOGE); break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////////
/*
   *** MENU PRINCIPAL
*/
//////////////////////////////////////////////////////////////////////////////////////

void afficherMenuPrincipal()
{
  // Affichage
  const byte NB_LIGNES_MENU = 8;
  String menu[] = {
    "0: ACCUEIL          ",
    "1: ENREGISTREUR     ",
    "2: EA1..3 / EN1..2  ",
    "#: A PROPOS         ",
    "--------------------",
    "    V." + VERSION + "    ",
    "    ENTROPIE.ORG    ",
    "--------------------"
  };
  afficheMenu(menu, NB_LIGNES_MENU);

  // Leds de couleur
  digitalWrite(LED_ROUGE, LOW);  // LED rouge éteinte
  digitalWrite(LED_JAUNE, LOW);  // LED jaune éteinte
  digitalWrite(LED_VERTE, HIGH); // LED verte allumée

  // Gestion du clavier
  switch(choixMenu(1)) { // On entre un nombre à 1 chiffre
    case 0: selectMenu(TAG_ECRAN_ACCUEIL); break;
    case 1: selectMenu(TAG_MENU_ENREGISTREUR); break;
    case 2: selectMenu(TAG_ENTREES); break;
    case TOUCHE_DIESE: defileMenu(NB_LIGNES_MENU); break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void afficherMenuEnregistreur()
{
  // Affichage
  const byte NB_LIGNES_MENU = 4;
  String menu[] = {
    "0: ACCUEIL          ",
    "1: PARAMETRES       ",
    "2: ENREGISTRER      ",
    "3: TRANSFERT -> USB ",
  };  
  afficheMenu(menu, NB_LIGNES_MENU);

  // Leds de couleur
  digitalWrite(LED_ROUGE, LOW);  // LED rouge éteinte
  digitalWrite(LED_JAUNE, LOW);  // LED jaune éteinte
  digitalWrite(LED_VERTE, HIGH); // LED verte allumée
  
  // Gestion du clavier
  switch(choixMenu(1)) { // On entre un nombre à 1 chiffre
    case 0: selectMenu(TAG_ECRAN_ACCUEIL); break;
    case 1: selectMenu(TAG_MENU_CADENCE); break;
    case 2: selectMenu(TAG_ENREGISTRER_FICHIER); break;
    case 3: selectMenu(TAG_TRANSFERER_FICHIER); break;
    case TOUCHE_DIESE: defileMenu(NB_LIGNES_MENU); break;
  }
}




//////////////////////////////////////////////////////////////////////////////////////
/*
   *** PARAMETRES D'ENREGISTREMENT
*/
//////////////////////////////////////////////////////////////////////////////////////

void afficherMenuCadence()
{
  // Affichage
  const byte NB_LIGNES_MENU = 9;
  String menu[] = {
    "1:     MANUEL",
    "2: 100 MILLISECONDES",
    "3:   1 SECONDE",
    "4:   5 SECONDES",
    "5:  15 SECONDES",
    "6:   1 MINUTE",
    "7:   5 MINUTES",
    "8:  15 MINUTES",
    "9:   1 HEURE"
  };  
  afficheMenu(menu, NB_LIGNES_MENU);
  
  // Leds de couleur
  digitalWrite(LED_ROUGE, LOW);  // LED rouge éteinte
  digitalWrite(LED_JAUNE, HIGH); // LED jaune allumée
  digitalWrite(LED_VERTE, LOW);  // LED verte éteinte  
  
  // Gestion du clavier
  byte choix = choixMenu(1); // On entre un nombre à 1 chiffre
  switch(choix) {
    case TOUCHE_ETOILE: break;
    case 0: selectMenu(TAG_MENU_ENREGISTREUR); break;
    case TOUCHE_DIESE: defileMenu(NB_LIGNES_MENU); break;
    default:
      if (choix <= NB_LIGNES_MENU) {
        deltaMesures = CADENCE[choix-1];
        int correction = 0;
        if (defilement > NB_LIGNES_MENU - NB_LIGNES_LCD) {
          if (choix <= defilement) {
          correction = NB_LIGNES_MENU;
          }
        }
        if ((choix+correction >= defilement+1) 
        && (choix+correction <= defilement+NB_LIGNES_LCD)) {
          lcd.clear();
          lcd.setCursor(0,(choix+NB_LIGNES_MENU-1-defilement)%NB_LIGNES_MENU);
          lcd.print(menu[choix-1]);
          delay(1000);
          initCapteurs();  // Initialisation des entrées des capteurs et de l'entête du fichier
          if (choix == 1) {
            selectMenu(TAG_MENU_MANUEL);
          }
          else {
            selectMenu(TAG_MENU_CAPTEURS);
          }
        }  
      }
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void afficherMenuManuel()
{
  // Affichage
  const byte NB_LIGNES_MENU = 4;
  String menu[] = {
  "                    ",
  "1: CLAVIER (#)      ",
  "2: DECLENCHEUR (EN2)",
  "                    "
  };
  afficheMenu(menu, NB_LIGNES_MENU);

  // Gestion du clavier
  byte choix = choixMenu(1); // On entre un nombre à 1 chiffre
  switch(choix) {
    case TOUCHE_ETOILE: break;
    case 0: selectMenu(TAG_MENU_ENREGISTREUR); break;
    case TOUCHE_DIESE: break;
    default:
      if (choix <= NB_LIGNES_MENU -2 ) {  // Première et dernière lignes vides
        switch(choix) {
          case 1: declencheurExterne = false; break; // La touche # du clavier est utilisée comme élément déclencheur de la mesure
          case 2: declencheurExterne = true; break;  // L'entrée numérique EN2 est utilisée comme élément déclencheur de la mesure
        }
        lcd.clear();
        lcd.setCursor(0,choix);
        lcd.print(menu[choix]);
        delay(1000);
        selectMenu(TAG_MENU_CAPTEURS);
      }
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void afficherMenuCapteurs()
{ 
  // Affichage
  const byte NB_LIGNES_MENU = NB_ENTREES_MAX;
  String menu[NB_LIGNES_MENU];
  for (int i=0; i<NB_LIGNES_MENU; i++) {
    int nbEspaces = NB_COLONNES_LCD - NOM_ENTREE[i].length() - 4;
    menu[i] += String(i+1,DEC) + ": " + NOM_ENTREE[i];
    for (int j=1; j<=nbEspaces; j++) {
      menu[i] += " ";
    }
    if (selectionCapteur[i]) {
      menu[i]=menu[i].substring(0,NB_COLONNES_LCD-1)+'*';
    }
    else {
      menu[i]=menu[i].substring(0,NB_COLONNES_LCD-1)+' ';
    }
  }
  afficheMenu(menu, NB_LIGNES_MENU);
  
  // Gestion du clavier
  byte choix = choixMenu(1); // On entre un nombre à 1 chiffre
  switch(choix) {
    case TOUCHE_ETOILE: 
      if (nCapteurs > 0) {
        int j = 0;
        for (int i=0; i<NB_ENTREES_MAX; i++) {
          if (selectionCapteur[i]) {
            adresseCapteur[j] = ENTREE[i];
            j++;
            enteteFichier += SEPARATEUR + NOM_MESURE[i];
          }
        }
        selectMenu(TAG_MENU_ENREGISTREUR);
      }
      else {
        bipErreur();
      }
      break;
    case 0: selectMenu(TAG_MENU_ENREGISTREUR); break;
    case TOUCHE_DIESE: defileMenu(NB_LIGNES_MENU); break;
    default :
      if (choix <= NB_LIGNES_MENU) {
        int correction = 0;
        if (defilement > NB_LIGNES_MENU - NB_LIGNES_LCD) {
          if (choix <= defilement) {
            correction = NB_LIGNES_MENU;
          }  
        }
        if ((choix+correction >= defilement+1) 
        && (choix+correction <= defilement+NB_LIGNES_LCD)) {
          if (selectionCapteur[choix-1] == false) {
            nCapteurs++;
          }
          else {
            nCapteurs--;
          }
          selectionCapteur[choix-1] = !selectionCapteur[choix-1];
          while (keypad.getKey() != NO_KEY); // On boucle tant que la touche pressée n'est pas relachée
        }        
      }
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void afficherEntrees()
{
  // Leds de couleur
  digitalWrite(LED_ROUGE, LOW);  // LED rouge éteinte
  digitalWrite(LED_JAUNE, HIGH); // LED jaune allumée
  digitalWrite(LED_VERTE, LOW);  // LED verte éteinte

  String liste[NB_ENTREES_MAX];
  const byte NB_ENTREES_ANALOGIQUES = 3;
  const byte NB_ENTREES_NUMERIQUES = 2;
  int valeurBrute;
  float tension;
  
  boolean quitter = false;
  do {
    for (int i=0; i<NB_ENTREES_ANALOGIQUES; i++) {
      valeurBrute = analogRead(ENTREE[i]);
      tension = (5.0 * valeurBrute) / 1023.0;  // Conversion en Volts
      dtostrf(valeurBrute, 4, 0, ligne);
      liste[i] = "EA" + String(1+i) + ": " + ligne;
      dtostrf(tension, 3, 1, ligne);
      liste[i] = liste[i] + " " + ligne;
      lcd.setCursor(0,i);
      lcd.print(liste[i]);
    }
    for (int i=0; i<NB_ENTREES_NUMERIQUES; i++) {
      dtostrf(digitalRead(ENTREE[NB_ENTREES_ANALOGIQUES+i]), 1, 0, ligne);
      liste[NB_ENTREES_ANALOGIQUES+i] = "EN" + String(1+i) + ": " + ligne;
      lcd.setCursor(14,i+2);
      lcd.print(liste[NB_ENTREES_ANALOGIQUES+i]);
    }
    
    for (int i=0; i<100; i++) {
      delay(1);
      char key = keypad.getKey();
      if (key == '0') {
        quitter = true;
        break;  
      }
    }
  } while (quitter == false);
  selectMenu(TAG_MENU_PRINCIPAL);
}

//////////////////////////////////////////////////////////////////////////////////////
