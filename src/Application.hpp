#include "Log.hpp"
#include "IMU.hpp"
#include "RoboClaw.hpp"

class Application
{
public:
  Application();

  void run();

private:
  const float POTENTIOMETER_CHANGE_THRESHOLD = 0.5f;
  const float YAW_CHANGE_THRESHOLD = 2.5f;

  Log log = Log::getLog("Application");
  IMU imu;
  RoboClaw motor;
  DigitalOut motorSignalGnd;
  AnalogIn potentiometer;
  Timer updateTimer;
  float lastPotentiometerValue;
  float lastAngle;
  float targetAngle;

  void setup();
  void loop();

  void updatePotentiometer();
  void updateYawLogger();
  void updateMotorSpeed();

  float getAngleBetween(float a, float b);
};
