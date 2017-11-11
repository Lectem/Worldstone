#include <dc6.h>
#include <fmt/format.h>

using WorldStone::DC6;
using WorldStone::Palette;

int main(int argc, char* argv[])
{
    if (argc == 4) {
        Palette palette;
        palette.decode(argv[2]);
        DC6 dc6;
        if (dc6.decode(argv[1])) {
            int frameIndex = 0;
            for (auto& frameHeader : dc6.getFameHeaders())
            {
                fmt::print("\nframe index {}\n", frameIndex);
                fmt::print("flip {}\n", frameHeader.flip);
                fmt::print("width {}\n", frameHeader.width);
                fmt::print("height {}\n", frameHeader.height);
                fmt::print("offsetX {}\n", frameHeader.offsetX);
                fmt::print("offsetY {}\n", frameHeader.offsetY);
                fmt::print("zeros {}\n", frameHeader.zeros);
                fmt::print("nextBlock {}\n", frameHeader.nextBlock);
                fmt::print("length {}\n", frameHeader.length);
            }
            dc6.exportToPPM(argv[3], palette);
        }
    }
    else
    {
        fmt::print("Usage : DC6extract file.dc6 palette.dat output\n");
        fmt::print(
            "This tool will extract all frames of dc6 file to output[direction][frame].ppm\n");
    }
    return 0;
}
