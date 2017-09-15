#include "Log.hpp"
#include "IMU.hpp"
#include "RoboClaw.hpp"

class Application
{
public:
  Application();

  void run();

private:
  Log log = Log::getLog("Application");
  IMU imu;
  RoboClaw motor;
  DigitalOut motorSignalGnd;

  void setup();
  void loop();

  float getAngleBetween(float a, float b);
};
