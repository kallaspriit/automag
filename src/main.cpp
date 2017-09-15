#include "Application.hpp"
#include "SerialLogHandler.hpp"

int main()
{
  Log::setLogHandler(new SerialLogHandler(Log::LogLevel::DEBUG));

  Serial serial(USBTX, USBRX);
  serial.baud(115200);

  Application application;
  application.run();
}