#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <DHT.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <pitches.h>
// #include "P.h"
#define DHT22_PIN  19        // ESP32 pin GPIO21 connected to DHT22 sensor
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define LED1_PIN 2
#define LED2_PIN 4
#define BUTTON1_PIN 27
#define BUTTON2_PIN 26
#define BUTTON3_PIN 25
#define BUTTON4_PIN 33
#define SPEAKER_PIN 32
#define BUZZER_PIN  32 // ESP32 pin GPIO18 connected to piezo buzzer
DHT dht22(DHT22_PIN, DHT22);
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7, 0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0, 0, NOTE_G6, 0, 0, 0,
  NOTE_C7, 0, 0, NOTE_G6, 0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6, 0, NOTE_AS6, NOTE_A6, 0,
  
  NOTE_G6, NOTE_E7, NOTE_G7, NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7, NOTE_D7, NOTE_B6, 0, 0,
  
  NOTE_C7, 0, 0, NOTE_G6, 0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6, 0, NOTE_AS6, NOTE_A6, 0,
  
  NOTE_G6, NOTE_E7, NOTE_G7, NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7, NOTE_D7, NOTE_B6, 0, 0
};

int noteDurations[] = {
  12, 12, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12,
  
  12, 12, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12,
  
  12, 12, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12,
  
  12, 12, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12
};

float humidity, temperature;
bool buttonState = false;
SemaphoreHandle_t button3Semaphore;
int button3SemaphoreCount = 0;
void Task1(void *param);
void Task2(void *param);
void Task3(void *param);
void Task4(void *param);
void Task5(void *param);

void setup() {
  Serial.begin(9600);
 pinMode(LED1_PIN, OUTPUT);
 pinMode(LED2_PIN, OUTPUT);
 pinMode(BUTTON1_PIN, INPUT_PULLUP);
 pinMode(BUTTON2_PIN, INPUT_PULLUP);
 pinMode(BUTTON3_PIN, INPUT_PULLUP);
 pinMode(BUTTON4_PIN, INPUT_PULLUP);
 pinMode(SPEAKER_PIN, OUTPUT);
  dht22.begin(); // initialize the DHT22 sensor

  Wire.begin(); // Start the default I2C bus
    display.begin();
    display.clearDisplay();
  display.clearDisplay();
   button3Semaphore = xSemaphoreCreateBinary();
  xTaskCreate(Task1, "Task1", 1024, NULL, 1, NULL);
  xTaskCreate(Task2, "Task2", 1024, NULL, 1, NULL);
 xTaskCreate(Task3, "Task3", 1024, NULL, 1, NULL);
 xTaskCreate(Task4, "Task4", 1024, NULL, 1, NULL);
 xTaskCreate(Task5, "Task5", 1024, NULL, 1, NULL);
}

void Task1(void *param) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  for (;;) {
    humidity = dht22.readHumidity();
    temperature = dht22.readTemperature();

    vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
  }
}
void Task2(void *param) {
    for (;;) {
       for (int thisNote = 0; thisNote < 64; thisNote++) {
    // Calculate the note duration
    int noteDuration = 1000 / noteDurations[thisNote];
    
    // Play the note
    tone(BUZZER_PIN, melody[thisNote], noteDuration);
    
    // Pause between notes
    delay(noteDuration * 1.30);
    
    // Stop the tone
    noTone(BUZZER_PIN);
  }
}}
void Task3(void *param) {
 for (;;) {
   if (digitalRead(BUTTON1_PIN) == LOW) {
     digitalWrite(LED1_PIN, HIGH);
   }
   if (digitalRead(BUTTON2_PIN) == LOW) {
     digitalWrite(LED1_PIN, LOW);
   }
 }
}
void Task4(void *param) {
 for (;;) {
   if (digitalRead(BUTTON3_PIN) == LOW) {
     if (button3SemaphoreCount < 5) {
       xSemaphoreGive(button3Semaphore);
       button3SemaphoreCount++;
     }
     vTaskDelay(pdMS_TO_TICKS(200)); // Debounce delay
   }
 }
}
void Task5(void *param) {
 for (;;) {
   if (xSemaphoreTake(button3Semaphore, portMAX_DELAY) == pdTRUE) {
     for (int i = 0; i < 3; i++) {
       digitalWrite(LED2_PIN, HIGH);
       vTaskDelay(pdMS_TO_TICKS(500));
       digitalWrite(LED2_PIN, LOW);
       vTaskDelay(pdMS_TO_TICKS(500));
     }
   }
 }
}

void loop() {
  display.clearDisplay(); // Clear the display buffer

  // Set text properties
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Print Humidity
  display.setCursor(0, 10);
  display.print("Humidity: ");
  display.print(humidity);
  display.print(" %");

  // Print Temperature
  display.setCursor(0, 20);
  display.print("Temperature: ");
  display.print(temperature);
  display.print(" C");

  display.display(); // Display buffer

  delay(1000); // Delay to control the refresh rate
}
