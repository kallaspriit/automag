#include "SerialLogHandler.hpp"

SerialLogHandler::SerialLogHandler(Log::LogLevel minimumLevel) : LogHandler(minimumLevel) {}

void SerialLogHandler::handleLogMessage(Log::LogLevel level, const char *component, const char *message)
{
  if (level < minimumLevel)
  {
    return;
  }

  serialMutex.lock();

  printf("# %-5s | %-35s | %s\n", logLevelToName(level), component, message);

  serialMutex.unlock();
};
