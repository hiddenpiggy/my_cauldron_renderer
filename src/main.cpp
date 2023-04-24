#include "App.hpp"
#include <VkContext.hpp>
#include <iostream>

int main(int argc, char **argv) {
  hiddenpiggy::App app{};
  app.OnCreate("HelloWindow", 640, 480);
  app.run();
  app.OnDestroy();
  return 0;
}
