#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <FileStream.h>
#include "doctest.h"

TEST_CASE("InexistantFile") {
    WorldStone::FileStream inexistantFile{"does-not-exist-file"};
    CHECK(!inexistantFile.is_open());
}
