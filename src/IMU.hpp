#include "Log.hpp"

#include "mbed.h"

class IMU
{
public:
  IMU(PinName tx, PinName rx, int baud = 57600);

  void initialize();
  void update();

  float yaw;
  float pitch;
  float roll;

private:
  void handleSerialRx();
  void handleMessage(char *command, int length);

  static const int MAX_MESSAGE_LENGTH = 64;
  static const int MESSAGE_BUFFER_SIZE = MAX_MESSAGE_LENGTH + 1;

  Log log = Log::getLog("IMU");

  Serial serial;
  char messageBuffer[MESSAGE_BUFFER_SIZE];
  int messageLength = 0;
};