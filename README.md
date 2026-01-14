
# ARDQUA – Automatisches Bewässerungssystem mit Arduino

ARDQUA ist ein Arduino-basiertes, automatisiertes Bewässerungssystem für Zimmerpflanzen.  
Es misst die Bodenfeuchtigkeit und steuert eine Wasserpumpe automatisch.

Das Projekt wurde im Rahmen des Moduls **Hardwarenahe Programmierung** im **CAS Computer Science 1 (ZHAW)** umgesetzt.

---

## Funktionalitäten

- In regelmässigen Zeitintervallen misst ein kapazitiver Bodenfeuchtigkeitssensor die Feuchtigkeit im Substrat
- Automatische Ansteuerung einer Wasserpumpe über ein Relais falls Grenzwert überschritten wird
- Drei Bewässerungsprofile: trocken / mittel / feucht (Auswahl über 3-stufigen Schalter)  
- TFT-Display mit Anzeige von:
  - aktuellem Bodenfeuchtigkeitswert  
  - aktivem Bewässerungsprofil  
  - grafischem Verlauf der Bodenfeuchtigkeit  
- Aktivierung TFT-Display und Wechsel zwischen Text- und Graph-Anzeige per Tastendruck  

---

## Hardware 

- Arduino Nano  
- Kapazitiver Bodenfeuchtigkeitssensor  
- TFT-Display (SPI)  
- Relais, Wasserpumpe, Schlauch  
- DPDT-Schalter (3-stufig)  
- Druckknopf  
- Wasserreservoir  

Die vollständige Pin-Belegung ist in der Projektdokumentation beschrieben.

---

## Software

- Arduino-Sketch mit eigener Klasse `Ardqua`  
- Verwendete Libraries:
  - `Adafruit_ST7735`
  - `Adafruit_GFX`
  - `ThreadController`
  - `Thread`
  - `SPI`
  

---

## Kalibrierung

Der Bodenfeuchtigkeitssensor wird über zwei Referenzmessungen kalibriert:
- Messung in Luft (trocken -> max_value)
- Messung in Wasser (nass -> min_value)

Die Bewässerungsgrenzwerte sowie die Pumpdauer sind statisch definiert und profilabhängig.

---

## Projektstatus

- MVP vollständig umgesetzt  
- Bewässerungsprofile, Display-Modi und Automatisierung integriert  
- Weitere Optimierungen (z. B. Energiesparmodus, Datenspeicherung) möglich

---
## Lizenz
Projektarbeit im Modul *Hardwarenahe Programmierung* (ZHAW).  
Nutzung nach Absprache.
