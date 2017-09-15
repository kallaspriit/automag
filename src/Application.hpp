#include "Log.hpp"

class Application
{
public:
  void run();

private:
  Log log = Log::getLog("Application");

  void setup();
  void loop();
};
