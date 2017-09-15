#include "IMU.hpp"

#include <string>

IMU::IMU(PinName tx, PinName rx, int baud) : serial(tx, rx, baud), yaw(0.0f), pitch(0.0f), roll(0.0f) {}

void IMU::initialize()
{
  log.info("initializing IMU");

  serial.printf("#o0");

  serial.attach(callback(this, &IMU::handleSerialRx), Serial::RxIrq);
}

void IMU::update()
{
  serial.printf("#f");
}

void IMU::handleSerialRx()
{
  char receivedChar = serial.getc();

  if (receivedChar == '\n')
  {
    handleMessage(messageBuffer, messageLength);

    messageBuffer[0] = '\0';
    messageLength = 0;

    // debug.setLedMode(LED_COMMAND_RECEIVED_INDEX, Debug::LedMode::BLINK_ONCE);
  }
  else
  {
    if (messageLength > MAX_MESSAGE_LENGTH - 1)
    {
      log.warn("maximum message length is %d characters, stopping at %s", MAX_MESSAGE_LENGTH, messageBuffer);

      return;
    }

    messageBuffer[messageLength++] = receivedChar;
    messageBuffer[messageLength] = '\0';
  }
}

void IMU::handleMessage(char *message, int length)
{
  if (length == 0)
  {
    return;
  }

  // log.debug("< %s", message);

  const char *token = strtok(message, "=,");

  int tokenIndex = 0;

  while (token != NULL)
  {
    // log.debug("  - %s", token);

    // expect message like #YPR=-82.55,50.39,-66.27
    if (tokenIndex == 0 && strcmp(token, "#YPR") != 0)
    {
      break;
    }

    if (tokenIndex == 1)
    {
      yaw = atof(token);
    }
    else if (tokenIndex == 2)
    {
      pitch = atof(token);
    }
    else if (tokenIndex == 3)
    {
      roll = atof(token);
    }

    token = strtok(NULL, "=,");

    tokenIndex++;
  }
}