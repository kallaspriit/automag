#include "Application.hpp"

#include <algorithm>

Application::Application() : imu(p9, p10, 57600), motor(128, 57600, p27, p28), motorSignalGnd(p29) {}

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
}

void Application::loop()
{
  // int encoder1 = motor.ReadSpeedM1();
  imu.update();

  float targetYaw = 0.0f;
  float currentYaw = imu.yaw;
  float errorYaw = getAngleBetween(currentYaw, targetYaw);
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