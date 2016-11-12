//
// Created by Lectem on 11/11/2016.
//

#pragma once

#include "Archive.h"

namespace WorldStone
{

class MpqArchive : public Archive
{
    using HANDLE = void*; // Do not expose stormlib
public:
    MpqArchive(const path& MpqFileName);
    ~MpqArchive() override;

    bool exists(const path& filePath) override;
    StreamPtr open(const path& filePath) override;

private:
    bool load() override;
    bool is_loaded() override;
    bool unload() override;

    path   mpqFileName;
    HANDLE mpqHandle = nullptr;
};
}
