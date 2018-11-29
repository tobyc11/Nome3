#include "Shader.h"

#include <fstream>

namespace tc
{

std::string ReadTextFile(const std::string& file)
{
	//TODO: address hardcoded path
	std::ifstream ifs("../../Application/Resources/" + file);
	if (!ifs)
		return std::string();

	std::string str;

	ifs.seekg(0, std::ios::end);
	str.reserve(ifs.tellg());
	ifs.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(ifs)),
			   std::istreambuf_iterator<char>());
	return str;
}

std::string FGLSLProgram::ShaderHeader = "#version 410\n";

FGLSLProgram::FGLSLProgram() : GLProgram(0)
{
}

FGLSLProgram::~FGLSLProgram()
{
    glDeleteProgram(GLProgram);
}

FGLSLProgram* FGLSLProgram::CreateFromFiles(const char* vertFilename, const char* fragFilename)
{
    FGLSLProgram* prog = new FGLSLProgram();
    if (!prog->SetSourceFromFiles(vertFilename, fragFilename))
    {
        delete prog;
        return nullptr;
    }
    return prog;
}

FGLSLProgram * FGLSLProgram::CreateFromFiles(const char * vertFilename, const char * fragFilename, const char * geomFilename)
{
    FGLSLProgram* prog = new FGLSLProgram();
    if (!prog->SetSourceFromFiles(vertFilename, fragFilename, geomFilename))
    {
        delete prog;
        return nullptr;
    }
    return prog;
}

FGLSLProgram* FGLSLProgram::CreateFromStrings(const char* vertSrc, const char* fragSrc)
{
    FGLSLProgram* prog = new FGLSLProgram();
    if (!prog->SetSourceFromStrings(vertSrc, fragSrc))
    {
        delete prog;
        return nullptr;
    }
    return prog;
}

FGLSLProgram* FGLSLProgram::CreateFromStrings(const char** vertSrcArray, int32_t vertSrcCount,
    const char** fragSrcArray, int32_t fragSrcCount)
{
    FGLSLProgram* prog = new FGLSLProgram();
    if (!prog->SetSourceFromStrings(vertSrcArray, vertSrcCount, fragSrcArray, fragSrcCount))
    {
        delete prog;
        return nullptr;
    }
    return prog;
}

bool FGLSLProgram::SetSourceFromFiles(const char* vertFilename, const char* fragFilename)
{
    auto vertSrc = ReadTextFile(vertFilename);
    auto fragSrc = ReadTextFile(fragFilename);
    bool success = SetSourceFromStrings(vertSrc.c_str(), fragSrc.c_str());
    return success;
}

bool FGLSLProgram::SetSourceFromFiles(const char* vertFilename, const char* fragFilename, const char* geomFilename)
{
    auto vertSrc = ReadTextFile(vertFilename);
    auto fragSrc = ReadTextFile(fragFilename);
    auto geomSrc = ReadTextFile(geomFilename);
    bool success = SetSourceFromStrings(vertSrc.c_str(), fragSrc.c_str());
    SetAdditionalShader(geomSrc.c_str(), GL_GEOMETRY_SHADER);
    return success;
}

bool FGLSLProgram::SetSourceFromStrings(const char* vertSrc, const char* fragSrc)
{
    if (GLProgram) {
        glDeleteProgram(GLProgram);
        GLProgram = 0;
    }
    GLProgram = CompileProgram(vertSrc, fragSrc);
    return GLProgram != 0;
}

bool FGLSLProgram::SetSourceFromStrings(const char** vertSrcArray, int32_t vertSrcCount,
    const char** fragSrcArray, int32_t fragSrcCount)
{
    if (GLProgram) {
        glDeleteProgram(GLProgram);
        GLProgram = 0;
    }
    GLProgram = CompileProgram(vertSrcArray, vertSrcCount, fragSrcArray, fragSrcCount);
    return GLProgram != 0;
}

void FGLSLProgram::SetAdditionalShader(const char* source, GLenum type)
{
    if (GetProgram() == 0)
        return;
    GLuint newShader = glCreateShader(type);

    const char* sourceItems[2];
    int sourceCount = 0;
    if (!ShaderHeader.empty())
        sourceItems[sourceCount++] = ShaderHeader.c_str();
    sourceItems[sourceCount++] = source;

    glShaderSource(newShader, sourceCount, sourceItems, 0);
    glCompileShader(newShader);
    if (!CheckCompileError(newShader, type))
        return;

    glAttachShader(GetProgram(), newShader);
    Relink();
}

void FGLSLProgram::Enable()
{
    glUseProgram(GLProgram);
}

void FGLSLProgram::Disable()
{
    glUseProgram(0);
}

GLint FGLSLProgram::GetAttribLocation(const char* attribute, bool isOptional)
{
    GLint result = glGetAttribLocation(GLProgram, attribute);
    return result;
}

