//
// Created by Lectem on 12/11/2016.
//

#include <FileStream.h>
#include <MpqArchive.h>
#include <fmt/format.h>
#include <fstream>

using namespace WorldStone;
int main(int argc, char* argv[])
{
    if (argc >= 4) {
        const char* mpqFilename   = argv[1];
        const char* fileToExtract = argv[2];
        MpqArchive  mpqArchive(mpqFilename);
        if (!mpqArchive.good()) fmt::print("Could not open {}\n", mpqFilename);
        if (!mpqArchive.exists(fileToExtract))
            fmt::print("The file {} was not found in {}\n", fileToExtract, mpqFilename);
        else
        {
            fmt::print("The file is in the MPQ !\n");
            StreamPtr     file = mpqArchive.open(fileToExtract);
            std::ofstream outFile(argv[3], std::ofstream::binary);
            if (!outFile) fmt::print("Couldn't create output file\n");
            while (file && file->good() && outFile)
            {
                char   buffer[1024];
                size_t readFromMpq = file->read(buffer, 1, sizeof(buffer));
                outFile.write(buffer, readFromMpq);
            }
        }
    }
    else
        fmt::print("MPQextract usage : MPQextract archive.mpq filetoextract outputfile\n");

    return 0;
}
