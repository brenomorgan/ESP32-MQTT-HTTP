#include "secrets.h"            // Inclui o arquivo "secrets.h", que contém as informações de segurança, como credenciais de Wi-Fi e certificados AWS.
#include <WiFiClientSecure.h>   // Inclui a biblioteca WiFiClientSecure, que permite a conexão segura com o Wi-Fi.
#include <PubSubClient.h>       // Inclui a biblioteca PubSubClient, que fornece funcionalidades MQTT para o ESP32.
#include <ArduinoJson.h>        // Inclui a biblioteca ArduinoJson, que facilita a manipulação de dados JSON.
#include "WiFi.h"               // Inclui a biblioteca WiFi, que permite a conexão com redes Wi-Fi.

#define AWS_IOT_PUBLISH_TOPIC  "*****"    // Define o tópico MQTT em que o ESP32 vai publicar mensagens.
#define AWS_IOT_SUBSCRIBE_TOPIC "*****"   // Define o tópico MQTT em que o ESP32 vai receber mensagens.

int anlg = 0;      // Variável para armazenar o valor analógico lido.
int dig = 0;       // Variável para armazenar o valor digital lido.
float tensao = 0;  // Variável para armazenar o valor da tensão calculada.
String tipo;       // Variável para armazenar o tipo.

WiFiClientSecure net = WiFiClientSecure(); // Cria um objeto WiFiClientSecure para conexão segura.
PubSubClient client(net);                  // Cria um objeto PubSubClient passando o objeto WiFiClientSecure.

void connectAWS()
{
  WiFi.mode(WIFI_STA);                  // Configura o ESP32 para operar em modo estação (cliente) Wi-Fi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Inicia a conexão Wi-Fi usando as credenciais definidas em "secrets.h".

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Configura o WiFiClientSecure para usar as credenciais do dispositivo AWS IoT.
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Conecta-se ao agente MQTT no endpoint da AWS que foi definido anteriormente.
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Define uma função de tratamento de mensagens recebidas.
  client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IOT");
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Inscreve-se em um tópico MQTT para receber mensagens.
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  StaticJsonDocument<200> doc; // Cria um objeto StaticJsonDocument para armazenar os dados JSON.
  char jsonBuffer[512];        // Cria um buffer de caracteres para armazenar o JSON serializado.

  doc["state"]["desired"]["caracter"] = tipo;     // Define o valor da chave "caracter" dentro de "state"->"desired" como o valor da variável "tipo".
  doc["state"]["desired"]["analogico"] = tensao;  // Define o valor da chave "analogico" dentro de "state"->"desired" como o valor da variável "tensao".
  doc["state"]["desired"]["digital"] = dig;       // Define o valor da chave "digital" dentro de "state"->"desired" como o valor da variável "dig".

  serializeJson(doc, jsonBuffer); // Serializa os dados JSON do objeto doc para o buffer de caracteres jsonBuffer.

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer); // Publica a mensagem MQTT com o conteúdo do buffer jsonBuffer no tópico AWS_IOT_PUBLISH_TOPIC.
}


void messageHandler(char* topic, byte* payload, unsigned int length)
{ 
  Serial.print("incoming: ");
  Serial.println(topic);
  StaticJsonDocument<200> doc; // Cria um objeto StaticJsonDocument para armazenar os dados JSON recebidos.
  char jsonBuffer[512];         // Cria um buffer de caracteres para armazenar o JSON serializado.

  deserializeJson(doc, payload);  // Desserializa os dados JSON recebidos e armazena no objeto doc.
  serializeJson(doc, jsonBuffer); // Serializa os dados JSON para o buffer de caracteres.
  Serial.println(jsonBuffer);     // Imprime o JSON recebido no monitor serial.
  
  String message = doc["state"]["desired"]["teste"]; // Obtém o valor da chave "teste" do objeto JSON.
  Serial.println(message);                           // Imprime o valor da chave "teste" no monitor serial.
}

void setup()
{
  Serial.begin(115200); // Inicializa a comunicação serial com uma taxa de 115200 bps.
  connectAWS();         // Conecta-se ao serviço AWS IoT.
  pinMode(34, INPUT);   
}
 
void loop()
{
  tipo = "TESTE";                
  dig = 1;                       
  anlg = analogRead(34);          // Lê o valor analógico do pino 34 e armazena na variável "anlg".
  tensao = (anlg * 3.3) / 4095.0; // Calcula o valor da tensão com base no valor analógico lido.
  publishMessage();               // Publica a mensagem MQTT com os dados coletados.
  client.loop();                  // Mantém a conexão MQTT ativa.
  delay(1000);
}
