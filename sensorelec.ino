#include <WiFi.h>
float Sensibilidad=0.185; //sensibilidad en Voltios/Amperio para sensor de 5A
#define calibration_const 355.55
//float SENSIBILITY = 0.100; // Modelo 20A
//float SENSIBILITY = 0.066; // Modelo 30A
//https://www.luisllamas.es/arduino-intensidad-consumo-electrico-acs712/
WiFiServer server(80);

const char* ssid = "GATITO";//"GATITO";//"CEMENTERIO";
const char* password =  "*rene5254@*";//"*rene5254@*";//"S1104413743a";

//Variable donde se almacena los datos que nos vienen en la cabecera
String cabecera;
//El tiempo de conexion a la red wifi
int conexion = 0;
//el dato que se espera de la peticion http: 0 siginifica apagado, 1 significa prendido
String estadoSalida = "0";
String valorAmperio = "0";
String valorIrms = "0";
String valorPotencia = "0";
//la salida de nuestro circuito
const int salida = 4;
//GPIO para leer el sensor de temperatuta
const int out_elect = A6;//34
//Nuestra pagina web html
String html = "";
int SAMPLESNUMBER = 100;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.write("El ESP32 se esta iniciando.");
  estadoSalida = "1";
  digitalWrite(salida, LOW);
  //delay(500);
  pinMode(salida, OUTPUT); //Configuramos el foco
  
  
  //digitalWrite(2, HIGH);//encendemos el led

  WiFi.begin(ssid, password);

  //mientras se conecta o han pasado 30 seg en la conexion
  while (WiFi.status() != WL_CONNECTED and conexion < 60) {
    conexion ++;
    delay(500);
    Serial.println("Conectando a la red ..");
  }
  if (conexion <= 50) {
    //para fijar IP
    //para fijar IP
    IPAddress ip(192, 168, 1, 50);//IPAddress ip(192, 168, 1, 151);
    //Para fijar la puerta de entrada
      IPAddress gateway(192, 168, 1, 1);//IPAddress gateway(192, 168, 1, 1);
      //Para fijar la mascara de sub red
      IPAddress subnet(255, 255, 255, 0);//IPAddress subnet(255, 255, 255, 0);
    //Para configurar todo
    WiFi.config(ip, gateway, subnet);

    Serial.println(".....");
    Serial.println("...WIFI CONECTADO..");
    Serial.println(WiFi.localIP());
    pinMode(2, OUTPUT); //Configuramos el led del hardware de la placa
    digitalWrite(2, HIGH);//encendemos el led
    WiFi.setAutoReconnect(true);
    server.begin();
  }
  else {
    Serial.println("ERROR DE CONEXION");
  }
  
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  /* // Lee el pin de entrada analógica 0 muestra 0-1023:
  int SensorValue = analogRead (A0);

  // Puede hacer cambios para mostrar en Amperaje
  // El siguiente código le dice al Arduino que el valor leído por el sensor
  // debe estar activado de 0-1.023 entre -30 a +30.

  int OutputValue = map (SensorValue, 0, 1023, -30, 30);
  // Mostrar valor leído por el sensor:

  Serial.print ("Sensor: ");
  Serial.print (SensorValue);

  // Espectáculo valor transformado en amperios:
  Serial.print (" Valor en Amperios: ");
  Serial.println (OutputValue);
  delay (100); // Tiempo entre lecturas*/
  //Serial.println(analogRead(A6));
  
  Serial.write("Se comienza");
  delay(100);
  WiFiClient client = server.available();   // Escucha a los clientes entrantes

  if (client) {                             // Si se conecta un nuevo cliente
    Serial.println("Nuevo cliente.");          //
    String currentLine = "";                //
    html = htmlData();
    while (client.connected()) {            // loop mientras el cliente está conectado
      if (client.available()) {             // si hay bytes para leer desde el cliente
        char c = client.read();             // lee un byte
        Serial.write(c);                    // imprime ese byte en el monitor serial
        cabecera += c;
        if (c == '\n') {                    // si el byte es un caracter de salto de linea
          // si la nueva linea está en blanco significa que es el fin del
          // HTTP request del cliente, entonces respondemos:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            // cabecera y apaga el GPIO
            if (cabecera.indexOf("GET /1") >= 0) {                            
              html = htmlData();
            } else if (cabecera.indexOf("GET /2") >= 0) {
              apagarFoco();
              html = htmlJsonTem();

            }else if (cabecera.indexOf("GET /3") >= 0) {
              
              prenderFoco();
              html = htmlJsonTem();

            }
            else if (cabecera.indexOf("GET /4") >= 0) {
              
              html = htmlJsonTem();
              
            } else {
              html = htmlData();
              }

            // Muestra la página web

            client.println(html);

            // la respuesta HTTP temina con una linea en blanco
            client.println();
            break;
          } else { // si tenemos una nueva linea limpiamos currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // si C es distinto al caracter de retorno de carro
          currentLine += c;      // lo agrega al final de currentLine
        }
      }
    }
    // Limpiamos la variable header
    cabecera = "";
    // Cerramos la conexión
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }


  
}

