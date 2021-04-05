//////////////////////////////////////////////////////////////////////////////////////////
/*
   *** MENU
*/
//////////////////////////////////////////////////////////////////////////////////////////

void afficheMenu(String menu[], byte nbLignes)
{
  for (int i=0; i<NB_LIGNES_LCD ; i++) {
    lcd.setCursor(0,i);
    int index = i + defilement;
    if (index >= nbLignes) {
      index -= nbLignes;
    }
    lcd.print(menu[index]);
  }
}

//////////////////////////////////////////////////////////////////////////////////////

void defileMenu(byte nbLignes)
{
  if (nbLignes > NB_LIGNES_LCD) {
    defilement++;
    defilement %= nbLignes;
    while (keypad.getKey() != NO_KEY); // On boucle tant que la touche pressée n'est pas relachée
    lcd.clear(); 
  }  
}

//////////////////////////////////////////////////////////////////////////////////////

void selectMenu(byte tag)
{
  ecran = tag;
  defilement = 0;
  while (keypad.getKey() != NO_KEY); // On boucle tant que la touche pressée n'est pas relachée 
  lcd.clear();
}

//////////////////////////////////////////////////////////////////////////////////////

byte choixMenu(byte nbChiffres)
{
  byte choix = 0;
  boolean quitter = false;
  do {
    char touche = keypad.getKey();
    if (touche != NO_KEY) {
      switch (touche) {
        case '#' :
          choix = TOUCHE_DIESE;
          quitter = true;
          break;
        case '*' :
          choix = TOUCHE_ETOILE;
          quitter = true;  
          break;
        default :
          choix = 10*choix + valeurTouche(touche);
          nbChiffres--;
          if (nbChiffres < 1) {
            quitter = true;
          }
          while (keypad.getKey() != NO_KEY); // On attend que la touche pressée soit relachée
        }
    }  
  } while (quitter == false);
  return choix;
}

//////////////////////////////////////////////////////////////////////////////////////

int valeurTouche(char key)
{
  int valeur = byte(key)-byte('1')+1;
  return valeur;
}

//////////////////////////////////////////////////////////////////////////////////////
