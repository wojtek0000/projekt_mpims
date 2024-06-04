#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  //ustawienie adresu wyświetlacza

const byte ROWS = 4;
const byte COLS = 4;
byte rowPins[ROWS] = {8,7,6,5};
byte colPins[COLS] = {9,10,11,12};

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

Keypad klawiatura = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

char kod[] = {'1', '2', '3'};
int lz = 3;
int poz = 0;
int haslo_ok = 0;
int zrodlo_alarmu = 0;
int lock = 0;

#define KONTAKTRON 2
#define BUZZER 3
#define PIR 4

volatile int stanAlarmu = 1;

unsigned long czas = 0;
unsigned long poprzedniczas = 0;

void setup() {

  lcd.init();                       //inicjalizacja wyświetlacza
  lcd.begin(16,2);
  lcd.backlight();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(KONTAKTRON, INPUT_PULLUP);  //Deklaracja wejść i wyjść
  pinMode(PIR, INPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);  //Ustawienie wartości początkowych
  digitalWrite(BUZZER, HIGH);       //domyślnie buzzer wyłączony

  Serial.begin(9600);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Inicjalizacja...");
  delay(5000);
}
  

void loop() {

  switch(stanAlarmu){
    case 1:
    Serial.write("czuwanie\n");
    //czuwanie - stan podstawowy
    lock = 0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Aby uzbroic");
    lcd.setCursor(0,1);
    lcd.print("wcisnij A");

    while(1){
    if(digitalRead(PIR) == HIGH){   //sprawdzenie czy czujka nie pozostaje w stanie wysokim podczas uzbrajania
      lcd.setCursor(15,0);          //jesli tak wyświetl znak '!'
      lcd.print("!");
    }
    else if(digitalRead(KONTAKTRON) == HIGH){  //sprawdzenie czy drzwi nie są otwarte
      lcd.setCursor(15,0);          //jesli tak wyświetl znak 'K'
      lcd.print("K");
    }
    else{
      lcd.setCursor(15,0);
      lcd.print(".");
    }
    if(Serial.available()){
      String command = Serial.readStringUntil('\n');
      if(command == "lock"){
        lock = 1;
        break;
      }
    }

    char klawisz = klawiatura.getKey();
    if(klawisz == 'A'){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Podaj haslo");
      haslo();
      break;
      }
    }

    if(lock == 1) {stanAlarmu = 2; break;}
    if(haslo_ok==1){stanAlarmu = 2; break;}
    else{stanAlarmu == 1; break;}

    case 2:
    //Monitorowanie
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Do wyjscia:");
    for(int i=3; i>=0; i--){
      lcd.setCursor(14,0);
      lcd.print(" ");
      lcd.setCursor(13,0);
      lcd.print(i);
      delay(1000);
    };
    Serial.write("monitorowanie\n");
    lock = 0;
    poz = 0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Alarm uzbrojony");
    
    while(1){
      if(digitalRead(KONTAKTRON) == HIGH) {
        zrodlo_alarmu = 1;
        stanAlarmu = 3;
        break;
      }
      else if(digitalRead(PIR) == HIGH){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Do wejscia");
        int t = 10;

        while(t >= 0) {
          char klawisz = klawiatura.getKey();
          if(klawisz){
            if(klawisz == kod[poz]) {
            lcd.setCursor(poz,1);
            lcd.print("*");
            poz++;
            } //Alarm poprawnie rozbrojony

          else {stanAlarmu = 3; break;}  //Alarm źle rozbrojony
            if( poz == lz) {
            delay(500);
            lcd.setCursor(0,0);
            lcd.print("Alarm rozbrojony");
            stanAlarmu = 1;
            break;}
        
          }
          zrodlo_alarmu = 2;  //jesli nie wprowadzono pełnego poprawnego hasła, to uruchom tryb alarmu
          stanAlarmu = 3;
          czas = millis();
        
          if(czas - poprzedniczas >= 1000UL){
          poprzedniczas = czas;
          t--;
          lcd.setCursor(14,0);
          lcd.print(t);
        }
        }
        break;
      }
      else if(Serial.available()){
        String command2 = Serial.readStringUntil('\n');
        if(command2 == "unlock"){
          stanAlarmu = 1;
          break;
        }
      }
      char klawisz = klawiatura.getKey();
      if(klawisz){
        if(klawisz == kod[poz]) {
          lcd.setCursor(poz,1);
          lcd.print("*");
          poz++;
          } //Alarm poprawnie rozbrojony
        else {stanAlarmu = 3; break;}  //Alarm źle rozbrojony
        if( poz == lz) {
          delay(500);
          lcd.setCursor(0,0);
          lcd.print("Alarm rozbrojony");
          stanAlarmu = 1;
          break;}
        
      }
    }
    break;

    case 3:
    //Alarm
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.write("alarm\n");
    digitalWrite(BUZZER, LOW);
    lcd.clear();
    lcd.setCursor(0,0);
    if(zrodlo_alarmu == 1){ lcd.print("Alarm kontaktron"); }
    else if(zrodlo_alarmu == 2) { lcd.print("Alarm czujka PIR"); }
    
    haslo();
    if(haslo_ok == 1) { 
      digitalWrite(BUZZER, HIGH);
      stanAlarmu = 1;
      }
    else{ stanAlarmu = 3;}
    break;
  }
}

void haslo(){
  poz = 0;
  while(1){
    char klawisz = klawiatura.getKey();
    if(klawisz){
      lcd.setCursor(poz,1);
      lcd.print("*");
      if(klawisz == kod[poz]) {haslo_ok = 1; }
      else{haslo_ok = 0; break;}
      poz++;
    }
    if(poz == lz) {delay(500); break;}
    
  }
}