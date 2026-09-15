#ifndef ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H
#define ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H

#include <memory>
#include <android/asset_manager.h>
#include <GLES3/gl3.h>
#include <string>
#include <vector>

class TextureAsset {
public:
    /*!
     * Loads a texture asset from the assets/ directory
     * @param assetManager Asset manager to use
     * @param assetPath The path to the asset
     * @return a shared pointer to a texture asset, resources will be reclaimed when it's cleaned up
     */
    static std::shared_ptr<TextureAsset>
    loadAsset(AAssetManager *assetManager, const std::string &assetPath);

    /*!
     * Creates a 1x1 opaque white texture. Combined with a Model tint this draws flat-coloured
     * quads through the same textured shader.
     */
    static std::shared_ptr<TextureAsset> createWhite();

    ~TextureAsset();

    /*!
     * @return the texture id for use with OpenGL
     */
    constexpr GLuint getTextureID() const { return textureID_; }

private:
    inline TextureAsset(GLuint textureId) : textureID_(textureId) {}

    GLuint textureID_;
};

#endif //ANDROIDGLINVESTIGATIONS_TEXTUREASSET_H