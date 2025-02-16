#pragma once
#include <glm/glm.hpp>
#include <cstdint>

struct FlipbookAnimation {
    double startTime_ = 0.0;
    uint32_t currentFrame_ = 0;
    uint32_t flipbookOffset_ = 0;
    uint32_t totalFrames_ = 64;
    float framesPerSecond_ = 30.0f;
    glm::vec2 position_ = glm::vec2(0.0f);

    bool IsFinished(double currentTime) const {
        double elapsed = currentTime - startTime_;
        uint32_t frame = flipbookOffset_ + static_cast<uint32_t>(elapsed * framesPerSecond_);
        return (frame >= flipbookOffset_ + totalFrames_);
    }
};

inline void UpdateFlipbook(FlipbookAnimation& anim, double currentTime, bool loop) {
    double elapsed = currentTime - anim.startTime_;
    uint32_t frame = anim.flipbookOffset_ + static_cast<uint32_t>(anim.framesPerSecond_ * elapsed);

    if (!loop) {
        if (frame >= anim.flipbookOffset_ + anim.totalFrames_) {
            frame = anim.flipbookOffset_ + anim.totalFrames_ - 1;
        }
    }
    else {
        uint32_t range = anim.totalFrames_;
        frame = anim.flipbookOffset_ + ((frame - anim.flipbookOffset_) % range);
    }

    anim.currentFrame_ = frame;
}