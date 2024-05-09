#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define EKRAN_GENISLIK 128
#define EKRAN_YUKSEKLIK 64

#define OLED_SIFIRLA    -1
Adafruit_SSD1306 display(EKRAN_GENISLIK, EKRAN_YUKSEKLIK, &Wire, OLED_SIFIRLA);

#define YUKARI_BUTON 0
#define ASAGI_BUTON 1
#define SECIM_BUTON 2

#define PALET_GENISLIK 30
#define PALET_YUKSEKLIK 4
#define PALET_Y EKRAN_YUKSEKLIK - 2

#define TOP_YARICAP 3
#define TUGLA_GENISLIK 25
#define TUGLA_YUKSEKLIK 8
#define TUGLA_SAYISI 2

#define CAN_SAYISI 6 

#define POT_PIN A0

struct Tugla {
  int x;
  int y;
};

Tugla tuglalar[TUGLA_SAYISI];

int secimPozisyonu = 0; // Seçim yapılacak konumu belirtmek için
bool secildi = false; // Seçim yapıldı mı?
bool oyunBasladi = false; // Oyun başladı mı?

int paletX = (EKRAN_GENISLIK - PALET_GENISLIK) / 2;
int topX = EKRAN_GENISLIK / 2;
int topY = EKRAN_YUKSEKLIK / 2;
int topHizX = 2;
int topHizY = -2;

int canSayisi = CAN_SAYISI;
int kirilanTuglaSayisi = 0;
int toplamSkor = 0;

int potDeger; // Potansiyometre değeri
int paletHizi = 5; // Palet hareket hızı

int EP=22;
int C=23;
int D=24;
int E=25;
int G=26;
int F=27;
int A=28;
int B=29;

bool topHizDegisti = false;


void setup() {
  pinMode(YUKARI_BUTON, INPUT_PULLUP);
  pinMode(ASAGI_BUTON, INPUT_PULLUP);
  pinMode(SECIM_BUTON, INPUT_PULLUP);

  pinMode(E, OUTPUT); 
  pinMode(D, OUTPUT); 
  pinMode(C, OUTPUT);
  pinMode(A, OUTPUT); 
  pinMode(B, OUTPUT);
  pinMode(F, OUTPUT); 
  pinMode(G, OUTPUT); 
  pinMode(EP, OUTPUT); 

  pinMode(40, OUTPUT); // 1. LED
  pinMode(41, OUTPUT); // 2. LED
  pinMode(42, OUTPUT); // 3. LED

  Serial.begin(9600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);  // 2 saniye bekleyin
  
  // Başlangıç ekranını göster
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10,10);
  display.println("HOS GELDINIZ");
  display.setCursor(10,25);
  display.println(">Baslat");
  display.setCursor(10,40);
  display.println("Cikis");
  display.display();

  randomSeed(analogRead(0)); // Rastgele sayı üretmek için seed belirle
  tuglaKonumlariniAyarla();
}

void loop() {
  // Potansiyometre değerini oku
  potDeger = analogRead(POT_PIN);

  if (!oyunBasladi) {
    menuKontrol();
  } else {
    // Potansiyometrenin değerini paletin konumuna dönüştürerek paletin hareket etmesini sağla
    paletX = map(potDeger, 0, 1023, 0, EKRAN_GENISLIK - PALET_GENISLIK);
    oyunKontrol();
    
    // Potansiyometre değerini güncellediğimizde paletin konumunu da güncellememiz gerekiyor
    // Bu nedenle paletin çizimini burada çağırıyoruz
    paletCiz();
  }

  Sayilar(kirilanTuglaSayisi); 
  isikSensoru();

}

void menuKontrol() {
  if (digitalRead(YUKARI_BUTON) == LOW) {
    if (secimPozisyonu > 0) {
      secimPozisyonu--;
      secimGuncelle();
    }
    delay(200); // Buton tepkime süresi
  }

  if (digitalRead(ASAGI_BUTON) == LOW) {
    if (secimPozisyonu < 1) {
      secimPozisyonu++;
      secimGuncelle();
    }
    delay(200); // Buton tepkime süresi
  }

  if (digitalRead(SECIM_BUTON) == LOW) {
    secildi = true;
  }

  if (secildi) {
    if (secimPozisyonu == 0) {
      // Başlat seçeneği seçildi
      oyunuBaslat();
    } else if (secimPozisyonu == 1) {
       // Çıkış seçeneği seçildi
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(10,10);
      display.println("Oyunumuza \n gosterdiginiz \n ilgi icin \n tesekkurler");
      display.display();
      while(true);
    }
  }
}

