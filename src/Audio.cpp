#include "Audio.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <algorithm>
#include <iostream>

namespace {
constexpr int idleChannel = 0;
constexpr int fastChannel = 1;
constexpr int whooshChannel = 2;
} // namespace

Mix_Chunk* Audio::load(const std::filesystem::path& path) {
    Mix_Chunk* chunk = Mix_LoadWAV(path.string().c_str());
    if (!chunk)
        std::cerr << "Falha ao carregar audio " << path << ": " << Mix_GetError() << '\n';
    return chunk;
}

bool Audio::init(const std::filesystem::path& root) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Audio indisponivel: " << SDL_GetError() << '\n';
        return false;
    }

    if ((Mix_Init(MIX_INIT_OGG) & MIX_INIT_OGG) == 0 ||
        Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0) {
        std::cerr << "Audio indisponivel: " << Mix_GetError() << '\n';
        return false;
    }

    const auto dir = root / "assets/audio";
    idle_ = load(dir / "engine_idle.wav");
    fast_ = load(dir / "engine_fast.wav");
    whoosh_ = load(dir / "meteor_whoosh.ogg");
    ground_ = load(dir / "ground_explosion.ogg");
    groundShort_ = load(dir / "ground_explosion_short.ogg");
    car_ = load(dir / "car_impact.wav");

    Mix_AllocateChannels(16);
    Mix_ReserveChannels(3);
    for (int channel : {idleChannel, fastChannel, whooshChannel})
        Mix_Volume(channel, 0);
    if (idle_)
        Mix_PlayChannel(idleChannel, idle_, -1);
    if (fast_)
        Mix_PlayChannel(fastChannel, fast_, -1);
    if (whoosh_)
        Mix_PlayChannel(whooshChannel, whoosh_, -1);

    ready_ = true;
    return true;
}

void Audio::update(float speedRatio, float meteorIntensity, float dt, bool playing) {
    if (!ready_)
        return;

    const float speed = playing ? std::clamp(speedRatio, 0.0f, 1.0f) : 0.0f;
    const float desiredWhoosh = playing ? std::clamp(meteorIntensity, 0.0f, 1.0f) * 88.0f : 0.0f;
    whooshVolume_ += (desiredWhoosh - whooshVolume_) * std::min(1.0f, dt * 7.0f);

    Mix_Volume(idleChannel, muted_ || !playing ? 0 : static_cast<int>((1.0f - speed) * 42));
    Mix_Volume(fastChannel, muted_ ? 0 : static_cast<int>(speed * 75));
    Mix_Volume(whooshChannel, muted_ ? 0 : static_cast<int>(whooshVolume_));
}

void Audio::play(Mix_Chunk* chunk, int volume) {
    if (!ready_ || muted_ || !chunk)
        return;

    const int channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel >= 0)
        Mix_Volume(channel, std::clamp(volume, 0, MIX_MAX_VOLUME));
}

void Audio::groundImpact(float distance) {
    if (distance > 65.0f)
        return;

    const float nearby = 1.0f - distance / 65.0f;
    play((++impactCount_ % 3 == 0) ? ground_ : groundShort_, static_cast<int>(20 + nearby * 72));
}

void Audio::carImpact() {
    play(car_, 110);
}

void Audio::gameOver() {
    play(ground_, 120);
}

void Audio::toggleMute() {
    muted_ = !muted_;
    if (ready_ && muted_)
        Mix_Volume(-1, 0);
}

Audio::~Audio() {
    if (ready_)
        Mix_HaltChannel(-1);

    for (Mix_Chunk* chunk : {idle_, fast_, whoosh_, ground_, groundShort_, car_})
        if (chunk)
            Mix_FreeChunk(chunk);

    if (ready_)
        Mix_CloseAudio();
    Mix_Quit();
    if (SDL_WasInit(SDL_INIT_AUDIO))
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
