#include "shader.h"

#include <glad/glad.h>

#include <glslang/Include/glslang_c_interface.h>
// Required for use of glslang_default_resource
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/resource_limits_c.h>

#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "utils.h"

extern std::string g_BasePath;

Shader::~Shader() {}

glsl_include_result_t*
GLSL_IncludeLocalCB(void* ctx, const char* header_name, const char* includer_name, size_t include_depth)
{
    glsl_include_result_t* result = (glsl_include_result_t*)malloc(sizeof(glsl_include_result_t));

    std::string headerFilePath = g_BasePath + header_name;
    HKD_File    file{};
    hkd_read_file(headerFilePath.c_str(), &file);

    result->header_name = header_name;
    result->header_data = (char*)malloc(file.size);
    memcpy((char*)(result->header_data), (char*)file.data, file.size);
    result->header_length = (size_t)file.size;

    printf("Includer name: %s\n", includer_name);
    printf("Header name: %s\n", header_name);
    printf("---- result ----\n");
    printf("result->header_length: %lu\n", result->header_length);
    printf("result->header_name: %s\n", result->header_name);
    printf("result->header_data: %s\n", result->header_data);

    hkd_destroy_file(&file);

    return result;
}

static SpirVBinary CompileShaderToSPIRV_OpenGL(glslang_stage_t stage, const char* shaderSource, const char* fileName)
{
    SpirVBinary              result{};
    glsl_include_callbacks_t includeCBs{};
    includeCBs.include_local    = GLSL_IncludeLocalCB;
    const glslang_input_t input = { .language                          = GLSLANG_SOURCE_GLSL,
                                    .stage                             = stage,
                                    .client                            = GLSLANG_CLIENT_OPENGL,
                                    .client_version                    = GLSLANG_TARGET_OPENGL_450,
                                    .target_language                   = GLSLANG_TARGET_SPV,
                                    .target_language_version           = GLSLANG_TARGET_SPV_1_0,
                                    .code                              = shaderSource,
                                    .default_version                   = 460,
                                    .default_profile                   = GLSLANG_CORE_PROFILE,
                                    .force_default_version_and_profile = true,
                                    .forward_compatible                = true,
                                    .messages                          = GLSLANG_MSG_VULKAN_RULES_BIT,
                                    .resource                          = glslang_default_resource(),
                                    .callbacks                         = includeCBs };

    glslang_spv_options_t options{ .generate_debug_info                  = true,
                                   .strip_debug_info                     = false,
                                   .disable_optimizer                    = true,
                                   .optimize_size                        = false,
                                   .disassemble                          = false,
                                   .validate                             = true,
                                   .emit_nonsemantic_shader_debug_info   = true,
                                   .emit_nonsemantic_shader_debug_source = true,
                                   .compile_only                         = false,
                                   .optimize_allow_expanded_id_bound     = false };

    result.shader = glslang_shader_create(&input);

    if ( !glslang_shader_preprocess(result.shader, &input) )
    {
        printf("GLSL preprocessing failed %s\n", fileName);
        printf("%s\n", glslang_shader_get_info_log(result.shader));
        printf("%s\n", glslang_shader_get_info_debug_log(result.shader));
        //printf("%s\n", input.code);
        glslang_shader_delete(result.shader);
        exit(1);
        //return result;
    }

    if ( !glslang_shader_parse(result.shader, &input) )
    {
        printf("GLSL parsing failed %s\n", fileName);
        printf("%s\n", glslang_shader_get_info_log(result.shader));
        printf("%s\n", glslang_shader_get_info_debug_log(result.shader));
        //printf("%s\n", glslang_shader_get_preprocessed_code(result.shader ));
        glslang_shader_delete(result.shader);
        exit(1);
        //return result;
    }

    glslang_program_t* program = glslang_program_create();

    glslang_program_add_shader(program, result.shader);

    if ( !glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT) )
    {
        printf("GLSL linking failed %s\n", fileName);
        printf("%s\n", glslang_program_get_info_log(program));
        printf("%s\n", glslang_program_get_info_debug_log(program));
        glslang_program_delete(program);
        glslang_shader_delete(result.shader);
        exit(1);
        //return result;
    }

    glslang_program_SPIRV_generate_with_options(program, stage, &options);

    result.size  = glslang_program_SPIRV_get_size(program);
    result.words = (uint32_t*)malloc(result.size * sizeof(uint32_t));
    glslang_program_SPIRV_get(program, result.words);

    const char* spirv_messages = glslang_program_SPIRV_get_messages(program);
    if ( spirv_messages ) printf("(%s) %s\b", fileName, spirv_messages);

    // TODO: Why do they delete this here (example code)?
    //glslang_program_delete(program);
    //glslang_shader_delete(shader);

    return result;
}

