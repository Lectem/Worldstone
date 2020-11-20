#pragma once
#include <stdint.h>
#include <Vector.h>
#include <Stream.h>

namespace WorldStone
{

/**
 * @brief Decoder for the DT1 file format
 * The DT1 format is used to store a collection of map tiles.
 * It contains both images and various information such as collisions and animations.
 * In the game, it is usually referred to as "Tile Project".
 */
class DT1 {
public:
    enum Flags : uint32_t{
        is24Bit = 1 << 0, ///< Used internally to know the number of bits per color
        unknownFlag0x2 = 1 << 1, ///< Used internally, seems related to validation of data
        isSerialized = 1 << 2, ///< Used internally to mark a project as serialized
        fewBlocks = 1 << 3, ///< Usage still not exactly known (TILEPROJECT_FEWBLOCKS in d2cmp.dll)
    };
    struct Header {
        uint32_t version;   ///< The game is using version 7, but we can find older versions in the files
        uint32_t flags;     /// @ref DT1::Flags
        char libraryName[260];      ///< Used internally to store the name as to load tiles only once
        uint32_t numTiles;  ///< Number of tiles in this tileset
        uint32_t firstTile; ///< Offset/Pointer to the first Tile

    };
    struct Tile {

        enum MaterialFlags : uint16_t {
            OTHER = 0x1,
            WATER = 0x2,
            WOOD_OBJ = 0x4,
            ISTONE = 0x8,
            OSTONE = 0x10,
            DIRT = 0x20,
            SAND = 0x40,
            WOOD = 0x80,
            LAVA = 0x100, // Special case: Always bright + animated
            SNOW = 0x400,
        };

        enum InternalFlags : uint8_t
        {
            TILETYPEFLAG_GAMESQAURE24 = 0x8,
            TILETYPEFLAG_0x10 = 0x10,
        };

        struct CollisionInfo
        {
            uint8_t colInfo[25];
        };


        struct Header {

            uint32_t lightDirection;
            uint16_t roofHeight;
            MaterialFlags materialFlags;
            uint32_t dwTotalHeight;
            uint32_t dwWidth;
            uint32_t dwHeightToBottom;
            uint32_t dwType;
            uint32_t dwIndex_Style;
            uint32_t dwSubIndex_Sequence;

            union
            {
                uint32_t rarity;
                uint32_t frame;
            };

            uint32_t transparentColorRGB24;
            CollisionInfo collisionInfo;
            uint8_t _padding;
            uint16_t hwHandle;
            uint16_t unkWord2;
            Tile::InternalFlags bTypeFlag;
            uint8_t unk2;
            uint32_t dwComponentsOffset;
            uint32_t dwComponentsSize;
            uint32_t nCompCount;
            ///@name runtime internals
            ///@{
            uint32_t ptCompArray;
            uint32_t szFileName;
            uint32_t ptLRUCache;
            ///@}

        };
        struct Component {

        };
    };

    /**Start decoding the stream and preparing data.
     * @return true on success
     *
     * Prepares the decoder to read the tiles (reads the headers).
     */
    bool initDecoder(StreamPtr&& streamPtr);


    const Header& getHeader() { return header; }
    const Vector<Tile::Header>& getTileHeaders() { return tileHeaders; }
private:

    bool readHeaders();

    StreamPtr                stream = nullptr;

    Header header;
    Vector<Tile::Header> tileHeaders;

    Vector<Tile> tiles;


};


} // namespace WorldStone
