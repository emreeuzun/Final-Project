#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

LiquidCrystal_I2C lcd(0x27,16,2);

#define RXD2 16  // ESP32'nin GPIO16 pinini RX olarak kullanıyoruz
#define TXD2 17  // ESP32'nin GPIO17 pinini TX olarak kullanıyoruz

#define M0  5   // M0 GPIO5'e bağlı
#define M1  18  // M1 GPIO18'e bağlı

const char* ssid = "Galaxy A21sE088";       // WiFi Ağ Adı
const char* password = "antalya123456"; // WiFi Şifresi
const char* thingSpeakURL = "http://api.thingspeak.com/update?api_key=DCA93XKSSTY3LEJX";

typedef struct{
	  float cur;
	  float temp;
	  float nem;
	  float volt;
}DataPacket;

 DataPacket receivedData;


float swapFloat(float value) {
  uint32_t *ptr = (uint32_t*)&value;
  *ptr = __builtin_bswap32(*ptr);
  return value;
}

void clearSerialBuffer() {
    while (Serial2.available()) {
        Serial2.read();  // Buffer içindeki tüm verileri temizle
    }
}

void setup() {
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  digitalWrite(M0, LOW); // M0 başlangıçta LOW
  digitalWrite(M1, LOW); // M1 başlangıçta LOW
  
  Serial.begin(9600);       // Seri monitör için baud rate
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  // UART2 (Bağlantı: RX = 16, TX = 17)
  lcd.begin();
  lcd.display();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  receivedData.cur = 0.0;
  receivedData.temp = 0.0;
  receivedData.nem = 0.0;
  receivedData.volt = 0.0;

}

void loop() {
if (Serial2.available() >= sizeof(DataPacket)+2) {
        
        uint8_t startByte;
        Serial2.readBytes(&startByte, 1);
        if (startByte != 0xAA) { // Başlangıç bayrağı yanlışsa paketi atla
            Serial.println("Hatalı veri paketi, baştan alınıyor...");
            clearSerialBuffer();
            return;
        }

        uint8_t rawData[sizeof(DataPacket)];
        Serial2.readBytes(rawData, sizeof(DataPacket));

        uint8_t endByte;
        Serial2.readBytes(&endByte, 1);
        if (endByte != 0xBB) { // Bitiş bayrağı yanlışsa paketi atla
            Serial.println("Hatalı veri paketi, baştan alınıyor...");
            clearSerialBuffer();
            return;
        }


        // Struct'a kopyala
        memcpy(&receivedData, rawData, sizeof(DataPacket));

       // **Gelen Ham Veriyi HEX Formatında Yazdır**
        Serial.print("Gelen Ham Veri: ");
        Serial.printf("%02X ", startByte); // Start byte yazdır
        for (int i = 0; i < sizeof(DataPacket); i++) {
            Serial.printf("%02X ", rawData[i]); // DataPacket HEX formatında yazdır
        }
        Serial.printf("%02X\n", endByte); // End byte yazdır

        

        // Veri kontrolü: Negatif veya "ovf" olan değerleri kontrol et
        if (receivedData.cur < -1000 || receivedData.cur > 1000) {
            Serial.println("Akım hatası: Overflow veya geçersiz değer.");
            receivedData.cur = 0;  // Hata durumunda sıfırla
        }
        if (receivedData.nem < 0 || receivedData.nem > 100) {
            Serial.println("Nem hatası: Overflow veya geçersiz değer.");
            receivedData.nem = 0;  // Hata durumunda sıfırla
        }

        // Veriyi ekrana yazdır
        Serial.println("Veri Alındı:");
        Serial.print("Akım: "); Serial.println(receivedData.cur, 2);
        Serial.print("Sıcaklık: "); Serial.println(receivedData.temp, 2);
        Serial.print("Nem: "); Serial.println(receivedData.nem, 2);
        Serial.print("Voltaj: "); Serial.println(receivedData.volt, 2);

        lcd.clear();

        // İlk Satır: Akım ve Sıcaklık
        lcd.setCursor(0, 0);
        lcd.print("A: "); lcd.print(receivedData.cur, 2);
        lcd.print(" T: "); lcd.print(receivedData.temp, 2);

        // İkinci Satır: Nem ve Voltaj
        lcd.setCursor(0, 1);
        lcd.print("N: "); lcd.print(receivedData.nem, 2);
        lcd.print(" V:"); lcd.print(receivedData.volt, 2);

        sendToThingSpeak(receivedData.temp);
    }


 delay(2000);

}


void sendToThingSpeak(float temperature) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String url = String(thingSpeakURL) + "&field1=" + String(temperature, 2); 
    
    http.begin(url);
    http.GET();
    
    http.end();
  }
}