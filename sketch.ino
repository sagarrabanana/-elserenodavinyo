

//https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <ArduinoJson.h>                  // https://github.com/bblanchon/ArduinoJson
#include "secret.h"                       // uncomment if using secret.h file with credentials
//#define TWI_TIMEOUT 3000                  // varies depending on network speed (msec), needs to be before TwitterWebAPI.h
#include <TwitterWebAPI.h>

#ifndef WIFICONFIG
const char* ssid = "mired";           // WiFi SSID
const char* password = "claveFalsa";   // WiFi Password
#endif

//-------------------
//std::string search_str = "#dog";          // Default search word for twitter
const char *ntp_server = "pool.ntp.org";  // time1.google.com, time.nist.gov, pool.ntp.org
int timezone = -5;                        // US Eastern timezone -05:00 HRS
unsigned long twi_update_interval = 10;   // (seconds) minimum 5s (180 API calls/15 min). Any value less than 5 is ignored!
//-------------------

#ifndef TWITTERINFO  // Obtain these by creating an app @ https://apps.twitter.com/
static char const consumer_key[]    = "k1mwVjIdotfeYArastB3Ju1AH";
static char const consumer_sec[]    = "RvMgz6nonoeseltokenrealDaxiychOifCNV58i34UDuueL7963N";
static char const accesstoken[]     = "11655854444tambiendementiravnYo74Di6c3bp1jgoWMDPwp";
static char const accesstoken_sec[] = "rZsdHxlZTymastrolasbwOlxhQkzB";
#endif


//   Dont change anything below this line    //
///////////////////////////////////////////////

unsigned long api_mtbs = twi_update_interval * 1000; //mean time between api requests
unsigned long api_lasttime = 0;
bool twit_update = false;
//std::string search_msg = "No Message Yet!";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntp_server, timezone * 3600, 60000); // NTP server pool, offset (in seconds), update interval (in milliseconds)
TwitterClient tcr(timeClient, consumer_key, consumer_sec, accesstoken, accesstoken_sec);


//-------------- sonido -----------------
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
double maxVal = 0;
float limite=4;

//----------------- frases ---------------------------
String msg1[] = {"Hay un ", "Otro ", "No se quedara mudo el ", "Una pena no tener balcon… un buen macetazo se iba a llevar el ", " No sirvais mas alcohol a este ", "No se si es un apuñalamiento o simplemente un turista ", "A ver esos secretas que andan por el MACBA poniendo multas a la chavaleria, que se pasen por Avinyo, que tenemos un "};
String msg2[] = {" hasta las cejas de orujo cantando debajo de mi casa. ", " desgraciado que tenemos gritando mierdas debajo de casa. ", " que tengo debajo de casa dando voces. ", " cabron que anda gritando en mi ventana. ", " que luego no sabe comportarse y se pone a gritar debajo de mi casa. ", " dando la nota debajo de casa. "," cantando a grito pelado paseandose por aqui. "};
String msg3[] = {"turista intraestatal", "aleman", "austriaco", "belga", "bulgaro", "danes", "finlandes", "frances", "griego", "hungaro", "irlandes", "italiano", "luxemburgues", "holandes", "polaco", "portugues", "ingles", "checo", "rumano", "sueco", "croata", "maltes", "estonio", "esloveno", "eslovaco", "chipriota", "leton", "canadiense", "argentino", "chileno", "uruguayo", "venezolano", "brazileño", "colombiano", "mejicano", "boliviano", "peruano", "islandes", "noruego", "suizo", "turco", "ruso", "andorrano", "japones", "chino", "gringo", "africano", "turista de procedencia desconocida"};
String msg4[] = {"turespana_", "auswaertigesamt", "ch_ebner", "BelgiumMFA", "Si alguien conoce el perfil de este consulado.. Agradeceria el dato", "embajador_dk", "EmbFinMadrid", "france_espagne", "GreeceinSpain", "Si alguien conoce el perfil de este consulado.. Agradeceria el dato", "IrlEmbMadrid", "ItalyinSPA", "Si alguien conoce el perfil de este consulado.. Agradeceria el dato", "NLinSpain", "PlinBarcelona", "Si alguien conoce el perfil de este consulado.. Agradeceria el dato", "ukinspain", "Si alguien conoce el perfil de este consulado.. Agradeceria el dato", "de_rumania", "EmbajadorSuecia", "EmbCroMadrid", "MaltaInSpain", "Si alguien conoce el perfil de este consulado.. Agradeceria el dato", "SLOinESP", "Si alguien conoce el perfil de este consulado.. Agradeceria el dato", "CyprusinSpain", "LVenEspana", "EmbCanEspana", "RGenesp", "EmbaChileEspana", "Si alguien conoce el perfil de este consulado.. Agradeceria el dato", "EmbVZLA_enEsp", "EmbBrasilEspana", "CancilleriaCol", "EmbaMexEsp", "EmbajadaBolEsp", "PeruEnEspana", "Si alguien conoce el perfil de este consulado.. Agradeceria el dato", "NoruegaenEspana", "Si alguien conoce el perfil de este consulado.. Agradeceria el dato", "EmbTurquia", "EmbajadaRusaES", "ANDORRA_Spain", "Consul_JPN_BCN", "ChinaEmbEsp", "USConsulateBarcelona", " ", " "};
String msg5[] = {"BarcelonaTurism", "VisitBCN_ES", "turismoBCN", "turismecat", "barcelonaturis", "barcelona_GUB", "BCN_GUB", "airbnb_es", "Turespana_"};
//--------------------------------------------

