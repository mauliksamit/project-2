#pragma once

#include <string>
#include <unordered_map>

#include <SFML/Audio.hpp>

namespace ECSEngine {

class SoundManager {
public:
    SoundManager() = default;
    ~SoundManager() = default;

    // if the file can't load it prints an error and the sound is just silent
    void RegisterSound(const std::string& soundPath, const std::string& soundName);

    // throws std::out_of_range if soundName wasn't registered
    // reference is good as long as the manager is alive
    // everyone using this name shares the sound, so a volume change sticks for all of them
    sf::Sound& GetSound(const std::string& soundName);

private:
    // sounds point at their buffers, unordered_map means the buffers don't move
    std::unordered_map<std::string, sf::SoundBuffer> mBuffers;
    std::unordered_map<std::string, sf::Sound> mSounds;
};

}  // namespace ECSEngine