void secimGuncelle() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10,10);
  display.println("HOS GELDINIZ");
  display.setCursor(10,25);
  if (secimPozisyonu == 0) {
    display.println("> Baslat");
    display.setCursor(10,40);
    display.println("Cikis");
  } else if (secimPozisyonu == 1) {
    display.println("Baslat");
    display.setCursor(10,40);
    display.println("> Cikis");
  }
  display.display();
}

void oyunuBaslat() {
  oyunBasladi = true;
  display.clearDisplay();

  // Topu paletin üstüne yerleştir
  topX = paletX + PALET_GENISLIK / 2;
  topY = PALET_Y - TOP_YARICAP;

  paletCiz();
  topCiz();
  display.display();

}

void oyunKontrol() {
  // Topu hareket ettir
  topHareket();

  // Yeniden çizim yap
  display.clearDisplay();
  paletCiz();
  topCiz();
  tuglalariCiz();
  display.display();

  // Tuğla-top çarpışmalarını kontrol et
  topTuglaCarpismalariKontrolEt();
  topSinirlarlaCarpistiMi();
  kontrolEtOyunBittiMi();
  ledler(canSayisi);
  
  // Gecikme ekle
  delay(20);
}

void topHareket() {
  // Topun yeni konumunu hesapla
  topX += topHizX;
  topY += topHizY;

  // Topun ekran sınırlarını kontrol et
  if (topX >= EKRAN_GENISLIK - TOP_YARICAP || topX <= TOP_YARICAP) {
    topHizX = -topHizX;
  }
  if (topY <= TOP_YARICAP) {
    topHizY = -topHizY;
  }

  // Paletin topu yansıtıp yansıtmadığını kontrol et
  if (topY >= PALET_Y - TOP_YARICAP && topX >= paletX && topX <= paletX + PALET_GENISLIK) {
    topHizY = -topHizY;
    // Topun yukarı sekip sekmemesi için hızını negatif yap
    topHizY = -abs(topHizY);
  }

  // Eğer top ekranın altına giderse
  /*if (topY >= EKRAN_YUKSEKLIK - TOP_YARICAP) {
    // Can sayısını azalt
    canSayisi--;
    // Topu paletin üstüne yerleştir
    topX = paletX + PALET_GENISLIK / 2;
    topY = PALET_Y - TOP_YARICAP;
    
    // Eğer canlar tükendi ise oyunu sonlandır
    if (canSayisi <= 0) {
      oyunuBitir();
    }
  }*/
}

void paletCiz() {
  // Paletin çizilmesi
  display.fillRect(paletX, PALET_Y, PALET_GENISLIK, PALET_YUKSEKLIK, SSD1306_WHITE);
  
  // Eğer oyun başladıysa ve palet hareket ediyorsa topun konumunu güncelle
  if (oyunBasladi && (digitalRead(YUKARI_BUTON) == LOW || digitalRead(ASAGI_BUTON) == LOW)) {
    // Topu paletin ortasına yerleştir
    topX = paletX + PALET_GENISLIK / 2;
  }
}

void topCiz() {
  display.fillCircle(topX, topY, TOP_YARICAP, SSD1306_WHITE);
}

void tuglaKonumlariniAyarla() {
  int tugla_sirasi = 0;
  int toplam_sira = 3; // Toplam sıra sayısı
  int sutun_sayisi = 5; // Her sırada 10 tuğla
  int ara_bosluk = 2; // Sıra arasındaki boşluk
  
  // Bir tuğla ile bir boşluk arasındaki toplam alan
  int total_width = sutun_sayisi * TUGLA_GENISLIK + (sutun_sayisi - 1) * 2;
  
  // Başlangıçta x ve y koordinatları
  int start_x = (EKRAN_GENISLIK - total_width) / 2;
  int start_y = 0;
  
  for (int i = 0; i < toplam_sira; i++) {
    for (int j = 0; j < TUGLA_YUKSEKLIK; j++) {
      for (int k = 0; k < sutun_sayisi; k++) {
        if (tugla_sirasi >= TUGLA_SAYISI) {
          // Tuğla sayısı kadar tuğla yerleştirildiyse döngüden çık
          return;
        }
        tuglalar[tugla_sirasi].x = start_x + k * (TUGLA_GENISLIK + 2); // Sütunlar arasına boşluk ekleniyor
        tuglalar[tugla_sirasi].y = start_y + (i * (TUGLA_YUKSEKLIK + ara_bosluk +10)) + j * (TUGLA_YUKSEKLIK); // Sıra arasına boşluk ekleniyor
        tugla_sirasi++;
      }
    }
  }
}

