#include "SpriteManager.h"

#include <iostream>

namespace ECSEngine {

SpriteID SpriteManager::RegisterTexture(const std::string& texturePath, const Rect& sourceRect) {
    if (mTextures.find(texturePath) == mTextures.end()) {
        if (!mTextures[texturePath].loadFromFile(texturePath)) {
            std::cerr << "Could not load texture: " << texturePath << "\n";
        }
    }

    int left = static_cast<int>(sourceRect.topLeft.x);
    int top = static_cast<int>(sourceRect.topLeft.y);
    int width = static_cast<int>(sourceRect.width);
    int height = static_cast<int>(sourceRect.height);
    sf::IntRect textureRect({left, top}, {width, height});

    SpriteID newID = mSprites.size();
    mSprites.emplace_back(mTextures[texturePath], textureRect);
    return newID;
}

sf::Sprite& SpriteManager::GetSprite(SpriteID id) { return mSprites.at(id); }

}  // namespace ECSEngine
