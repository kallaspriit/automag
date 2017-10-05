#include "Application.hpp"
#include "SerialLogHandler.hpp"

int main()
{
  Serial serial(USBTX, USBRX);
  serial.baud(115200);

  Thread::wait(5000);

  serial.printf("**********************************************\n");
  serial.printf("*    AUTOMATED MAGNETOMETER ANTENNA AIMER    *\n");
  serial.printf("*   Priit Kallas <priit@stagnationlab.com>   *\n");
  serial.printf("*   https://github.com/kallaspriit/automag   *\n");
  serial.printf("*               v1.1 27.09.2017              *\n");
  serial.printf("**********************************************\n");
  serial.printf("\n");

  Log::setLogHandler(new SerialLogHandler(Log::LogLevel::DEBUG));

  Application application;
  application.run();
}