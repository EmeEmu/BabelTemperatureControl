#include <Adafruit_MAX31856.h>
#include <LiquidCrystal.h>
#include <AutoPID.h>

// THERMAL
// Use software SPI:                            CS, DI, DO, CLK
Adafruit_MAX31856 maxthermo = Adafruit_MAX31856( 9, 10, 11,  12);
double temp;

// SMOOTHING
float smooth_f = 0.05;
double temp_smooth;
double temp_prev = 0;

// CONTROL
const int potPIN = A0; // potentiometer to set the temperature
int pot_val; // value of the potentiometer
double set_temp; // set temperature
const int heaterPINhot = A1; // peltier PWM control pin
const int heaterPINcool = A2; // peltier PWM control pin
unsigned long window = 10*1000; // window time in milliseconds
unsigned long now; // current time
unsigned long window_start; // time of current window start
double heater_percent; // percent of window time when Peltier should be ON
int heater_ONOFF = 0; // peltier control
double KP = 0.15;
double KI = 0.0001;//0.01;
double KD = 0;//10;
AutoPID myPID(&temp, &set_temp, &heater_percent, 0., 1., KP, KI, KD); // PID on the percent of window time during which peltier is ON

// DISPLAY
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void PeltierControl(int onoff){
  if (onoff > 0) {
    analogWrite(heaterPINcool,0);
    analogWrite(heaterPINhot, 255);
  } else if (onoff < 0) {
    analogWrite(heaterPINcool, 255);
    analogWrite(heaterPINhot, 0);
  } else {
    analogWrite(heaterPINcool, 0);
    analogWrite(heaterPINhot, 0);
  }
}


void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // THERMOCOUPLE
  maxthermo.begin();
  maxthermo.setThermocoupleType(MAX31856_TCTYPE_K);
  Serial.print("Thermocouple type: ");
  switch (maxthermo.getThermocoupleType() ) {
    case MAX31856_TCTYPE_B: Serial.println("B Type"); break;
    case MAX31856_TCTYPE_E: Serial.println("E Type"); break;
    case MAX31856_TCTYPE_J: Serial.println("J Type"); break;
    case MAX31856_TCTYPE_K: Serial.println("K Type"); break;
    case MAX31856_TCTYPE_N: Serial.println("N Type"); break;
    case MAX31856_TCTYPE_R: Serial.println("R Type"); break;
    case MAX31856_TCTYPE_S: Serial.println("S Type"); break;
    case MAX31856_TCTYPE_T: Serial.println("T Type"); break;
    case MAX31856_VMODE_G8: Serial.println("Voltage x8 Gain mode"); break;
    case MAX31856_VMODE_G32: Serial.println("Voltage x8 Gain mode"); break;
    default: Serial.println("Unknown"); break;
  }

  // DISPLAY
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("hello !");

  // CONTROL
  //myPID.setBangBang(10);
  //myPID.setTimeStep(4000);
  pinMode(heaterPINhot, OUTPUT);
  pinMode(heaterPINcool, OUTPUT);
  PeltierControl(0);
  window_start = millis();
}

void loop() {

  // GET REAL TEMP
  temp = maxthermo.readThermocoupleTemperature();
  // Check and print any faults
  uint8_t fault = maxthermo.readFault();
  if (fault) {
    if (fault & MAX31856_FAULT_CJRANGE) Serial.println("Cold Junction Range Fault");
    if (fault & MAX31856_FAULT_TCRANGE) Serial.println("Thermocouple Range Fault");
    if (fault & MAX31856_FAULT_CJHIGH)  Serial.println("Cold Junction High Fault");
    if (fault & MAX31856_FAULT_CJLOW)   Serial.println("Cold Junction Low Fault");
    if (fault & MAX31856_FAULT_TCHIGH)  Serial.println("Thermocouple High Fault");
    if (fault & MAX31856_FAULT_TCLOW)   Serial.println("Thermocouple Low Fault");
    if (fault & MAX31856_FAULT_OVUV)    Serial.println("Over/Under Voltage Fault");
    if (fault & MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
  }

  // SMOOTH TEMP
  //if (temp_prev == 0){
  //  temp_prev = temp;
  //  }
  //temp_smooth = (temp*smooth_f) + (temp_prev*(1-smooth_f));
  //temp_prev = temp_smooth;


  // TARGET TEMPERATURE
  pot_val = analogRead(potPIN);
  set_temp = map(pot_val, 0,1023, 15,33);

  // PID
  myPID.run();

  // Peltier Control
  now = millis();
  if (now > window_start + window){
      heater_ONOFF = 0;
      window_start = now;
  } else if (now > window_start + heater_percent*window){
      heater_ONOFF = 0;    
  } else {
      heater_ONOFF = 1;
  }
  PeltierControl(heater_ONOFF);

  // DISPLAY
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.print((float) temp);
  lcd.print("C");

  lcd.setCursor(0,1);
  lcd.print("S:");
  lcd.print((int) set_temp);
  lcd.print("C");

  lcd.setCursor(8,1);
  lcd.print("PWM:");
  //lcd.print((int) heater_percent);
  lcd.print(heater_percent, 3);
  //lcd.print("%");

  // SERIAL
  Serial.print("Data:,");
  Serial.print(temp);
  //Serial.print(",");Serial.print(temp_smooth);
  Serial.print(",");Serial.print(set_temp);
  Serial.print(",");Serial.print(heater_percent);
  Serial.print(",");Serial.println(heater_ONOFF);
  
  delay(50);
}
