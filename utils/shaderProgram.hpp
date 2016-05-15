/**
 * shader program, singleton
 *
 * author: alei  mailto:rayingecho@hotmail.com
 */
#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#include <stdlib.h>
#include <string>
using namespace std;

class ShaderProgram {
public:
    static ShaderProgram& init(string vertexShaderPath, string fragmentShaderPath);
    
    GLuint getUniform(string uniformName) const;

    GLuint getProgramId() const { return _programId; }

    void finalize();
private:
    static ShaderProgram* _instance;

    GLuint _programId;

    // singleton, forbid instantiate from client.
    ShaderProgram() {}
};

#endif