#ifndef AUDIO_H
#define AUDIO_H

#include <filesystem>

struct Mix_Chunk;

class Audio {
  public:
    bool init(const std::filesystem::path& root);
    void update(float speedRatio, float meteorIntensity, float dt, bool playing);
    void groundImpact(float distance);
    void carImpact();
    void gameOver();
    void toggleMute();
    bool isMuted() const {
        return muted_;
    }
    ~Audio();

  private:
    Mix_Chunk* load(const std::filesystem::path& path);
    void play(Mix_Chunk* chunk, int volume);

    Mix_Chunk* idle_ = nullptr;
    Mix_Chunk* fast_ = nullptr;
    Mix_Chunk* whoosh_ = nullptr;
    Mix_Chunk* ground_ = nullptr;
    Mix_Chunk* groundShort_ = nullptr;
    Mix_Chunk* car_ = nullptr;
    bool ready_ = false;
    bool muted_ = false;
    float whooshVolume_ = 0.0f;
    int impactCount_ = 0;
};

#endif
