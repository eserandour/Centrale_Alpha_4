//////////////////////////////////////////////////////////////////////////////////////
/*
   *** ENREGISTRER FICHIER
*/
//////////////////////////////////////////////////////////////////////////////////////

boolean recording = false;
unsigned long numeroMesure = 0;
unsigned long duree = 0;

void enregistrerFichier()
{
  do {
    if (!recording) {
      nouveauFichier();
      recording = true;
      numeroMesure = 0;
      // Leds de couleur
      digitalWrite(LED_ROUGE, HIGH); // LED rouge allumée.
      digitalWrite(LED_JAUNE, LOW);  // LED jaune éteinte.
      digitalWrite(LED_VERTE, LOW);  // LED verte éteinte
    }
    else {
      // LECTURE + ENVOI DES DONNEES (commun aux modes manuel et automatique)
        time = millis();
        DateHeure = RTC.now();
        formaterDateHeure();
        if (numeroMesure == 0) {
          timeOffset = time;
        }
        duree = time - timeOffset;
        lectureCapteurs();
        donneesVersCarteSD();    // Ecriture sur carte SD
        donneesVersPortSerie();  // Envoi des données vers le port série
      
      // MODE MANUEL
      if (deltaMesures == MODE_MANUEL) {
        // Affichage sur l'écran LCD
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ATTENTE");      
        lcd.setCursor(0,1);
        lcd.print("NB MESURES: ");     
        lcd.print(numeroMesure+1);
        lcd.setCursor(0,2);
        if (declencheurExterne) {
          lcd.print("*: STOP"); 
        }
        else {
          lcd.print("#: ENREGISTRER");
          lcd.setCursor(0,3);
          lcd.print("*: STOP");        
        }

        // Gestion de l'attente et du clavier
        boolean attente = true;
        do {
          // Lecture du clavier
          char key = keypad.getKey();
          switch (key) {
            case '*' :
              // Stoppe l'enregistrement
              recording = false;
              // Marqueur de fin d'enregistrement (pour Python)
              Serial.write("\r\n"); // Retour à la ligne + Saut de ligne
              selectMenu(TAG_MENU_ENREGISTREUR);
              break;
            case '#' :
              if (not declencheurExterne) {
                attente = false;
                while (keypad.getKey() != NO_KEY); // On boucle tant que la touche pressée n'est pas relachée               
              }
              break;
          }
          // Lecture du déclencheur à distance
          if (declencheurExterne && digitalRead(DECLENCHEUR) == HIGH) {
            delay(50); // Delai supérieur à celui des rebondissements d'une éventuelle touche
            attente = false;
            while (digitalRead(DECLENCHEUR) == HIGH); // On boucle en attendant que le déclencheur passe à LOW          
          }
        } while (attente && recording);
        numeroMesure++;
      }
      
      // MODE AUTOMATIQUE
      else {
        // Affichage sur l'écran
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("ECRITURE");      
        lcd.setCursor(0,1);
        lcd.print("NB MESURES: ");     
        lcd.print(numeroMesure+1);
        lcd.setCursor(0,2);
        lcd.print("DUREE: ");
        String dureeFormatee = "";
        int nbHeures = long(numeroMesure*deltaMesures/3600000);
        int nbMinutes = long(numeroMesure*deltaMesures/60000-nbHeures*60);
        int nbSecondes = long(numeroMesure*deltaMesures/1000-nbHeures*3600-nbMinutes*60);
        if (nbHeures < 10) {
          dureeFormatee = "0";
        }
        dureeFormatee += String(nbHeures, DEC);
        if (deltaMesures < 3600000) {
          dureeFormatee += ":";  
          if (nbMinutes < 10) {
            dureeFormatee += "0";
          }
          dureeFormatee += String(nbMinutes, DEC);
        }
        if (deltaMesures < 60000) {
          dureeFormatee += ":";      
          if (nbSecondes < 10) {
            dureeFormatee += "0";
          }      
          dureeFormatee += String(nbSecondes, DEC);
        }
        lcd.print(dureeFormatee);
        lcd.setCursor(0,3);
        lcd.print("*: STOP");        
        
        // Mécanisme de régulation. Gestion du clavier.
        // On regarde où on en est au niveau temps parce qu'une boucle dure ici environ 33 ms
        time = millis();
        duree = time - timeOffset;
        long correction = duree-(numeroMesure*deltaMesures);
        long tempsPause = deltaMesures-correction; // Pour éviter de rares bugs lors de
        if (tempsPause < 0) {                      // la régulation : il peut arriver que
          tempsPause = 0;                          // correction dépasse 100 ms.
        }
        unsigned long tempsEcoule;
        do {
          char key = keypad.getKey(); // Lecture de la touche actionnée 
          if (key == '*') {
            // Stoppe l'enregistrement
            recording = false;
            // Marqueur de fin d'enregistrement (pour Python)
            Serial.write("\r\n"); // Retour à la ligne + Saut de ligne
            selectMenu(TAG_MENU_ENREGISTREUR);
            break;  // On sort de la boucle do...while  
          }
          tempsEcoule = millis() - time;     
        } while (tempsEcoule < tempsPause);
        numeroMesure++;
      }
    }  
  } while (recording == true);
}

