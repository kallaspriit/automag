#include <mbed.h>
#include <rtos.h>

#include "Log.hpp"

class SerialLogHandler : public Log::LogHandler
{

public:
  SerialLogHandler(Log::LogLevel minimumLevel);

  void handleLogMessage(Log::LogLevel level, const char *component, const char *message);

private:
  Mutex serialMutex;
};