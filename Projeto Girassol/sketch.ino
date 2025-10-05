// pinos do arduino
#define LDR_ESQ   A0        // ldr da esquerda
#define LDR_DIR   A1        // ldr da direita
#define ENC_POS   A2        // potenciometro que simula o encoder (posição -90 a +90)

#define IN1_DIR   7         // led verde -> motor indo pra direita
#define IN2_DIR   8         // led vermelho -> motor indo pra esquerda
#define EN_PWM    9         // pino de pwm que seria a velocidade do motor

#define FC_ESQ    2         // push button fim de curso lado esquerdo
#define FC_DIR    3         // push button fim de curso lado direito

// limites do movimento em graus
const float ANG_MIN = -90.0;
const float ANG_MAX = +90.0;

// tempos e ajustes de resposta
const unsigned long TS_CTRL_MS   = 20;    // controle roda a cada 20ms
const unsigned long TS_REF_MS    = 2000;  // referência da luz atualiza a cada 2s
const float HYST_REF             = 0.04;  // histerese da luz (evita tremer se valores quase iguais)
const float RATE_LIM_DEG_PER_S   = 10.0;  // limite de variação por segundo da referência
const int   PWM_MIN              = 40;    // valor mínimo de pwm pra vencer atrito
const int   PWM_MAX              = 255;   // valor máximo de pwm

// ganhos do pid
float Kp = 2.0;
float Ki = 0.8;
float Kd = 0.08;

// variáveis internas do pid
float pid_i = 0.0;
float prev_err = 0.0;

// estado atual
float ang_ref_deg = 0.0;    // ângulo que deveria estar (referência)
float ang_meas_deg = 0.0;   // ângulo medido pelo potenciômetro
float u_cmd = 0.0;          // comando do pid (-255..255)

unsigned long t_last_ctrl = 0;
unsigned long t_last_ref  = 0;

// valores lidos dos ldrs
int lL = 0;
int lR = 0;

// função de limite (evita passar do mínimo/máximo)
static inline float clamp(float x, float lo, float hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}

void setup() {
  pinMode(IN1_DIR, OUTPUT);
  pinMode(IN2_DIR, OUTPUT);
  pinMode(EN_PWM, OUTPUT);

  pinMode(FC_ESQ, INPUT_PULLUP);
  pinMode(FC_DIR, INPUT_PULLUP);

  // começa desligado
  digitalWrite(IN1_DIR, LOW);
  digitalWrite(IN2_DIR, LOW);
  analogWrite(EN_PWM, 0);

  Serial.begin(9600);
  Serial.println("log do girassol iniciado");
}

// ler potenciometro e transformar em ângulo de -90 até +90
float readAngleDeg() {
  int adc = analogRead(ENC_POS);      
  return map(adc, 0, 1023, -90, 90);  
}

// calcula referência de posição com base nos ldrs
void updateReference() {
  unsigned long now = millis();
  if (now - t_last_ref < TS_REF_MS) return;
  t_last_ref = now;

  lL = analogRead(LDR_ESQ);
  lR = analogRead(LDR_DIR);

  float sum = (float)lL + (float)lR;
  if (sum < 1.0) sum = 1.0;
  float diff = (float)lR - (float)lL;
  float lux_norm = diff / sum;

  static float target_norm = 0.0;
  if (lux_norm > (target_norm + HYST_REF)) {
    target_norm = lux_norm;
  } else if (lux_norm < (target_norm - HYST_REF)) {
    target_norm = lux_norm;
  }

  float target_ref = clamp(target_norm * 90.0f, ANG_MIN, ANG_MAX);

  float max_step = RATE_LIM_DEG_PER_S * (TS_REF_MS / 1000.0f);
  float delta = target_ref - ang_ref_deg;
  if (delta >  max_step) delta =  max_step;
  if (delta < -max_step) delta = -max_step;
  ang_ref_deg = clamp(ang_ref_deg + delta, ANG_MIN, ANG_MAX);
}

// roda o pid e manda o motor/leds
void runPID() {
  unsigned long now = millis();
  if (now - t_last_ctrl < TS_CTRL_MS) return;
  float dt = (now - t_last_ctrl) / 1000.0f;
  t_last_ctrl = now;

  ang_meas_deg = readAngleDeg();

  float err = ang_ref_deg - ang_meas_deg;
  pid_i += err * Ki * dt;
  float i_max = 255.0f / 2.0f;
  pid_i = clamp(pid_i, -i_max, i_max);

  float d = (err - prev_err) / (dt > 1e-4 ? dt : 1e-4);
  prev_err = err;

  float p = Kp * err;
  float di = pid_i;
  float dd = Kd * d;

  float u = p + di + dd;
  u = clamp(u, -255.0f, 255.0f);
  u_cmd = u;

  bool limite_esq = (ang_meas_deg <= ANG_MIN + 0.5f) || (digitalRead(FC_ESQ) == LOW);
  bool limite_dir = (ang_meas_deg >= ANG_MAX - 0.5f) || (digitalRead(FC_DIR) == LOW);

  int pwm = 0;
  if (u > 0) {
    if (!limite_dir) {
      pwm = (int)u;
      if (pwm > 0 && pwm < PWM_MIN) pwm = PWM_MIN;
      digitalWrite(IN1_DIR, HIGH);
      digitalWrite(IN2_DIR, LOW);
      analogWrite(EN_PWM, clamp(pwm, 0, PWM_MAX));
    } else {
      digitalWrite(IN1_DIR, LOW);
      digitalWrite(IN2_DIR, LOW);
      analogWrite(EN_PWM, 0);
      pid_i = 0;
    }
  } else if (u < 0) {
    if (!limite_esq) {
      pwm = (int)(-u);
      if (pwm > 0 && pwm < PWM_MIN) pwm = PWM_MIN;
      digitalWrite(IN1_DIR, LOW);
      digitalWrite(IN2_DIR, HIGH);
      analogWrite(EN_PWM, clamp(pwm, 0, PWM_MAX));
    } else {
      digitalWrite(IN1_DIR, LOW);
      digitalWrite(IN2_DIR, LOW);
      analogWrite(EN_PWM, 0);
      pid_i = 0;
    }
  } else {
    digitalWrite(IN1_DIR, LOW);
    digitalWrite(IN2_DIR, LOW);
    analogWrite(EN_PWM, 0);
  }

  // log a cada 4s pra acompanhar
  static unsigned long t_last_log = 0;
  if (now - t_last_log >= 4000) {
    t_last_log = now;
    Serial.print("ref: "); Serial.print(ang_ref_deg, 1); Serial.print("° | ");
    Serial.print("medido: "); Serial.print(ang_meas_deg, 1); Serial.print("° | ");
    Serial.print("erro: "); Serial.print(err, 1); Serial.print("° | ");
    Serial.print("pwm: "); Serial.print((u >= 0) ? pwm : -pwm); Serial.print(" | ");
    Serial.print("ldr esq: "); Serial.print(lL);
    Serial.print(" | ldr dir: "); Serial.println(lR);
  }
}

void loop() {
  updateReference();  // atualiza referência da luz
  runPID();           // roda o controle pid
}
