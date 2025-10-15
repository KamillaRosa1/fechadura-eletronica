// ===================================
//        FECHADURA ELETRÔNICA        
// ===================================

// --- INCLUSÃO DE BIBLIOTECAS ---
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h> 

// --- CONFIGURAÇÃO DO HARDWARE ---

// Configuração do LCD 20x4 (com Módulo I2C)
LiquidCrystal_I2C lcd(0x27, 20, 4); 

// Configuração do Teclado 4x4
const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {5, 4, 3, 2}; 
byte colPins[COLS] = {9, 8, 7, 6}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// Configuração do Relé
const int RELAY_PIN = 10; 

// --- DEFINIÇÕES DA MÁQUINA DE ESTADOS ---
enum State {
    ESTADO_BLOQUEADO,
    ESTADO_ABERTO,
    ESTADO_PEDIR_PIN_ATUAL, 
    ESTADO_DEFINIR_NOVO_PIN
};

State currentState = ESTADO_BLOQUEADO; 

// --- VARIÁVEIS DA FECHADURA ---
const String DEFAULT_PIN = "1234";
String currentPIN;                   
String enteredPIN = "";              
const int PIN_LENGTH = 4;            

const int EEPROM_FLAG_ADDRESS = 0;   // Endereço para a flag de PIN salvo
const byte EEPROM_FLAG_VALUE = 0xAA; // Valor para indicar que o PIN foi salvo
const int EEPROM_PIN_START = 1;      // Endereço inicial para o PIN

// --- VARIÁVEIS DE TEMPO ---
unsigned long openTimeStart = 0;
const unsigned long OPEN_DURATION = 5000; 

// =======================================
//        FUNÇÕES AUXILIARES EEPROM       
// =======================================

void readPinFromEEPROM() {
    byte flag = EEPROM.read(EEPROM_FLAG_ADDRESS);
    
    // 1. Checa a Flag
    if (flag == EEPROM_FLAG_VALUE) {
        // Flag encontrada: Lê o PIN salvo
        String tempPin = "";
        for (int i = 0; i < PIN_LENGTH; i++) {
            tempPin += (char)EEPROM.read(EEPROM_PIN_START + i);
        }
        currentPIN = tempPin;
        Serial.println("PIN carregado da EEPROM: " + currentPIN);
    } else {
        // Flag não encontrada: Primeira inicialização
        currentPIN = DEFAULT_PIN;
        Serial.println("EEPROM limpa. Usando PIN padrao: " + currentPIN);
        // Salva o PIN padrão E a Flag para a próxima inicialização
        savePinToEEPROM(currentPIN); 
    }
}

// Salva o novo PIN na memória EEPROM
void savePinToEEPROM(String newPin) {
    if (newPin.length() != PIN_LENGTH) return; 
    
    // 1. Grava o PIN
    for (int i = 0; i < PIN_LENGTH; i++) {
        EEPROM.write(EEPROM_PIN_START + i, newPin.charAt(i));
    }
    
    // 2. Grava a Flag de confirmação
    EEPROM.write(EEPROM_FLAG_ADDRESS, EEPROM_FLAG_VALUE);
    
    Serial.println("Novo PIN salvo na EEPROM e Flag definida: " + newPin);
}

// =============================================
//        FUNÇÕES AUXILIARES DE CONTROLE        
// =============================================

void setLockState(bool isOpen) {
    if (isOpen) {
        digitalWrite(RELAY_PIN, HIGH); 
        Serial.println(">>> ACESSO LIBERADO: RELÉ HIGH <<<");
    } else {
        digitalWrite(RELAY_PIN, LOW); 
        Serial.println(">>> FECHADO: RELÉ LOW <<<");
    }
}

void displayTemporaryMessage(String msg, int duration) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(msg);
    delay(duration); 
}

void checkPin(String input, String target) {
    if (input.equals(target)) {
        currentState = ESTADO_ABERTO;
        openTimeStart = millis(); 
    } else {
        displayTemporaryMessage("PIN INCORRETO!", 1500);
    }
    enteredPIN = ""; 
}

// =================================================================
//          FUNÇÕES DE EXIBIÇÃO NO LCD (STATUS DISPLAYS)
// =================================================================

void displayLockedStatus() {
    lcd.setCursor(0, 0);
    lcd.print("FECHADURA ELETRONICA "); 
    lcd.setCursor(0, 1);
    lcd.print("PIN: "); 
    for (int i = 0; i < PIN_LENGTH; i++) {
        if (i < enteredPIN.length()) {
            lcd.print("*");
        } else {
            lcd.print(" ");
        }
    }
    lcd.print("          "); 
    
    lcd.setCursor(0, 3);
    lcd.print("A=Conf. *=Limpar #=Entr");
}

