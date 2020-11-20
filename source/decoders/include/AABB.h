#pragma once
#include <algorithm>
#include <limits>

namespace WorldStone
{
/** Axis-Aligned Bounding Box
 * @tparam T The type to use for the positions
 * @note The members were named lower/upper in opposition to min/max so that we can use the same
 * code for both integers and float. For integers width = xMax - xMin + 1 as 'max' means included
 * while for floats width = xMax - xMin. (This is basicly the difference between counting pixels and
 * the distance between the pixels) As Upper means the bound is excluded, the data (and user) is
 * responsible for handling this difference, not the class. An alternative would have been to store
 * the width/height instead.
 */
template<typename T>
struct AABB
{
    T xLower; ///< x lower bound : excluded
    T yLower; ///< y lower bound : excluded
    T xUpper; ///< x upper bound : excluded
    T yUpper; ///< y upper bound : excluded

    T width() const { return xUpper - xLower; }
    T height() const { return yUpper - yLower; }

    void maximize()
    {
        xLower = std::numeric_limits<T>::lowest();
        yLower = std::numeric_limits<T>::lowest();
        xUpper = std::numeric_limits<T>::max();
        yUpper = std::numeric_limits<T>::max();
    }

    void initializeForExtension() { *this = AABB::getInitializedForExtension(); }

    void extend(const AABB& other)
    {
        xLower = std::min(xLower, other.xLower);
        yLower = std::min(yLower, other.yLower);
        xUpper = std::max(xUpper, other.xUpper);
        yUpper = std::max(yUpper, other.yUpper);
    }

    /** Used to initialize before computing the bounding box of objects.
     * Usage example:
     * @code
     * AABB extents = AABB::getInitializedForExtension;
     * extents.extend(someAABB); // Now equal to someAABB
     * extents.extend(otherAABB); // Now equal to the bounding box of someAABB and otherAABB
     * @endcode
     * @note The returned AABB is invalid unless you call extend or override its values.
     */
    static constexpr AABB getInitializedForExtension()
    {
        return {std::numeric_limits<T>::max(), std::numeric_limits<T>::max(),
                std::numeric_limits<T>::lowest(), std::numeric_limits<T>::lowest()};
    }
};
} // namespace WorldStone
