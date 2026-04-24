#include "ShaderProgram.h"

ShaderProgram::ShaderProgram()
{
    mLoaded = false;
}

ShaderProgram::~ShaderProgram() { }

bool ShaderProgram::load(const std::string &vertexPath, const std::string &fragmentPath)
{
    mShader = LoadShader(vertexPath.c_str(), fragmentPath.c_str());
    mLoaded = true;
    return true;
}

void ShaderProgram::begin()
{
    if (mLoaded) BeginShaderMode(mShader);
}

void ShaderProgram::end()
{
    if (mLoaded) EndShaderMode();
}

void ShaderProgram::unload()
{
    if (mLoaded)
    {
        UnloadShader(mShader);
        mLoaded = false;
    }
}

void ShaderProgram::setVector2(const std::string &name, const Vector2 &value)
{
    if (!mLoaded) return;

    int location = GetShaderLocation(mShader, name.c_str());
    float values[2] = { value.x, value.y };
    SetShaderValue(mShader, location, values, SHADER_UNIFORM_VEC2);
}

void ShaderProgram::setFloat(const std::string &name, float value)
{
    if (!mLoaded) return;

    int location = GetShaderLocation(mShader, name.c_str());
    SetShaderValue(mShader, location, &value, SHADER_UNIFORM_FLOAT);
}

void ShaderProgram::setInt(const std::string &name, int value)
{
    if (!mLoaded) return;

    int location = GetShaderLocation(mShader, name.c_str());
    SetShaderValue(mShader, location, &value, SHADER_UNIFORM_INT);
}