#pragma once

#include <stdint.h>
#include <Stream.h>
#include <Vector.h>
#include <memory>
#include <type_traits>

namespace WorldStone
{
/**
 * @brief Decoder for the COF (Components Object File) file format.
 *
 * Layout of a COF file:
 *
 * Name        | Type                            | Size in bytes
 * ----------- | ------------------------------- | ------------------
 * header      | COF::Header                     | 32
 * layers      | COF::Layer[layers]              | 9 * header.layers
 * keyframes   | COF::Keyframe[frames]           | 1 * header.frames
 * layersOrder | uint8_t[dirs][frames][layers]   | header.directions * header.frames * header.layers
 *
 */
class COF
{
public:
    struct Header
    {
        uint8_t  layers;     ///< Number of components layers
        uint8_t  frames;     ///< Number of frames in the animation
        uint8_t  directions; ///< Number of directions in the animation
        uint8_t  version;    ///< The version of the COF, always 20 in the game files.
        uint32_t unknown1;   ///< Possible bitfield values : loopAnim / Overlay color when hit
        ///@name Bounding box
        ///@{
        int32_t xMin;
        int32_t xMax;
        int32_t yMin;
        int32_t yMax;
        ///@}
        int16_t animRate; ///< Animation rate(speed). 256
        int16_t zeros;    ///< Always zero
    };

    struct Layer
    {
        uint8_t nComponent;
        uint8_t bShadow;
        uint8_t bSelectable;
        uint8_t nOverrideTranslvl;
        uint8_t nNewTranslvl;
        char    szWeaponClass[4];
    };

    /// Frame trigger type
    enum Keyframe : uint8_t
    {
        COFKEY_NONE,
        COFKEY_ATTACK,
        COFKEY_MISSILE,
        COFKEY_SOUND,
        COFKEY_SKILL,
        COFKEY_MAX,
    };

    bool read(const StreamPtr& streamPtr);

    const Header&           getHeader() const { return header; }
    const Vector<Layer>&    getLayers() const { return layers; }
    const Vector<Keyframe>& getKeyframes() const { return keyframes; }
    const Vector<uint8_t>&  getAllLayersOrders() const { return layersOrder; }

    /// @return A pointer to the list of layers in the order of draw
    const uint8_t* getFrameLayerOrder(size_t direction, size_t frame) const
    {

        const size_t frameIdx = (direction * header.frames + frame) * header.layers;
        return &layersOrder[frameIdx];
    }
    /// @note This list of components comes from the file data\\global\\excel\\Composit.txt
    static constexpr const char* componentsNames[] = {"HD", "TR", "LG", "RA", "LA", "RH",
                                                      "LH", "SH", "S1", "S2", "S3", "S4",
                                                      "S5", "S6", "S7", "S8"};
    static constexpr uint8_t componentsNumber = std::extent<decltype(COF::componentsNames)>::value;

private:
    Header           header;
    Vector<Layer>    layers;
    Vector<Keyframe> keyframes;
    /**The draw order of the layers.
     * Each entry gives what layer component should be drawn, the layout is:
     * for each direction, for each frame, 1 entry per layer
     *
     */
    Vector<uint8_t> layersOrder;
};
} // namespace WorldStone
