# -*- coding: utf-8 -*-
"""
########################################################################
#
#  Centrale Alpha 4 : Récupération et traitement des données brutes
#  Version 2021.04.13
#  Copyright 2019-2021 - Eric Sérandour
#  http://3615.entropie.org
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 3 of
#  the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this program; if not, write to the Free
#  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02110-1301, USA.
#
########################################################################
"""

"""
########################################################################
#
#  Pour régler certains paramètres, voir vers le bas de ce fichier :
#  Dans la marge, j'indique "A modifier éventuellement".
#
########################################################################
"""
#!/usr/bin/env python

import serial
import time
import csv
import matplotlib.pyplot as plt  # Pour faire des graphiques
import numpy
import scipy.optimize  # Routines d'optimisation
import scipy.fftpack  # Routines de FFT

########################################################################






########################################################################
#  FONCTION DE RECUPERATION DES DONNEES DU PORT SERIE => FICHIER CSV
########################################################################

def enregistrerDonnees(nomPort, vitessePort, nomFichier):
    """Enregistrement des données dans un fichier CSV"""
    # Ouverture du port série
    serialPort = serial.Serial(port = nomPort, baudrate = vitessePort)
    # Réinitialisation du microcontrôleur via la broche DTR
    serialPort.setDTR(False)
    time.sleep(0.1)
    serialPort.setDTR(True)
    # On vide le tampon (buffer)
    serialPort.flushInput()
    # -----------------------------------------------------------------
    # Enregistrement dans le fichier CSV
    file = open(nomFichier, "wb")  # Ecriture en mode binaire
    finFichier = False
    data = False
    while not finFichier:
        # Lecture du port série
        ligne = serialPort.readline()
        # Affichage des données en provenance d'Arduino dans la console
        if data == False:
            print("Données brutes :")
            data = True
        print(ligne)
        if ligne == b'\r\n':
            finFichier = True
        else:
            # Ecriture dans le fichier CSV
            file.write(ligne)
    file.close();
    # -----------------------------------------------------------------
    # Fermeture du port série
    serialPort.close()

########################################################################






########################################################################
#  FONCTION D'EXTRACTION DES DONNEES (NOMBRES) DEPUIS LE .CSV
########################################################################

def readColCSV(nomFichier, numCol):
    """Lit une colonne du fichier CSV (la numérotation commence à 0)"""
    file = open(nomFichier, "r")
    reader = csv.reader(file, delimiter =";")
    colonne = []  # Création de la liste "colonne" (vide)
    for row in reader:  # On balaye toutes les lignes du fichier CSV
        try:
            # On remplace les virgules éventuelles par des points 
            notationPoint = row[numCol].replace(",", ".")
            # Remplissage de la liste "colonne" avec des réels
            colonne.append(float(notationPoint))
        except:
            pass
    file.close()
    return colonne

########################################################################

def extraireDonnees(nomFichier, colX, colY):
    """Extraction des données depuis le fichier CSV"""
    listeX = readColCSV(nomFichier, colX)  # Colonne choisie pour x
    listeY = readColCSV(nomFichier, colY)  # Colonne choisie pour y
    x = numpy.array(listeX)    # Liste => Tableau
    y = numpy.array(listeY)    # Liste => Tableau
    return numpy.array([x,y])

########################################################################

def selectionnerZoneDonnees(x, y, debut, fin):
    """Sélection d'une zone de données"""
    x = x[debut:fin+1]
    y = y[debut:fin+1]
    return numpy.array([x,y])

########################################################################






########################################################################
#  AFFICHAGE DES DONNEES EXTRAITES ET TRAITEES
########################################################################

def afficherDonnees(message, x, y):
    """Affichage des données"""
    print(message)
    print("Abscisses :", x)
    print("Ordonnées :", y)
    print("-----------------------------------------------------------")

########################################################################






########################################################################
#  CALCUL DE LA TEMPORISATION
########################################################################

def calculTemporisation(debut, fin):
    """
    Dans la centrale Alpha 4, les temporisations possibles sont :
    manuel,   => On n'en tient pas compte ici
    100 ms,
    1 s, 5 s, 15 s,   cad 1000 ms, 5000 ms, 15000 ms
    1 min, 5 min, 15 min,   cad 60000 ms, 300000 ms, 900000 ms
    1 h.   cad 3600000 ms
    """
    # Extraction du fichier CSV
    COLONNE_N = 0
    COLONNE_T = 2
    n, t = extraireDonnees(FICHIER_CSV, COLONNE_N, COLONNE_T)
    n, t = selectionnerZoneDonnees(n, t, debut+1, fin)  # debut+1 : On enlève éventuellement la mesure n°0 pour éviter une division par 0
    tableauTemporisation = numpy.round(t/n)
    temporisation = round(numpy.mean(tableauTemporisation))  # Moyenne arrondie
    # La temporisation est en millisecondes
    if temporisation >= (3600000 * 0.90) :  # On se laisse un grosse marge d'erreur de 10 %
        temporisation = round(temporisation / 3600000)  # La temporisation est en heures
        uniteT = "h"
    elif temporisation >= (60000 * 0.90) :  # On se laisse un grosse marge d'erreur de 10 %
        temporisation = round(temporisation / 60000)  # La temporisation est en minutes
        uniteT = "min"
    elif temporisation >= (1000 * 0.90) :  # On se laisse un grosse marge d'erreur de 10 %
        temporisation = round(temporisation / 1000)  # La temporisation est en secondes
        uniteT = "s"
    else:
        temporisation = 0.1  # 100 ms = 0.1 s
        uniteT = "s"
    return [temporisation, uniteT]

