//
// Created by Lectem on 12/11/2016.
//

#include <FileStream.h>
#include <MpqArchive.h>
#include <fmt/format.h>

using namespace WorldStone;
int main(int argc, char* argv[])
{
    if (argc >= 3) {
        const char* mpqFile       = argv[1];
        const char* fileToExtract = argv[2];
        (void)mpqFile;
        (void)fileToExtract;
    }
    MpqArchive mpqArchive("D:\\Program Files (x86)\\Diablo II\\d2data.mpq");
    if (mpqArchive.exists("data\\global\\excel\\TreasureClass.txt"))
        fmt::print("The file is in the MPQ !\n");

    return 0;
}
