#include <pgmspace.h>
 
#define SECRET
#define THINGNAME  "*****"                   //Nome do seu Dispositivo 
const char WIFI_SSID[] = "*****";              //Nome da sua Rede Wifi 
const char WIFI_PASSWORD[] = "*****";       //Senha do seu Wifi 
const char AWS_IOT_ENDPOINT[] = "*****";    //Seu EndPoint 

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(*****)EOF";            

 
// Device Certificate                                          
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(*****)KEY";
 
// Device Private Key                                             
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(*****)KEY";