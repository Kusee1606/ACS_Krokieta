#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define MPU_ADDR 0x68
#define SD_CS_PIN 5
#define SAMPLING_PERIOD 0.02
const String filename = "/Lot_";
int16_t aX, aY, aZ, gX, gY, gZ, fX, fY, fZ;
long eX = 0, eY = 0, eZ = 0;
float roll, pitch, yaw;
float gRoll, gPitch;
float aRoll, aPitch;

uint32_t masterLoopIteration = 0;
File logFile;
int filenumber = 1;

void updateAccelData();
void updateGyroData();
void gyroError(int testCount);
void getRollPitch(float *roll, float *pitch);
void updateAngles();
uint16_t fifoCount();
void logData();

void setup()
{
  // Inicjalizacja wymaganych interfejsów itp.
  Serial.begin(115200);
  Wire.begin();
  SD.begin(SD_CS_PIN);

  // Reset rejestrów MPU
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  // Ustawienie rozdzielczości żyroskopa
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x08);
  Wire.endTransmission(true);
  // Ustawienie Filtrowania
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1A);
  Wire.write(0x04);
  Wire.endTransmission(true);
  // Ustawienie FIFO
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x19);
  Wire.write(0x13);
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x23);
  Wire.write(0x70);
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6A);
  Wire.write(0x40);
  Wire.endTransmission(true);

  // inicjalizacja pliku
  while (SD.exists(filename + filenumber + ".txt"))
    filenumber++;
  logFile = SD.open(filename + filenumber + ".txt", FILE_APPEND);
  logFile.println("Początek lotu");
  logFile.close();

  // Wylicznie 'dryfu' żyroskopa oraz wczytanie pozycji początkowej
  gyroError(100);
  logFile = SD.open(filename + filenumber + ".txt", FILE_APPEND);
  logFile.printf("X Error: %d Y Error: %d Z Error: %d\n", eX, eY, eZ);
  logFile.close();
  updateAccelData();
  getRollPitch(&gRoll, &gPitch);
  roll = gRoll;
  pitch = gPitch;
  yaw = 0.0;
  logData();
}


void loop()
{
  masterLoopIteration++;
  updateAccelData();
  getRollPitch(&aRoll, &aPitch);
  updateGyroData();
  updateAngles();
  logData();
  Serial.printf("Accel Roll: %f Accel Pitch: %f Roll: %f Pitch: %f Yaw: %f\n", aRoll, aPitch, roll, pitch, yaw);
}

void updateAccelData()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, 1);
  aX = (Wire.read() << 8) | Wire.read();
  aY = (Wire.read() << 8) | Wire.read();
  aZ = (Wire.read() << 8) | Wire.read();
}

void updateGyroData()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(MPU_ADDR, 6, 1);
  gX = (Wire.read() << 8) | Wire.read();
  gY = (Wire.read() << 8) | Wire.read();
  gZ = (Wire.read() << 8) | Wire.read();

  gX -= eX;
  gY -= eY;
  gZ -= eZ;
}

// Funcja wylicza średnie watości odczytów z żyroskopu który powinien znajodwać w stanie spoczynk
void gyroError(int testCount)
{
  for (int i = 0; i < testCount; i++)
  {
    updateGyroData();
    eX += gX;
    eY += gY;
    eZ += gZ;
    delay(20);
  }
  eX /= testCount;
  eY /= testCount;
  eZ /= testCount;
}

void getRollPitch(float *roll, float *pitch)
{
  *roll = 180 * atan2((aX), sqrt(aY * aY + aZ * aZ)) / PI;
  *pitch = 180 * atan2((aY), sqrt(aX * aX + aZ * aZ)) / PI;
  if (aZ < 0)
  {
    *roll = ((*roll > 0) ? 180 : -180) - *roll;
    *pitch = ((*pitch > 0) ? 180 : -180) - *pitch;
  }
}

void updateAngles()
{

  while (fifoCount())
  {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x74);
    Wire.endTransmission();
    Wire.requestFrom(MPU_ADDR, 1, 1);
    fX = Wire.read() << 8;
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x74);
    Wire.endTransmission();
    Wire.requestFrom(MPU_ADDR, 1, 1);
    fX |= Wire.read();

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x74);
    Wire.endTransmission();
    Wire.requestFrom(MPU_ADDR, 1, 1);
    fY = Wire.read() << 8;
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x74);
    Wire.endTransmission();
    Wire.requestFrom(MPU_ADDR, 1, 1);
    fY |= Wire.read();

    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x74);
    Wire.endTransmission();
    Wire.requestFrom(MPU_ADDR, 1, 1);
    fZ = Wire.read() << 8;
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x74);
    Wire.endTransmission();
    Wire.requestFrom(MPU_ADDR, 1, 1);
    fZ |= Wire.read();

    roll += (fX / 65.5) * SAMPLING_PERIOD;
    pitch += (fY / 65.5) * SAMPLING_PERIOD;
    yaw += (fZ / 65.5) * SAMPLING_PERIOD;
  }
}

uint16_t fifoCount()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x72);
  Wire.endTransmission();
  Wire.requestFrom(MPU_ADDR, 2, 1);
  return ((Wire.read() << 8) | Wire.read());
}

void logData()
{
  logFile = SD.open(filename + filenumber + ".txt", FILE_APPEND);
  logFile.printf("I: %lu T: %lu aX: %d aY: %d aZ: %d gX: %d gY: %d gZ: %d Accel Roll: %f Accel Pitch: %f Roll: %f Pitch: %f Yaw: %f\n", masterLoopIteration, millis(), aX, aY, aZ, gX, gY, gZ, aRoll, aPitch, roll, pitch, yaw);
  logFile.close();
}