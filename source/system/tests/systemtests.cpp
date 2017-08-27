#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <FileStream.h>
#include <MpqArchive.h>
#include <fstream>
#include "doctest.h"

using WorldStone::FileStream;
using WorldStone::MpqArchive;
using WorldStone::MpqFileStream;

namespace test_helpers
{

// We are not testing the MpqArchive here, just assume it works for some tests.
class MpqFileWrapper : public MpqFileStream
{
    MpqArchive archive;

public:
    MpqFileWrapper(const char* filename) : MpqFileStream(), archive("testArchive.mpq")
    {
        REQUIRE_MESSAGE(archive.good(), "This archive should be valid, wrong working directory ?");
        open(archive, filename);
    }
    ~MpqFileWrapper() { close(); }
};

// Small helper since we don't have the exactly the same API as the std library
template<class T>
size_t readAndReturnReadBytes(T& stream, char* buffer, size_t nbToRead)
{
    return stream.read(buffer, nbToRead);
}
template<>
size_t readAndReturnReadBytes(std::ifstream& stream, char* buffer, size_t nbToRead)
{
    return static_cast<size_t>(stream.read(buffer, nbToRead).gcount());
}
}
typedef doctest::Types<std::ifstream, WorldStone::FileStream, test_helpers::MpqFileWrapper>
    stream_types;
TYPE_TO_STRING(std::ifstream);
TYPE_TO_STRING(WorldStone::FileStream);
TYPE_TO_STRING(test_helpers::MpqFileWrapper);

TEST_CASE_TEMPLATE("  Scenario: "
                   "Filestream",
                   StreamType, stream_types)
{
    GIVEN("A file that does not exist")
    {
        const char* invalidFileName = "does-not-exist-file";
        WHEN("Trying to open it")
        {
            StreamType stream{invalidFileName};
            THEN("File is not opened and stream is invalid")
            {
                REQUIRE_FALSE(stream.is_open());
                CHECK_FALSE(stream.good());
                CHECK_FALSE(stream.bad());
                CHECK(stream.fail());
                CHECK_FALSE(stream.eof());
            }
            // Do not try to call anything else since it causes system API to exit the application.
            // We do not let the user call read if the file is invalid by design for performance
            // reasons Note that most calls are guarded by asserts anyway
        }
    }

    GIVEN("A file containing the word \"test\"")
    {
        const char* filename = "test.txt";
        WHEN("Opening the file containing \"test\"")
        {
            StreamType stream{filename};
            THEN("File is opened and stream is valid")
            {
                REQUIRE(stream.is_open());
                CHECK(stream.good());
                CHECK_FALSE(stream.bad());
                CHECK_FALSE(stream.fail());
                CHECK_FALSE(stream.eof());

                AND_WHEN("We read the whole file")
                {
                    char buffer[256] = {};
                    stream.read(buffer, 4);
                    THEN("The buffer contains the same things as the file")
                    {
                        CHECK(!strcmp("test", buffer));
                        CHECK_FALSE(stream.eof());
                        AND_WHEN("We read more")
                        {
                            stream.read(buffer, 1);
                            THEN("EOF and fail flags are set")
                            {
                                CHECK(stream.eof());
                                CHECK(stream.fail());
                                CHECK_FALSE(stream.bad());
                                CHECK_FALSE(stream.good());
                            }
                        }
                    }
                }
                AND_WHEN("We read more than the whole file")
                {
                    char                   buffer[256] = {};
                    size_t readCount = test_helpers::readAndReturnReadBytes(stream, buffer, 128);
                    THEN("EOF is reached and correct number of bytes read is reported")
                    {
                        CHECK(readCount == 4); // We know the content of the file is exactly "test"
                        CHECK(strncmp("test", buffer, 4) == 0);
                    }
                }
            }
        }
    }
}
