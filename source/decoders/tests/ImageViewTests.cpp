/**
 * @file ImageViewTests.cpp
 * @brief Implementation of the tests for ImageView and IImageViewProviders
 */

#include <Platform.h>
#include <SystemUtils.h>
#include <doctest.h>
#include "ImageView.h"

using WorldStone::Vector;
using WorldStone::ImageView;
using WorldStone::SimpleImageProvider;

static_assert(std::is_copy_constructible<ImageView<uint8_t>>::value,
              "Image view copy must be possible");
static_assert(std::is_copy_assignable<ImageView<uint8_t>>::value,
              "Image view copy must be possible");
static_assert(std::is_convertible<ImageView<uint8_t>, ImageView<const uint8_t>>::value,
              "Must be able to convert to an ImageView to an ImageView of const values.");

static_assert(std::is_assignable<decltype(ImageView<uint8_t>().operator()(0, 0)), uint8_t>::value,
              "Must be able to assign values using operator()(size_t,size_t)");

using ImmutableImageView = ImageView<const uint8_t>;
static_assert(
    !std::is_assignable<decltype(ImmutableImageView().operator()(0, 0)), uint8_t>::value,
    "Must not be able to assign using operator()(size_t,size_t) on an ImageView<const Color>");

TEST_CASE("ImageView")
{
    const size_t    size      = 10_z;
    const uint8_t   initColor = 0;
    Vector<uint8_t> buffer(size * size, initColor);
    SUBCASE("Using a valid view")
    {
        ImageView<uint8_t> view;
        CHECK(!view.isValid());

        CHECK(view.buffer == nullptr);
        view.buffer = buffer.data();
        CHECK(!view.isValid());

        CHECK(view.width == 0);
        view.width = size;
        CHECK(!view.isValid());

        CHECK(view.height == 0);
        view.height = size;
        CHECK(!view.isValid());

        CHECK(view.stride == 0);
        view.stride = view.width;
        CHECK(view.isValid());

        CHECK(view == ImageView<uint8_t>{buffer.data(), size, size, size});
        CHECK(view != ImageView<uint8_t>{});

        for (int x = 0; x < size; x++)
        {
            for (int y = 0; y < size; y++)
            {
                CHECK(view(x, y) == initColor);
            }
        }
        const uint8_t testColor = 5;
        view(0, 0) = testColor;
        CHECK(buffer[0] == testColor);
        CHECK(view(0, 0) == testColor);
        view(0, 1) = testColor;
        CHECK(buffer[0 + 1 * size] == testColor);
        CHECK(view(0, 1) == testColor);

        const ImageView<const uint8_t> viewOnConst = view;
        CHECK(viewOnConst(0, 1) == testColor);
    }
    SUBCASE("Check invalid views")
    {
        ImageView<uint8_t> view;
        CHECK(!view.isValid());
        // Invalid when any of the fields is null
        view = {nullptr, size, size, size};
        CHECK(!view.isValid());
        view = {buffer.data(), 0, size, size};
        CHECK(!view.isValid());
        view = {buffer.data(), size, 0, size};
        CHECK(!view.isValid());
        view = {buffer.data(), size, size, 0};
        CHECK(!view.isValid());
        // Invalid if stride is more than width
        view = {buffer.data(), size + 1, size, size};
        CHECK(!view.isValid());
    }
}

TEST_CASE("SimpleImageViewProvider")
{
    SimpleImageProvider<uint8_t> imageProvider;
    CHECK(imageProvider.getImagesNumber() == 0);
    SUBCASE("Asking for an image of valid dimensions")
    {
        ImageView<uint8_t> newImageView = imageProvider.getNewImage(256, 128);
        CHECK(newImageView.isValid());
        CHECK(newImageView.width == 256);
        CHECK(newImageView.height == 128);
        REQUIRE(imageProvider.getImagesNumber() == 1);
        CHECK(imageProvider.getImage(0) == newImageView);
        newImageView = imageProvider.getNewImage(1024, 2048);
        CHECK(newImageView.width == 1024);
        CHECK(newImageView.height == 2048);
        REQUIRE(imageProvider.getImagesNumber() == 2);

        // Make sure we can move data from the provider
        Vector<uint8_t> imageBuffer = imageProvider.moveImageBuffer(1);
        CHECK(imageBuffer.size() >= newImageView.width * newImageView.height);
        CHECK(imageBuffer.data() == newImageView.buffer);
        // Make sure that the views returned after moving are not valid anymore
        if (imageProvider.getImagesNumber() == 2) {
            CHECK(!imageProvider.getImage(1).isValid());
        }
    }
    SUBCASE("Asking for an image of invalid dimensions returns invalid ImageView")
    {
        ImageView<uint8_t> invalidImageView = imageProvider.getNewImage(256, 0);
        CHECK(!invalidImageView.isValid());
        invalidImageView = imageProvider.getNewImage(0, 256);
        CHECK(!invalidImageView.isValid());
        CHECK(imageProvider.getImagesNumber() == 0);
    }
}
