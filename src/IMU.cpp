#include "IMU.hpp"

IMU::IMU(PinName tx, PinName rx, int baud) : serial(tx, rx, baud) {}

void IMU::initialize()
{
  log.info("initializing IMU");

  serial.attach(callback(this, &IMU::handleSerialRx), Serial::RxIrq);
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

void IMU::handleMessage(const char *message, int length)
{
  log.debug("< %s", message);
}