//////////////////////////////////////////////////////////////////////////////////////

void nouveauFichier()
{
  // Initialisation de l'entete du fichier
  if (enteteFichier.substring(0,6) != "MESURE") {  // On ne change l'entête que si les paramètres d'enregistrement ont changé
    enteteFichier = "MESURE" + SEPARATEUR + "DATE HEURE" + SEPARATEUR + "MILLISECONDES" + enteteFichier;
  }

  // Sortie sur carte SD
  // Effacer le fichier data.txt précédent
  if (SD.exists("data.txt")) {
    SD.remove("data.txt");
  }
  // Créer le fichier data.txt et sa première ligne
  dataFile = SD.open("data.txt", FILE_WRITE);  
  // If the file is available, write to it.
  if (dataFile) {
    dataFile.println(enteteFichier);
    dataFile.close(); 
  }
  // If the file isn't open, pop up an error.
  else {
    Serial.println("Error opening data.txt");
    bipErreur();
  }

  // Sortie sur port série
  Serial.println(enteteFichier);
}

//////////////////////////////////////////////////////////////////////////////////////////

void donneesVersCarteSD()
{
  // Open the file. Note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("data.txt", FILE_WRITE);
  // If the file is available, write to it.
  if (dataFile) {
    dataFile.print(numeroMesure);
    dataFile.print(SEPARATEUR);
    dataFile.print(Date);
    dataFile.print(" ");
    dataFile.print(Heure);
    dataFile.print(SEPARATEUR);
    dataFile.print(duree);
    for (int i=0; i<(nbMesures); i++) {
      dataFile.print(SEPARATEUR);
      dataFile.print(mesureBrute[i]);
    }
    dataFile.println("");
    dataFile.close(); 
  }
  // If the file isn't open, pop up an error.
  else {
    Serial.println("Error opening datalog.txt");  
    bipErreur();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////

void donneesVersPortSerie()
{
  Serial.print(numeroMesure);
  Serial.print(SEPARATEUR);
  Serial.print(Date);
  Serial.print(" ");
  Serial.print(Heure);
  Serial.print(SEPARATEUR);
  Serial.print(duree);
  for (int i=0; i<(nbMesures); i++) {
    Serial.print(SEPARATEUR);
    Serial.print(mesureBrute[i]);
  }
  Serial.println("");
}

//////////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////
/*
   *** TRANSFERER FICHIER
*/
//////////////////////////////////////////////////////////////////////////////////////

void transfererFichier()
{
  // Leds de couleur
  digitalWrite(LED_ROUGE, LOW);  // LED rouge éteinte
  digitalWrite(LED_JAUNE, HIGH); // LED jaune allumée
  digitalWrite(LED_VERTE, LOW);  // LED verte éteinte  
  
  lcd.setCursor(0,0);
  lcd.print("TRANSFERT DU FICHIER");
  // Lecture du fichier data.txt sur le port série  
  dataFile = SD.open("data.txt");
  if (dataFile) {
    unsigned long tailleFichier = dataFile.size();
    // Read from the file until there's nothing else in it
    while (dataFile.available()) {
      Serial.write(dataFile.read());
      // Affichage du pourcentage de transfert
      lcd.setCursor(15,3);
      byte pourcentage = 100 * dataFile.position() / tailleFichier;
      dtostrf(pourcentage, 3, 0, ligne);
      lcd.print(ligne);
      lcd.print(" %");    
      // Pour abandonner le transfert
      char key = keypad.getKey(); // Lecture de la touche actionnée 
      if (key == '0') {
        break;  // On sort de la boucle while  
      }
    }
    // Marqueur de fin de fichier (pour Python)
    Serial.write("\r\n"); // Retour à la ligne + Saut de ligne
    // Close the file
    dataFile.close();
    delay(3000); // Le temps que le programme de récupération se ferme
    bipOK();
    selectMenu(TAG_MENU_ENREGISTREUR);
  }
  // If the file didn't open, print an error
  else { 
    Serial.println("Error opening data.txt");
    bipErreur();
  }  
}

//////////////////////////////////////////////////////////////////////////////////////////
