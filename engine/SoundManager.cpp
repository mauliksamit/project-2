#include "SoundManager.h"

#include <iostream>

namespace ECSEngine {

void SoundManager::RegisterSound(const std::string& soundPath, const std::string& soundName) {
    sf::SoundBuffer& buffer = mBuffers[soundName];
    if (!buffer.loadFromFile(soundPath)) {
        std::cerr << "Could not load sound: " << soundPath << "\n";
    }
    mSounds.emplace(soundName, buffer);
}

sf::Sound& SoundManager::GetSound(const std::string& soundName) { return mSounds.at(soundName); }

}  // namespace ECSEngine
