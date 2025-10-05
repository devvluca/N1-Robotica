#include <math.h>

#define SENSOR_TEMP_PIN  A5
#define MOTOR_AQUECIMENTO_PIN 5
#define MOTOR_RESFRIAMENTO_PIN 6

// PID variáveis
double temperaturaAtual;
double temperaturaAnterior;
double erro;
double P, I, D;
double pidOutput;
double dt;
long tempoAnterior;

double setPoint = 50; // Temperatura desejada
int pwmMotor = 0;

// Constantes do PID
double kP = 1.0;
double kI = 0.0;
double kD = 0.0;

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_TEMP_PIN, INPUT);
  pinMode(MOTOR_AQUECIMENTO_PIN, OUTPUT);
  pinMode(MOTOR_RESFRIAMENTO_PIN, OUTPUT);
}

void loop() {
  // Leitura da temperatura em graus Celsius
  temperaturaAtual = (analogRead(SENSOR_TEMP_PIN) * (5000.0 / 1024.0) - 500) / 10.0;
  
  // PID
  erro = setPoint - temperaturaAtual;
  dt = (millis() - tempoAnterior) / 1000.0;
  tempoAnterior = millis();

  P = erro * kP;
  I = I + (erro * kI) * dt;
  D = (temperaturaAnterior - temperaturaAtual) * kD / dt;
  temperaturaAnterior = temperaturaAtual;

  pidOutput = P + I + D;
  pwmMotor = pidOutput;

  // Saída para motores
  if (pwmMotor < 0) {
    pwmMotor = sqrt(pwmMotor * pwmMotor); // garante valor positivo
    analogWrite(MOTOR_RESFRIAMENTO_PIN, pwmMotor);
    analogWrite(MOTOR_AQUECIMENTO_PIN, 0);
  } else {
    analogWrite(MOTOR_AQUECIMENTO_PIN, pwmMotor);
    analogWrite(MOTOR_RESFRIAMENTO_PIN, 0);
  }

  // Monitor serial para debug
  Serial.print("Temp: ");
  Serial.print(temperaturaAtual);
  Serial.print(" °C | PID: ");
  Serial.print(pidOutput);
  Serial.print(" | PWM: ");
  Serial.println(pwmMotor);

  delay(10); // Pequeno delay para estabilidade
}
