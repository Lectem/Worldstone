#include <FileStream.h>
#include <dc6.h>
#include <fmt/format.h>

using WorldStone::DC6;
using WorldStone::Palette;
using WorldStone::FileStream;

int main(int argc, char* argv[])
{
    if (argc == 4) {
        Palette palette;
        if (!palette.decode(argv[2])) {
            fmt::print("Couldn't read the palette file\n");
            return 1;
        }
        DC6 dc6;
        if (dc6.initDecoder(std::make_unique<FileStream>(argv[1]))) {
            int frameIndex = 0;
            for (auto& frameHeader : dc6.getFrameHeaders())
            {
                fmt::print("\nframe index {}\n", frameIndex);
                fmt::print("flip {}\n", frameHeader.flip);
                fmt::print("width {}\n", frameHeader.width);
                fmt::print("height {}\n", frameHeader.height);
                fmt::print("offsetX {}\n", frameHeader.offsetX);
                fmt::print("offsetY {}\n", frameHeader.offsetY);
                fmt::print("allocatedSize {}\n", frameHeader.allocSize);
                fmt::print("nextBlock {}\n", frameHeader.nextBlock);
                fmt::print("length {}\n", frameHeader.length);
            }
            dc6.exportToPPM(argv[3], palette);

            return 0;
        }
    }

    fmt::print("Usage : DC6extract file.dc6 palette.dat output\n");
    fmt::print("This tool will extract all frames of dc6 file to output[direction][frame].ppm\n");
    return 1;
}
