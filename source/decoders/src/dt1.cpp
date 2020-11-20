#include "dt1.h"
#include <SystemUtils.h>
#include <assert.h>

struct TileProjectHeader
{
    uint32_t dwVersion;
    uint32_t dwFlags;
    char szFileName1[4];
    uint32_t field_10[16];
    uint32_t compArraySize;
    uint32_t nCompCount;
    char *ptCompArray;
    char *szFileName;
    void *ptLRUCache;
    uint32_t unkbis[43];
    uint32_t nNumTiles;
};

namespace WorldStone
{

    bool DT1::initDecoder(StreamPtr&& streamPtr)
    {
        assert(!stream);
        stream = std::move(streamPtr);
        if (stream && stream->good()) {
            return readHeaders();
        }
        return false;
    }

    bool DT1::readHeaders()
    {

        static_assert(std::is_trivially_copyable<Header>(), "DT1::Header must be trivially copyable");
        static_assert(Utils::StaticCheckSize<DT1::Header,276>(), "The DT1 file header size is 276 bytes");
        stream->read(&header, sizeof(header));
        if (stream->fail()) return false;

        if (header.version != 7) return false;
        tileHeaders.resize(header.numTiles);
        static_assert(Utils::StaticCheckSize<DT1::Tile::Header, 96>(), "The tiles header size is 96 bytes");
        for (Tile::Header& tileHeader : tileHeaders)
        {
            stream->read(&tileHeader, sizeof(tileHeader));
        }
        return stream->good();
    }


} // namespace WorldStone
