#include "App.hpp"
#include <VkContext.hpp>
#include <iostream>

int main(int argc, char **argv) {
  hiddenpiggy::App app("Window", 640, 480);
  app.OnCreate();
  app.run();
  app.OnDestroy();
  return 0;
}