void tuglalariCiz() {
  for (int i = 0; i < TUGLA_SAYISI; i++) {
    display.fillRect(tuglalar[i].x, tuglalar[i].y, TUGLA_GENISLIK, TUGLA_YUKSEKLIK, SSD1306_WHITE);
  }
}

void topTuglaCarpismalariKontrolEt() {
  for (int i = 0; i < TUGLA_SAYISI; i++) {
    // Tuğlanın çarpışma kutusu
    int tuglaSol = tuglalar[i].x;
    int tuglaSag = tuglalar[i].x + TUGLA_GENISLIK;
    int tuglaUst = tuglalar[i].y;
    int tuglaAlt = tuglalar[i].y + TUGLA_YUKSEKLIK;
    
    // Topun çarpışma kutusu
    int topSol = topX - TOP_YARICAP;
    int topSag = topX + TOP_YARICAP;
    int topUst = topY - TOP_YARICAP;
    int topAlt = topY + TOP_YARICAP;
    
    // Çarpışma kontrolü
    if (topSag >= tuglaSol && topSol <= tuglaSag && topAlt >= tuglaUst && topUst <= tuglaAlt) {
      // Top, bir tuğla ile çarpıştı
      // Tuğlayı sil
      tuglalar[i].x = -TUGLA_GENISLIK; // Tuğlayı ekranda gösterilmemesi için geçersiz bir konumla değiştirin
      tuglalar[i].y = -TUGLA_YUKSEKLIK;
      
      // Topun yönünü değiştirme (isteğe bağlı)
      topHizY = -topHizY;

      kirilanTuglaSayisi++;
      
      // Eğer tüm tuğlalar yok edildiyse oyunu bitirme veya bir sonraki seviyeye geçme işlemi buraya eklenir
      
      break; // Birden fazla tuğla ile çarpışma olmayacağı için döngüyü sonlandır
    }
  }
}

void topSinirlarlaCarpistiMi() {
  // Eğer top ekranın altına giderse
  if (topY >= EKRAN_YUKSEKLIK - TOP_YARICAP) {
    // Can sayısını azalt
    canSayisi--;

    // Topu paletin üstüne yerleştir
    topX = paletX + PALET_GENISLIK / 2;
    topY = PALET_Y - TOP_YARICAP;
    
    // Eğer canlar tükendi ise oyunu sonlandır
    if (canSayisi <= 0) {
      oyunuBitir();
    }
  }
}

void kontrolEtOyunBittiMi() {
  if (kirilanTuglaSayisi == TUGLA_SAYISI) {
    yeniBolum(); // Yeni bölüm başl
  }
}


void ledler(int can) {
  if (can == 6) {
    digitalWrite(40, HIGH);
    digitalWrite(41, HIGH);
    digitalWrite(42, HIGH);
  } else if (can == 4) {
    digitalWrite(40, HIGH);
    digitalWrite(41, HIGH);
    digitalWrite(42, LOW);
  } else if (can == 2) {
    digitalWrite(40, HIGH);
    digitalWrite(41, LOW);
    digitalWrite(42, LOW);
  } else if (can == 0) {
    digitalWrite(40, LOW);
    digitalWrite(41, LOW);
    digitalWrite(42, LOW);
  }
}

