/**
* @file FileStreamTests.cpp
*/

#include <FileStream.h>
#include <MpqArchive.h>
#include <fstream>
#include <string.h>
#include "doctest.h"

using WorldStone::FileStream;
using WorldStone::MpqArchive;
using WorldStone::MpqFileStream;
using WorldStone::StreamPtr;

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
}
typedef doctest::Types<WorldStone::FileStream, test_helpers::MpqFileWrapper> stream_types;

TYPE_TO_STRING(WorldStone::FileStream);
TYPE_TO_STRING(test_helpers::MpqFileWrapper);

/// @testimpl{WorldStone::IStream,RO_filestreams}
SCENARIO_TEMPLATE("Read-only filestreams", StreamType, stream_types)
{
    GIVEN("A file that does not exist")
    {
        const char* invalidFileName = "does-not-exist-file";
        WHEN("Trying to open it")
        {
            StreamType           stream{invalidFileName};
            WorldStone::IStream& streamRef = stream; // Test it through the interface
            THEN("File is not opened and stream is invalid")
            {
                REQUIRE_FALSE(stream.is_open()); // Not in IStream, but present for files
                CHECK_FALSE(streamRef.good());
                CHECK_FALSE(streamRef.bad());
                CHECK(streamRef.fail());
                CHECK_FALSE(streamRef.eof());
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
            StreamType           stream{filename};
            WorldStone::IStream& streamRef = stream; // Test it through the interface
            THEN("File is opened and stream is valid")
            {
                REQUIRE(stream.is_open()); // Not in IStream, but present for files
                CHECK(streamRef.good());
                CHECK_FALSE(streamRef.bad());
                CHECK_FALSE(streamRef.fail());
                CHECK_FALSE(streamRef.eof());
                CHECK(streamRef.tell() == 0);
                const size_t fileSize    = streamRef.size();
                const char*  fileContent = "test";
                CHECK(fileSize == strlen(fileContent));

                AND_WHEN("We read the whole file")
                {
                    char buffer[256] = {};
                    CHECK(fileSize < 256);
                    streamRef.read(buffer, fileSize);
                    THEN("The buffer contains the same things as the file")
                    {
                        CHECK(!strcmp("test", buffer));
                        CHECK_FALSE(streamRef.eof());
                        CHECK(streamRef.tell() == fileSize);
                        AND_WHEN("We read more")
                        {
                            streamRef.read(buffer, 1);
                            THEN("EOF and fail flags are set")
                            {
                                CHECK(streamRef.eof());
                                CHECK(streamRef.fail());
                                CHECK_FALSE(streamRef.bad());
                                CHECK_FALSE(streamRef.good());
                            }
                        }
                    }
                }
                AND_WHEN("We read the whole file using getc")
                {
                    char buffer[256] = {};
                    CHECK(fileSize < 256);
                    for (size_t i = 0; i < fileSize; i++)
                    {
                        int val = streamRef.getc();
                        CHECK(val >= 0);
                        buffer[i] = static_cast<char>(val);
                    }
                    THEN("The buffer contains the same things as the file")
                    {
                        CHECK(!strcmp("test", buffer));
                        CHECK_FALSE(streamRef.eof());
                        CHECK(streamRef.tell() == fileSize);
                        AND_WHEN("We read more")
                        {
                            int val = streamRef.getc();
                            CHECK(val < 0);
                            THEN("EOF and fail flags are set")
                            {
                                CHECK(streamRef.eof());
                                CHECK(streamRef.fail());
                                CHECK_FALSE(streamRef.bad());
                                CHECK_FALSE(streamRef.good());
                            }
                        }
                    }
                }
                AND_WHEN("We read more than the whole file")
                {
                    char buffer[256] = {};
                    CHECK(fileSize * 2 < 256);
                    size_t readCount = streamRef.read(buffer, fileSize * 2);
                    THEN("EOF is reached and correct number of bytes read is reported")
                    {
                        // We know the content of the file is exactly "test"
                        CHECK(readCount == fileSize);
                        CHECK(strncmp("test", buffer, fileSize) == 0);
                    }
                }
                AND_WHEN("You seek past the end of file")
                {
                    streamRef.seek(10, WorldStone::IStream::end);
                    THEN("There is no failure") // That's the case for fseek since you can start
                                                // writing at a given offset
                    {
                        CHECK_FALSE(streamRef.fail());
                        CHECK_FALSE(streamRef.bad());
                        CHECK(streamRef.good());
                    }
                }
                AND_WHEN("You seek before the file beginning")
                {
                    streamRef.seek(-1, WorldStone::IStream::beg);
                    THEN("Fail flag is set") { CHECK(streamRef.fail()); }
                }
            }
        }
    }
}
