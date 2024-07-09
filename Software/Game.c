/*
 * File:   Game.c
 * Author: 24SMC02
 *
 * Created on 2. Mai 2024, 10:39
 */


#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <util/delay.h>
#include "Game.h"
#include "apa102.h"

/*
 * Initialisiert das Feld mit dem Typen EMPTY
 * @param field: Das Feld
 */
void init_field(board* field) {
    for(int x = 0 ; x < ROWS ; x++){
        for (int y = 0; y < COLS; y++) {
            (*field)[x][y] = EMPTY;
        }
    }
}

/*
 * Dient der Ausgabe des Gesamten Feldes über UART
 * @param field: Das Feld
 */
void debug_printField(board* field) {
    for(int x = 0 ; x < ROWS ; x++){
        for (int y = 0; y < COLS; y++) {
            UART_putC((*field)[x][y]);           
        }
    }
}

/*
 * Initialisiert Das Feld mit zufälligen Werten des Typs Fieldstate
 * @param field: Das Feld
 * @param seed: seed für Zufallszahl
 */
void init_field_random(board* field, uint16_t seed) {
    // Zufallszahlengenerator initialisieren
    srand(seed);

    for(int x = 0; x < ROWS; x++) {
        for (int y = 0; y < COLS; y++) {
            // Zufälligen Fieldstate auswählen
            (*field)[x][y] = rand() % (PREVIEW_SHIP + 1);
        }
    }
}

/*
 * Kopiert ein gesamtes zugeführtes board an den plats von dest
 * @param src: Das zu kopierende board
 * @param dest: Das Ergebnisboard
 */
void copyBoard(board* src, board* dest){
    for(int x = 0 ; x < ROWS ; x++){
        for (int y = 0; y < COLS; y++) {
            (*dest)[x][y] = (*src)[x][y];            
    }
    }
}

/*
 * Initialisiert die Hardware
 */
void init_hardware() {
    apa_Init();
    //UP
    PORTD |= _BV(PD5);//PB6 Pullup
    DDRD &= ~_BV(PD5);//PB6 Eingang
    //DOWN
    PORTB |= _BV(PB0);//PB7 Pullup
    DDRB &= ~_BV(PB0);//PB7 Eingang
    //LEFT
    PORTD |= _BV(PD7);//PD5 Pullup
    DDRD &= ~_BV(PD7);//PD5 Eingang
    //RIGHT
    PORTB |= _BV(PB6);//PD6 Pullup
    DDRB &= ~_BV(PB6);//PD6 Eingang
    //TURNTWIST 
    PORTD |= _BV(PD6);//PD7 Pullup
    DDRD &= ~_BV(PD6);//PD7 Eingang
    //FIRE! & OK
    PORTD |= _BV(PD2);//PB0 Pullup
    DDRD &= ~_BV(PD2);//PB0 Eingang
    
    initTimer();
}

/*
 * Konvertiert abgeschlossende Messages in ein, vom Code verbarbeitbares, Format.
 * @param message: Ursprüngliche Nachricht
 * @param command: Resultierendes Kommando
 * @param x: resultierende X-Koordinate
 * @param y: resultierende Y-Koordinate
 * @param opponentRdy: Hilfsvariable für Konvertierungsentscheidung
 */
void parseMessage(char* message, char* command, int *x, int *y, bool opponentRdy) {
    //Parse Befehl
    (*command) = (*message);
    command++;
    message++;
    (*command) = (*message);
    command++;
    message++;
    (*command) = (*message);
    message++;
    if (opponentRdy) {
        (*x) = (*message) - 'A';
    } else {
    (*x) = (*message) - '0';
    }
    message++;
    (*y) = (*message) - '0'; 
}

/*
 * Validiert einkommende Messages
 * @param message: einkommende Nachricht
 * @ret: 1, wenn true, 0 wenn nicht
 */
