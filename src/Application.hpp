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
  const int ENCODER_COUNTS_PER_REVOLUTION = 7000;
  const int MAX_CORRECTIVE_SPEED = 2000; // max around 8000
  const int RETURN_HOME_SPEED = 2000;
  const int UPDATE_INTERVAL_MS = 16;

  Log log = Log::getLog("Application");
  IMU imu;
  RoboClaw motor;
  DigitalOut motorSignalGnd;
  AnalogIn potentiometer;
  Timer updateTimer;
  uint32_t initialEncoderValue;
  float lastPotentiometerValue;
  float lastAngle;
  float targetAngle;
  int encoderDelta;
  bool isReturningHome;
  int returnHomeDirection;

  void setup();
  void loop();

  void updatePotentiometer();
  void updateYawLogger();
  void updateEncoder();
  void loopMotorSpeed();

  float getAngleBetween(float a, float b);
};
