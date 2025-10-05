#define PINO_PWM 5         // LED simula esforço do motor
#define PINO_TOUCH 4       // sensor touch digital
#define PINO_POT A0        // potenciômetro analógico
#define DELTA_PULSO 1000   // intervalo de cálculo (ms)

unsigned long ultimaVerificacao, proximaVerificacao;
float rpm_simulado = 0;
int pwm = 0;

const int RPM_ALVO = 1500;
const float KP = 0.2; // reduzido para evitar saturação
const float FATOR_RPM = 6.0; // fator de conversão PWM → RPM

void setup() {
  pinMode(PINO_PWM, OUTPUT);
  pinMode(PINO_TOUCH, INPUT);
  Serial.begin(9600);

  ultimaVerificacao = millis();
  proximaVerificacao = ultimaVerificacao + DELTA_PULSO;
}

void loop() {
  unsigned long agora = millis();

  if (agora >= proximaVerificacao) {
    ultimaVerificacao = agora;
    proximaVerificacao = agora + DELTA_PULSO;

    // Leitura do potenciômetro → perturbação
    int leituraPot = analogRead(PINO_POT);
    int P = map(leituraPot, 0, 1023, -30, 30); // perturbação entre -30% e +30%

    // Sensor touch → perturbação extrema
    if (digitalRead(PINO_TOUCH) == HIGH) {
      P = -50;
    }

    // Simula RPM como função do PWM e da perturbação
    rpm_simulado = pwm * FATOR_RPM * (1.0 - P / 100.0);

    // Controle proporcional
    int erro = RPM_ALVO - rpm_simulado;
    int ajuste = KP * erro;

    // Aplica perturbação como ajuste fixo
    int impacto = P * 0.3;

    pwm += ajuste + impacto;

    // Estabilização: desacelera se estiver próximo do alvo
    if (abs(erro) < 10) {
      pwm -= 1;
    }

    // Saturação
    pwm = constrain(pwm, 0, 255);
    analogWrite(PINO_PWM, pwm);

    // Exibição no monitor serial
    Serial.print("RPM: ");
    Serial.print(rpm_simulado, 2);
    Serial.print(" | PWM: ");
    Serial.print(pwm);
    Serial.print(" | Perturbação: ");
    Serial.print(P);
    Serial.print("% | Erro: ");
    Serial.println(erro);
  }
}