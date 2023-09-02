# DeskClock
# Présentation

Afin de présenter un petit projet intérressant pour initier à la programmation embarquée pour la domotique, nous avons décider d’adapter un projet de la chaine youtube https://www.youtube.com/c/TheWrench.

[Building a Small Desk Clock with Weather Station by The_Wrench_DIY](https://www.thingiverse.com/thing:6054399)

# Construction

Détails des étapes et apprentissages du projet

## Code & Prototypage

Le code original et la librairie https://github.com/adafruit/Adafruit-ST7735-Library n’ont pas fonctionné dans notre cas. Nous avons donc cherché d’autres options pour contrôler l’écran. Toutefois, le site ci-dessous, proposant
 d’utiliser la librairie d’Adafruit, offre une explication détaillée du câblage nécessaire, facilitant l’utilisation de matériel similaire mais pas identique.

[Interfacing ESP8266 NodeMCU with ST7789 TFT Display](https://simple-circuit.com/esp8266-nodemcu-st7789-tft-ips-display/)

Il existe aussi la librairie https://github.com/Bodmer/TFT_eSPI permettant un contrôle en SPI de l’écran, dans notre cas cette méthode eut du succès en plus d’être compatible avec le code d’origine.

La configuration de cette librairie nécessite la modification du fichier `User_Setup.h` que vous trouverez probablement dans le dossier `Document\Arduino\Librairies\TFT_eSPI\User_Setup.h`.

Ce fichier ainsi que tous les fichiers finaux sont disponible sur https://github.com/ice-efrei/ice-desk-clock.

# Références

## Code

https://github.com/ice-efrei/ice-desk-clock

- Original Code : https://drive.google.com/drive/folders/1-kIkPsD7Dc-LH1g9TgA9zTaHiB09Fu5Z
- Screen Librairy for NodeMCU v1.1 : https://github.com/Bodmer/TFT_eSPI
- https://simple-circuit.com/esp8266-nodemcu-st7789-tft-ips-display/

## Matériaux

- NodeMCU v1.1 with ESP-WROOM-32
- MRD-1.54’IPS Screen with ST7789 driver