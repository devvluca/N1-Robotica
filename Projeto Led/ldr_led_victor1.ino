#include <Adafruit_NeoPixel.h>

#define PIN 6        // Pino de dados da faixa NeoPixel
#define NUMPIXELS 6  // Quantidade de LEDs

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int sensorPin = A0;  
int valorLuz = 0;    
int brilho = 0;      

void setup() {
  pixels.begin();
  Serial.begin(9600);
}

void loop() {
  valorLuz = analogRead(sensorPin);  
  Serial.println(valorLuz);

  // Converte leitura (0-1023) para brilho (0-255)
  brilho = map(valorLuz, 0, 1023, 0, 255);

  // Aplica o brilho em todos os LEDs
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(brilho, brilho, brilho)); // Branco com intensidade variável
  }
  pixels.show();

  delay(100);
}