void prenderFoco() {
  
    estadoSalida = "0";
    digitalWrite(salida, HIGH);    
  
  
}

void apagarFoco() {
  
    estadoSalida = "1";
    digitalWrite(salida, LOW);    
  
}

float sensorLuz(int samplesNumber) {

  float voltajeSensor;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(out_elect);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
   }
   voltajeSensor= (maxValue * 5.0); //lectura del sensor   123   
  //voltajeSensor= (maxValue * 5.0) / 1024.0; //lectura del sensor   123
  //Serial.println(voltajeSensor);
  //float I=(voltajeSensor-2.5)/Sensibilidad; //Ecuación  para obtener la corriente  
  return voltajeSensor;
  
}

String htmlData() {
  
  float nVPP = sensorLuz(SAMPLESNUMBER);
  float voltaje = nVPP / 1024.0;
  float nCurrThruResistorRMS = (nVPP * 0.707) / 1024.0;
  float nCurrThruResistorIRMS = nCurrThruResistorRMS * calibration_const;
  //float nCurrThruResistorPP = (nVPP/200.0) * 1000.0;
  //float nCurrThruResistorRMS = nCurrThruResistorPP * 0.707;
  //float nCurrentThruWire = nCurrThruResistorRMS * 1000;
  
  valorAmperio = String(nCurrThruResistorRMS) + " mA";
  valorIrms = String(nCurrThruResistorIRMS) + " mA";
  valorPotencia = String((voltaje)) + "Volts";
  String foco = "APAGADO"; 
  if (estadoSalida == "1") {
    foco = "ENCENDIDO";
  }
//aCS712

  html = "<!DOCTYPE html>"
         "<html>"
         "<head>"
         "<meta charset='utf-8' />"
         "<title>Servidor Web</title>"
         "</head>"
         "<body>"
         "<center>"
         "<h1>Servidor Web con la placa ESP32</h1>"
         "<h1>aCS712</h1>"         
         "<p>AMPERIOS " + valorAmperio + "</p>"
         "<p>IRMS " + valorIrms + "</p>"
         "<p>VOLTAJE " + valorPotencia + "</p>"
         "<p>FOCO " + foco + "</p>"
         "</center>"
         "</body>"
         "</html>";
  return html;
}
String htmlJsonTem() {
  float nVPP = sensorLuz(SAMPLESNUMBER);
  float voltaje = nVPP / 1024.0;
  float nCurrThruResistorRMS = (nVPP * 0.707) / 1024.0;
  float nCurrThruResistorIRMS = nCurrThruResistorRMS * calibration_const;
  //float nCurrThruResistorPP = (nVPP/200.0) * 1000.0;
  //float nCurrThruResistorRMS = nCurrThruResistorPP * 0.707;
  //float nCurrentThruWire = nCurrThruResistorRMS * 1000;
  
  valorAmperio = String(nCurrThruResistorRMS) + " mA";
  valorIrms = String(nCurrThruResistorIRMS) + " mA";
  valorPotencia = String((voltaje)) + "Volts";
  
  
  //"{ \"estado\": \""+estado+"\"}"
  html = "{ \"foco\": \""+estadoSalida+"\""+","+ "\"amperage\": \""+valorAmperio + "\""+"," +  "\"irms\": \""+valorIrms+"\""+"," + "\"potencia\": \""+valorPotencia+"\""+ "}";
  //html = "{ \"estado\": \""+salida+"\", \"temperatura\": \""+valorTemp+"\"}";
  
    return html;
}
