#ifndef ANDROIDGLINVESTIGATIONS_RENDERER_H
#define ANDROIDGLINVESTIGATIONS_RENDERER_H

#include <EGL/egl.h>
#include <memory>

#include "App.h"
#include "Canvas.h"
#include "Model.h"
#include "Shader.h"

struct android_app;

class Renderer {
public:
    /*!
     * @param pApp the android_app this Renderer belongs to, needed to configure GL
     * @param game the application to display and forward input to; must outlive the renderer
     */
    inline Renderer(android_app *pApp, App &game) :
            app_(pApp),
            game_(game),
            display_(EGL_NO_DISPLAY),
            surface_(EGL_NO_SURFACE),
            context_(EGL_NO_CONTEXT),
            width_(0),
            height_(0),
            shaderNeedsNewProjectionMatrix_(true) {
        initRenderer();
    }

    virtual ~Renderer();

    /*!
     * Handles input from the android_app.
     *
     * Note: this will clear the input queue
     */
    void handleInput();

    /*!
     * Renders all the models in the renderer
     */
    void render();

private:
    /*!
     * Performs necessary OpenGL initialization. Customize this if you want to change your EGL
     * context or application-wide settings.
     */
    void initRenderer();

    /*!
     * @brief we have to check every frame to see if the framebuffer has changed in size. If it has,
     * update the viewport accordingly
     */
    void updateRenderArea();

    /*!
     * Loads the textures and creates the canvas.
     */
    void createCanvas();

    android_app *app_;
    App &game_;
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLint width_;
    EGLint height_;

    bool shaderNeedsNewProjectionMatrix_;

    std::unique_ptr<Shader> shader_;
    std::unique_ptr<Canvas> canvas_;
};

#endif //ANDROIDGLINVESTIGATIONS_RENDERER_H