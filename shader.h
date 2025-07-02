#ifndef _SHADER_H_
#define _SHADER_H_

#include <stdint.h>
#include <string>

#include <glad/glad.h>

#include <glslang/Include/glslang_c_interface.h>

struct SpirVBinary
{
    uint32_t*         words; // SPIR-V words
    int               size;  // number of words in SPIR-V binary
    glslang_shader_t* shader;
};

class Shader
{
  public:
    Shader() = default;
    ~Shader();

    bool Load(const std::string& vertName, const std::string& fragName, uint32_t shaderFeatureBits);
    bool IsValidProgram();
    void Use();

  private:
    GLuint m_ShaderProgram;
    GLuint m_VertexShader;
    GLuint m_FragmentShader;
    GLuint m_ComputeShader;

    SpirVBinary m_VertexShaderSPIRV;
    SpirVBinary m_FragmentShaderSPIRV;
    SpirVBinary m_ComputeShaderSPIRV;
};

#endif
