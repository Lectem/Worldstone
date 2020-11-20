/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */
#pragma once
#include <bgfx/bgfx.h>
#include <bx/readerwriter.h>
bgfx::ShaderHandle  loadShader(const char* _name);
bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName);
bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName);
