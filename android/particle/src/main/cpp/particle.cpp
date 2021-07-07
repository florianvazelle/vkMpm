#include <android/log.h>
#include <android_native_app_glue.h>
#include <memory>
#include <ParticleSystem.hpp>

std::unique_ptr<vkl::ParticleSystem> particle;
bool launch = false;

// Process the next main command.
void handle_cmd(android_app* app, int32_t cmd) {
  __android_log_print(ANDROID_LOG_INFO, "handle_cmd", "toto");

  if (cmd == APP_CMD_INIT_WINDOW) {
    // The window is being shown, get it ready.
    __android_log_print(ANDROID_LOG_INFO, "handle_cmd", "APP_CMD_INIT_WINDOW");

    vkl::DebugOption debugOption = {
        .debugLevel  = 0,
        .exitOnError = false,
    };

    particle = std::make_unique<vkl::ParticleSystem>(app, "vkLavaMpm", debugOption);

    launch = true;
  } else if (cmd == APP_CMD_TERM_WINDOW) {
    // The window is being hidden or closed, clean it up.
    __android_log_print(ANDROID_LOG_INFO, "handle_cmd", "APP_CMD_TERM_WINDOW");
    launch = false;
  } else {
    __android_log_print(ANDROID_LOG_INFO, "Vulkan Tutorials", "event not handled: %d", cmd);
  }
}

int32_t handle_input(android_app* app, AInputEvent* event) {
  if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION && AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN) {
    particle->togglePause();
  }

  return 0;
}

void android_main(struct android_app* app) {
  // Set the callback to process system events
  app->onAppCmd     = handle_cmd;
  app->onInputEvent = handle_input;

  // Used to poll the events in the main loop
  int events;
  android_poll_source* source;

  // Main loop
  do {
    if (ALooper_pollAll(launch ? 1 : 0, nullptr, &events, (void**)&source) >= 0) {
      if (source != NULL) source->process(app, source);
    }

    // render if vulkan is ready
    if (launch) {
      break;
    }
  } while (app->destroyRequested == 0);

  particle->run();
}