void displayOpenStatus() {
    long timeLeft = (openTimeStart + OPEN_DURATION - millis()) / 1000;
    
    lcd.setCursor(0, 0);
    lcd.print("ACESSO CONCEDIDO!   ");
    lcd.setCursor(0, 1);
    lcd.print("DESTRAVADO          ");
    lcd.setCursor(0, 2);
    lcd.print("ABERTO POR: ");
    lcd.print(timeLeft);
    lcd.print("s   ");
    
    setLockState(true); 
}

void displayConfigAuthStatus() {
    lcd.setCursor(0, 0);
    lcd.print("MODO CONFIGURACAO   ");
    lcd.setCursor(0, 1);
    lcd.print("PIN ATUAL: ");
    for (int i = 0; i < PIN_LENGTH; i++) {
        if (i < enteredPIN.length()) {
            lcd.print("*");
        } else {
            lcd.print(" ");
        }
    }
    lcd.print("        ");
    lcd.setCursor(0, 3);
    lcd.print("#=Entrar  *=Cancelar  ");
}

void displayNewPinEntryStatus() {
    lcd.setCursor(0, 0);
    lcd.print("NOVO PIN (4 digitos)  ");
    lcd.setCursor(0, 1);
    lcd.print("PIN: ");
    lcd.print(enteredPIN); 
    for (int i = 0; i < PIN_LENGTH - enteredPIN.length(); i++) {
        lcd.print(" ");
    }
    lcd.print("        ");
    lcd.setCursor(0, 3);
    lcd.print("#=Salvar  *=Cancelar  ");
}

// =====================================================
//           FUNÇÕES DE TRANSIÇÃO DE ESTADO
// =====================================================

void handleKey(char key) {
    
    if (key == '*') {
        if (currentState != ESTADO_ABERTO) {
            enteredPIN = "";
            currentState = ESTADO_BLOQUEADO;
            lcd.clear(); 
            return;
        }
    }

    switch (currentState) {
        case ESTADO_BLOQUEADO:
            if (isDigit(key)) {
                if (enteredPIN.length() < PIN_LENGTH) {
                    enteredPIN += key;
                }
            } else if (key == '#') {
                checkPin(enteredPIN, currentPIN);
            } else if (key == 'A') {
                enteredPIN = ""; 
                currentState = ESTADO_PEDIR_PIN_ATUAL;
                lcd.clear();
            }
            break;

        case ESTADO_PEDIR_PIN_ATUAL:
            if (isDigit(key)) {
                if (enteredPIN.length() < PIN_LENGTH) {
                    enteredPIN += key;
                }
            } else if (key == '#') {
                if (enteredPIN.equals(currentPIN)) {
                    enteredPIN = ""; 
                    currentState = ESTADO_DEFINIR_NOVO_PIN;
                    lcd.clear();
                } else {
                    displayTemporaryMessage("PIN ATUAL INCORRETO!", 1500);
                    enteredPIN = "";
                    currentState = ESTADO_BLOQUEADO;
                    lcd.clear();
                }
            }
            break;

        case ESTADO_DEFINIR_NOVO_PIN:
            if (isDigit(key)) {
                if (enteredPIN.length() < PIN_LENGTH) {
                    enteredPIN += key;
                }
            } else if (key == '#') {
                if (enteredPIN.length() == PIN_LENGTH) {
                    // Atualiza e Salva
                    currentPIN = enteredPIN; 
                    savePinToEEPROM(currentPIN); 
                    displayTemporaryMessage("NOVO PIN SALVO!", 1500);
                } else {
                    displayTemporaryMessage("4 DIGITOS NECESSARIOS", 1500);
                }
                enteredPIN = "";
                currentState = ESTADO_BLOQUEADO; 
                lcd.clear();
            }
            break;
    }
}

// ===========================================
//            SETUP E LOOP PRINCIPAL
// ===========================================

void setup() {
    Serial.begin(9600);
    
    // Tenta carregar o PIN salvo ou configura o padrão
    readPinFromEEPROM(); 
    
    // Inicialização do LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();

    // Configuração do pino do relé
    pinMode(RELAY_PIN, OUTPUT);
    setLockState(false); 
    
    lcd.setCursor(0, 0);
    lcd.print("SISTEMA PRONTO");
    delay(1500);
    lcd.clear();
}

void loop() {
    char key = customKeypad.getKey();

    if (key != NO_KEY) {
        handleKey(key); 
    }

    switch (currentState) {
        case ESTADO_BLOQUEADO:
            displayLockedStatus();
            break;
        case ESTADO_ABERTO:
            if (millis() - openTimeStart >= OPEN_DURATION) {
                currentState = ESTADO_BLOQUEADO; 
                setLockState(false);
                lcd.clear();
            } else {
                displayOpenStatus();
            }
            break;
        case ESTADO_PEDIR_PIN_ATUAL:
            displayConfigAuthStatus();
            break;
        case ESTADO_DEFINIR_NOVO_PIN:
            displayNewPinEntryStatus();
            break;
    }
}
