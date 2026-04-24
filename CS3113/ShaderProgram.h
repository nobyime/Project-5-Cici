#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "raylib.h"
#include <string>

class ShaderProgram
{
private:
    Shader mShader;
    bool mLoaded;

public:
    ShaderProgram();
    ~ShaderProgram();

    bool load(const std::string &vertexPath, const std::string &fragmentPath);
    void begin();
    void end();
    void unload();

    void setVector2(const std::string &name, const Vector2 &value);
    void setFloat(const std::string &name, float value);
    void setInt(const std::string &name, int value);

    Shader &getShader() { return mShader; }
    bool isLoaded() const { return mLoaded; }
};

#endif