########################################################################






########################################################################
#  DEFINITIONS DES FONCTIONS POUR LA REGRESSION
########################################################################
"""
Quelques fonctions mathématiques :
x**n               : puissance
numpy.sqrt(x)      : racine carré
numpy.exp(x)       : exponentielle
numpy.log(x)       : logarithme népérien
numpy.log10(x)     : logarithme décimal
numpy.sin(x)       : sinus
numpy.cos(x)       : cosinus
numpy.tan(x)       : tangente
numpy.arcsin(x)    : arcsinus
numpy.arccos(x)    : arccosinus
numpy.arctan(x)    : arctangente
"""

def lineaire(x, a, b):
    # Linéaire : y = a.x + b
    return a*x + b

def quadratique(x, a, b, c):
    # Quadratique : y = a.x^2 + b.x + c
    return a*x**2 + b*x + c

def cubique(x, a, b, c, d):
    # Cubique : y = a.x^3 + b.x^2 + c.x + d
    return a*x**3 + b*x**2 + c*x + d

def quartique(x, a, b, c, d, e):
    # Quartique : y = a.x^4 + b.x^3 + c.x^2 + d.x + e
    return a*x**4 + b*x**3 + c*x**2 + d*x + e

def exponentielle(x, a, b, c):
    # Exponentielle : y = a.e^(b.x) + c 
    return a*numpy.exp(b*x) + c

def logarithmique(x, a, b):
    # Logarithmique : y = a.ln(x) + b
    return a*numpy.log(x) + b

def puissance(x, a, b):
    # Puissance : y = a.x^b
    return a*x**b

def trigonometrique(x, a, b, c, d):
    # Trigonométrique : y = a.sin(b.x + c) + d
    return a*numpy.sin(b*x + c) + d

########################################################################






########################################################################
#  CHOIX DE LA REGRESSION
########################################################################

def choixRegression(choix):
    """Choix de la régression"""
    if choix == 1:
        regressionChoisie = lineaire
        print("Régression linéaire : y = a.x + b")
    elif choix == 2:
        regressionChoisie = quadratique
        print("Régression quadratique : y = a.x^2 + b.x + c")
    elif choix == 3:
        regressionChoisie = cubique
        print("Régression cubique : y = a.x^3 + b.x^2 + c.x + d")
    elif choix == 4:
        regressionChoisie = quartique
        print("Régression quartique : y = a.x^4 + b.x^3 + c.x^2 + d.x + e")
    elif choix == 5:
        regressionChoisie = exponentielle
        print("Régression exponentielle : y = a.e^(b.x) + c")
    elif choix == 6:
        regressionChoisie = logarithmique
        print("Régression logarithmique : y = a.ln(x) + b")
    elif choix == 7:
        regressionChoisie = puissance
        print("Régression puissance : y = a.x^b")
    elif choix == 8:
        regressionChoisie = trigonometrique
        print("Régression trigonométrique : y = a.sin(b.x + c) + d")
    return regressionChoisie

########################################################################






########################################################################
#  REGRESSION D'UNE FONCTION
########################################################################