void Sayilar(int rakam)
{
   switch(rakam)
   {
   case 0 :  // 0 yazmak için pin durumları 
     digitalWrite(E,LOW);
     digitalWrite(D,LOW); 
     digitalWrite(C,LOW); 
     digitalWrite(B,LOW); 
     digitalWrite(A,LOW); 
     digitalWrite(F,LOW); 
     digitalWrite(G,HIGH); 
   break;
   case 1:  // 1 yazmak için pin durumları 
     digitalWrite(E,HIGH);
     digitalWrite(D,HIGH); 
     digitalWrite(C,LOW); 
     digitalWrite(B,LOW); 
     digitalWrite(A,HIGH); 
     digitalWrite(F,HIGH); 
     digitalWrite(G,HIGH); 
   break;
   case 2:  // 2 yazmak için pin durumları 
     digitalWrite(E,LOW);
     digitalWrite(D,LOW); 
     digitalWrite(C,HIGH); 
     digitalWrite(B,LOW); 
     digitalWrite(A,LOW); 
     digitalWrite(F,HIGH); 
     digitalWrite(G,LOW); 
   break;
   case 3:  // 3 yazmak için pin durumları 
     digitalWrite(E,HIGH);
     digitalWrite(D,LOW); 
     digitalWrite(C,LOW); 
     digitalWrite(B,LOW); 
     digitalWrite(A,LOW); 
     digitalWrite(F,HIGH); 
     digitalWrite(G,LOW); 
   break;
   case 4:  // 4 yazmak için pin durumları 
     digitalWrite(E,HIGH);
     digitalWrite(D,HIGH); 
     digitalWrite(C,LOW); 
     digitalWrite(B,LOW); 
     digitalWrite(A,HIGH); 
     digitalWrite(F,LOW); 
     digitalWrite(G,LOW); 
   break;
   case 5:  // 5 yazmak için pin durumları 
     digitalWrite(E,HIGH);
     digitalWrite(D,LOW); 
     digitalWrite(C,LOW); 
     digitalWrite(B,HIGH); 
     digitalWrite(A,LOW); 
     digitalWrite(F,LOW); 
     digitalWrite(G,LOW); 
   break;
   case 6:  // 6 yazmak için pin durumları 
     digitalWrite(E,LOW);
     digitalWrite(D,LOW); 
     digitalWrite(C,LOW); 
     digitalWrite(B,HIGH); 
     digitalWrite(A,LOW); 
     digitalWrite(F,LOW); 
     digitalWrite(G,LOW); 
   break;
   case 7:  // 7 yazmak için pin durumları 
     digitalWrite(E,HIGH);
     digitalWrite(D,HIGH); 
     digitalWrite(C,LOW); 
     digitalWrite(B,LOW); 
     digitalWrite(A,LOW); 
     digitalWrite(F,HIGH); 
     digitalWrite(G,HIGH); 
   break;
   case 8:  // 8 yazmak için pin durumları 
     digitalWrite(E,LOW);
     digitalWrite(D,LOW); 
     digitalWrite(C,LOW); 
     digitalWrite(B,LOW); 
     digitalWrite(A,LOW); 
     digitalWrite(F,LOW); 
     digitalWrite(G,LOW); 
   break;
   case 9:  // 9 yazmak için pin durumları 
     digitalWrite(E,HIGH);
     digitalWrite(D,LOW); 
     digitalWrite(C,LOW); 
     digitalWrite(B,LOW); 
     digitalWrite(A,LOW); 
     digitalWrite(F,LOW); 
     digitalWrite(G,LOW); 
   break;
   }
}

void isikSensoru() {
  int analogDeger = analogRead(A1);
  float voltaj = analogDeger / 1024. * 5;
  float resistance = 2000 * voltaj / (1 - voltaj / 5);
  float lux = pow(50 * 1e3 * pow(10, 0.7) / resistance, (1 / 0.7));
  
  if (lux > 512)
    display.invertDisplay(true);
  else
    display.invertDisplay(false);
}


void oyunuBitir() {
  // Skoru ekrana yazdır
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10,10);
  display.print("Skor: ");
  display.println(kirilanTuglaSayisi);
  display.display();
  
  // 3 saniye bekle
  delay(3000);
  
  // Ana menüyü göster
  oyunBasladi = false;
  secimPozisyonu = 0;
  secildi = false;
  display.clearDisplay();
  secimGuncelle();
  
   // Oyunu başlat
  oyunBasladi = true;
  kirilanTuglaSayisi = 0; // Skoru sıfırla
  tuglaKonumlariniAyarla(); // Tuğlaları yeniden yerleştir
  oyunuBaslat();
}

void yeniBolum() {
  // 5 saniye bekle
  delay(5000);

  // Yeni bölüm başlat
  oyunBasladi = true;
  tuglaKonumlariniAyarla(); // Tuğlaları yeniden yerleştir
  topHizX *= 1.2; // Top hızını %20 artır
  topHizY *= 1.2; // Top hızını %20 artır
  kirilanTuglaSayisi = 0; // Skoru sıfırla
  oyunuBaslat();
}