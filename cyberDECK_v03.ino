/*
    cyberDECK - Cartridge Security Module v0.3n (Nano)

    LATEST BUILD 10/01/2023
    STATUS: WORKING
    KNOWN BUGS/ISSUES: NONE

    DESCRIPTION
      - Nano-based cartridge system to authenticate a user and boot up RPi cyberDECK.
        When cartridge is plugged into the cyberDECK, the Nano circuit gets power and
        access to a data line for a relay. The Nano boots into a splash screen, then an
        animation while waiting for RFID card swipe. If a recognized card is read, the
        Nano sends power to the data/relay line, which then switches power to the RPi.
    
    HARDWARE
      - Arduino Nano
      - 0.91" OLED display 128x32
      - RFID reader
      - LED x 2
      - Buzzer
      - Relay
    
    WIRING
      - OLED
        - SCK to Nano A5
        - SDA to Nano A4
      - RFID reader
        - RST to Nano D9
        - MISO to Nano D12
        - MOSI to Nano D11
        - SCK to Nano D13
        - SDA to Nano D10
      - Buzzer to Nano D4
      - Green LED to Nano D7
      - Red LED to Nano D8
      - Relay to Nano D2

    SOFTWARE FLOW
      - Splash screen, then animation is displayed while waiting for RFID swipe
      - When a recognized RFID tag is detected, power is sent to the relay and a countdown
        timer is displayed on the OLED screen.
      - When the timer finishes, power is cut to the relay

    CHANGE NOTES FOR v0.3n
      - Changed to smaller OLED
      - Adjusted UI to new display size
      - Changed animation to a single ASCII character (">>" = ASCII #175) vs. random character
      - Adjusted countdown timer for accuracy
      - Verified 9V rechargable battery as power source

    OTHER NOTES
      - none
*/

#include <MFRC522.h>                   //RFID reader library
#include <SPI.h>                       //serial peripheral library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Servo.h>

#define OLED_Address 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

MFRC522 mfrc522(10, 9);                //define RFID reader pins (SS_PIN, RST_PIN)

//LED and buzzer pins
constexpr uint8_t greenLED = 7;
constexpr uint8_t redLED = 8;
#define buzzerPin 4

String tagUID = "8A CA 27 B6";         //UID of authorized RFID tag (orange fob)

uint8_t i = 0;                         //counter

int t = 3600;                          //Boot Timer in seconds ***CHANGE HERE***

void setup() {

//pin assignments
oled.begin(SSD1306_SWITCHCAPVCC, OLED_Address);
oled.setTextColor(WHITE);

pinMode(buzzerPin, OUTPUT);
pinMode(redLED, OUTPUT);
pinMode(greenLED, OUTPUT);
pinMode(2, OUTPUT);                    //relay pin
digitalWrite(2, LOW);                  //set relay pin to LOW

SPI.begin();                           //start SPI bus
mfrc522.PCD_Init();                    //start RFID reader
oled.clearDisplay();

//splash screen
oled.clearDisplay();
oled.setCursor(10,0);
oled.setTextSize(2);
oled.print("cyberDECK");
oled.setCursor(5,18);      
oled.print("SECUREcart");
oled.display();
digitalWrite(greenLED, HIGH);
tone(buzzerPin, 2000);
delay(100);
noTone(buzzerPin);
tone(buzzerPin, 2500);
delay(100);
noTone(buzzerPin);
tone(buzzerPin, 3000);
delay(100);
noTone(buzzerPin);
delay(200);
digitalWrite(greenLED, LOW);
digitalWrite(redLED, HIGH);
delay(500);
digitalWrite(redLED, LOW);
digitalWrite(greenLED, HIGH);
delay(500);
digitalWrite(greenLED, LOW);
digitalWrite(redLED, HIGH);
delay(500);
digitalWrite(redLED, LOW);
}

void loop() {
  
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0,0);

  oled.print(" authSCAN ");
  oled.display();

  oled.setCursor(5,20);

  for(int i = 0; i < 10; i++) {
    char asciiChar = (char) 175;
    oled.print(asciiChar);
    oled.display();
    digitalWrite(greenLED, HIGH);
    delay(10);
    digitalWrite(greenLED, LOW);
  }  

    digitalWrite(redLED, HIGH);
    delay(1);
    digitalWrite(redLED, LOW);
  
    //wait for RFID tag
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
    
    //RFID tag detected
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }
    
    //read RFID UID
    String tag = "";
    for (byte j = 0; j < mfrc522.uid.size; j++)
    {
      tag.concat(String(mfrc522.uid.uidByte[j] < 0x10 ? " 0" : " "));
      tag.concat(String(mfrc522.uid.uidByte[j], HEX));
    }
    tag.toUpperCase();

    //verify UID of tag
    if (tag.substring(1) == tagUID)
    {
      oled.clearDisplay();
      oled.setCursor(0, 0);
      oled.print("VALID KEY");
      oled.setCursor(0, 18);
      oled.print("BOOTING...");
      oled.display();
      digitalWrite(greenLED, HIGH);
      digitalWrite(2, HIGH);           //activate relay to switch on cyberDECK power circuit
      tone(buzzerPin, 2000);
      delay(200);
      noTone(buzzerPin);
      tone(buzzerPin, 3000);
      delay(100);
      noTone(buzzerPin);
      delay(1700);
      digitalWrite(greenLED, LOW);
      bootTimer();
    }
    else
    {
      // If UID of tag is not matched.
      oled.clearDisplay();
      oled.setCursor(0, 0);
      oled.print("UNKNWN KEY");
      oled.setCursor(0, 18);
      oled.print("NO ACCESS!");
      oled.display();
      digitalWrite(redLED, HIGH);
      tone(buzzerPin, 300);
      delay(2000);
      noTone(buzzerPin);
      digitalWrite(redLED, LOW);
      oled.clearDisplay();
      oled.display();
    }
  }

void bootTimer() {

  for(int i = 0; i < (t + 1); i++) {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.print("Timer");
    oled.setTextSize(3);
    oled.setCursor(33, 10);
    oled.print(t-i);
    oled.setTextSize(1);
    oled.setCursor(110, 24);
    oled.print("sec");
    oled.display();
    if ((t-i) < 31) {                  //timer expiration alert length (< 31 = last 30 seconds)
      digitalWrite(redLED, HIGH);
      tone(buzzerPin, 3000);
      delay(10);
      digitalWrite(redLED, LOW);
      noTone(buzzerPin);
      }    
      else {
        digitalWrite(greenLED, HIGH);
        delay(10);
        digitalWrite(greenLED, LOW);
      }
    delay(960);                        //modify here to fine tune the one second duration
  }
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setCursor(0, 10);
  oled.print("RE-BOOTING");
  oled.display();
  tone(buzzerPin, 300);
  delay(1500);
  noTone(buzzerPin);
  digitalWrite(2, LOW);                //reset relay to off/open to cut power to cyberDECK
  setup();
}