GLint FGLSLProgram::GetUniformLocation(const char* uniform, bool isOptional)
{
    GLint result = glGetUniformLocation(GLProgram, uniform);
    return result;
}

void FGLSLProgram::BindTexture2D(const char *name, int32_t unit, GLuint tex)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform1i(loc, unit);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, tex);
    }
}

void FGLSLProgram::BindTexture2D(GLint index, int32_t unit, GLuint tex)
{
    glUniform1i(index, unit);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, tex);
}

void FGLSLProgram::BindTexture2DMultisample(const char *name, int32_t unit, GLuint tex)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform1i(loc, unit);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(0x9100/*GL_TEXTURE_2D_MULTISAMPLE*/, tex);
    }
}

void FGLSLProgram::BindTexture2DMultisample(GLint index, int32_t unit, GLuint tex)
{
    glUniform1i(index, unit);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(0x9100/*GL_TEXTURE_2D_MULTISAMPLE*/, tex);
}

void FGLSLProgram::BindTextureRect(const char *name, int32_t unit, GLuint tex)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform1i(loc, unit);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(0x84F5/*GL_TEXTURE_RECT*/, tex);
    }
}

void FGLSLProgram::BindTextureRect(GLint index, int32_t unit, GLuint tex)
{
    glUniform1i(index, unit);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(0x84F5/*GL_TEXTURE_RECT*/, tex);
}

void FGLSLProgram::BindTextureArray(const char *name, int32_t unit, GLuint tex)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform1i(loc, unit);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(0x8c1a, tex); // GL_TEXTURE_2D_ARRAY
    }
}

void FGLSLProgram::BindTextureArray(GLint index, int32_t unit, GLuint tex)
{
    glUniform1i(index, unit);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(0x8c1a, tex); // GL_TEXTURE_2D_ARRAY

}

void FGLSLProgram::SetUniform1i(const char *name, int32_t value)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform1i(loc, value);
    }
}

void FGLSLProgram::SetUniform1i(GLint index, int32_t value)
{
    if (index >= 0) {
        glUniform1i(index, value);
    }
}

void FGLSLProgram::SetUniform2i(const char *name, int32_t x, int32_t y)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform2i(loc, x, y);
    }
}

void FGLSLProgram::SetUniform2i(GLint index, int32_t x, int32_t y)
{
    if (index >= 0) {
        glUniform2i(index, x, y);
    }
}

void FGLSLProgram::SetUniform3i(const char *name, int32_t x, int32_t y, int32_t z)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform3i(loc, x, y, z);
    }
}

void FGLSLProgram::SetUniform3i(GLint index, int32_t x, int32_t y, int32_t z)
{
    if (index >= 0) {
        glUniform3i(index, x, y, z);
    }
}

void FGLSLProgram::SetUniform1f(const char *name, float value)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform1f(loc, value);
    }
}

void FGLSLProgram::SetUniform1f(GLint index, float value)
{
    if (index >= 0) {
        glUniform1f(index, value);
    }
}

void FGLSLProgram::SetUniform2f(const char *name, float x, float y)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform2f(loc, x, y);
    }
}

void FGLSLProgram::SetUniform2f(GLint index, float x, float y)
{
    if (index >= 0) {
        glUniform2f(index, x, y);
    }
}

void FGLSLProgram::SetUniform3f(const char *name, float x, float y, float z)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform3f(loc, x, y, z);
    }
}

void FGLSLProgram::SetUniform3f(GLint index, float x, float y, float z)
{
    if (index >= 0) {
        glUniform3f(index, x, y, z);
    }
}

void FGLSLProgram::SetUniform4f(const char *name, float x, float y, float z, float w)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform4f(loc, x, y, z, w);
    }
}

void FGLSLProgram::SetUniform4f(GLint index, float x, float y, float z, float w)
{
    if (index >= 0) {
        glUniform4f(index, x, y, z, w);
    }
}

void FGLSLProgram::SetUniform3fv(const char *name, const float *value, int32_t count)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform3fv(loc, count, value);
    }

}

void FGLSLProgram::SetUniform3fv(GLint index, const float *value, int32_t count)
{
    if (index >= 0) {
        glUniform3fv(index, count, value);
    }

}

void FGLSLProgram::SetUniform4fv(const char *name, const float *value, int32_t count)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniform4fv(loc, count, value);
    }
}

void FGLSLProgram::SetUniform4fv(GLint index, const float *value, int32_t count)
{
    if (index >= 0) {
        glUniform4fv(index, count, value);
    }
}

void FGLSLProgram::SetUniformMatrix3fv(const GLchar *name, const GLfloat *m, int32_t count, bool transpose)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniformMatrix3fv(loc, count, transpose, m);
    }
}

void FGLSLProgram::SetUniformMatrix3fv(GLint index, const GLfloat *m, int32_t count, bool transpose)
{
    if (index >= 0) {
        glUniformMatrix3fv(index, count, transpose, m);
    }
}

