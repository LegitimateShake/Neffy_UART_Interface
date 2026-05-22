# NeffyInterface für den Austausch von Informationen über UART mit dem ESP32.

Main GitHub-Repo für [Neffy 3.0](https://github.com/YounesJamil/Neffy-3.0).

## Inkludierung des GitHub Repos:

  1. GitHub Clone Link aus dem Repo kopieren (grünes Rechteck auf dem "Code" steht)
  2. In lib Ordner des PlatformIO Projektes das Terminal öffnen
  3. Potenziell noch Git installieren, falls nicht schon geschehen
  4. folgenden Command im Terminal ausführen: git clone <link hier einfügen>
  5. Jetzt kann der Code über <ins>#include "Neffy_Interface.h"</ins> in die main.cpp eingebunden werden.

## Aufbau:

### Neffy_Interface.h

- Enthält implementierte Funktionen zur Kommunikation 
- Nur dieser Header muss inkludiert werden

### Neffy_Interface_Types.h

- Enthält Datentyp-Definitionen

### Supported_Commands.h

- Enthält eine enum mit allen unterstützen Commands
- Hier können Commands hinzugefügt werden, falls nötig
- Dabei muss der Command in der NeffyCommands struct ergänzt werden, COMMAND_AMOUNT inkrementiert werden und der command in Supported_Commands.cpp in
  das entsprechende Array eingetragen werden. Danach kann eine Funktion für den entsprechenden Command registriert werden.

## Protokollaufbau:

  Das Übertragungsprotokoll ist basiert auf der Übertragung einzelner Bytes. Jede Message, die geschickt / empfangen wird muss den folgenden Aufbau haben:
  | Byte | Beschreibung |
  | ------------- | ------------- |
  | Byte 0        |   START_OF_FRAME_IDENTIFIER (Makro ist definiert in Neffy_Interface_Types.h)               |
  | Byte 1        |   CommandID                 (Commands, die in Supported_Commands.h definiert sind)         |
  | Byte 2        |   Payload Length in Bytes                                                                  |
  | Byte 3 - n    |   Menge an Payload Bytes, die in Byte 2 angegeben ist. Alle weiteren Bytes werden ignoriert|
  
   ### Beispiel: 
  
  - START_OF_FRAME_IDENTIFIER = 0xFA
  - CommandID                 = 0xA1
  - Payload Length            = 0x04
  - Payload                   = 0x01, 0x02, 0x03, 0x04
  ```
            Start    ID   Length           Payload
  Message: [0xFA]  [0xA1] [0x04]  [0x01][0x02][0x03][0x04]
  Binary : [11111111][10100001][00000100][00000001][00000010][00000011][00000100]
  ```
  Die Maximale Größe einer Message ist definiert in Neffy_Interface_Types.h

Messages, die aus der Class heraus verschickt werden haben den gleichen Aufbau.
Es kann je nach Command individuell implementiert werden, wie der Payload zu interpretieren ist.
Für jeden Command kann eine Funktion registiert werden, die aufgerufen wird, wenn der Command empfangen wird.
