#pragma once
#include <glm/glm.hpp>

// Holds animation data for a single spawned flipbook instance
struct FlipbookAnimation {
    double startTime = 0.0;
    uint32_t currentFrame = 0;
    uint32_t flipbookOffset = 0;
    uint32_t totalFrames = 64;
    float framesPerSecond = 30.0f;
    glm::vec2 position = glm::vec2(0.0f);

    bool IsFinished(double currentTime) const {
        double elapsed = currentTime - startTime;
        uint32_t frame = flipbookOffset + static_cast<uint32_t>(elapsed * framesPerSecond);
        return (frame >= flipbookOffset + totalFrames);
    }
};

inline void UpdateFlipbook(FlipbookAnimation& anim, double currentTime, bool loop) {
    double elapsed = currentTime - anim.startTime;
    uint32_t frame = anim.flipbookOffset + static_cast<uint32_t>(anim.framesPerSecond * elapsed);

    if (!loop) {
        if (frame >= anim.flipbookOffset + anim.totalFrames) {
            frame = anim.flipbookOffset + anim.totalFrames - 1;
        }
    }
    else {
        uint32_t range = anim.totalFrames;
        frame = anim.flipbookOffset + ((frame - anim.flipbookOffset) % range);
    }

    anim.currentFrame = frame;
}