int checkMessage(char* message) {
    if (strlen(message) <= 5) {
        return 0;
    } else return 1;
}

/*
 * Diese Funktion versucht ein Schiff an einer, vorerst nicht weiter überprüften
 * Position, zu Platzieren. Bevor es Platziert wird wird überprüft ob:
 * - das Schiff im Spielfeld liegt
 * - das Schiff sich auf leeren Feldern befindet
 * - Die Nachbarschiffe weit genug entfernt sind
 * Ist die Position verboten, wird false zuzrück gegeben und die Felder, die das 
 * Schiff gerne belegt hätte rot eingefärbt.
 * @param int x: X Koordinate
 * @param int y: Y-Koordinate
 * @param bool isRotated: Schiff Senkrecht?
 * @param int ShipLegth: laenge des Schiffs
 * @param field: Das Spielfeld
 * @ret: false, wenn nicht erlaubt
 */
bool placeShip(int x, int y, bool isRotated, int shipLength, board* field) {
    bool isNotAllowed = false;
    // Überprüfen, ob das Schiff vollständig innerhalb des Spielfelds liegt
    if (!isRotated) {
        if (y + shipLength > ROWS) {
            isNotAllowed = true;
        }
    } else if (!isNotAllowed) {
        if (x + shipLength > COLS) {
            isNotAllowed = true;
        }
    }
 
    // Überprüfen, ob alle Felder, die das Schiff belegt, leer sind
    if (!isRotated && !isNotAllowed) {
        for (int i = 0; i < shipLength; i++) {
            if (y + i >= ROWS || (*field)[x][y + i] != EMPTY) {
                isNotAllowed = true;
            }
        }
    } else if (!isNotAllowed) {
        for (int i = 0; i < shipLength; i++) {
            if (x + i >= COLS || (*field)[x + i][y] != EMPTY) {
                isNotAllowed = true;
            }
        }
    }
    
    if (!isNotAllowed) {
    for (int i = -1; i <= (isRotated ? shipLength : 1) && !isNotAllowed; i++) {
        for (int j = -1; j <= (!isRotated ? shipLength : 1) && !isNotAllowed; j++) {
            int posX = x + i;
            int posY = y + j;
            // Überprüfen, ob das Nachbarfeld innerhalb des Spielfelds liegt und ob es besetzt ist
            if (posX >= 0 && posX < ROWS && posY >= 0 && posY < COLS && (*field)[posX][posY] != EMPTY) {
                // Mindestens ein Nachbarfeld ist besetzt, daher kann das Schiff nicht platziert werden
                isNotAllowed = true;
            }
        }
    }
}
 
    if (!isNotAllowed){
    // Alle Bedingungen sind erfüllt, das Schiff kann platziert werden
    if (!isRotated) {
        for (int i = 0; i < shipLength; i++) {
            (*field)[x][y + i] = SHIP;
        }
    } else {
        for (int i = 0; i < shipLength; i++) {
            (*field)[x + i][y] = SHIP;
        }
    }
    return true;
    }else{
       //Fehlerhafte Platzierung wird hier angezeigt
        board errorField = {};
        copyBoard(field, &errorField);
        if (!isRotated) {
        for (int i = 0; i < shipLength && (y+i < ROWS); i++) {
            errorField[x][y + i] = SUNK;
        }
    } else {
        for (int i = 0; i < shipLength && (x+i < COLS); i++) {
            errorField[x + i][y] = SUNK;
        }
    }
        apa_showBoard(errorField);
    }
    
    return false;
 
   
}




//Diese Funktion wird beim Schießen aufgerufen. Sie Überprüft, ob der angefrasagte Schuss im Spielfeld liegt
//und ob das Feld schon beschossen wurde.
//return: true, falls Schuss erlaubt, false falls Schuss verboten
bool placeShipShoot(int x, int y, board* field) {
    // Überprüfen, ob das Schiff vollständig innerhalb des Spielfelds lieg

    if ((*field)[y][x] == EMPTY){
        return true;
    }
    return false;
}

