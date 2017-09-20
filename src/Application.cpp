#include "Application.hpp"

#include <algorithm>

Application::Application() : imu(p9, p10, 57600),
                             motor(128, 57600, p27, p28),
                             motorSignalGnd(p29),
                             potentiometer(p20),
                             lastPotentiometerValue(0.0f),
                             lastAngle(0.0f),
                             targetAngle(0.0f)
{
}

void Application::run()
{
  log.info("running application");

  setup();

  log.info("starting main loop");

  while (true)
  {
    loop();
  }
}

void Application::setup()
{
  log.info("setting up application");

  motorSignalGnd = 0;
  motor.ForwardM1(0);
  imu.initialize();
  updateTimer.start();
}

void Application::loop()
{
  if (updateTimer.read_ms() >= 100)
  {
    updatePotentiometer();
    updateYawLogger();

    updateTimer.reset();
  }

  updateMotorSpeed();
}

void Application::updatePotentiometer()
{
  float currentValue = potentiometer;
  float deltaValue = fabs(currentValue - lastPotentiometerValue);

  if (deltaValue > POTENTIOMETER_CHANGE_THRESHOLD / 360.0f)
  {
    targetAngle = currentValue * 360.0f - 180.0f;
    lastPotentiometerValue = currentValue;

    log.info("target angle was changed to %.1f degrees (%.2f)", targetAngle, currentValue);
  }
}

void Application::updateYawLogger()
{
  float currentAngle = imu.yaw;
  float deltaValue = getAngleBetween(currentAngle, lastAngle);

  if (fabs(deltaValue) > YAW_CHANGE_THRESHOLD)
  {
    lastAngle = currentAngle;
    float errorAngle = getAngleBetween(currentAngle, targetAngle);

    log.info("current angle changed to %.0f degrees (target: %.0f, error: %.0f)", currentAngle, targetAngle, errorAngle);
  }
}

void Application::updateMotorSpeed()
{
  // int encoder1 = motor.ReadSpeedM1();
  imu.update();

  float currentYaw = imu.yaw;
  float errorYaw = getAngleBetween(currentYaw, targetAngle);
  int maxSpeed = 2000;
  float maxSpeedError = 45.0f;
  int correctiveSpeed = min(max((int)floor((errorYaw / maxSpeedError) * (float)maxSpeed), -maxSpeed), maxSpeed);

  // log.info("YAW: %.1f    PITCH: %.1f    ROLL: %.1f    TARGET: %.1f    ERROR: %.1f    SPEED: %d", imu.yaw, imu.pitch, imu.roll, targetYaw, errorYaw, correctiveSpeed);
  // Thread::wait(100);

  motor.SpeedM1(correctiveSpeed);
}

float Application::getAngleBetween(float a, float b)
{
  // float c = b - a;
  // return fmod(c + 180.0f, 360.0f) - 180.0f;

  float r = b - a;
  r += (r > 180.0f) ? -360.0f : (a < -180.0f) ? 360.0f : 0.0f;

  return r;

  // return atan2(sin(a - b), cos(a - b));
}