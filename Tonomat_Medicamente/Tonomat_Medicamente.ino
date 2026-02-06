#include <Keypad.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <DHT.h>

#define DHTPIN 36     
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2); 

Servo servo1, servo2;

const byte numRows = 4;
const byte numCols = 4;
char keymap[numRows][numCols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[numRows] = {9, 8, 7, 6}; 
byte colPins[numCols] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

struct Medicine {
  char name[12];
  const char code[6];
  int stock;
  int eepromAddr;
  Servo* servo;
};

Medicine medicines[] = {
  {"Algocalmin",  "1234A", 10, 100, &servo1},
  {"Paracetamol", "5678B", 10, 105, &servo2}
};

const int medicineCount = 2;
const int MAX_STOCK = 10;

char enteredCode[6];
const int codeLength = 5;
unsigned long lastActivityTime = 0;
const unsigned long TIMEOUT = 15000;
unsigned long lastTempRead = 0;

void setup() {
  Serial.begin(9600);
  
  lcd.init();
  lcd.backlight();
  dht.begin();
  
  lcd.clear();
  lcd.print("Initializare...");
  delay(1000);

  servo1.attach(11);
  servo2.attach(12);
  
  servo1.write(0);
  servo2.write(0);

  for (int i = 0; i < medicineCount; i++) {
    int savedStock = EEPROM.read(medicines[i].eepromAddr);
    if (savedStock > MAX_STOCK || savedStock < 0) {
      medicines[i].stock = MAX_STOCK;
      EEPROM.write(medicines[i].eepromAddr, MAX_STOCK);
    } else {
      medicines[i].stock = savedStock;
    }
  }

  showWelcomeScreen();
}

void loop() {
  if (millis() - lastActivityTime > TIMEOUT) {
    if (millis() - lastTempRead > 3000) {
      updateTempScreen();
      lastTempRead = millis();
    }
  }

  char key = keypad.getKey();
  
  if (key) {
    readCode(key);
    lastActivityTime = millis();
    showWelcomeScreen();
  }
}

void updateTempScreen() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    lcd.setCursor(0, 0);
    lcd.print("Senzor Off/Err ");
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("Temp: "); lcd.print((int)t); lcd.print((char)223); lcd.print("C    ");
  lcd.setCursor(0, 1);
  lcd.print("Umid: "); lcd.print((int)h); lcd.print("%     ");
}

void showWelcomeScreen() {
  lcd.clear();
  updateTempScreen();
}

void readCode(char firstKey) {
  int i = 0;
  enteredCode[i++] = firstKey;
  lastActivityTime = millis();
  
  lcd.clear();
  lcd.print("Cod:");
  lcd.setCursor(0, 1);
  lcd.print(firstKey);
  
  while (i < codeLength) {
    char key = keypad.getKey();
    if (key) {
      enteredCode[i] = key;
      lcd.print('*');
      i++;
      lastActivityTime = millis();
    }
    if (millis() - lastActivityTime > 10000) return; 
  }
  
  if (strncmp(enteredCode, "9999A", 5) == 0) {
    enterAdminMode();
    return;
  }
  
  verifyMedicineCode();
}

void verifyMedicineCode() {
  bool found = false;
  for (int i = 0; i < medicineCount; i++) {
    if (strncmp(enteredCode, medicines[i].code, 5) == 0) {
      found = true;
      processDispense(&medicines[i]);
      break;
    }
  }
  if (!found) {
    lcd.clear();
    lcd.print("Cod Invalid!");
    delay(2000);
  }
}

void processDispense(Medicine* med) {
  lcd.clear();
  lcd.print(med->name);
  lcd.setCursor(0, 1);
  lcd.print("Stoc: "); lcd.print(med->stock);
  delay(1500);

  if (med->stock <= 0) {
    lcd.clear(); 
    lcd.print("Stoc Epuizat!"); 
    delay(2000); 
    return;
  }

  lcd.clear(); 
  lcd.print("Cantitate (1-9):");
  
  int quantity = 0;
  unsigned long waitStart = millis();
  
  while(millis() - waitStart < 10000) {
    char key = keypad.getKey();
    if (key >= '1' && key <= '9') {
      quantity = key - '0';
      lcd.setCursor(0, 1); 
      lcd.print(quantity); 
      delay(500); 
      break;
    }
    if (key == '#') return;
  }

  if (quantity > 0) {
    if (quantity > med->stock) {
      lcd.clear(); 
      lcd.print("Stoc Insuficient"); 
      delay(2000);
    } else {
      lcd.clear(); 
      lcd.print("Distribuire...");
      
      
      for(int k=0; k<quantity; k++) {
        med->servo->write(90); 
        delay(400);            
        med->servo->write(0);  
        delay(400);            
      }
      
      med->stock -= quantity;
      EEPROM.write(med->eepromAddr, med->stock);
      
      lcd.clear(); 
      lcd.print("Multumim!"); 
      delay(2000);
    }
  }
}

void enterAdminMode() {
  lcd.clear(); 
  lcd.print("Mod Admin"); 
  delay(1000);
  lcd.clear(); 
  lcd.print("1:Umple Stoc");
  lcd.setCursor(0,1);
  lcd.print("#:Iesire");
  
  unsigned long adminStart = millis();
  while(millis() - adminStart < 10000) {
    char key = keypad.getKey();
    if (key == '1') {
      for (int i = 0; i < medicineCount; i++) {
        medicines[i].stock = MAX_STOCK;
        EEPROM.write(medicines[i].eepromAddr, MAX_STOCK);
      }
      lcd.clear(); 
      lcd.print("Stoc Refacut!"); 
      delay(2000); 
      return;
    }
    if (key == '#') return;
  }
}