/*
 * Liest Benutzereingaben durch Button Abfragen und versucht in Abhängigkeit
 * davon Schiffe zu platzieren. Nach jeder Benutzereingabe wird ein Zwischenstand
 * angezeigt 
 * @param ShipLegth: laenge des zu platzierenden Schiffes
 * @param field: Das Spielfeld
 */
void updatePosition(int shipLength, board* field) {
    uint8_t x = 0;
    uint8_t y = 0;
    bool isRotated = false;
    bool placed = false;
    
    uint8_t xPrev = x;
    uint8_t yPrev = y;
    bool isRotatedPrev = isRotated;
    //neues Null Feld erstellen fuer preview
    static board newField = {};
    init_field(newField);
    
    while(!placed){
    if (!(PIND & _BV(PD7))) { //PINB & _BV(PB6)
        y = (y > 0) ? y - 1 : y;
        _delay_ms(25); // Entprellen
    }
    if (!(PINB & _BV(PB6))) { //PIND & _BV(PD7)
        y = (y < ROWS - 1) ? y + 1 : y;
        _delay_ms(25); // Entprellen
    }
    if (!(PINB & _BV(PB0))) { //Links PIND & _BV(PD5)
        x = (x > 0) ? x - 1 : x;
        _delay_ms(25); // Entprellen
    }
    if (!(PIND & _BV(PD5))) { //Rechts PINB & _BV(PB0)
        
        x = (x < COLS - 1) ? x + 1 : x;
        _delay_ms(25); // Entprellen
    }
    if (!(PIND & _BV(PD6))) { //Drehen PD7
        isRotated = !isRotated;
        _delay_ms(25); // Entprellen
    }
    if (!(PIND & _BV(PD2))) { //PLACE! PB0
        placed = placeShip(x, y, isRotated, shipLength, field);     
        _delay_ms(25); // Entprellen
    }
    //Bei veraenderung der aktuellen Koordinaten sollen diese angezeigt werden
   if (isRotated != isRotatedPrev || x != xPrev || y != yPrev || (x + y == 0)){
        xPrev = x;
        yPrev = y;
        isRotatedPrev = isRotated;
        
        //Neues Feld mit den Werten des alten bespielen
        copyBoard(field, &newField);
        
        if (!isRotated) {
        // Horizontal placement
        for (int i = 0; i < shipLength && (y + i < 8); i++) {
            newField[x][y + i] = PREVIEW_SHIP;
        }
    } else {
        // Vertical placement
        for (int j = 0; j < shipLength && (x + j < 8); j++) {
            newField[x + j][y] = PREVIEW_SHIP;
        }
    }

        apa_showBoard(&newField);
        
        }
    }
}

/*
 * Liest Benutzereingaben durch Button Abfragen und versucht in Abhängigkeit
 * davon Schiffe abzuschießen. Nach jeder Benutzereingabe wird ein Zwischenstand
 * angezeigt 
 * @param x,y : dient hier der Rückmeldung an die Main, wohin geschossen wurde.
 * @param field: Das Spielfeld
 */
