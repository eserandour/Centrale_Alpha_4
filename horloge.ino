//////////////////////////////////////////////////////////////////////////////////////////
/*
   *** HORLOGE
*/
//////////////////////////////////////////////////////////////////////////////////////////

void initHorloge()
{
  RTC.begin();  // Initialisation de l'horloge.
  // Si la RTC n'est pas configurée, le faire avec les valeurs de l'ordinateur
  // au moment de la compilation.
  if (! RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__, __TIME__)); 
  }
  // Pas nécessairement utile mais on prend toutes les précautions
  if (!testHorloge()) {
    RTC.adjust(DateTime(2000,1,1,0,0,0));
  }
}

//////////////////////////////////////////////////////////////////////////////////////////

boolean testHorloge()
{
  DateHeure = RTC.now();
  int annees = DateHeure.year();
  int mois = DateHeure.month();
  int jours = DateHeure.day();
  int heures = DateHeure.hour();
  int minutes = DateHeure.minute();
  int secondes = DateHeure.second();
  boolean etat = true;
  if (annees < 2000 || annees > 2099 ||
      mois < 1 || mois > 12 ||
      jours < 1 || jours > 31 ||
      heures < 0 || heures > 23 ||
      minutes < 0 || minutes > 59 ||
      secondes < 0 || secondes > 59) {
    etat = false;
    }
  return etat;  
}

//////////////////////////////////////////////////////////////////////////////////////////

void afficherHorloge()
{
  DateHeure = RTC.now();
  formaterDateHeure();
  lcd.setCursor(5,2);
  lcd.print(Date);
  lcd.setCursor(6,3);
  lcd.print(Heure); 
}

//////////////////////////////////////////////////////////////////////////////////////

void formaterDateHeure()
{
  String ddmmyy = "";  
  if (DateHeure.day() < 10) {
    ddmmyy = "0";
  }
  ddmmyy += String(DateHeure.day(),DEC) + '/'; 
  if (DateHeure.month() < 10) {
    ddmmyy += "0";
  }
  Date = ddmmyy + String(DateHeure.month(),DEC) + '/' + String(DateHeure.year(),DEC);
  
  String hhmmss = ""; 
  if (DateHeure.hour() < 10) {
    hhmmss = "0";
  }  
  hhmmss += String(DateHeure.hour(),DEC) + ':';  
  if (DateHeure.minute() < 10) {
    hhmmss += "0";
  }   
  hhmmss += String(DateHeure.minute(),DEC) + ':';  
  if (DateHeure.second() < 10) {
    hhmmss += "0";
  }   
  Heure = hhmmss + String(DateHeure.second(),DEC);  
}

//////////////////////////////////////////////////////////////////////////////////////////

void afficherReglageHorloge()
{
  // Leds de couleur
  digitalWrite(LED_JAUNE, HIGH); // LED jaune allumée
  digitalWrite(LED_VERTE, LOW);  // LED verte éteinte  
  
  colonneLCD = 4;
  String chaine = "";
  // Affichage
  lcd.setCursor(4,2);
  lcd.print("JJMMAAAAhhmm"); 
  // Gestion du clavier
  do {
    char touche = keypad.getKey();
    if ((touche != NO_KEY) && (touche!='*') && (touche!='#')) { // Si une touche est pressée
      lcd.setCursor(colonneLCD, 1);
      lcd.print(touche);
      chaine += touche;
      colonneLCD++;
      while (keypad.getKey() != NO_KEY); // On boucle tant que la touche pressée n'est pas relachée
    }
  } while (colonneLCD < 16);  
  decodeChaineHorloge(chaine);
  selectMenu(TAG_ECRAN_ACCUEIL);
}

//////////////////////////////////////////////////////////////////////////////////////////

void decodeChaineHorloge(String chaine)
{
  int jours = chaine.substring(0,2).toInt();
  int mois = chaine.substring(2,4).toInt();
  int annees = chaine.substring(4,8).toInt();
  int heures = chaine.substring(8,10).toInt();
  int minutes = chaine.substring(10,12).toInt();
  if (annees >= 2000 && annees <= 2099 &&
      mois >= 1 && mois <= 12 &&
      jours >= 1 && jours <= 31 &&
      heures >= 0 && heures <= 23 &&
      minutes >= 0 && minutes <= 59) {
    RTC.adjust(DateTime(annees, mois, jours, heures, minutes, 0));
  }
 else {
   bipErreur();
 } 
}

//////////////////////////////////////////////////////////////////////////////////////