void FGLSLProgram::SetUniformMatrix4fv(const GLchar *name, const GLfloat *m, int32_t count, bool transpose)
{
    GLint loc = GetUniformLocation(name, false);
    if (loc >= 0) {
        glUniformMatrix4fv(loc, count, transpose, m);
    }
}

void FGLSLProgram::SetUniformMatrix4fv(GLint index, const GLfloat *m, int32_t count, bool transpose)
{
    if (index >= 0) {
        glUniformMatrix4fv(index, count, transpose, m);
    }
}

bool FGLSLProgram::Relink()
{
    glLinkProgram(GLProgram);

    // check if program linked
    GLint success = 0;
    glGetProgramiv(GLProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        GLint bufLength = 0;
        glGetProgramiv(GLProgram, GL_INFO_LOG_LENGTH, &bufLength);
        if (bufLength) {
            char* buf = new char[bufLength];
            if (buf) {
                glGetProgramInfoLog(GLProgram, bufLength, NULL, buf);
                LOGDEBUG("Could not link program:\n%s\n", buf);
                delete[] buf;
            }
        }
        return false;
    }
    return true;
}

bool FGLSLProgram::CheckCompileError(GLuint shader, int32_t target)
{
    // check if shader compiled
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        LOGINFO("Error compiling shader %d, target %d\n", shader, target);
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen) {
            char* buf = new char[infoLen];
            if (buf) {
                glGetShaderInfoLog(shader, infoLen, NULL, buf);
                LOGINFO("Shader log:\n%s\n", buf);
                delete[] buf;
            }
        }
        if (!compiled) {
            glDeleteShader(shader);
            return false;
        }
    }
    return true;
}

GLuint FGLSLProgram::CompileProgram(const char *vsource, const char *fsource)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    const char* sourceItems[2];
    int sourceCount = 0;
    if (!ShaderHeader.empty())
        sourceItems[sourceCount++] = ShaderHeader.c_str();
    sourceItems[sourceCount++] = vsource;

    glShaderSource(vertexShader, sourceCount, sourceItems, 0);

    sourceCount = 0;
    if (!ShaderHeader.empty())
        sourceItems[sourceCount++] = ShaderHeader.c_str();
    sourceItems[sourceCount++] = fsource;

    glShaderSource(fragmentShader, sourceCount, sourceItems, 0);

    glCompileShader(vertexShader);
    if (!CheckCompileError(vertexShader, GL_VERTEX_SHADER))
        return 0;

    glCompileShader(fragmentShader);
    if (!CheckCompileError(fragmentShader, GL_FRAGMENT_SHADER))
        return 0;

    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // can be deleted since the program will keep a reference
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glLinkProgram(program);

    // check if program linked
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        GLint bufLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
        if (bufLength) {
            char* buf = new char[bufLength];
            if (buf) {
                glGetProgramInfoLog(program, bufLength, NULL, buf);
                LOGDEBUG("Could not link program:\n%s\n", buf);
                delete[] buf;
            }
        }
        glDeleteProgram(program);
        program = 0;
    }

    return program;
}

GLuint FGLSLProgram::CompileProgram(
    const char** vertSrcArray, int32_t vertSrcCount,
    const char** fragSrcArray, int32_t fragSrcCount)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    {
        const char** sourceItems = new const char*[vertSrcCount + 1];
        int sourceCount = 0;
        if (!ShaderHeader.empty())
            sourceItems[sourceCount++] = ShaderHeader.c_str();

        for (int i = 0; i < vertSrcCount; i++)
            sourceItems[sourceCount++] = vertSrcArray[i];

        glShaderSource(vertexShader, sourceCount, sourceItems, 0);

        delete[] sourceItems;
    }

    {
        const char** sourceItems = new const char*[fragSrcCount + 1];
        int sourceCount = 0;
        if (!ShaderHeader.empty())
            sourceItems[sourceCount++] = ShaderHeader.c_str();

        for (int i = 0; i < fragSrcCount; i++)
            sourceItems[sourceCount++] = fragSrcArray[i];

        glShaderSource(fragmentShader, sourceCount, sourceItems, 0);

        delete[] sourceItems;
    }

    glCompileShader(vertexShader);
    if (!CheckCompileError(vertexShader, GL_VERTEX_SHADER))
        return 0;

    glCompileShader(fragmentShader);
    if (!CheckCompileError(fragmentShader, GL_FRAGMENT_SHADER))
        return 0;

    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    // can be deleted since the program will keep a reference
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glLinkProgram(program);

    // check if program linked
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        GLint bufLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
        if (bufLength) {
            char* buf = new char[bufLength];
            if (buf) {
                glGetProgramInfoLog(program, bufLength, NULL, buf);
                LOGDEBUG("Could not link program:\n%s\n", buf);
                delete[] buf;
            }
        }
        glDeleteProgram(program);
        program = 0;
    }

    return program;
}

}
