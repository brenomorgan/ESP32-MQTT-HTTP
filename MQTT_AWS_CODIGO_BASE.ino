#include "secrets.h"          // Inclui o arquivo "secrets.h", que contém as informações de segurança, como credenciais de Wi-Fi e certificados AWS.
#include <WiFiClientSecure.h> // Inclui a biblioteca WiFiClientSecure, que permite a conexão segura com o Wi-Fi.
#include <PubSubClient.h>     // Inclui a biblioteca PubSubClient, que fornece funcionalidades MQTT para o ESP32.
#include <ArduinoJson.h>      // Inclui a biblioteca ArduinoJson, que facilita a manipulação de dados JSON.
#include "WiFi.h"             

#define AWS_IOT_PUBLISH_TOPIC   "*****"   // Define o tópico MQTT em que o ESP32 vai publicar mensagens.
#define AWS_IOT_SUBSCRIBE_TOPIC "*****"   // Define o tópico MQTT em que o ESP32 vai receber mensagens.

int anlg = 0;     // Variável para armazenar o valor Bits lido.
int dig = 0;      // Variável para armazenar o valor digital lido.
float tensao = 0; // Variável para armazenar o valor da tensão calculada.

WiFiClientSecure net = WiFiClientSecure(); // Cria um objeto WiFiClientSecure para conexão segura.
PubSubClient client(net);                  // Cria um objeto PubSubClient passando o objeto WiFiClientSecure.

// Função de conexão com o Wi-Fi e com os serviços AWS.
void connectAWS()
{
  WiFi.mode(WIFI_STA);                   // Configura o ESP32 para operar em modo estação (cliente) Wi-Fi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // Inicia a conexão Wi-Fi usando as credenciais definidas em "secrets.h".

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

  Serial.println("Connecting to AWS IoT");
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

// Função responsável por publicar uma mensagem via MQTT em um tópico.
void publishMessage()
{
  StaticJsonDocument<200> doc;  // Cria um objeto StaticJsonDocument para armazenar os dados a serem publicados.
  doc["digital"] = dig;         
  doc["analogico"] = tensao;    
  doc["caracter"] = "Ola";      
  char jsonBuffer[512];         // Cria um buffer de caracteres para armazenar o JSON serializado.

  serializeJson(doc, jsonBuffer); // Converte o objeto JSON em uma representação de caracteres.

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer); // Publica a mensagem no tópico MQTT especificado, usando o JSON serializado como conteúdo da mensagem.
  //client.publish( topico em que a mensagem vai ser publicada , mensagem a ser enviada );
}

// Função de tratamento de mensagens recebidas pelo MQTT.
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  // Exibe o tópico da mensagem recebida.
  Serial.print("incoming: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;          // Cria um objeto StaticJsonDocument para armazenar os dados do JSON recebido.

  deserializeJson(doc, payload);        // Converte o payload (dados da mensagem) em um objeto JSON.

  const char* message = doc["message"]; // Obtém o valor da chave "message" do JSON recebido, doc["chave"]["sub-chave"]["sub-chave"];

  Serial.println(message);              // Exibe o valor da mensagem.
}

void setup()
{
  Serial.begin(115200);
  connectAWS();         // Realiza a conexão com o Wi-Fi e com os serviços AWS.
  pinMode(34, INPUT);
}

void loop()
{
  dig = 10;                       
  anlg = analogRead(34);            
  tensao = (anlg * 3.3) / 4095.0;   
  publishMessage();                 // Publica os valores obtidos via MQTT.
  client.loop();                    // Mantém a conexão MQTT ativa.
  delay(1000);                      
}