SpirVBinary CompileShaderToSPIRV(glslang_stage_t stage, const std::string& fileName)
{
    std::string shaderFilePath = g_BasePath + fileName;
    HKD_File    shaderCode;
    if ( hkd_read_file(shaderFilePath.c_str(), &shaderCode) != HKD_FILE_SUCCESS )
    {
        printf("CompileShaderToSPIRV: Could not read file: %s\n", fileName.c_str());
        exit(66); // SEE: sysexits.h
    }

    return CompileShaderToSPIRV_OpenGL(stage, (char*)shaderCode.data, fileName.c_str());
}

bool Shader::Load(const std::string& vertName, const std::string& fragName, uint32_t shaderFeatureBits)
{
    m_VertexShaderSPIRV   = CompileShaderToSPIRV(GLSLANG_STAGE_VERTEX, vertName);
    m_FragmentShaderSPIRV = CompileShaderToSPIRV(GLSLANG_STAGE_FRAGMENT, fragName);

    m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1,
                   &m_VertexShader,
                   GL_SHADER_BINARY_FORMAT_SPIR_V,
                   m_VertexShaderSPIRV.words,
                   m_VertexShaderSPIRV.size * sizeof(uint32_t));
    glSpecializeShader(m_VertexShader, "main", 0, NULL, NULL);

    m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderBinary(1,
                   &m_FragmentShader,
                   GL_SHADER_BINARY_FORMAT_SPIR_V,
                   m_FragmentShaderSPIRV.words,
                   m_FragmentShaderSPIRV.size * sizeof(uint32_t));
    glSpecializeShader(m_FragmentShader, "main", 0, NULL, NULL);

    // TODO: Should only be done if explicitly requested, I guess.
    hkd_write_file((g_BasePath + vertName + ".spv").c_str(),
                   m_VertexShaderSPIRV.words,
                   sizeof(uint32_t),
                   m_VertexShaderSPIRV.size);
    hkd_write_file((g_BasePath + fragName + ".spv").c_str(),
                   m_FragmentShaderSPIRV.words,
                   sizeof(uint32_t),
                   m_FragmentShaderSPIRV.size);

    /*
    if ( !CompileShader(vertName, GL_VERTEX_SHADER, m_VertexShader)
         || !CompileShader(fragName, GL_FRAGMENT_SHADER, m_FragmentShader) )
    {

        return false;
    }
    */

    m_ShaderProgram = glCreateProgram();
    glAttachShader(m_ShaderProgram, m_VertexShader);
    glAttachShader(m_ShaderProgram, m_FragmentShader);
    glLinkProgram(m_ShaderProgram);

    if ( !IsValidProgram() ) return false;

    return true;
}

bool Shader::IsValidProgram()
{
    GLint status;
    glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &status);
    if ( status != GL_TRUE )
    {
        char buffer[ 512 ];
        memset(buffer, 0, 512);
        glGetProgramInfoLog(m_ShaderProgram, 512, nullptr, buffer);
        printf("GLSL compile error:\n%s\n", buffer);

        return false;
    }

    return true;
}

void Shader::Use()
{
    glUseProgram(m_ShaderProgram);
}
