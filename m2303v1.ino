#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

//-------- порты для rs 485
#define SSerialRx        9  // Serial Receive pin RO
#define SSerialTx        8   // Serial Transmit pin DI
//-------- инициализация
SoftwareSerial RS485Serial(SSerialRx, SSerialTx); // Rx, Tx

//// линия управления передачи приема
#define SerialControl 5   // RS485 Direction control
/////// флаг приема передачи
#define RS485Transmit    HIGH
#define RS485Receive     LOW

const char chipSelect = 4; // для SD карты

const PROGMEM unsigned int crcTable[] = { // таблицу загоняем на флешку для экономии места
  0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
  0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
  0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
  0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
  0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
  0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
  0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
  0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
  0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
  0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
  0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
  0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
  0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
  0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
  0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
  0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
  0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
  0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
  0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
  0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
  0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
  0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
  0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
  0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
  0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
  0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
  0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
  0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
  0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
  0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
  0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
  0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};
/////// команды
byte testConnect[] = { 0x00, 0x00 };
byte Access[]      = { 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
byte Sn[]          = { 0x00, 0x08, 0x00 }; // серийный номер
byte Time[]        = { 0x00, 0x04, 0x00 }; // время дата
//byte Freq[]        = { 0x00, 0x08, 0x16, 0x40 }; // частота
byte Current[]     = { 0x00, 0x08, 0x16, 0x21 };//  ток
byte Suply[]       = { 0x00, 0x08, 0x16, 0x11 }; // напряжение
byte Power[]       = { 0x00, 0x08, 0x16, 0x00 };// мощность p
//byte PowerQ[]       = { 0x00, 0x08, 0x16, 0x08 };// мощность Q
//byte PowerS[]       = { 0x00, 0x08, 0x16, 0x04 };// мощность S

//byte CosF[]       = { 0x00, 0x08, 0x16, 0x30 };// cosf

//byte Angle[]       = { 0x00, 0x08, 0x16, 0x51 }; // углы
byte energyT0[]  =   { 0x00, 0x05, 0x00, 0x00 };///  суммарная энергия прямая + обратная + активная + реактивная
//byte energyT1[]  =   { 0x00, 0x05, 0x00, 0x01 };///  суммарная энергия прямая + обратная + активная + реактивная
//byte energyT2[]  =   { 0x00, 0x05, 0x00, 0x02 };///  суммарная энергия прямая + обратная + активная + реактивная
//byte energyT3[]  =   { 0x00, 0x05, 0x00, 0x03 };///  суммарная энергия прямая + обратная + активная + реактивная
//byte energyT4[]  =   { 0x00, 0x05, 0x00, 0x04 };///  суммарная энергия прямая + обратная + активная + реактивная

byte response[19];
byte byteReceived;
byte byteSend;
byte netAdr;
char buff[3];

uint32_t myTimer; //переменная хранения времени
uint32_t myTimerC1; //переменная для коррекции
long period = 60000; //задержка на минуту
int correction = 0; //коррекция на опрос счетчика

byte res;

void(* resetFunc) (void) = 0; // объявляем функцию reset

void setup() {
  RS485Serial.begin(9600);
  Serial.begin(9600);
  // 5 пин в режим выхода
  pinMode(SerialControl, OUTPUT);
  pinMode(3, OUTPUT); // зеленый светодиод
  pinMode(2, OUTPUT); // красный светодиод
  // ставим на прием
  digitalWrite(SerialControl, RS485Receive);
  delay(300);
  Serial.println(F("Start_v2303v1\r\n"));

  Serial.print(F("Initializing SD card..."));

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Card failed, or not present"));
    blinkRed(200); // здемь мигнуть красным два раза
    delay(200);
    blinkRed(200);
    Serial.println(F("Resetting"));
    delay(1000);
    resetFunc(); // нужен перезапуск МК при неудачной попытке инициализации
  }
  else {
    Serial.println(F("card initialized."));
    blinkGreen(1000);

    File dataFile = SD.open("netAdr.txt"); // читаем если есть сетевой адрес из файла

    // if the file is available, write to it:
    if (dataFile) {
      byte i = 0;
      while (dataFile.available()) {
        if (i == 3) {
          break;
        }

        byteReceived = dataFile.read();   // Read received byte

        buff[i] = byteReceived; // помещаем прочтенные байты в char массив

        i++;
      }
      netAdr = atoi(buff); // конвертируем массив из ascii в int
      dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else {
      netAdr = 0;
    }
  }
}

void loop() {

  if (millis() - myTimer >= period - correction) {
    myTimer += period;

    myTimerC1 = millis(); //время на начало опроса

    testConnect[0] = netAdr;
    response[0] = 0;
    send(testConnect, sizeof(testConnect), response);
    if (response[0] == netAdr)
    {
      Serial.print(F("Connect_OK\r\n"));
      blinkGreen(300);

      Access[0] = netAdr;
      response[0] = 0;
      send(Access, sizeof(Access), response);
      if (response[0] == netAdr)
      {
        Serial.print(F("Access_OK\r\n"));
        blinkGreen(300);

        String serNum = getSerialNumber(netAdr);
        Serial.print("s:" + serNum + "\r\n");
        String valTime = getTime(netAdr);
        Serial.print("t:" + valTime + " " + valTime.substring(4, 5) + "\r\n"); // для тестов выводим разряд минут
        String valDate = getDate(netAdr);
        Serial.print("d:" + valDate + "\r\n");
        String ARPower = getEnergyT0(netAdr);
        Serial.print("p:" + ARPower + "\r\n");
        //String valFreq = getFreq(netAdr);
        //Serial.print("f:"+ valFreq+"\r\n");
        String U = getSuply(netAdr);
        Serial.print("u:" + U + "\r\n");
        String A = getCurrent(netAdr);
        Serial.print("a:" + A + "\r\n");
        //String Angle = getAngle(netAdr);
        //Serial.print("g:"+ Angle+"\r\n");
        String PowerNow = getPowerNow(netAdr);
        Serial.print("e:" + PowerNow + "\r\n");
        //String Tarif1 = getEnergyT1(netAdr);
        // Serial.print("t1:"+ Tarif1+"\r\n");
        //String Tarif2 = getEnergyT2(netAdr);
        //Serial.print("t2:"+ Tarif2+"\r\n");
        //String Tarif3 = getEnergyT3(netAdr);
        //Serial.print("t3:"+ Tarif3+"\r\n");
        //String Tarif4 = getEnergyT4(netAdr);
        //Serial.print("t4:"+ Tarif4+"\r\n");
        //String PQ = getPowerQ(netAdr);
        //Serial.print("q:"+ PQ+"\r\n");
        //String PS = getPowerS(netAdr);
        //Serial.print("c:"+ PS+"\r\n");
        //String CSF = getCosF(netAdr);
        //Serial.print("k:"+ CSF+"\r\n");

        File dataFile = SD.open("datalog.txt", FILE_WRITE);

        if (dataFile) {
          dataFile.println(serNum + " " + valDate + " " + valTime + " " + U + " " + A + " " + PowerNow + " " + ARPower);
          dataFile.close();
          Serial.println(F("Write OK")); // мигнуть красным долго
          blinkRed(1000);

          correction = millis() - myTimerC1; // высчитываем коррекцию после записи
          Serial.println("Correction time: " + String(correction) + "\r\n");
        }
        // if the file isn't open, pop up an error:
        else {
          Serial.println(F("error opening datalog.txt"));
          blinkRed(200); // здемь мигнуть красным два раза
          delay(200);
          blinkRed(200);
          Serial.println(F("Resetting"));
          delay(1000);
          resetFunc(); // нужен перезапуск МК при неудачной попытке записи
        }
        dataFile.close();


      }
      else
      {
        Serial.print(F("Access_FAIL\r\n"));
        blinkRed(300);
      }
    }
    else
    {
      Serial.print(F("Connect_FAIL\r\n"));
      blinkRed(300);
      res++;
      if (res == 3)
      {
        delay(200);
        blinkRed(200); // здемь мигнуть красным два раза
        delay(200);
        blinkRed(200);
        Serial.println(F("Resetting"));
        delay(1000);
        resetFunc();
      }
    }
  }
  else
  {
    Serial.println(millis() - myTimer);
    blinkGreen(300);
    delay(200);
  }
}

void blinkGreen(int del)
{
  digitalWrite(2, HIGH);
  delay(del);
  digitalWrite(2, LOW);
}

void blinkRed(int del)
{
  digitalWrite(3, HIGH);
  delay(del);
  digitalWrite(3, LOW);
}

String getSerialNumber(int netAdr)
{
  String s1, s2, s3, s4;
  response[0] = 0;
  Sn[0] = netAdr;
  send(Sn, sizeof(Sn), response);
  if ((int)response[1] < 10) {
    s1 = "0" + String((int)response[1]);
  } else {
    s1 = String((int)response[1]);
  }
  if ((int)response[2] < 10) {
    s2 = "0" + String((int)response[2]);
  } else {
    s2 = String((int)response[2]);
  }
  if ((int)response[3] < 10) {
    s3 = "0" + String((int)response[3]);
  } else {
    s3 = String((int)response[3]);
  }
  if ((int)response[4] < 10) {
    s4 = "0" + String((int)response[4]);
  } else {
    s4 = String((int)response[4]);
  }
  String n = s1 + s2 + s3 + s4;

  return n;
}

String getTime(int netAdr)
{
  String ti1, ti2, ti3;

  response[0] = 0;
  Time[0] = netAdr;
  send(Time, sizeof(Time), response);
  if ((int)response[1] < 10) {
    ti1 = "0" + String((int)response[1]);
  } else {
    ti1 = String(response[1] / 16) + String(response[1] % 16);
  }
  if ((int)response[2] < 10) {
    ti2 = "0" + String((int)response[2]);
  } else {
    ti2 = String(response[2] / 16) + String(response[2] % 16);
  }
  if ((int)response[3] < 10) {
    ti3 = "0" + String((int)response[3]);
  } else {
    ti3 = String(response[3] / 16) + String(response[3] % 16);
  }

  String n = ti3 + ":" + ti2 + ":" + ti1;

  return n;
}

String getDate(int netAdr)
{
  String ti5, ti6, ti7;

  if ((int)response[5] < 10) {
    ti5 = "0" + String((int)response[5]);
  } else {
    ti5 = String(response[5] / 16) + String(response[5] % 16);
  }
  if ((int)response[6] < 10) {
    ti6 = "0" + String((int)response[6]);
  } else {
    ti6 = String(response[6] / 16) + String(response[6] % 16);
  }
  if ((int)response[7] < 10) {
    ti7 = "0" + String((int)response[7]);
  } else {
    ti7 = String(response[7] / 16) + String(response[7] % 16);
  }

  String n = ti5 + "." + ti6 + "." + ti7;

  return n;
}

String getPowerNow(int netAdr)
{

  response[0] = 0;
  Power[0] = netAdr;
  send(Power, sizeof(Power), response);
  long r = 0;
  //  int dir_U0 = 0;
  //  int dir_U1 = 0;
  //  int dir_U2 = 0;
  //  int dir_U3 = 0;

  //if((long)response[1]<<16 == 0x40) dir_U0=1;
  //if((long)response[1]<<16 == 0x80) dir_U0=-1; // это скорее всего не правильно, надо тестить с нагрузкой больше 1 кВТ
  r |= (long)response[3] << 8;                   // как в функции тока
  r |= (long)response[2];
  String U0 = String(float(r) / 100);
  r = 0;

  //if((long)response[4]<<16 == 0x40) dir_U1=1;
  //if((long)response[4]<<16 == 0x80) dir_U1=-1;
  r |= (long)response[6] << 8;
  r |= (long)response[5];
  String U1 = String(float(r) / 100);
  r = 0;

  //if((long)response[7]<<16 == 0x40) dir_U2=1;
  //if((long)response[7]<<16 == 0x80) dir_U2=-1;
  r |= (long)response[9] << 8;
  r |= (long)response[8];
  String U2 = String(float(r) / 100);
  r = 0;

  //if((long)response[10]<<16 == 0x40) dir_U3=1;
  //if((long)response[10]<<16 == 0x80) dir_U3=-1;
  r |= (long)response[12] << 8;
  r |= (long)response[11];
  String U3 = String(float(r) / 100);
  if (response[0] == netAdr)   return String(U0 + " " + U1 + " " + U2 + " " + U3);
  else   return String(F("Error"));

}

String getCurrent(int netAdr)
{
  response[0] = 0;
  Current[0] = netAdr;
  send(Current, sizeof(Current), response);
  long r = 0;
  r |= (long)response[1] << 16;
  r |= (long)response[3] << 8;
  r |= (long)response[2];
  String U1 = String(float(r) / 1000);
  r = 0;
  r |= (long)response[4] << 16;
  r |= (long)response[6] << 8;
  r |= (long)response[5];
  String U2 = String(float(r) / 1000);
  r = 0;
  r |= (long)response[7] << 16;
  r |= (long)response[9] << 8;
  r |= (long)response[8];
  String U3 = String(float(r) / 1000);
  if (response[0] == netAdr)   return String(U1 + " " + U2 + " " + U3);
  else   return String(F("Error"));

}

String getSuply(int netAdr)
{

  response[0] = 0;
  Suply[0] = netAdr;
  send(Suply, sizeof(Suply), response);
  long r = 0;
  r |= (long)response[1] << 16;
  r |= (long)response[3] << 8;
  r |= (long)response[2];
  String U1 = String(float(r) / 100);
  r = 0;
  r |= (long)response[4] << 16;
  r |= (long)response[6] << 8;
  r |= (long)response[5];
  String U2 = String(float(r) / 100);
  r = 0;
  r |= (long)response[7] << 16;
  r |= (long)response[9] << 8;
  r |= (long)response[8];
  String U3 = String(float(r) / 100);
  if (response[0] == netAdr)   return String(U1 + " " + U2 + " " + U3);
  else   return String(F("Error"));

}

String getEnergyT0(int netAdr)
{
  response[0] = 0;
  energyT0[0] = netAdr;
  send(energyT0, sizeof(energyT0), response);
  if (response[0] == netAdr)
  {
    long r = 0;
    r |= (long)response[2] << 24;
    r |= (long)response[1] << 16;
    r |= (long)response[4] << 8;
    r |= (long)response[3];
    String A_plus = String(r);
    r = 0;
    r |= (long)response[6] << 24;
    r |= (long)response[5] << 16;
    r |= (long)response[8] << 8;
    r |= (long)response[7];
    String A_minus = String(r);
    r = 0;
    r |= (long)response[10] << 24;
    r |= (long)response[9] << 16;
    r |= (long)response[12] << 8;
    r |= (long)response[11];
    String R_plus = String(r);
    r = 0;
    r |= (long)response[14] << 24;
    r |= (long)response[13] << 16;
    r |= (long)response[16] << 8;
    r |= (long)response[15];
    String R_minus = String(r);
    return String(A_plus + " " + A_minus + " " + R_plus + " " + R_minus);
  }

  else   return String(F("Error"));


}

//////////////////////////////////////////////////////////////////////////////////
void send(byte *cmd, int s, byte *response) {
  // Serial.print("sending...");

  unsigned int crc = crc16MODBUS(cmd, s);

  unsigned int crc1 = crc & 0xFF;
  unsigned int crc2 = (crc >> 8) & 0xFF;
  delay(10);
  digitalWrite(SerialControl, RS485Transmit);  // Init Transceiver
  for (int i = 0; i < s; i++)
  {
    RS485Serial.write(cmd[i]);
  }
  RS485Serial.write(crc1);
  RS485Serial.write(crc2);
  byte i = 0;
  digitalWrite(SerialControl, RS485Receive);  // Init Transceiver
  delay(200);
  if (RS485Serial.available())
  {
    while (RS485Serial.available())
    {
      byteReceived = RS485Serial.read();   // Read received byte
      delay(10);
      response[i++] = byteReceived;
    }
  }
  delay(20);
}

unsigned int crc16MODBUS(byte *s, int count) {
  unsigned int crc = 0xFFFF;

  for (int i = 0; i < count; i++) {
    crc = ((crc >> 8) ^ pgm_read_word(&crcTable[(crc ^ s[i]) & 0xFF]));
  }
  return crc;
}
