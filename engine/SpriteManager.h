#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "MathUtil.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

using SpriteID = size_t;

class SpriteManager {
public:
    SpriteManager() = default;
    ~SpriteManager() = default;

    // sourceRect is the part of the texture this sprite uses
    // each file only gets loaded once, prints an error if it can't load
    [[nodiscard]] SpriteID RegisterTexture(const std::string& texturePath, const Rect& sourceRect);

    // throws std::out_of_range on a bad id
    // reference breaks on the next RegisterTexture, so keep the id instead
    // everyone using this id shares the sprite, so set its position right before drawing
    sf::Sprite& GetSprite(SpriteID id);

private:
    // sprites point at their textures, unordered_map means the textures don't move
    std::unordered_map<std::string, sf::Texture> mTextures;
    std::vector<sf::Sprite> mSprites;
};

}  // namespace ECSEngine
