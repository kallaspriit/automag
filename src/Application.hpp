#include "Log.hpp"
#include "IMU.hpp"

class Application
{
public:
  Application();

  void run();

private:
  Log log = Log::getLog("Application");
  IMU imu;

  void setup();
  void loop();
};
