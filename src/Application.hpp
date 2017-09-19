#include "Log.hpp"
#include "IMU.hpp"
#include "RoboClaw.hpp"

class Application
{
public:
  Application();

  void run();

private:
  const float POTENTIOMETER_CHANGE_THRESHOLD = 1.0f / 360.0f;

  Log log = Log::getLog("Application");
  IMU imu;
  RoboClaw motor;
  DigitalOut motorSignalGnd;
  AnalogIn potentiometer;
  Timer potentiometerTimer;
  float lastPotentiometerValue;
  float targetAngle;

  void setup();
  void loop();

  void updatePotentiometer();
  void updateMotorSpeed();

  float getAngleBetween(float a, float b);
};
