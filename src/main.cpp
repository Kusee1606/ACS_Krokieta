#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define MPU_ADDR 0x68
#define SD_CS_PIN 5
#define INT_PIN 4

const String filename = "/Lot_";
int16_t aX, aY, aZ, gX, gY, gZ;
long eX = 0, eY = 0, eZ = 0;
float roll, pitch, yaw;
float gRoll, gPitch;
float aRoll, aPitch;

uint32_t masterLoopIteration = 0;
File logFile;
int filenumber = 1;

void updateSensorData();
void gyroError(int testCount);
void getRollPitch(float *roll, float *pitch);
void upgradeAngles(float period);
void logData();

void setup()
{
  // Inicjalizacja wymaganych interfejsów itp.
  Serial.begin(115200);
  Wire.begin();
  SD.begin(SD_CS_PIN);
  pinMode(INT_PIN, INPUT);

  // Konfugarcja IMU
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x08);
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
  updateSensorData();
  getRollPitch(&gRoll, &gPitch);
  roll = gRoll;
  pitch = gPitch;
  yaw = 0.0;
  logData();
}

unsigned long lastTime = 0;
unsigned long currentTime = 0;

void loop()
{
  currentTime = millis();
  masterLoopIteration++;
  updateSensorData();
  getRollPitch(&aRoll, &aPitch);
  upgradeAngles(currentTime - lastTime);
  lastTime = currentTime;
  logData();
  Serial.printf("Accel Roll: %f Accel Pitch: %f Roll: %f Pitch: %f Yaw: %f\n", aRoll, aPitch, roll, pitch, yaw);
}

// Funkcja pobiera dane
void updateSensorData()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, 1);
  aX = (Wire.read() << 8) | Wire.read();
  aY = (Wire.read() << 8) | Wire.read();
  aZ = (Wire.read() << 8) | Wire.read();

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
    updateSensorData();
    eX += aX;
    eY += aY;
    eZ += aZ;
    delay(1);
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

void upgradeAngles(float period)
{
  period /= 1000;
  roll += (gX / 65.5) * period;
  pitch += (gY /65.5) * period;
  yaw += (gZ / 65.5) * period;
}

void logData()
{
  logFile = SD.open(filename + filenumber + ".txt", FILE_APPEND);
  logFile.printf("I: %lu T: %lu aX: %d aY: %d aZ: %d gX: %d gY: %d gZ: %d Accel Roll: %f Accel Pitch: %f Roll: %f Pitch: %f Yaw: %f\n", masterLoopIteration, millis(), aX, aY, aZ, gX, gY, gZ, aRoll, aPitch, roll, pitch, yaw);
  logFile.close();
}