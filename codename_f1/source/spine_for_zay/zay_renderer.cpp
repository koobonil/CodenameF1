#include "zay_types.h"
#include "zay_renderer.h"
#include "zay_render_command.h"
#include "zay_matrix4.h"

namespace ZAY
{
    Renderer::Renderer(void)
    {
        _viewportX = 0;
        _viewportY = 0;
        _viewportWidth = 1920;
        _viewportHeight = 1080;

        _mvpMatrix = Matrix4::IDENTITY;
    }

    Renderer::~Renderer(void)
    {
    }
    
    void Renderer::setViewport(int32_t viewportX, int32_t viewportY, int32_t viewportWidth, int32_t viewportHeight)
    {
        if (_viewportX != viewportX ||
            _viewportY != viewportY ||
            _viewportWidth != viewportWidth ||
            _viewportHeight != viewportHeight)
        {
            _viewportX = viewportX;
            _viewportY = viewportY;
            _viewportWidth = viewportWidth;
            _viewportHeight = viewportHeight;

            glViewport(_viewportX,
                       _viewportY,
                       _viewportWidth,
                       _viewportHeight);
        }
    }
    
    int32_t Renderer::getViewportX() const
    {
        return _viewportX;
    }
    
    int32_t Renderer::getViewportY() const
    {
        return _viewportY;
    }
    
    int32_t Renderer::getViewportWidth() const
    {
        return _viewportWidth;
    }
    
    int32_t Renderer::getViewportHeight() const
    {
        return _viewportHeight;
    }

    void Renderer::testGL()
    {
        auto errorCode = glGetError();
        
        if (errorCode != GL_NO_ERROR)
        {
            BOSS_ASSERT(BOSS::String::Format("SPINE_FOR_ZAY(GLError: %d)", errorCode), false);
        }
    }

    void Renderer::testShader(GLuint shader)
    {
        auto errorCode = glGetError();
        
        GLint status;
        BOSS_GL(GetShaderiv, shader, GL_COMPILE_STATUS, &status);

        if (status == GL_FALSE)
        {
            GLchar log[4096];
            GLsizei s;
            BOSS_GL(GetShaderInfoLog, shader, 4096, &s, log);
            BOSS_ASSERT(BOSS::String::Format("SPINE_FOR_ZAY(ShaderError: %s)", log), false);
        }
        else if(errorCode != GL_NO_ERROR)
            BOSS_ASSERT(BOSS::String::Format("SPINE_FOR_ZAY(GLError: %d)", errorCode), false);
    }
    
    void Renderer::testProgram(GLuint program)
    {
        auto errorCode = glGetError();

        if (errorCode != GL_NO_ERROR)
        {
            GLint linked;
            BOSS_GL(GetProgramiv, program, GL_LINK_STATUS, &linked);
            if (!linked)
            {
                int i32InfoLogLength, i32CharsWritten;
                BOSS_GL(GetProgramiv, program, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
                char* pszInfoLog = new char[i32InfoLogLength];
                BOSS_GL(GetProgramInfoLog, program, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
                
                BOSS_ASSERT(BOSS::String::Format("SPINE_FOR_ZAY(ProgramError: %s)", pszInfoLog), false);
                delete [] pszInfoLog;
            }
            BOSS_ASSERT(BOSS::String::Format("SPINE_FOR_ZAY(GLError: %d)", errorCode), false);
        }
    }

    void Renderer::setMVPMatrix(const Matrix4& matrix)
    {
        if (_mvpMatrix != matrix)
        {
            _mvpMatrix = matrix;
            _notifyMVPMatrixChanged();
        }
    }
    
    const Matrix4& Renderer::getMVPMatrix() const
    {
        return _mvpMatrix;
    }

    //bx
    void Renderer::setRenderMode(const float mode)
    {
        _setRenderMode(mode);
    }
}
