/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */
#include "bgfxUtils.h"
#include <bx/bx.h>
#include <bx/debug.h>
#include <bx/file.h>
#include <bx/readerwriter.h>

static const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath)
{
    if (bx::open(_reader, _filePath))
    {
        uint32_t            size = (uint32_t)bx::getSize(_reader);
        const bgfx::Memory* mem  = bgfx::alloc(size + 1);
        bx::read(_reader, mem->data, static_cast<int32_t>(size));
        bx::close(_reader);
        mem->data[mem->size - 1] = '\0';
        return mem;
    }

    bx::debugPrintf("Failed to load %s.\n", _filePath);
    return nullptr;
}

static bgfx::ShaderHandle loadShader(bx::FileReaderI* _reader, const char* _name)
{
    char filePath[512];

    const char* shaderPath = "???";

    switch (bgfx::getRendererType())
    {
    case bgfx::RendererType::Noop:
    case bgfx::RendererType::Direct3D9: shaderPath = "shaders/dx9/"; break;
    case bgfx::RendererType::Direct3D11:
    case bgfx::RendererType::Direct3D12: shaderPath = "shaders/dx11/"; break;
    case bgfx::RendererType::Gnm: shaderPath = "shaders/pssl/"; break;
    case bgfx::RendererType::Metal: shaderPath = "shaders/metal/"; break;
    case bgfx::RendererType::OpenGL: shaderPath = "shaders/glsl/"; break;
    case bgfx::RendererType::OpenGLES: shaderPath = "shaders/essl/"; break;
    case bgfx::RendererType::Vulkan: shaderPath = "shaders/spirv/"; break;

    case bgfx::RendererType::Count: BX_CHECK(false, "You should not be here!"); break;
    }

    bx::strCopy(filePath, BX_COUNTOF(filePath), shaderPath);
    bx::strCat(filePath, BX_COUNTOF(filePath), _name);
    bx::strCat(filePath, BX_COUNTOF(filePath), ".bin");

    bgfx::ShaderHandle handle = bgfx::createShader(loadMem(_reader, filePath));
    bgfx::setName(handle, filePath);

    return handle;
}

bgfx::ShaderHandle loadShader(const char* _name)
{
    bx::FileReader fileReader;
    return loadShader(&fileReader, _name);
}

bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName)
{
    bgfx::ShaderHandle vsh = loadShader(_reader, _vsName);
    bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
    if (nullptr != _fsName) { fsh = loadShader(_reader, _fsName); }

    return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
}

bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName)
{
    bx::FileReader fileReader;
    return loadProgram(&fileReader, _vsName, _fsName);
}
