#include "FileBrowser.h"
#include <SystemUtils.h>
#include <cof.h>
#include <dc6.h>
#include <dcc.h>
#include <fmt/format.h>
#include "DrawSprite.h"
#include "bx/debug.h"
#include "imgui/imgui_bgfx.h"

// The following should probably go to a PlatformIncludes.h
#ifdef WS_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

/**
 * @return The installation path of the game, or an empty path if unknown.
 */
static WorldStone::IOBase::Path GetInstallDirectory()
{
#ifdef WS_PLATFORM_WINDOWS
#define DIABLO2_KEY "Software\\Blizzard Entertainment\\Diablo II"
    const HKEY allowedKeys[] = {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE};
    BYTE       buffer[1024]  = {0};
    for (HKEY baseKey : allowedKeys)
    {
        HKEY    openedKey  = 0;
        LSTATUS statusCode = RegOpenKeyExA(baseKey, DIABLO2_KEY, 0, KEY_READ, &openedKey);
        if (statusCode == ERROR_SUCCESS) {
            DWORD valueType = 0;
            DWORD valueSize = sizeof(buffer);
            statusCode =
                RegQueryValueExA(openedKey, "InstallPath", 0, &valueType, buffer, &valueSize);
            RegCloseKey(openedKey);
            if (statusCode == ERROR_SUCCESS && valueType == REG_SZ) {
                return (const char*)buffer;
            }
        }
    }

#endif
    return {};
}

char const* const FileBrowser::mpqFiles[]  = {"d2char.mpq",  "d2data.mpq",  "d2exp.mpq",
                                             "d2music.mpq", "d2sfx.mpq",   "d2speech.mpq",
                                             "d2video.mpq", "d2xtalk.mpq", "d2xvideo.mpq"};
char const* const FileBrowser::listFiles[] = {"listfile.txt"};

FileBrowser::IFileView::~IFileView() {}

FileBrowser::~FileBrowser()
{
    // Has to be destroyed before the MpqArchive
    currentView = nullptr;
}

