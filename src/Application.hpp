#include "Log.hpp"
#include "IMU.hpp"
#include "RoboClaw.hpp"

class Application
{
public:
  Application();

  void run();

private:
  const float POTENTIOMETER_CHANGE_THRESHOLD = 1.0f;
  const float YAW_CHANGE_THRESHOLD = 2.5f;
  const int ENCODER_COUNTS_PER_REVOLUTION = 7200; // visual
  // const int ENCODER_COUNTS_PER_REVOLUTION = 12000; // 64 counts/rev * 18.75 motor ratio * 10 pulley ratio
  const float MAX_CORRECTION_SPEED_ANGLE = 45.0f;
  const float MAX_ROTATIONS = 1.0f;
  const int MAX_CORRECTIVE_SPEED = 8000; // max around 8000
  const int RETURN_HOME_SPEED = 8000;
  const int UPDATE_INTERVAL_MS = 16;

  Log log = Log::getLog("Application");
  IMU imu;
  RoboClaw motor;
  DigitalOut motorSignalGnd;
  AnalogIn potentiometer;
  Timer updateTimer;
  uint32_t initialEncoderValue;
  uint32_t lastEncoderValue;
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
