#ifndef ANDROIDGLINVESTIGATIONS_MODEL_H
#define ANDROIDGLINVESTIGATIONS_MODEL_H

#include <vector>
#include "TextureAsset.h"

union Vector3 {
    struct {
        float x, y, z;
    };
    float idx[3];
};

union Vector4 {
    struct {
        float r, g, b, a;
    };
    float idx[4];
};

union Vector2 {
    struct {
        float x, y;
    };
    struct {
        float u, v;
    };
    float idx[2];
};

struct Vertex {
    constexpr Vertex(const Vector3 &inPosition, const Vector2 &inUV) : position(inPosition),
                                                                       uv(inUV) {}

    Vector3 position;
    Vector2 uv;
};

typedef uint16_t Index;

class Model {
public:
    inline Model(
            std::vector<Vertex> vertices,
            std::vector<Index> indices,
            std::shared_ptr<TextureAsset> spTexture,
            Vector4 tint = Vector4{1.f, 1.f, 1.f, 1.f})
            : vertices_(std::move(vertices)),
              indices_(std::move(indices)),
              spTexture_(std::move(spTexture)),
              tint_(tint) {}

    inline const Vertex *getVertexData() const {
        return vertices_.data();
    }

    inline const size_t getIndexCount() const {
        return indices_.size();
    }

    inline const Index *getIndexData() const {
        return indices_.data();
    }

    inline const TextureAsset &getTexture() const {
        return *spTexture_;
    }

    /*! Colour the texture is multiplied with; use a white texture for flat-coloured quads. */
    inline const Vector4 &getTint() const {
        return tint_;
    }

private:
    std::vector<Vertex> vertices_;
    std::vector<Index> indices_;
    std::shared_ptr<TextureAsset> spTexture_;
    Vector4 tint_;
};

#endif //ANDROIDGLINVESTIGATIONS_MODEL_H