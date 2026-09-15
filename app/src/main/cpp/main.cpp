#include <jni.h>

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>

#include <memory>

#include "AndroidOut.h"
#include "App.h"
#include "Renderer.h"

/*!
 * Everything that lives for the whole android_main. The app (game state, settings, AI) is kept
 * outside the Renderer because the renderer (and its GL context) is torn down and recreated
 * with the window, e.g. when the app goes to the background, and the game must survive that.
 */
struct AppState {
    App app;
    std::unique_ptr<Renderer> renderer;
};

extern "C" {

/*!
 * Handles commands sent to this Android application
 * @param pApp the app the commands are coming from
 * @param cmd the command to handle
 */
void handle_cmd(android_app *pApp, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {
            // A new window is created, associate a renderer with it. The game itself already
            // exists in AppState and is simply handed to the new renderer.
            auto *state = reinterpret_cast<AppState *>(pApp->userData);
            state->renderer = std::make_unique<Renderer>(pApp, state->app);
            break;
        }
        case APP_CMD_TERM_WINDOW: {
            // The window is being destroyed. Release the renderer (and its GL resources) but keep
            // the game state so play resumes where it left off.
            auto *state = reinterpret_cast<AppState *>(pApp->userData);
            state->renderer.reset();
            break;
        }
        default:
            break;
    }
}

/*!
 * Enable the motion events you want to handle; not handled events are
 * passed back to OS for further processing. For this example case,
 * only pointer and joystick devices are enabled.
 *
 * @param motionEvent the newly arrived GameActivityMotionEvent.
 * @return true if the event is from a pointer or joystick device,
 *         false for all other input devices.
 */
bool motion_event_filter_func(const GameActivityMotionEvent *motionEvent) {
    auto sourceClass = motionEvent->source & AINPUT_SOURCE_CLASS_MASK;
    return (sourceClass == AINPUT_SOURCE_CLASS_POINTER ||
            sourceClass == AINPUT_SOURCE_CLASS_JOYSTICK);
}

/*!
 * This the main entry point for a native activity
 */
void android_main(struct android_app *pApp) {
    // Can be removed, useful to ensure your code is running
    aout << "Welcome to android_main" << std::endl;

    // The game state lives for the whole activity; the renderer comes and goes with the window.
    AppState state;
    state.app.setDataDir(pApp->activity->internalDataPath);
    pApp->userData = &state;

    // Register an event handler for Android events
    pApp->onAppCmd = handle_cmd;

    // Set input event filters (set it to NULL if the app wants to process all inputs).
    // Note that for key inputs, this example uses the default default_key_filter()
    // implemented in android_native_app_glue.c.
    android_app_set_motion_event_filter(pApp, motion_event_filter_func);

    // This sets up a typical game/event loop. It will run until the app is destroyed.
    do {
        // Process all pending events before running game logic.
        bool done = false;
        while (!done) {
            // 0 is non-blocking.
            int timeout = 0;
            int events;
            android_poll_source *pSource;
            int result = ALooper_pollOnce(timeout, nullptr, &events,
                                          reinterpret_cast<void**>(&pSource));
            switch (result) {
                case ALOOPER_POLL_TIMEOUT:
                    [[clang::fallthrough]];
                case ALOOPER_POLL_WAKE:
                    // No events occurred before the timeout or explicit wake. Stop checking for events.
                    done = true;
                    break;
                case ALOOPER_EVENT_ERROR:
                    aout << "ALooper_pollOnce returned an error" << std::endl;
                    break;
                case ALOOPER_POLL_CALLBACK:
                    break;
                default:
                    if (pSource) {
                        pSource->process(pApp, pSource);
                    }
            }
        }

        // Only run the frame when a window (and thus a renderer) exists.
        if (state.renderer) {
            // Process game input
            state.renderer->handleInput();

            // Advance the app (e.g. let the computer opponent move)
            state.app.update();

            // Render a frame
            state.renderer->render();
        }
    } while (!pApp->destroyRequested);

    // Make sure the renderer is gone before the AppState on the stack goes out of scope.
    state.renderer.reset();
    pApp->userData = nullptr;
}
}