#include "Application.hpp"

#include <algorithm>

Application::Application() : imu(p9, p10, 57600),
                             motor(128, 57600, p27, p28),
                             motorSignalGnd(p29),
                             potentiometer(p20),
                             initialEncoderValue(0),
                             lastEncoderValue(0),
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
    targetAngle = -(currentValue * 360.0f - 180.0f);
    lastPotentiometerValue = currentValue;

    log.info("target angle was changed to %.1f degrees (%.2f)", targetAngle, currentValue);

    // recenter at current location to avoid shake when moving to new angle far from initial
    initialEncoderValue = lastEncoderValue;
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

  lastEncoderValue = currentEncoderValue;

  if (initialEncoderValue == 0)
  {
    initialEncoderValue = currentEncoderValue;

    return;
  }

  encoderDelta = currentEncoderValue > initialEncoderValue ? currentEncoderValue - initialEncoderValue : -(initialEncoderValue - currentEncoderValue);

  // log.info("encoder current: %lu, initial: %lu, delta: %d, status: %d", lastEncoderValue, initialEncoderValue, encoderDelta, status);
}

void Application::loopMotorSpeed()
{
  imu.update();

  int msSinceLastImuUpdate = imu.getTimeSinceLastUpdate();

  if (!imu.hasReceivedData || msSinceLastImuUpdate > 1000)
  {
    log.info("have not received data from IMU for %d ms, check connection (and note that IMU is powered by the battery not USB)", msSinceLastImuUpdate);

    return;
  }

  // perform returning home
  if (isReturningHome)
  {
    motor.SpeedM1(returnHomeDirection * RETURN_HOME_SPEED);

    bool isHome = (returnHomeDirection == 1 && encoderDelta > 0) || (returnHomeDirection == -1 && encoderDelta < 0);

    if (isHome)
    {
      log.info("arrived home\n");

      isReturningHome = false;
    }

    return;
  }

  // check for excessive rotation and return home if needed
  int maxEncoderValue = (int)floor(MAX_ROTATIONS * (float)ENCODER_COUNTS_PER_REVOLUTION);

  if (abs(encoderDelta) > maxEncoderValue)
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

  // calculate corrective speed to maintain target angle
  float currentAngle = imu.yaw;
  float errorAngle = getAngleBetween(currentAngle, targetAngle);
  int maxSpeed = MAX_CORRECTIVE_SPEED;
  float maxSpeedError = MAX_CORRECTION_SPEED_ANGLE;
  int correctiveSpeed = min((int)floor((fabs(errorAngle) / maxSpeedError) * (float)maxSpeed), maxSpeed);
  int correctionDirection = errorAngle > 0 ? 1 : -1;

  // log.info("CURRENT: %.1f    TARGET: %.1f    ERROR: %.1f    SPEED: %d    DELTA: %d", currentAngle, targetAngle, errorAngle, correctiveSpeed * correctionDirection, encoderDelta);
  // Thread::wait(500);

  motor.SpeedM1(correctiveSpeed * correctionDirection);
}

float Application::getAngleBetween(float a, float b)
{
  float angle = a - b;

  angle += (angle > 180.0f) ? -360.0f : (a < -180.0f) ? 360.0f : 0.0f;

  return angle;
}