def regressionFonction(x, y, regression):
    """Régression d'une fonction"""
    a, b, c, d, e = 1, 1, 1, 1, 1
    if regression == lineaire:
        # Valeurs d'initialisation pour la régression
        p0 = numpy.array([a, b])
    elif regression == quadratique:
        # Valeurs d'initialisation pour la régression
        p0 = numpy.array([a, b, c])
    elif regression == cubique:
        # Valeurs d'initialisation pour la régression
        p0 = numpy.array([a, b, c, d])
    elif regression == quartique:
        # Valeurs d'initialisation pour la régression
        p0 = numpy.array([a, b, c, d, e])
    elif regression == exponentielle:
        # On prend 3 points de coordonnées (xk, yk) tels que  x2 - x1 = x3 - x2
        # qu'on suppose solutions de y = a.e^(b.x) + c
        # Après résolution du système (avec quelques changements de variables),
        # on obtient a, b et c mais seul b est vraiment intéressant.
        ecart = (x.size - 1 ) // 2
        x1 = x[0]
        x2 = x[ecart]
        x3 = x[2*ecart]
        y1 = y[0]
        y2 = y[ecart]
        y3 = y[2*ecart]
        # Valeurs d'initialisation pour la régression
        if (y3 - y2 < y2 - y1):
            a = -1
        if (y1 != y2) and (y2 != y3):
            b = numpy.log(numpy.abs((y3-y2)/(y2-y1))) / (x2-x1)
        p0 = numpy.array([a, b, c])
    elif regression == logarithmique:
        if (x[x <= 0].size == 0):  # S'il n'existe pas d'éléments négatifs ou nuls dans le tableau x
            # On linéarise
            popt, pcov = scipy.optimize.curve_fit(lineaire, numpy.log(x), y)
            # Valeurs d'initialisation pour la régression
            a = popt[0]
            b = popt[1]
        p0 = numpy.array([a, b])
    elif regression == puissance:
        if (x[x <= 0].size == 0) and (y[y <= 0].size == 0):  # S'il n'existe pas d'éléments négatifs ou nuls dans les tableaux x et y
            # On linéarise
            popt, pcov = scipy.optimize.curve_fit(lineaire, numpy.log(x), numpy.log(y))
            # Valeurs d'initialisation pour la régression
            a = numpy.exp(popt[1]) 
            b = popt[0]
        p0 = numpy.array([a, b])
    elif regression == trigonometrique:
        # Transformée de Fourier rapide (FFT)
        FFT = abs(scipy.fft(y)) / (y.size / 2)  # Amplitudes
        freqs = scipy.fftpack.fftfreq(y.size, x[1]-x[0])  # Fréquences
        FFT = FFT[0:len(FFT) // 2]  # On supprime les fréquences négatives
        freqs = freqs[0:len(freqs) // 2]  # Idem
        freqPicMax = freqs[numpy.argmax(FFT[1:])+1]  # 1: et +1 => Exclusion du pic à 0 Hz qui correspond à un d non nul
        # Valeurs d'initialisation pour la régression
        a = numpy.std(y)*2**0.5  # Ecart type de l'échantillon * racine carré de 2
        b = 2 * numpy.pi * freqPicMax
        c = 0
        d = numpy.mean(y)  # Moyenne de l'échantillon
        p0 = numpy.array([a, b, c, d])
    # Régression
    try:
        """
        La routine curve_fit peut échouer dans la recherche des paramètres.
        En cas d’échec, il est possible d’aider curve_fit en modifiant p0
        qui correspond au point de départ dans la recherche des paramètres.
        """
        coefReg, coefCov = scipy.optimize.curve_fit(regression, x, y, p0)
    except:
        print("Echec de l'ajustement.")
        print("Modifiez éventuellement les valeurs d'initialisation.")
        print("-----------------------------------------------------------")
    # Coordonnées de points de la fonction de régression
    NB_POINTS = 1000
    xReg = numpy.linspace(min(x), max(x), NB_POINTS)
    yReg = regression(xReg, *coefReg)
    return numpy.array([coefReg, coefCov, xReg, yReg])
    
########################################################################

def afficheCoefReg(coefReg, coefCov):
    """Affiche les coefficients de régression"""
    nbCoef = numpy.size(coefReg)
    for i in range(nbCoef):
        print(chr(97+i), "=", coefReg[i])
    # coefCov : covariance de coefReg.
    # Les termes diagonaux de coefCov renvoient les variances.
    # Pour estimer l'écart type sur les coefficients :
    print("Ecart type de l'erreur sur les coefficients :")
    coefErr = numpy.sqrt(numpy.diag(coefCov))
    for i in range(nbCoef):
        print("sigma_"+chr(97+i), "=", coefErr[i])

########################################################################

def afficheEcartTypeErreurY(x, y, yReg):
    """Affiche l'écart type de l'erreur sur l'ordonnée"""
    yErreur = (y - regressionChoisie(x, *coefReg))
    ySigmaErreur = numpy.std(yErreur)
    print ("Ecart type de l'erreur sur l'ordonnée :")
    print("sigma_y =", ySigmaErreur)
    return ySigmaErreur

########################################################################






########################################################################
#  GENERATEUR DE DONNEES (POUR LA MISE AU POINT DU PROGRAMME)
########################################################################

def generateurDonnees():
    """Générateur de données"""
    # Fonction avec du bruit
    numpy.random.seed(0)
    x = numpy.linspace(0, 30, num=1000)
    y = puissance(x , 5, -2.6) + 0*numpy.random.normal(size=1000)
    # Listes données
    #x = (0,1,2,3,4)
    #y = (2,4,3,5,8)
    print(y)
    return numpy.array([x,y])

########################################################################






"""
########################################################################
#  DEBUT DU PROGRAMME
########################################################################
"""






########################################################################
#  TRAITEMENT DES DONNEES BRUTES
########################################################################

# Enregistrement des données Arduino dans un fichier CSV
print ("En attente de données...")
print ()
# Indiquer le port sélectionné dans le menu Arduino (Outils >  Port) :
# Sous Linux : /dev/ttyACM ou /dev/ttyUSB suivis d'un numéro (0,1,...)
# Sous Windows : COM suivi d'un numéro (1,2,...)
PORT = "/dev/ttyACM0"                                                   # A modifier éventuellement
VITESSE = 9600  # Vitesse en bauds                                      
FICHIER_CSV = "data.csv"                                                # A modifier éventuellement
enregistrerDonnees(PORT, VITESSE, FICHIER_CSV)                          # A mettre en commentaire si on veut travailler sur un fichier CSV déjà existant
print("-----------------------------------------------------------")

# Extraction du fichier CSV
COLONNE_X = 0                                                           # A modifier éventuellement
COLONNE_Y = 3                                                           # A modifier éventuellement
x, y = extraireDonnees(FICHIER_CSV, COLONNE_X, COLONNE_Y)
"""afficherDonnees("Données extraites :", x, y)"""

# Générateur de données (pour la mise au point du programme)
#x, y = generateurDonnees()
"""afficherDonnees("Données générées :", x, y)"""

# Sélectionner une zone de données (x, y, ligne début, ligne fin)
DEBUT = 0
FIN = numpy.size(x) - 1
x, y = selectionnerZoneDonnees(x, y, DEBUT, FIN)                        # A modifier éventuellement
"""afficherDonnees("Données sélectionnées :", x, y)"""

# Calcul de la temporisation
MODE_MANUEL = False                                                     # A modifier éventuellement
temporisation, uniteT  = calculTemporisation(DEBUT, FIN)
if MODE_MANUEL:
    temporisation = 1
    uniteT = "Mode Manuel"

# Conversion des données
# x : Temps en s, min, ou h
x = x * temporisation                                                   # A modifier éventuellement
grandeurX = "Temps"                                                     # A modifier éventuellement
uniteX = uniteT                                                         # A modifier éventuellement
# y : Tension en volts
y = 5.0 * y / 1023                                                      # A modifier éventuellement
grandeurY = "Tension"                                                   # A modifier éventuellement
uniteY = "V"                                                            # A modifier éventuellement
"""afficherDonnees("Données converties :", x, y)"""

########################################################################








########################################################################
#  ANALYSE DES DONNEES : REGRESSION
########################################################################
"""
Choix du type de régression (définies plus haut) :
    O : ne pas faire de régression
    1 : lineaire : y = a.x + b
    2 : quadratique : y = a.x^2 + b.x + c
    3 : cubique : y = a.x^3 + b.x^2 + c.x + d
    4 : quartique : y = a.x^4 + b.x^3 + c.x^2 + d.x + e
    5 : exponentielle : y = a.e^(b.x) + c
    6 : logarithmique : y = a.ln(x) + b
    7 : puissance : y = a.x^b
    8 : trigonometrique : y = a.sin(b.x + c) + d
"""
choix = 1                                                               # A modifier éventuellement
 
if (choix > 0):
    regressionChoisie = choixRegression(choix)
    try:
        coefReg, coefCov, xReg, yReg = regressionFonction(x, y, regressionChoisie)
        afficheCoefReg(coefReg, coefCov)
        afficheEcartTypeErreurY(x, y, yReg)
        print("-----------------------------------------------------------")
        # Calcul de la période et de la fréquence pour une régression trigonométrique
        # Attention : La base de temps doit être en secondes !
        if regressionChoisie == trigonometrique:
            periode = (2 * numpy.pi) / coefReg[1]
            frequence = coefReg[1] / (2 * numpy.pi)
            print("Période   =", periode, "s")
            print("Fréquence =", frequence, "Hz")
            print("-----------------------------------------------------------")
    except:
        pass

########################################################################






########################################################################
#  AUTRES CALCULS
########################################################################

"""
Commentaires
"""



########################################################################






########################################################################
#  AFFICHAGE DU GRAPHIQUE
########################################################################

plt.title("Titre")                                                      # A modifier (Titre)
plt.xlabel(grandeurX + " (" + uniteX + ")")  # Abscisses
plt.ylabel(grandeurY + " (" + uniteY + ")")  # Ordonnées

#plt.plot(x, y, ".r")  # Les points ne sont pas reliés (r : rouge)
plt.plot(x,y)  # Les points sont reliés
try:
    plt.plot(xReg,yReg)  # Courbe de régression
except:
    pass

plt.grid(True)  # Grille
plt.savefig("graphique.png")  # Sauvegarde du graphique au format PNG
plt.show()

########################################################################
