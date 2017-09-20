#include "Application.hpp"

#include <algorithm>

Application::Application() : imu(p9, p10, 57600),
                             motor(128, 57600, p27, p28),
                             motorSignalGnd(p29),
                             potentiometer(p20),
                             initialEncoderValue(0),
                             lastPotentiometerValue(0.0f),
                             lastAngle(0.0f),
                             targetAngle(0.0f),
                             encoderDelta(0),
                             isReturningHome(false),
                             returnHomeDirection(1)
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
  if (updateTimer.read_ms() >= UPDATE_INTERVAL_MS)
  {
    updatePotentiometer();
    updateYawLogger();
    updateEncoder();

    updateTimer.reset();
  }

  loopMotorSpeed();
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

void Application::updateEncoder()
{
  uint8_t status = 0;
  bool valid = false;
  uint32_t currentEncoderValue = motor.ReadEncM1(&status, &valid);

  if (!valid)
  {
    // log.info("encoder invalid: %lu, status: %d", currentEncoderValue, status);

    return;
  }

  if (initialEncoderValue == 0)
  {
    initialEncoderValue = currentEncoderValue;

    return;
  }

  encoderDelta = currentEncoderValue > initialEncoderValue ? currentEncoderValue - initialEncoderValue : -(initialEncoderValue - currentEncoderValue);

  // log.info("encoder current: %lu, initial: %lu, delta: %d, status: %d", currentEncoderValue, initialEncoderValue, encoderDelta, status);
}

void Application::loopMotorSpeed()
{
  imu.update();

  if (isReturningHome)
  {
    motor.SpeedM1(returnHomeDirection * RETURN_HOME_SPEED);

    bool isHome = (returnHomeDirection == 1 && encoderDelta > 0) || (returnHomeDirection == -1 && encoderDelta < 0);

    if (isHome)
    {
      log.info("got home\n");

      isReturningHome = false;
    }

    return;
  }

  if (abs(encoderDelta) > ENCODER_COUNTS_PER_REVOLUTION)
  {
    if (encoderDelta > 0)
    {
      returnHomeDirection = -1;
    }
    else
    {
      returnHomeDirection = 1;
    }

    log.info("return home %s\n", returnHomeDirection > 0 ? "clockwise" : "anti-clockwise");

    isReturningHome = true;

    return;
  }

  float currentYaw = imu.yaw;
  float errorYaw = getAngleBetween(currentYaw, targetAngle);
  int maxSpeed = MAX_CORRECTIVE_SPEED;
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