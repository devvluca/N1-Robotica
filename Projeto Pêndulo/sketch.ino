// ===== Controle PID Simples com ESP32 e Potenciômetro =====
#define POT_PIN 32
#define LED_OK 2
#define LED_FAIL 4

float Kp = 3.5;
float Ki = 0.6;
float Kd = 0.8;

float setpoint = 2048;  // Valor ideal (posição central)
float erro, erro_anterior = 0;
float integral = 0;
float derivativo = 0;
float saida = 0;
int leitura;

void setup() {
  Serial.begin(115200);
  pinMode(LED_OK, OUTPUT);
  pinMode(LED_FAIL, OUTPUT);
  Serial.println("=== Simulação PID no Wokwi ===");
}

void loop() {
  leitura = analogRead(POT_PIN);
  erro = setpoint - leitura;

  // Cálculo do PID
  integral += erro;
  derivativo = erro - erro_anterior;
  saida = (Kp * erro) + (Ki * integral) + (Kd * derivativo);

  erro_anterior = erro;

  // LEDs de estado
  if (abs(erro) < 100) { // Está estável
    digitalWrite(LED_OK, HIGH);
    digitalWrite(LED_FAIL, LOW);
  } else {
    digitalWrite(LED_OK, LOW);
    digitalWrite(LED_FAIL, HIGH);
  }

  // Exibição serial
  Serial.print("Leitura: ");
  Serial.print(leitura);
  Serial.print(" | Erro: ");
  Serial.print(erro);
  Serial.print(" | Saída PID: ");
  Serial.println(saida);

  delay(100);
}