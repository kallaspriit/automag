#include "Application.hpp"

Application::Application() : imu(p9, p10, 57600) {}

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

  imu.initialize();
}

void Application::loop()
{
}