int cont = 0; //contador para acomular los picos de ruido en un plazo de tiempo determinado para separar el ruido puntual del continuado.
unsigned long prevTime; //

void setup(void) {
  //Begin Serial
  delay(2000);
  Serial.begin(115200);
  // WiFi Connection
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to ");
  Serial.print(ssid);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Estamos dentro");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  delay(100);
  // Connect to NTP and force-update time
  tcr.startNTP();
  Serial.println("NTP Synced");
  delay(100);
  if (twi_update_interval < 5) api_mtbs = 5000; // Cant update faster than 5s.

  randomSeed(analogRead(0));
}

void loop(void) {


  //------------------- esta parte se encarga de medir la potencia del sonido
  unsigned long startMillis = millis(); // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(0);
    if (sample < 1024)  // toss out spurious readings
    {
      if (sample > signalMax)
      {
        signalMax = sample;  // save just the max levels
      }
      else if (sample < signalMin)
      {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  double volts = (peakToPeak * 5.0) / 1024;  // convert to volts
//---------------------------------------------------

//-------------------------
  if (volts > maxVal)
  {
    maxVal = volts;
  }

  Serial.print("volts: ");
  Serial.println(volts);
  Serial.print("maxVal: ");
  Serial.println(maxVal);

  if(volts>limite)
  {
  Serial.print("dia: ");
  Serial.println(timeClient.getDay());
  Serial.print("hora: ");
  Serial.println(timeClient.getHours() + 6);   
  }

  if ((volts > limite && (timeClient.getDay() <= 4 && ((timeClient.getHours() + 6) >= 1 || (timeClient.getHours() + 6) < 20 )) || (volts > limite && timeClient.getDay() > 4 && ((timeClient.getHours() + 6) >= 1 || (timeClient.getHours() + 6) < 20 ))))
  {
    Serial.print("Cont: ");
    Serial.println(cont);
    if (cont == 0)
  {
    prevTime = millis();
    }
    cont++;
  }
  if ((cont >= 35) && ((millis() - prevTime) <= 60000))
  {
    Serial.println("-------------- ANORMAL DETECTADO ----------");
    delay(50);

    int rz = (int)randomizer();
    int r = (int)random(0, 7);
    Serial.print("rz: ");
    Serial.println(rz);
    Serial.print("r: ");
    Serial.println(r);
    String texto = msg1[r] + msg3[rz] + msg2[r] + "@"+msg4[rz] + " @barcelona_GUB";
    Serial.print("texto: ");
    Serial.println(texto);
    tuitea(texto);
    cont = 0;
    maxVal=0;
  }
  else if ((millis() - prevTime) > 60000)
  {
    cont = 0;
  }

}

void tuitea(String a) //envia el tuit
{
  Serial.println("arranca tuiteo");
  delay(200);
  //String text = "se me ve";
  //if (millis() > api_lasttime + api_mtbs)  {
  std::string twitter_post_msg;
  // LA cLAVE https://www.arduino.cc/reference/en/language/variables/data-types/string/functions/c_str/
  //float t = rand();
  tcr.tweet(a.c_str());
  Serial.print("Search: ");
  Serial.println("parace q hay tweet");
  //  api_lasttime = millis();
  // }

}
int randomizer() //devuelve el nombre de un pais de manera aleatoria pero manteniendo una proporcion definida a partir de datos estadisticos de turismo https://ajuntament.barcelona.cat/barcelonaeconomia/ca/turisme-fires-i-congressos/activitat-turistica/turistes-segons-procedencia
{
  int i;
  long rn = random(1000);
  if (rn >= 0 and rn < 187) i = 0;          //españa
  else if (rn >= 187 and rn < 240) i = 1;   //alemania
  else if (rn >= 240 and rn < 248) i = 2;   //austria
  else if (rn >= 248 and rn < 264) i = 3;   //belgica
  else if (rn >= 264 and rn < 266) i = 4;   //bulgaria
  else if (rn >= 266 and rn < 275) i = 5;   //dinamarca
  else if (rn >= 275 and rn < 281) i = 6;   //finlandia
  else if (rn >= 281 and rn < 362) i = 7;   //francia
  else if (rn >= 362 and rn < 367) i = 8;   //grecia
  else if (rn >= 367 and rn < 370) i = 9;   //hungria
  else if (rn >= 370 and rn < 379) i = 10;   //irlanda
  else if (rn >= 379 and rn < 442) i = 11;   //italia
  else if (rn >= 442 and rn < 443) i = 12;   //luxemburgo
  else if (rn >= 443 and rn < 467) i = 13;   //holanda
  else if (rn >= 467 and rn < 476) i = 14;   //polonoa
  else if (rn >= 476 and rn < 490) i = 15;   //portugal
  else if (rn >= 490 and rn < 574) i = 16;   //UK
  else if (rn >= 574 and rn < 577) i = 17;   //R.Checa
  else if (rn >= 577 and rn < 582) i = 18;   //rumenia
  else if (rn >= 582 and rn < 596) i = 19;   //suecia
  else if (rn >= 596 and rn < 599) i = 20;   //croatia
  else if (rn >= 599 and rn < 600) i = 21;   //malta
  else if (rn >= 600 and rn < 601) i = 22;   //estonia
  else if (rn >= 601 and rn < 602) i = 23;   //eslovebia
  else if (rn >= 602 and rn < 604) i = 24;   //eslovaquia
  else if (rn >= 604 and rn < 605) i = 25;   //chipre
  else if (rn >= 605 and rn < 606) i = 26;   //letonia
  else if (rn >= 606 and rn < 618) i = 27;   //canada
  else if (rn >= 618 and rn < 635) i = 28;   //argentina
  else if (rn >= 635 and rn < 646) i = 29;   //chile
  else if (rn >= 646 and rn < 648) i = 30;   //uruguay
  else if (rn >= 648 and rn < 653) i = 31;   //venezuela
  else if (rn >= 653 and rn < 666) i = 32;   //brazil
  else if (rn >= 666 and rn < 669) i = 33;   //colombia
  else if (rn >= 669 and rn < 677) i = 34;   //mexico
  else if (rn >= 677 and rn < 680) i = 35;   //bolivia
  else if (rn >= 680 and rn < 682) i = 36;   //peru
  else if (rn >= 682 and rn < 684) i = 37;   //islandia
  else if (rn >= 684 and rn < 692) i = 38;   //noruega
  else if (rn >= 692 and rn < 701) i = 39;   //siuza
  else if (rn >= 701 and rn < 710) i = 40;   //turquia
  else if (rn >= 710 and rn < 748) i = 41;   //rusia
  else if (rn >= 748 and rn < 749) i = 42;   //andorra
  else if (rn >= 749 and rn < 768) i = 43;   //japon
  else if (rn >= 768 and rn < 786) i = 44;   //china
  else if (rn >= 786 and rn < 892) i = 45;   //USA
  else if (rn >= 892 and rn < 907) i = 46; //africa
  else i = 47; //resto mundo
  return i;
}
