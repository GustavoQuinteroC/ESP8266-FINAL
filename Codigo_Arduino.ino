
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>


const byte espRx = 3;
const byte espTx = 4;

// alarma
const byte ledAlarm = 5;
const byte buzzerAlarm = 6;
const byte relay = 9;

// lcd
const byte rs = 7;
const byte e = 8;
const byte d4 = 10;
const byte d5 = 11;
const byte d6 = 13;
const byte d7 = 19;  //A5

// sensores
const byte mq = A0;
const byte lm35 = A3;

// bboton
const byte ack = 2; // pin capaz de interrumpir
/***** Pin References ******/

// varibles globales
long co = 0;   //(if saturated > 65535)
long lpg = 0;
long smoke = 0;
float temperature = 0;
bool generalAlarm = false; // si se dispara alguna alarma será cierto
volatile bool ackPressed = false;

// set de alarma
const int lpgSet = 2100;
const int coSet = 200;
const int smokeSet = 1150;
const float temperatureSet = 45.0; // °C



// Intancias
SoftwareSerial SerialEsp(espRx, espTx); // RX, TX
LiquidCrystal lcd(rs, e, d4, d5, d6, d7); // Crea un objeto LCD 


// Tiempos
long startingLoopTime = 0;
long currentLoopTime = 0;
const int loopTimeSet = 1000; // 1.5s

long startingSerialTime = 0; // para enviar datos a esp
long currentSerialTime = 0;
const int serialTimeSet = 10000; // 30s


// Caracter personalizado de campana
 byte bell[8] = {
	0b00100,
	0b01110,
	0b01110,
	0b01110,
	0b11111,
	0b11111,
	0b11111,
	0b00000
 }; // eof bell


/************************* MQ & GASES ********************************************/
/************************Macros relacionadas con el hardware************************************/
// ledAlarm = 4;                      //cuando comience la calibración, el LED pin 13 se iluminará, se apagará cuando termine de calibrar
//mq = A0;                                //defina qué canal de entrada analógica va a utilizar
int RL_VALUE = 1;                                     //definir la resistencia de carga en la placa, en kilo ohms
float RO_CLEAN_AIR_FACTOR = 9.86;                     //(JADSA cal) RO_CLEAR_AIR_FACTOR = (Resistencia del sensor en aire limpio) / RO,
                                                    //que se deriva del gráfico en la hoja de datos
 
/***********************Macros relacionados con el software************************************/
int CALIBRATION_SAMPLE_TIMES=50;                    //definir cuántas muestras va a tomar en la fase de calibración
int CALIBRATION_SAMPLE_INTERVAL=500;                //definir el tiempo interal (en milisegundos) entre cada muestra en el
                                                    //fase de calibracion
int READ_SAMPLE_INTERVAL=50; //50                        //definir cuántas muestras va a tomar en funcionamiento normal
int READ_SAMPLE_TIMES=5; //5                            //definir el tiempo interal (en milisegundos) entre cada muestra en operacion normal
 
/**********************Macros relacionados con la aplicacion**********************************/
#define         GAS_LPG             0   
#define         GAS_CO              1   
#define         GAS_SMOKE           2    
 
/*****************************Globales***********************************************/
float           LPGCurve[3]  =  {2.3,0.21,-0.47};   //se toman dos puntos de la curva.
                                                    //con estos dos puntos se forma una línea que es "aproximadamente equivalente"a la curva original.
                                                    //formato de datos: {x, y, pendiente}; punto1: (lg200, 0.21), punto2: (lg10000, -0.59)
float           COCurve[3]  =  {2.3,0.72,-0.34};    //se toman dos puntos de la curva. 
                                                    //con estos dos puntos se forma una línea que es "aproximadamente equivalente" a la curva original
                                                    //formato de datos: {x, y, pendiente}; punto1: (lg200, 0,72), punto2: (lg10000, 0,15) 
float           SmokeCurve[3] ={2.3,0.53,-0.44};    //se toman dos puntos de la curva.
                                                    //con estos dos puntos se forma una línea que es "aproximadamente equivalente" a la curva original
                                                    //formato de datos: {x, y, pendiente}; punto1: (lg200, 0.53), punto2: (lg10000, -0.22)                                                    
float           Ro           =  10;                 //Ro se inicializa a 10 kilo ohmios
// EOF gases



