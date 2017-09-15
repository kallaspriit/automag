#include "Application.hpp"

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
}

void Application::loop()
{
}