void updatePositionShoot(int *xPointer, int *yPointer, board* field) {
    uint8_t x = 0;
    uint8_t y = 0;
    bool validTarget = false;
    
    uint8_t xPrev = x;
    uint8_t yPrev = y;
    //neues Null Feld erstellen fuer preview
    static board newField = {};
    init_field(newField);
    
    while(!validTarget){
    if (!(PIND & _BV(PD7))) { //Hoch
        y = (y > 0) ? y - 1 : y;
        _delay_ms(25); // Entprellen
    }
    if (!(PINB & _BV(PB6))) { //Runter
        y = (y < ROWS - 1) ? y + 1 : y;
        _delay_ms(25); // Entprellen
    }
    if (!(PINB & _BV(PB0))) { //Links
        x = (x > 0) ? x - 1 : x;
        _delay_ms(25); // Entprellen
    }
    if (!(PIND & _BV(PD5))) { //Rechts
        x = (x < COLS - 1) ? x + 1 : x;
        _delay_ms(25); // Entprellen
    }
    if (!(PIND & _BV(PD2))) { //SHOOT!
        validTarget = placeShipShoot(x,y,field);
        (*xPointer) = x;
        (*yPointer) = y;
        
        _delay_ms(25); // Entprellen
    }
    //Bei veraenderung der aktuellen Koordinaten sollen diese angezeigt werden
   if (x != xPrev || y != yPrev || (x + y == 0)){
        xPrev = x;
        yPrev = y;
        
        //Neues Feld mit den Werten des alten bespielen
        copyBoard(field, &newField);
        
        newField[x][y] = PREVIEW_SHIP;

        apa_showBoard(&newField);
        
        }
    }
}

/*
 * Ueberprüft, ob ein uebergebenes Feld versunken sein könnte, falls es getroffen
 * wurde. Falls ja, wird das Feld entsprechend angepasst.
 * @param field: das Spielfeld
 * @param x,y: die Koordinaten in frage 
 * @ret: true, falls versunken ? false. falls nicht
 */
bool isSunk(board* field, int x, int y){
    //Prinzip: Schuldvermutung. Das nicht gesunken sein muss bewiesen werden
    int offset = 1;
    int positions[8] = {0};
    //speichert die Positionen des potenziell gesunkenen Shiffs ab
    uint8_t detectedHits = 0;
    bool sunk = true;
    
    while(y+offset < COLS && sunk){
        if ((*field)[x][y+offset] == SHIP){
            sunk = false;
        }else if((*field)[x][y+offset] == HIT){
            positions[detectedHits] = x;
            positions[detectedHits+1] = y+offset;
            detectedHits+=2;
        }else{offset = COLS;}
        offset++;
    }
    offset = 1;
    while(y-offset >= 0 && sunk){
        if ((*field)[x][y-offset] == SHIP){
            sunk = false;
        }else if((*field)[x][y-offset] == HIT){
            positions[detectedHits] = x;
            positions[detectedHits+1] = y-offset;
            detectedHits+=2;
        }else{offset = COLS;}
        offset++;
    }
    offset = 1;
    while(x-offset >= 0 && sunk){
        if ((*field)[x-offset][y] == SHIP){
            sunk = false;
        }else if((*field)[x-offset][y] == HIT){
            positions[detectedHits] = x-offset;
            positions[detectedHits+1] = y;
            detectedHits+=2;
        }else{offset = COLS;}
        offset++;
    }
    offset = 1;
    while(x+offset < COLS && sunk){
        if ((*field)[x+offset][y] == SHIP){
            sunk = false;
        }else if((*field)[x+offset][y] == HIT){
            positions[detectedHits] = x+offset;
            positions[detectedHits+1] = y;
            detectedHits+=2;
        }else{offset = COLS;}
        offset++;
    }
    if(sunk){
        int p = 0;
        (*field)[x][y] = SUNK;
        while(p < detectedHits){     
            (*field)[positions[p]][positions[p+1]] = SUNK;
            p+=2;
        }
    }
    
    return sunk;
}

/*
 * Initialisiert einen Timer für die späteres seed Erstellung
 */
void initTimer() {
    // Timer1 initialisieren: Kein Prescaler
    TCCR1B |= (1 << CS10);
}

/*
 * Gibt den aktuellen Wert des Timers zurück (für Zufallszahl)
 * @ret: Wert des Timers
 */
uint16_t waitForButtonPress() {
    // Warte, bis die Taste gedrückt wird (PD2 wird auf LOW gezogen)
    while (PIND & (1 << PD2));
    // Geben Sie den Timerwert zurück
    return TCNT1;
}