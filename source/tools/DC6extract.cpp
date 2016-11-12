#include <dc6.h>
#include <fmt/format.h>

int main(int argc, char* argv[])
{
    if (argc == 4) {
        Palette palette;
        palette.Decode(argv[2]);
        DC6 dc6;
        dc6.Decode(argv[1]);
        dc6.exportToPPM(argv[3], palette);
    }
    else
    {
        fmt::print("Usage : DC6extract file.dc6 palette.dat output\n");
        fmt::print(
            "This tool will extract all frames of dc6 file to output{direction}{frame}.ppm\n");
    }
    return 0;
}
