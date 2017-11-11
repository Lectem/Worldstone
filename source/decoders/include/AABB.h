#pragma once
#include <algorithm>
#include <limits>

namespace WorldStone
{
/** Axis-Aligned Bounding Box
 * @tparam T The type to use for the positions
 */
template<typename T>
struct AABB
{
    T xMin;
    T yMin;
    T xMax;
    T yMax;

    T width() const { return xMax - xMin; }
    T height() const { return yMax - yMin; }

    void maximize()
    {
        xMin = std::numeric_limits<T>::lowest();
        yMin = std::numeric_limits<T>::lowest();
        xMax = std::numeric_limits<T>::max();
        yMax = std::numeric_limits<T>::max();
    }

    void initializeForExtension() { *this = AABB::getInitializedForExtension(); }

    void extend(const AABB& other)
    {
        xMin = std::min(xMin, other.xMin);
        yMin = std::min(yMin, other.yMin);
        xMax = std::max(xMax, other.xMax);
        yMax = std::max(yMax, other.yMax);
    }

    /** Used to initialize before computing the bounding box of objects.
     * Usage example:
     * @code
     * AABB extents = AABB::getInitializedForExtension;
     * extents.extend(someAABB); // Now equal to someAABB
     * @endcode
     */
    static constexpr AABB getInitializedForExtension()
    {
        return {std::numeric_limits<T>::max(), std::numeric_limits<T>::max(),
                std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest()};
    }
};
}