void FileBrowser::display(SpriteRenderer& spriteRenderer)
{
    displayMenuBar();
    ImGui::SetNextWindowSizeConstraints(ImVec2{300.0f, 250.f}, ImVec2{FLT_MAX, FLT_MAX});
    ImGui::Begin("Files", nullptr, 0 & ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(-1.f);

    if (fileListWidget.display()) {
        bx::debugPrintf("New file selected %s\n", fileListWidget.getSelectedElement());
        onFileSelected(fileListWidget.getSelectedElement());
    }
    ImGui::PopItemWidth();
    ImGui::End();

    if (currentView) currentView->display(spriteRenderer);
}

void FileBrowser::displayMenuBar()
{
    // See https://github.com/ocornut/imgui/issues/331
    const char* openPopupRequest = nullptr;
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {

                openPopupRequest = "Open file";
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (openPopupRequest) {
        if (mpqDirectory.size() == 0) {
            auto installDirectory = GetInstallDirectory();
            mpqDirectory          = installDirectory.size() ? installDirectory : "./";
        }
        ImGui::OpenPopup(openPopupRequest);
    }

    if (ImGui::BeginPopupModal("Open file", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Choose the MPQ file to open.");
        ImGui::Text("Installation path used:%s", mpqDirectory.c_str());
        static int item = 0;
        ImGui::Combo("Mpq File", &item, mpqFiles, sizeof(mpqFiles) / sizeof(*mpqFiles));

        if (ImGui::Button("Open")) {

            currentView    = nullptr;
            currentArchive = WorldStone::MpqArchive{(mpqDirectory + mpqFiles[item]).c_str(),
                                                    (mpqDirectory + listFiles[0]).c_str()};
            if (!currentArchive.good())
                currentArchive = WorldStone::MpqArchive{(mpqDirectory + mpqFiles[item]).c_str()};
            if (currentArchive.good()) {
                auto fileList = currentArchive.findFiles();
                std::sort(fileList.begin(), fileList.end());
                fileListWidget.replaceElements(std::move(fileList));
                ImGui::CloseCurrentPopup();
            }
            else
            {
                fileListWidget.replaceElements({});
                currentArchive = WorldStone::MpqArchive{};
                ImGui::OpenPopup("Error");
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();

        if (ImGui::BeginPopupModal("Error")) {
            ImGui::Text("Failed to open file %s", (mpqDirectory + mpqFiles[item]).c_str());
            if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::EndPopup();
    }
}

namespace
{

struct SpriteAnim
{

    SpriteRenderer::SpriteRenderDataHandle spriteDataHdl;

    ImVec2                    pos{200.f, 200.f};
    float                     scale           = 1.f;
    float                     animationTime   = 0.f;
    float                     framesPerSecond = 20.f;
    uint32_t                  nbFrames        = 0;
    WorldStone::AABB<int32_t> extents;

    // Options
    bool drawAABB  = true;
    bool drawGizmo = true;

    void Display(SpriteRenderer& spriteRenderer)
    {
        ImGui::InputFloat2("Translation", (float*)&pos);
        ImGui::InputFloat("Scale", &scale);

        float curFrameFloat = animationTime * framesPerSecond;

        ImGui::SliderFloat("FPS", &framesPerSecond, 0.f, 255.f);
        if (framesPerSecond > 0.f) {
            ImGui::Text("Current frame: %f ", double(curFrameFloat));
        }
        else
        {
            int sliderFrame = int(animationTime);
            ImGui::SliderInt("Current frame", &sliderFrame, 0, int(nbFrames - 1));
            sliderFrame   = std::min(std::max(sliderFrame, 0), int(nbFrames - 1));
            curFrameFloat = animationTime = float(sliderFrame);
        }

        assert(curFrameFloat >= 0);
        const uint32_t curFrame = uint32_t(curFrameFloat);
        assert(curFrame < nbFrames);

        { // Render
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->PushClipRectFullScreen();

            // Bounding box
            ImGui::Checkbox("Box", &drawAABB);
            if (drawAABB) {
                drawList->AddRect(
                    ImVec2{pos.x + extents.xLower * scale, pos.y + extents.yLower * scale},
                    ImVec2{pos.x + extents.xUpper * scale, pos.y + extents.yUpper * scale},
                    0xFFAAAAAA);
            }
            ImGui::SameLine();
            ImGui::Checkbox("Gizmo", &drawGizmo);
            if (drawGizmo) {
                const float gizmoSize = 10.f;
                drawList->AddLine(pos, ImVec2{pos.x + gizmoSize, pos.y}, 0xFF0000FF, 1.f);
                drawList->AddLine(pos, ImVec2{pos.x, pos.y + gizmoSize}, 0xFF00FF00, 1.f);

                drawList->AddCircle(pos, 20.f, 0xFF00FFFF);
            }
            drawList->PopClipRect();

            spriteRenderer.pushDrawRequest(spriteDataHdl, {{pos.x, pos.y}, curFrame, scale});
        }

        // Advance timer
        if (nbFrames && framesPerSecond > 0.f) {
            animationTime += ImGui::GetIO().DeltaTime;
            if (uint32_t(animationTime * framesPerSecond) >= nbFrames)
                animationTime = std::fmod(animationTime, nbFrames / framesPerSecond);
            assert(uint32_t(animationTime * framesPerSecond) < nbFrames);
        }
    }
};

struct DccView : public FileBrowser::IFileView
{

    DccView(const WorldStone::MpqArchive::Path& _filePath) : IFileView(_filePath) {}
    WorldStone::DCC                        dccFile;
    WorldStone::DCC::Direction             currentDir;
    int                                    currentDirIndex = 0;

    SpriteAnim spriteAnim;

    void display(SpriteRenderer& spriteRenderer) override
    {
        const auto& header = dccFile.getHeader();
        if (ImGui::Begin("DCC")) {
            IFileView::display();
            // clang-format off
                ImGui::Text("Header");
                ImGui::Separator();
                ImGui::Text("Version:"); ImGui::SameLine(); ImGui::Text("%hhu", header.version);
                ImGui::Text("Directions:"); ImGui::SameLine(); ImGui::Text("%hhu", header.directions);
                ImGui::Text("Frames per dir:"); ImGui::SameLine(); ImGui::Text("%hhu", header.framesPerDir);
                ImGui::Text("Tag:"); ImGui::SameLine(); ImGui::Text("%u", header.tag);
                ImGui::Separator();
            // clang-format on
        }

        bool dirChanged =
            ImGui::SliderInt("Direction", &currentDirIndex, 0, int(header.directions - 1));
        currentDirIndex = std::min(std::max(currentDirIndex, 0), int(header.directions - 1));
        if (spriteAnim.spriteDataHdl.expired() || dirChanged) {
            loadDirectionIntoAnim(uint32_t(currentDirIndex), spriteRenderer);
        }
        if (!spriteAnim.spriteDataHdl.expired()) {
            spriteAnim.Display(spriteRenderer);
        }
        else
        {
            ImGui::Separator();
            const ImVec4 red{1.f, 0.f, 0.f, 1.f};
            ImGui::TextColored(red, "Unable to decode direction %d", currentDirIndex);
        }
        ImGui::End();
    }
    void loadDirectionIntoAnim(uint32_t direction, SpriteRenderer& spriteRenderer)
    {
        spriteAnim = {}; // Reset the sprite animation

        spriteAnim.spriteDataHdl       = spriteRenderer.createSpriteRenderData();
        const auto spriteRenderDataPtr = spriteAnim.spriteDataHdl.lock();

        const auto& header = dccFile.getHeader();
        currentDir         = {};
        WorldStone::SimpleImageProvider<uint8_t> imageprovider;
        if (dccFile.readDirection(currentDir, direction, imageprovider)) {
            for (size_t i = 0; i < currentDir.frameHeaders.size(); i++)
            {
                const auto& frameHeader = currentDir.frameHeaders[i];
                spriteRenderDataPtr->addSpriteFrame(
                    {int16_t(frameHeader.extents.xLower), int16_t(frameHeader.extents.yLower),
                     uint16_t(frameHeader.extents.width()), uint16_t(frameHeader.extents.height()),
                     imageprovider.getImage(i).buffer});
            }
            spriteAnim.extents  = currentDir.extents;
            spriteAnim.nbFrames = header.framesPerDir;
        }
    }
};

struct Dc6View : public FileBrowser::IFileView
{
    Dc6View(const WorldStone::MpqArchive::Path& _filePath) : IFileView(_filePath) {}
    WorldStone::DC6 dc6File;
    SpriteAnim      spriteAnim;
    int             currentDirIndex = 0;

    void display(SpriteRenderer& spriteRenderer) override
    {
        const auto& header = dc6File.getHeader();
        if (ImGui::Begin("DC6")) {
            IFileView::display();
            // clang-format off
                ImGui::Text("Header");
                ImGui::Separator();
                ImGui::Text("Version:"); ImGui::SameLine(); ImGui::Text("%d", header.version);
                ImGui::Text("Flags:"); ImGui::SameLine(); ImGui::Text("%d", header.flags);
                ImGui::Text("Directions:"); ImGui::SameLine(); ImGui::Text("%u", header.directions);
                ImGui::Text("Frames per dir:"); ImGui::SameLine(); ImGui::Text("%u", header.framesPerDir);
                ImGui::Separator();
            // clang-format on
        }

        bool dirChanged =
            ImGui::SliderInt("Direction", &currentDirIndex, 0, int(header.directions - 1));
        currentDirIndex = std::min(std::max(currentDirIndex, 0), int(header.directions - 1));
        if (spriteAnim.spriteDataHdl.expired() || dirChanged) {
            loadDirectionIntoAnim(uint32_t(currentDirIndex), spriteRenderer);
        }
        if (!spriteAnim.spriteDataHdl.expired()) {
            spriteAnim.Display(spriteRenderer);
        }
        else
        {
            ImGui::Separator();
            const ImVec4 red{1.f, 0.f, 0.f, 1.f};
            ImGui::TextColored(red, "Unable to decode direction %d", currentDirIndex);
        }

        static bool warningIgnored = false;
        if (!warningIgnored && ImGui::BeginPopupModal("Warning")) {
            ImGui::Text("This file contains a lot of frames.\nThis is poorly handled right now and "
                        "might crash if you reload such files too quickly.");
            if (ImGui::Button("Ignore")) {
                warningIgnored = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void loadDirectionIntoAnim(uint32_t direction, SpriteRenderer& spriteRenderer)
    {
        spriteAnim = {}; // Reset the sprite animation

        spriteAnim.spriteDataHdl       = spriteRenderer.createSpriteRenderData();
        const auto spriteRenderDataPtr = spriteAnim.spriteDataHdl.lock();

        const auto&  header               = dc6File.getHeader();
        const auto&  frameHeaders         = dc6File.getFrameHeaders();
        const size_t directionFrameOffset = direction * header.framesPerDir;

        spriteAnim.extents.initializeForExtension();
        if (header.framesPerDir > 500) ImGui::OpenPopup("Warning");

        for (size_t frameIndex = 0; frameIndex < header.framesPerDir; frameIndex++)
        {
            const size_t                    frameIndexInFile = directionFrameOffset + frameIndex;
            const auto&                     frameHeader      = frameHeaders[frameIndexInFile];
            const WorldStone::AABB<int32_t> frameExtents{frameHeader.offsetX, frameHeader.offsetY,
                                                         frameHeader.offsetX + frameHeader.width,
                                                         frameHeader.offsetY + frameHeader.height};
            assert(frameExtents.width() == frameHeader.width);
            assert(frameExtents.height() == frameHeader.height);

            spriteAnim.extents.extend(frameExtents);
            auto frameData = dc6File.decompressFrame(frameIndexInFile);
            spriteRenderDataPtr->addSpriteFrame(
                {int16_t(frameExtents.xLower), int16_t(frameExtents.yLower),
                 uint16_t(frameExtents.width()), uint16_t(frameExtents.height()),
                 frameData.data()});

            spriteAnim.nbFrames = header.framesPerDir;
        }
    }
};

struct CofView : public FileBrowser::IFileView
{
    CofView(const WorldStone::MpqArchive::Path& _filePath) : IFileView(_filePath) {}
    using COF = WorldStone::COF;
    COF cofFile;
    int currentLayerIdx = 1;

    void display() override
    {
        const auto& header = cofFile.getHeader();
        if (ImGui::Begin("COF")) {
            IFileView::display();
            // clang-format off
                ImGui::Text("Header");
                ImGui::Separator();
                ImGui::Text("Layers:"); ImGui::SameLine(); ImGui::Text("%hhu", header.layers);
                ImGui::Text("Frames:"); ImGui::SameLine(); ImGui::Text("%hhu", header.frames);
                ImGui::Text("Directions:"); ImGui::SameLine(); ImGui::Text("%hhu", header.directions);
                ImGui::Text("Version:"); ImGui::SameLine(); ImGui::Text("%hhu", header.version);
                ImGui::NewLine();
                ImGui::Text("unknown dword(bytes 7-4):");
                {
                    const uint8_t byte0 = (header.unknown1 >> (8 * 0)) & 0xff;
                    const uint8_t byte1 = (header.unknown1 >> (8 * 1)) & 0xff;
                    const uint8_t byte2 = (header.unknown1 >> (8 * 2)) & 0xff;
                    const uint8_t byte3 = (header.unknown1 >> (8 * 3)) & 0xff;
                    ImGui::Text("0x%08x", header.unknown1);
                    ImGui::SameLine(); ImGui::ColorButton("##color", ImVec4{ byte3 / 255.f, byte2 / 255.f, byte1 / 255.f, byte0 / 255.f }, ImGuiColorEditFlags_NoPicker);
                    const std::string unkAsBin = fmt::format("{:08b} {:08b} {:08b} {:08b}", byte3, byte2, byte1, byte0);
                    ImGui::Text(unkAsBin.data());
                    const std::string unkAsInt = fmt::format("{:03} {:03} {:03} {:03}", byte3, byte2, byte1, byte0);
                    ImGui::Text(unkAsInt.data());
                }
                ImGui::NewLine();
                ImGui::Text("AABB:"); ImGui::SameLine(); ImGui::Text("(%d,%d) -> (%d,%d)", header.xMin, header.yMin, header.xMax, header.yMax);
                ImGui::Text("AnimRate:"); ImGui::SameLine(); ImGui::Text("%hd", header.animRate);
                ImGui::Text("zeros:"); ImGui::SameLine(); ImGui::Text("%hx", header.zeros);

            // clang-format on
            ImGui::Separator();
            ImGui::SliderInt("Layer", &currentLayerIdx, 1, header.layers);
            const COF::Layer& layer = cofFile.getLayers()[size_t(currentLayerIdx - 1)];
            ImGui::Text("Component:%s", layer.component < COF::componentsNumber
                                            ? COF::componentsNames[layer.component]
                                            : "Invalid");
            ImGui::Text("Casts shadow: %d", layer.castsShadow);
            ImGui::Text("Is selectable: %d", layer.isSelectable);
            ImGui::Text("Override transparency level: %d", layer.overrideTranslvl);
            ImGui::Text("New transparency level: %d", layer.newTranslvl);
            ImGui::Text("Weapon class: %s", layer.weaponClass);
        }
        ImGui::End();
    }
};

struct PaletteView final : public FileBrowser::IFileView
{
    using Palette = WorldStone::Palette;
    std::unique_ptr<Palette> palettePtr;
    bgfx::TextureHandle      paletteTexture = BGFX_INVALID_HANDLE;
    bool                     alreadySetAsCurrentPalette = false;

    PaletteView(const WorldStone::MpqArchive::Path& _filePath, std::unique_ptr<Palette> inPalette)
        : IFileView(_filePath), palettePtr(std::move(inPalette))
    {
        const Palette& palette = *palettePtr;
        static_assert(sizeof(WorldStone::Palette::Color24Bits) == 3, "");
        auto paletteRGB888 =
            bgfx::alloc(sizeof(WorldStone::Palette::Color24Bits) * WorldStone::Palette::colorCount);
        for (size_t i = 0; i < WorldStone::Palette::colorCount; i++)
        {
            const WorldStone::Palette::Color color                      = palette.colors[i];
            ((WorldStone::Palette::Color24Bits*)paletteRGB888->data)[i] = {color.r, color.g,
                                                                           color.b};
        }
        paletteTexture =
            bgfx::createTexture2D(256, 1, false, 1, bgfx::TextureFormat::RGB8,
                                  BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT, paletteRGB888);
    }
    ~PaletteView() { bgfx::destroy(paletteTexture); }
    void display(SpriteRenderer& spriteRenderer) override
    {
        ImGui::SetNextWindowSize(ImGui::GetContentRegionAvail(), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Palette")) {
            IFileView::display();
            if (!alreadySetAsCurrentPalette && ImGui::Button("Set as current palette")) {
                spriteRenderer.setPalette(*palettePtr);
                alreadySetAsCurrentPalette = true;
            }
            // Can add some more info here
            ImVec2 curWindowSize = ImGui::GetContentRegionAvail();
            ImGui::Image(paletteTexture, ImVec2{curWindowSize.x, curWindowSize.y});
        }
        ImGui::End();
    }
};

class PL2View final : public FileBrowser::IFileView
{
private:
    using Palette           = WorldStone::Palette;
    using PalShiftTransform = WorldStone::PalShiftTransform;
    using PL2               = WorldStone::PL2;
    std::unique_ptr<PL2> pl2;
    bgfx::TextureHandle  paletteTexture = BGFX_INVALID_HANDLE;
    size_t               textureWidth, textureHeight;

    int currentMin = 0;
    int currentMax = 0;

public:
    PL2View(const WorldStone::MpqArchive::Path& _filePath, std::unique_ptr<PL2> _pl2)
        : IFileView(_filePath), pl2(std::move(_pl2))
    {

        using WorldStone::Utils::Size;
        static_assert(sizeof(Palette::Color24Bits) == 3, "");

        // clangformat off
        textureWidth  = WorldStone::Palette::colorCount;
        textureHeight = 1 // pl2->basePalette
                        + Size(pl2->lightLevelVariations) + Size(pl2->invColorVariations)
                        + 1 // pl2->selectedUnitShift
                        + Size(pl2->alphaBlend) * Size(pl2->alphaBlend[0])
                        + Size(pl2->additiveBlend) + Size(pl2->multiplicativeBlend)
                        + Size(pl2->hueVariations) + 1 // pl2->redTones
                        + 1                            // pl2->greenTones
                        + 1                            // pl2->blueTones
                        + Size(pl2->unknownColorVariations) + Size(pl2->maxComponentBlend)
                        + 1 // pl2->darkenedColorShift
                        + Size(pl2->textColorShifts);
        currentMax = int(textureHeight - 1);
        // clangformat on

        const size_t textureSizeInBytes =
            sizeof(Palette::Color24Bits) * textureWidth * textureHeight;
        auto           paletteRGB888 = bgfx::alloc(uint32_t(textureSizeInBytes));
        const Palette& palette       = pl2->basePalette;
        // dstColors pointer will be incremented accordingly
        const auto palShiftToRGB = [&palette](const PalShiftTransform& transform,
                                              Palette::Color24Bits*&   dstColors) {
            for (uint8_t shiftIndex : transform.indices)
            {
                const Palette::Color color = palette.colors[shiftIndex];
                *(dstColors++)             = Palette::Color24Bits{color.r, color.g, color.b};
            }
        };
        Palette::Color24Bits* const textureDataBegin = (Palette::Color24Bits*)paletteRGB888->data;
        Palette::Color24Bits*       textureData      = textureDataBegin;
        // Copy the base palette
        for (const Palette::Color color : palette.colors)
        {
            *(textureData++) = {color.r, color.g, color.b};
        }

        // Copy the light variations
        for (const PalShiftTransform& transform : pl2->lightLevelVariations)
        {
            palShiftToRGB(transform, textureData);
        }
        for (const PalShiftTransform& transform : pl2->invColorVariations)
        {
            palShiftToRGB(transform, textureData);
        }
        palShiftToRGB(pl2->selectedUnitShift, textureData);
        for (const auto& alphaBlends : pl2->alphaBlend)
        {
            for (const PalShiftTransform& transform : alphaBlends)
            {
                palShiftToRGB(transform, textureData);
            }
        }
        for (const PalShiftTransform& transform : pl2->additiveBlend)
        {
            palShiftToRGB(transform, textureData);
        }
        for (const PalShiftTransform& transform : pl2->multiplicativeBlend)
        {
            palShiftToRGB(transform, textureData);
        }
        for (const PalShiftTransform& transform : pl2->hueVariations)
        {
            palShiftToRGB(transform, textureData);
        }
        palShiftToRGB(pl2->redTones, textureData);
        palShiftToRGB(pl2->greenTones, textureData);
        palShiftToRGB(pl2->blueTones, textureData);
        for (const PalShiftTransform& transform : pl2->unknownColorVariations)
        {
            palShiftToRGB(transform, textureData);
        }
        for (const PalShiftTransform& transform : pl2->maxComponentBlend)
        {
            palShiftToRGB(transform, textureData);
        }
        palShiftToRGB(pl2->darkenedColorShift, textureData);
        for (const PalShiftTransform& transform : pl2->textColorShifts)
        {
            palShiftToRGB(transform, textureData);
        }

        assert(textureData == textureDataBegin + textureWidth * textureHeight);
        paletteTexture = bgfx::createTexture2D(
            uint16_t(textureWidth), uint16_t(textureHeight), false, 1, bgfx::TextureFormat::RGB8,
            BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT, paletteRGB888);
    }

    ~PL2View() { bgfx::destroy(paletteTexture); }
    void display() override
    {
        ImGui::SetNextWindowSize(ImGui::GetContentRegionAvail(), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("PL2")) {
            IFileView::display();
            const int maxVal = int(textureHeight - 1);
            ImGui::DragIntRange2("Display range", &currentMin, &currentMax, 1.f, 0, maxVal);
            // Need to clamp, see issue https://github.com/ocornut/imgui/issues/1441
            currentMin = std::min(std::max(currentMin, 0), maxVal);
            currentMax = std::min(std::max(currentMax, currentMin), maxVal);

            ImVec2      curWindowSize = ImGui::GetContentRegionAvail();
            const float pixelSize     = 1.f / textureHeight;

            const float startY = currentMin * pixelSize;
            const float endY   = (currentMax + 1) * pixelSize;
            ImGui::Image(paletteTexture, ImVec2{curWindowSize.x, curWindowSize.y},
                         ImVec2{0.f, startY}, ImVec2{1.f, endY});
        }
        ImGui::End();
    }
};

} // anonymous namespace

static void toLowerCase(std::string& str)
{
    for (char& c : str)
    {
        c = char(tolower(c));
    }
}

void FileBrowser::onFileSelected(const char* fileName)
{
    std::string fileNameStr{fileName};
    size_t      lastDot = fileNameStr.find_last_of('.');
    if (lastDot != std::string::npos) {
        std::string extension = fileNameStr.substr(lastDot + 1);
        toLowerCase(extension);
        if (extension == "dcc") {
            auto dccView = std::make_unique<DccView>(fileNameStr);
            if (dccView->dccFile.initDecoder(currentArchive.open(fileNameStr))) {
                currentView = std::move(dccView);
            }
        }
        else if (extension == "dc6")
        {
            auto dc6View = std::make_unique<Dc6View>(fileNameStr);
            if (dc6View->dc6File.initDecoder(currentArchive.open(fileNameStr))) {
                currentView = std::move(dc6View);
            }
        }
        else if (extension == "cof")
        {
            auto cofView = std::make_unique<CofView>(fileNameStr);
            if (cofView->cofFile.read(currentArchive.open(fileNameStr))) {
                currentView = std::move(cofView);
            }
        }
        else if (extension == "dat")
        {
            auto palette     = std::make_unique<WorldStone::Palette>();
            auto paletteFile = currentArchive.open(fileNameStr);
            if (palette->decode(paletteFile.get())) {
                currentView = std::make_unique<PaletteView>(fileNameStr, std::move(palette));
            }
        }
        else if (extension == "pl2")
        {
            auto paletteFile = currentArchive.open(fileNameStr);
            auto pl2         = WorldStone::PL2::ReadFromStream(paletteFile.get());
            if (pl2) {
                currentView = std::make_unique<PL2View>(fileNameStr, std::move(pl2));
            }
        }
        else
        {
            currentView = nullptr;
        }
    }
}
