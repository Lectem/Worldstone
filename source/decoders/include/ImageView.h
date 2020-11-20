/**@file ImageView.h
 * Implements image manipulation helpers
 */
#pragma once

#include <Platform.h>
#include <Vector.h>
#include <string.h> // memcpy & memset

namespace WorldStone
{
/**A view on an image buffer of type Color.
 * This class can be used to access a buffer of the Color type as if it was an image.
 * It should be small enough so that you can directly copy the view instead of passing it by
 * reference. Note that this class does not do bounds checking, and the user is the one responsible
 * for handling the data correctly.
 * A good way to abstract an image allocator could be to implement an @ref IImageProvider, which
 * returns an ImageView.
 * @note Unlike the usage of the term 'view' in the standard, such as in std::string_view, an
 *       ImageView maps on a mutable buffer unless the template parameter is const.
 *       What this means is that a gsl::span is the equivalent of ImageView<uint8_t>
 *       while a std::view would be ImageView<const uint8_t> for example.
 */
template<class Color>
struct ImageView
{
    static_assert(std::is_trivially_copyable<Color>::value,
                  "Color must be trivially copyable for memcpy/memset");
    Color* buffer = nullptr; ///< Pointer to the memory considered as the first pixel
    size_t width  = 0;       ///< Width of the image, can be dfferent from the one of the buffer
    size_t height = 0;       ///< Number of scanlines of the image
    size_t stride = 0;       ///< Actual width of the buffer scanlines

    ImageView() = default;

    ImageView(Color* _buffer, size_t _width, size_t _height, size_t _stride)
        : buffer(_buffer), width(_width), height(_height), stride(_stride)
    {
    }

    /// Checks if the view seems to be valid based on its characteristics
    bool isValid() const { return buffer && width && height && width <= stride; }

    bool operator==(const ImageView& rhs) const
    {
        return buffer == rhs.buffer && width == rhs.width && height == rhs.height
               && stride == rhs.stride;
    }
    bool operator!=(const ImageView& rhs) const { return !(*this == rhs); }

    /// Enable conversion from ImageView<Color> to ImageView<const Color>
    operator ImageView<const Color>() const { return {buffer, width, height, stride}; }

    /**Create an ImageView that is contained by the current view.
     * @param xOffset   First column of the subview
     * @param yOffset   First row of the subview
     * @param subWidth  Number of columns, must verify @code xOffset + subWidth <= width @endcode
     * @param subHeight Number of rows, must verify @code yOffset + subHeight > height @endcode
     * @return A subview of given characteristics, or an invalid view if not contained by the
     * current one
     */
    ImageView subView(size_t xOffset, size_t yOffset, size_t subWidth, size_t subHeight) const
    {
        if (xOffset + subWidth > width || yOffset + subHeight > height) return {};
        return {buffer + xOffset + yOffset * stride, subWidth, subHeight, stride};
    }

    /**Access the pixel at the given coordinates.
     * @return A reference to the pixel of coordinates (x,y)
     * @warning This function does not perform any bounds checking,
     *          the only valid inputs verify @code x < width && y < height @endcode
     */
    Color& operator()(size_t x, size_t y) const { return buffer[x + y * stride]; }

    /**Copy (blits) the content of an image to another.
     * If the current view and destination memory overlap, result is undefined.
     * @warning No bounds checking done.
     */
    void copyTo(ImageView destination) const
    {
        for (size_t y = 0; y < height; y++)
        {
            memcpy(destination.buffer + y * destination.stride, buffer + y * stride,
                   width * sizeof(Color));
        }
    }

    /**Fills a part of the image.
     * @param x          First column to fill
     * @param y          First row to fill
     * @param columns    Number of pixels to fill per scanline
     * @param rows       Number of scanlines to fill
     * @param colorValue The value to set every byte of the pixels to
     * @see fillBytes for a faster version when all the bytes have the same value
     */
    void fill(size_t x, size_t y, size_t columns, size_t rows, Color colorValue)
    {
        Color* lineStart = buffer + x + y * stride;
        for (size_t row = 0; row < rows; ++row)
        {
            for (size_t column = 0; column < columns; column++)
            {
                lineStart[column] = colorValue;
            }
            lineStart += stride;
        }
    }

    /**Fills a part of the image using a byte pattern
     * @param x         First column to fill
     * @param y         First row to fill
     * @param columns   Number of pixels to fill per scanline
     * @param rows      Number of scanlines to fill
     * @param byteValue The value to set every byte of the pixels to
     */
    void fillBytes(size_t x, size_t y, size_t columns, size_t rows, uint8_t byteValue)
    {
        Color* dstPtr = buffer + x + y * stride;
        for (size_t row = 0; row < rows; ++row)
        {
            memset(dstPtr, byteValue, columns * sizeof(Color));
            dstPtr += stride;
        }
    }
};

/**An interface of a class that can provide images views.
 * One example would be to reuse the same texture to store multiple images.
 */
template<class Color>
class IImageProvider
{
public:
    /// Returns an ImageView of dimensions width * height to be used by the consumer.
    virtual ImageView<Color> getNewImage(size_t width, size_t height) = 0;
    virtual ~IImageProvider() {}
};

/// A simple image provider that allocates a new buffer for each call to getNewImage()
template<class Color>
class SimpleImageProvider : public IImageProvider<Color>
{
    struct Image
    {
        size_t        width  = 0;
        size_t        height = 0;
        Vector<Color> buffer;
        Image(size_t _width, size_t _height)
            : width(_width), height(_height), buffer(_width * _height)
        {
        }
    };
    Vector<Image> images;

public:
    /// Allocates a new Image of dimensions width * height
    ImageView<Color> getNewImage(size_t width, size_t height) override
    {
        if (!width || !height) return {};
        // if (!width || !height) return ImageView<Color>::getInvalidView();
        images.emplace_back(width, height);
        return {images.back().buffer.data(), width, height, width};
    }

    /// @return The number of images allocated
    size_t getImagesNumber() const { return images.size(); }

    /// @return An ImageView of the imageIndex-th image allocated.
    ImageView<Color> getImage(size_t imageIndex)
    {
        Image& img = images[imageIndex];
        return {img.buffer.data(), img.width, img.height, img.width};
    }
    /// @overload ImageView<const Color> getImage(size_t) const
    ImageView<const Color> getImage(size_t imageIndex) const
    {
        const Image& img = images[imageIndex];
        return {img.buffer.data(), img.width, img.height, img.width};
    }

    /**Move an image buffer out of the provider.
     * @param imageIndex The index of the image to return, in order of allocation.
     * @note  This means further calls to getImage(imageIndex) will return an invalid ImageView.
     */
    Vector<Color> moveImageBuffer(size_t imageIndex)
    {
        // We could swap with back() to free memory, but that would invalidate indices.
        Image& img = images[imageIndex];
        img.width  = 0;
        img.height = 0;
        return std::move(img.buffer);
    }
};
} // namespace WorldStone
