#include "MapLoader.h"

#include <fstream>
#include <iostream>
#include <unordered_map>

#include "GameObjects.h"

namespace {

constexpr char kSpawnerChar = 'S';
constexpr char kSpikeChar = 'x';

struct TileDef {
    ECSEngine::SpriteID sprite = 0;
    bool hasBox = false;
    ECSEngine::Rect box;  // relative to the tile's top-left
};

bool Fail(const std::string& file, const std::string& why) {
    std::cerr << "LoadMap(" << file << "): " << why << "\n";
    return false;
}

}  // namespace

bool LoadMap(Engine& engine, const std::string& path, const std::string& file,
             ECSEngine::SpriteID starSprite) {
    using ECSEngine::Point2D;
    using ECSEngine::Rect;

    std::ifstream in(path + file);
    if (!in) {
        return Fail(file, "could not open " + path + file);
    }

    std::string word;
    int dictType = 0;
    if (!(in >> word >> dictType) || word != "dictionary" || (dictType != 1 && dictType != 2)) {
        return Fail(file, "expected 'dictionary 1' or 'dictionary 2'");
    }

    // dictionary entries until the "map" keyword
    std::unordered_map<char, TileDef> dictionary;
    while (in >> word && word != "map") {
        if (word.size() != 1) {
            return Fail(file, "dictionary key must be one character, got '" + word + "'");
        }
        std::string texture;
        float sx, sy, sw, sh;
        in >> texture >> sx >> sy >> sw >> sh;
        TileDef def;
        if (dictType == 2) {
            float bx, by, bw, bh;
            in >> bx >> by >> bw >> bh;
            def.hasBox = true;
            def.box = Rect(bx, by, bw, bh);
        }
        if (!in) {
            return Fail(file, "bad dictionary entry for '" + word + "'");
        }
        def.sprite = engine.GetSpriteManager().RegisterTexture(path + texture, Rect(sx, sy, sw, sh));
        dictionary[word[0]] = def;
    }

    // map origin L T tile W H size GW GH
    std::string origin, tile, size;
    float left, top, tileW, tileH;
    int gridW, gridH;
    in >> origin >> left >> top >> tile >> tileW >> tileH >> size >> gridW >> gridH;
    if (!in || origin != "origin" || tile != "tile" || size != "size") {
        return Fail(file, "bad 'map origin ... tile ... size ...' line");
    }

    std::string row;
    std::getline(in, row);  // rest of the header line
    const Rect bounds(0, 0, tileW, tileH);
    for (int r = 0; r < gridH && std::getline(in, row); ++r) {
        if (!row.empty() && row.back() == '\r') {
            row.pop_back();  // maps saved on Windows
        }
        const int cols = std::min(gridW, static_cast<int>(row.size()));
        for (int c = 0; c < cols; ++c) {
            auto it = dictionary.find(row[static_cast<std::size_t>(c)]);
            if (it == dictionary.end()) {
                continue;
            }
            const TileDef& def = it->second;
            const char key = it->first;
            Point2D pos(left + static_cast<float>(c) * tileW, top + static_cast<float>(r) * tileH);
            if (!def.hasBox) {
                ECSEngine::CreateTile(&engine, pos, def.sprite, bounds, ECSEngine::kLayerBackground);
            } else if (key == kSpawnerChar) {
                ECSEngine::CreateSpawner(&engine, pos, def.sprite, bounds, starSprite);
            } else {
                ECSEngine::CreateSolidTile(&engine, pos, def.sprite, bounds, def.box,
                                           key == kSpikeChar);
            }
        }
    }
    return true;
}
