#include <dc6.h>
#include <fmt/format.h>

using WorldStone::DC6;
using WorldStone::Palette;

int main(int argc, char* argv[])
{
    if (argc == 4) {
        Palette palette;
        palette.Decode(argv[2]);
        DC6 dc6;
        if (dc6.Decode(argv[1])) {
            int frameIndex = 0;
            for (auto& frameHeader : dc6.getFameHeaders())
            {
                fmt::print("\nframe index {}\n", frameIndex);
                fmt::print("flip {}\n", frameHeader.flip);
                fmt::print("width {}\n", frameHeader.width);
                fmt::print("height {}\n", frameHeader.height);
                fmt::print("offset_x {}\n", frameHeader.offset_x);
                fmt::print("offset_y {}\n", frameHeader.offset_y);
                fmt::print("zeros {}\n", frameHeader.zeros);
                fmt::print("next_block {}\n", frameHeader.next_block);
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