void setup() {
  Serial.begin(115200);
  SerialEsp.begin(9600);
  lcd.begin(16, 2); //dimensiones  ancho y alto (16 X 2)

  // pin configacion
  pinMode(ledAlarm, OUTPUT);
  pinMode(buzzerAlarm, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(ack, INPUT_PULLUP);

  handleRelay(false); // asegúrese de que el relé esté desactivado
  handleBuzzer(false);
  handleLedAlarm(false);


  // creacion de un nuevo caracter
  lcd.createChar(0, bell);

  // Configuración de interrupción
  attachInterrupt(digitalPinToInterrupt(ack), ISR_acknowledge, FALLING);
 

  /******** MQ CALIBRACION ************/
  digitalWrite(ledAlarm, HIGH);                      // En esta etapa el led significa que está calibrando
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibrando...");                        //LCD display
  Ro = MQCalibration(mq);                            //Calibrando el sensor. Asegúrese de que el sensor esté en aire limpio
  digitalWrite(ledAlarm,LOW);              
  
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("Listo!");                                //LCD display
  lcd.setCursor(0,1);
  lcd.print("Ro= ");
  lcd.print(Ro);
  lcd.print("kohm");
  delay(1500);
  
  /******** FIN DE CALIBRACIÓN MQ ************/
  
  // inicializar la monitorización
  startingLoopTime = millis();
  startingSerialTime = startingLoopTime;

  showVariablesOnLCD();

  ackPressed = false;
  generalAlarm = false;
} // DIN DE CONFIGURACION

void loop() {

  if (ackPressed){
    generalAlarm = false;
    mainAlarmSwitch(false);
    ackPressed = false;
    Serial.println("Interrupción detectada en el loop");
  }

  /*** Execute this block once per loopTimeSet *****/
  currentLoopTime = millis();
  if (currentLoopTime - startingLoopTime >= loopTimeSet){
    temperature = analogRead(lm35) * 500.0 / 1024;
    Serial.print("La temperatura actual es: ");
    Serial.println(temperature);
    delayMicroseconds(350);

    // read mq gases
    readMQ();


    showVariablesOnLCD();
    // reset time vars
    startingLoopTime = millis();
  }
  /***** fin del bloque ****************************/

  /******* Serial Com una vez por serialTimeSet ********/
  currentSerialTime = millis();
  if (currentSerialTime - startingSerialTime >= serialTimeSet){
    generalAlarm = checkAlarms();
    if (generalAlarm){
      // activar todos los actuadores
      mainAlarmSwitch(true);
      Serial.println("Alarma activada");
    }
    else
    {
      mainAlarmSwitch(false);
    }

    // enviar datos a ESP
    SendData();

    // reset timer set
    startingSerialTime = millis();
  }

} // EOF loop


void readMQ(){
  lpg = MQGetGasPercentage(MQRead(mq)/Ro,GAS_LPG);
  co = MQGetGasPercentage(MQRead(mq)/Ro,GAS_CO);
  smoke = MQGetGasPercentage(MQRead(mq)/Ro,GAS_SMOKE);

  // restringir las variables (alcance de ppm)
  if (co > 1000000 || co < 0){
    co = 9999;
  }
  if (lpg > 1000000 || lpg < 0){
    lpg = 9999;
  }
  if (smoke > 1000000 || smoke < 0){
    smoke = 9999;
  }
}

bool checkAlarms(){
  // Verificará si alguna alarma está activa
  if (lpg >= lpgSet || co >= coSet || smoke >= smokeSet || temperature >= temperatureSet){
    return true;
  }
  else{
    return false;
  }
}


void showVariablesOnLCD(){
  lcd.clear();   
  lcd.setCursor( 0 , 0 );

  lcd.print("GLP:");
  if (lpg > 999){
    lcd.print(999);
  }
  else{
    lcd.print(lpg);
  }

  lcd.setCursor(8,0);
  lcd.print("CO:"); 
  lcd.print(co); // Ya restringido

  // si la alarma muestra la campana
  if (generalAlarm){
    lcd.setCursor(15, 0);
    lcd.write((uint8_t)0);
  }

  lcd.setCursor( 0, 1 );
  lcd.print("Humo:");
  if (smoke > 999){
    lcd.print(999);
  }
  else{
    lcd.print(smoke);  
  }
  
  lcd.setCursor(9,1);
  lcd.print("T:");
  lcd.print(temperature); 
}


void handleBuzzer(bool activate){
  if (activate){
    digitalWrite(buzzerAlarm, HIGH);
  }
  else{
    digitalWrite(buzzerAlarm, LOW);
  }
}

void handleLedAlarm(bool activate){
  if (activate){
    digitalWrite(ledAlarm, HIGH);
  }
  else{
    digitalWrite(ledAlarm, LOW);
  }
}

void handleRelay(bool activate){
  if(activate){
    digitalWrite(relay, LOW); // lógica inversa
  }
  else{
    digitalWrite(relay, HIGH);
  }
}

void mainAlarmSwitch(bool activate){
  if (activate){
    handleBuzzer(true);
    handleLedAlarm(true);
    handleRelay(true);
  }
  else{
    handleBuzzer(false);
    handleLedAlarm(false);
    handleRelay(false);
  }
}

Nuestra implementación de ISR
void ISR_acknowledge(){
  ackPressed = true;
}

void SendData(){
  String s_alarm = String(generalAlarm);
  String s_co = String(co);
  String s_lpg = String(lpg);
  String s_smoke = String(smoke);
  String s_temp = String(temperature);
  SerialEsp.println(s_alarm + "_" + s_co + "_" + s_lpg + "_" + s_smoke + "_" + s_temp);
  Serial.println(s_alarm + "_" + s_co + "_" + s_lpg + "_" + s_smoke + "_" + s_temp);
}


float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}
 
float MQCalibration(int mq_pin)
{
  int i;
  float val=0;

  for (i=0;i<CALIBRATION_SAMPLE_TIMES;i++) {            //tomar varias muestras
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBRATION_SAMPLE_TIMES;                   //calcular el valor medio
  val = val/RO_CLEAN_AIR_FACTOR;                        //Dividido por RO_CLEAN_AIR_FACTOR produce el Ro                                        
  return val;                                                      //de acuerdo con la tabla en la hoja de datos 

}

float MQRead(int mq_pin)
{
  int i;
  float rs=0;
 
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
 
  rs = rs/READ_SAMPLE_TIMES;
 
  return rs;  
}
 

long MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }    
 
  return 0;
}
 
long  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
