#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>


#define SSID "Totalplay-459D"
#define PASS  "459D3609AXbwE2eN"

const char* ssid     = SSID;
const char* password = PASS;

const char* host = "192.168.100.11";
const uint16_t port = 80;



const byte espRx = 5;
const byte espTx = 4;
SoftwareSerial SerialEsp(espRx, espTx); /se utilizan los puertos RX y TX

// RX msg
bool received = false;
String receivedMsg;

// variables to control
String s_alarm;
String s_co;
String s_smoke;
String s_lpg;
String s_temp;
const byte numVars = 5;
//                          0         1     2       3        4
String controlledVars[] = {s_alarm, s_co, s_lpg, s_smoke, s_temp};

// variables float y int
int alarm = 0;
int co = 0;
int lpg = 0;
int smoke = 0;
float temp = 0.0;


void setup() 
{
  Serial.begin(115000);
  SerialEsp.begin(9600);

  // Empezamos por conectarnos a una red WiFi

  Serial.println();
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  /*Configure explícitamente el ESP8266 para que sea un cliente WiFi, de lo contrario,
   * de forma predeterminada,intentaría actuar como cliente y punto de acceso y podría 
   * causar problemas de red con sus otros dispositivos WiFi en su red WiFi. */
   
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{  
  checkSerialCom();
  if (received){
    ProcessMsg();
    receivedMsg = "";
    received = false;
    ConvertVariables();
    delay(50);
    // Intentar enviar datos al host
    SendDataToHost(); 
  }

  delay(10000); //ejecutamos cada 10s
} // EOF loop


/************ Funciones auxiliares **********************/
void checkSerialCom()
{
  char data;
  if (SerialEsp.available() > 0)
  { // Comprueba si los datos provienen del puerto serie
    while (SerialEsp.available() > 0)
    {
      data = (char)SerialEsp.read(); // Lee los datos del puerto serie
      receivedMsg += data;
    }
    received = true;
  }
}


void ProcessMsg(){
  receivedMsg.trim();
  Serial.println("Recibido: ");
  Serial.println(receivedMsg);
  int index;
  for (size_t i = 0; i < numVars; i++)
  {
    index = receivedMsg.indexOf('_');
    controlledVars[i] = receivedMsg.substring(0, index);
    receivedMsg = receivedMsg.substring(index + 1);
  }
  s_alarm = controlledVars[0];
  s_co = controlledVars[1];
  s_lpg = controlledVars[2];
  s_smoke = controlledVars[3];
  s_temp = controlledVars[4];
}


void ConvertVariables(){
  alarm = s_alarm.toInt();
  co = s_co.toInt();
  lpg = s_lpg.toInt();
  smoke = s_smoke.toInt();
  temp = s_temp.toFloat(); // si la conversión no es posible, se devuelve 0
}


void SendDataToHost(){
  Serial.print(">>>>>>> conectando a ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Utilice la clase WiFiClient para crear conexiones TCP
  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("coneccion fallida");
    delay(5000);
    return;
  }

  String query = "esp8266/save.php?alarm=";
  query += alarm;
  query += "&co=";
  query += co;
  query += "&lpg=";
  query += lpg;
  query += "&smoke=";
  query += smoke;
  query += "&temp=";
  query += temp;
  
  Serial.println(query);

  Serial.println("[Enviando una solicitud]");
  client.print(String("GET /") + query + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n" +
               "\r\n"
              );

  // esperar a que los datos estén disponibles
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Se acabo el tiempo de espera del cliente !");
      client.stop();
      delay(3000);
      return;
    }
  }

  // Leer todas las líneas de la respuesta del servidor.
  Serial.println("recibiendo datos del servidor remoto");
  String msg;
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    Serial.print(ch); // imprimir la respuesta
    msg += ch;
  }
  
  Serial.println();
  if (msg.indexOf("Guardado correcto!") != -1){
    Serial.println("Datos guardos");
  }
  else{
    Serial.println("No se pudieron guardar los datos");
  }

  // Close the connection
  Serial.println();
  Serial.println(">>>>>>>>>> cierre de conexión");
  Serial.println();
  client.stop();
} 
