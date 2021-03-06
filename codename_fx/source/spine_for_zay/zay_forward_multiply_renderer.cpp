﻿#include "zay_types.h"
#include "zay_forward_multiply_renderer.h"
#include "zay_render_command.h"
#include "zay_texture2d.h"

#include <boss.hpp>

namespace ZAY
{
    namespace ForwardMultiplyRender
    {
        ForwardMultiplyRenderCommand::ForwardMultiplyRenderCommand()
        {
        }
        
        ForwardMultiplyRenderCommand::~ForwardMultiplyRenderCommand()
        {
        }

        TrianglesMeshRenderCommand::TrianglesMeshRenderCommand()
        {
            _texture = nullptr;

            _blendType = BlendType::Alpha;

            _multiplyMapMultiplier = 1.0f;
        }
        
        TrianglesMeshRenderCommand::~TrianglesMeshRenderCommand()
        {
            if (_texture)
            {
                _texture->release();
                _texture = nullptr;
            }
        }


        void TrianglesMeshRenderCommand::setTexture(Texture2D* texture)
        {
            if (_texture != texture)
            {
                if (_texture)
                {
                    _texture->release();
                }
                
                _texture = texture;
                
                if (_texture)
                {
                    _texture->retain();
                }
            }
        }
        
        Texture2D* TrianglesMeshRenderCommand::getTexture() const
        {
            return _texture;
        }
        
        void TrianglesMeshRenderCommand::setBlendType(BlendType blendType)
        {
            _blendType = blendType;
        }
        
        BlendType TrianglesMeshRenderCommand::getBlendType() const
        {
            return _blendType;
        }

        ArrayBuffer<ZAY_Vertice>& TrianglesMeshRenderCommand::getVerticesBuffer()
        {
            return _verticesBuffer;
        }

        const ArrayBuffer<ZAY_Vertice>& TrianglesMeshRenderCommand::getVerticesBuffer() const
        {
            return _verticesBuffer;
        }

        ArrayBuffer<GLushort>& TrianglesMeshRenderCommand::getIndicesBuffer()
        {
            return _indicesBuffer;
        }

        const ArrayBuffer<GLushort>& TrianglesMeshRenderCommand::getIndicesBuffer() const
        {
            return _indicesBuffer;
        }

        void TrianglesMeshRenderCommand::setMultiplyMapMultiplier(float multiplier)
        {
            _multiplyMapMultiplier = multiplier;
        }
        
        float TrianglesMeshRenderCommand::getMultiplyMapMultiplier() const
        {
            return _multiplyMapMultiplier;
        }

        
        
        
        
        
        
        
        MultiplyMapRenderCommand::MultiplyMapRenderCommand()
        {
            _texture = nullptr;
        }

        MultiplyMapRenderCommand::~MultiplyMapRenderCommand()
        {
            if (_texture)
            {
                _texture->release();
                _texture = nullptr;
            }
        }

        void MultiplyMapRenderCommand::setTexture(Texture2D* texture)
        {
            if (_texture != texture)
            {
                if (_texture)
                {
                    _texture->release();
                }
                
                _texture = texture;
                
                if (_texture)
                {
                    _texture->retain();
                }
            }
        }

        Texture2D* MultiplyMapRenderCommand::getTexture() const
        {
            return _texture;
        }

        ArrayBuffer<ZAY_Vertice>& MultiplyMapRenderCommand::getVerticesBuffer()
        {
            return _verticesBuffer;
        }
        
        ArrayBuffer<GLushort>& MultiplyMapRenderCommand::getIndicesBuffer()
        {
            return _indicesBuffer;
        }
        
        
        
        
        
        ForwardMultiplyRenderer::ForwardMultiplyRenderer()
        {
            _vbo = 0;
            _ibo = 0;
            
            _multiplyMapWidth = 2048;
            _multiplyMapHeight = 1024;
            _multiplyMapBorder = 1;
            _needToRegenerateMultiplyMap = false;

            _firstFBO = 0; //bx
            _currentFBO = 0;
            
            _multiplyPassFBO = 0;
            
            // render texture
            _multiplyMapRenderTexture = 0;
            
            // white texture
            _whiteTexture = nullptr;
            
            // multiply map pass shader
            //_multiplyMapPassProgram = 0;
            //_multiplyMapPassVertexShader = 0;
            //_multiplyMapPassFragmentShader = 0;
            //_multiplyMapPassLocationTexture = 0;
            //_multiplyMapPassLocationMVPMatrix = 0;
            //_multiplyMapPassVAO = 0;
            
            // final(multiply) pass shader
            _finalPassProgram = 0;
            _finalPassVertexShader = 0;
            _finalPassFragmentShader = 0;
            //_finalPassLocationMultiplyMapTexture = 0;
            //_finalPassLocationMultiplyMapMultiplier = 0;
            _finalPassLocationColorTexture = 0;
            _finalPassLocationMVPMatrix = 0;
            _finalPassVAO = 0;

            //bx
            _finalPassRenderModeLocation = 0;
            _finalPassRenderMode = 1.0;

            _currentTexture = nullptr;
            _currentBlendType = BlendType::DEFAULT;
            //_currentMultiplyMapMultiplier = 0.0f;

            //bx
            _forcedBlendType = BlendType::DEFAULT;

            _verticesCapacity = 16 * 1024;
            _vertices = new ZAY_Vertice[_verticesCapacity];
            _verticesCount = 0;

            _indicesCapacity = 16 * 1024;
            _indices = new GLushort[_indicesCapacity];
            _indicesCount = 0;
        }
        

        ForwardMultiplyRenderer::~ForwardMultiplyRenderer()
        {
            clearRenderCommands();

            if (_finalPassVAO)
            {
#ifdef ZAY_RENDER_API_OPENGL3
                glDeleteVertexArrays(1, &_finalPassVAO);
                _finalPassVAO = 0;
#endif
            }

            if (_vbo)
            {
                BOSS_GL(DeleteBuffers, 1, &_vbo);
                _vbo = 0;
            }
            
            if (_ibo)
            {
                BOSS_GL(DeleteBuffers, 1, &_ibo);
                _ibo = 0;
            }
            
            if (_currentTexture)
            {
                _currentTexture->release();
                _currentTexture = nullptr;
            }
            
            if (_vertices)
            {
                delete [] _vertices;
                _vertices = nullptr;
            }
            _verticesCapacity = 0;
            _verticesCount = 0;
            
            if (_indices)
            {
                delete [] _indices;
                _indices = nullptr;
            }
            _indicesCapacity = 0;
            _indicesCount = 0;

            term();
        }

        void ForwardMultiplyRenderer::setMultiplyMapWidth(int32_t width)
        {
            if (_multiplyMapWidth != width)
            {
                _multiplyMapWidth = width;
                
                _needToRegenerateMultiplyMap = true;
            }
        }
        
        void ForwardMultiplyRenderer::setMultiplyMapHeight(int32_t height)
        {
            if (_multiplyMapHeight != height)
            {
                _multiplyMapHeight = height;
                
                _needToRegenerateMultiplyMap = true;
            }
        }
        
        void ForwardMultiplyRenderer::setMultiplyMapBorder(int32_t border)
        {
            if (_multiplyMapBorder != border)
            {
                _multiplyMapBorder = border;
            }
        }
        
        int32_t ForwardMultiplyRenderer::getMultiplyMapWidth() const
        {
            return _multiplyMapWidth;
        }
        
        int32_t ForwardMultiplyRenderer::getMultiplyMapHeight() const
        {
            return _multiplyMapHeight;
        }
        
        int32_t ForwardMultiplyRenderer::getMultiplyMapBorder() const
        {
            return _multiplyMapBorder;
        }

        void ForwardMultiplyRenderer::init()
        {
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_firstFBO);
            _currentFBO = _firstFBO; //bx
            testGL();

            
            // multiply map
            glGenTextures(1, &_multiplyMapRenderTexture);
            testGL();
            
            glBindTexture(GL_TEXTURE_2D, _multiplyMapRenderTexture);
            testGL();
            
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, _multiplyMapWidth, _multiplyMapHeight, 0, GL_RGB, GL_FLOAT, NULL);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _multiplyMapWidth, _multiplyMapHeight, 0, GL_RGB, GL_HALF_FLOAT, NULL);
            testGL();
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            testGL();
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            testGL();
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            testGL();
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            testGL();
            
            _needToRegenerateMultiplyMap = false;

            
            
            
            
            
            // primitive texture
            GLubyte whiteTextureImage[4] = { 0xff, 0xff, 0xff, 0xff };

            _whiteTexture = new Texture2D();
            _whiteTexture->retain();
            
            _whiteTexture->initWithData(whiteTextureImage,
                                        4,
                                        PixelFormat::RGBA8888,
                                        1,
                                        1);
            TexParams params;
            params.wrapS = GL_REPEAT;
            params.wrapT = GL_REPEAT;
            params.minFilter = GL_NEAREST;
            params.magFilter = GL_NEAREST;
            _whiteTexture->setTexParameters(params);
            
            
            
            
            // create multiply map FBO
            if (_multiplyPassFBO == 0)
            {
                BOSS_GL(GenFramebuffers, 1, &_multiplyPassFBO);
                testGL();
                
                BOSS_GL(BindFramebuffer, GL_FRAMEBUFFER, _multiplyPassFBO);
                testGL();
                
                BOSS_GL(FramebufferTexture2D, GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _multiplyMapRenderTexture, 0);
                testGL();

                //bx:GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
                //bx:BOSS_GL(DrawBuffers, 1, drawBuffers);
                //bx:testGL();
            }
            
            BOSS_GL(BindFramebuffer, GL_FRAMEBUFFER, _currentFBO);
            
            checkAndRemakeShaders();
        }
        
        void ForwardMultiplyRenderer::term()
        {
            if (_multiplyPassFBO != 0)
            {
                BOSS_GL(DeleteFramebuffers, 1, &_multiplyPassFBO);
                testGL();
                
                _multiplyPassFBO = 0;
            }

            if (_multiplyMapRenderTexture != 0)
            {
                glDeleteTextures(1, &_multiplyMapRenderTexture);
                testGL();
                
                _multiplyMapRenderTexture = 0;
            }

            if (_whiteTexture)
            {
                _whiteTexture->release();
                _whiteTexture = nullptr;
            }
            
            
            /*if (_multiplyMapPassProgram != 0)
            {
                BOSS_GL(DeleteProgram, _multiplyMapPassProgram);
                testGL();
                
                _multiplyMapPassProgram = 0;
            }

            if (_multiplyMapPassVertexShader != 0)
            {
                BOSS_GL(DeleteShader, _multiplyMapPassVertexShader);
                testGL();
                
                _multiplyMapPassVertexShader = 0;
            }
            
            if (_multiplyMapPassFragmentShader != 0)
            {
                BOSS_GL(DeleteShader, _multiplyMapPassFragmentShader);
                testGL();
                
                _multiplyMapPassFragmentShader = 0;
            }

            _multiplyMapPassLocationTexture = 0;
            _multiplyMapPassLocationMVPMatrix = 0;*/
            
            
            
            if (_finalPassProgram != 0)
            {
                BOSS_GL(DeleteProgram, _finalPassProgram);
                testGL();
                
                _finalPassProgram = 0;
            }

            if (_finalPassVertexShader != 0)
            {
                BOSS_GL(DeleteShader, _finalPassVertexShader);
                testGL();
                
                _finalPassVertexShader = 0;
            }

            if (_finalPassFragmentShader != 0)
            {
                BOSS_GL(DeleteShader, _finalPassFragmentShader);
                testGL();
                
                _finalPassFragmentShader = 0;
            }

            //_finalPassLocationMultiplyMapTexture = 0;
            //_finalPassLocationMultiplyMapMultiplier = 0;
            _finalPassLocationColorTexture = 0;
            _finalPassLocationMVPMatrix = 0;

            //bx
            _finalPassRenderModeLocation = 0;
            _finalPassRenderMode = 1.0;
        }
        
        Texture2D* ForwardMultiplyRenderer::getWhiteTexture() const
        {
            return _whiteTexture;
        }

        void ForwardMultiplyRenderer::_notifyMVPMatrixChanged()
        {
            /*if (_multiplyMapPassProgram != 0)
            {
                BOSS_GL(UseProgram, _multiplyMapPassProgram);
                testGL();
                
                BOSS_GL(UniformMatrix4fv, _multiplyMapPassLocationMVPMatrix, 1, GL_FALSE, (const GLfloat*)getMVPMatrix()._getRaws());
                testGL();
            }*/
            
            if (_finalPassProgram != 0)
            {
                BOSS_GL(UseProgram, _finalPassProgram);
                testGL();
                
                BOSS_GL(UniformMatrix4fv, _finalPassLocationMVPMatrix, 1, GL_FALSE, (const GLfloat*)getMVPMatrix()._getRaws());
                testGL();
            }
        }

        void ForwardMultiplyRenderer::setOriginalFBO(GLint fbo)
        {
            _currentFBO = (fbo == -1)? _firstFBO : fbo;
        }

        GLint ForwardMultiplyRenderer::getOriginalFBO() const
        {
            return _currentFBO;
        }

        //bx
        void ForwardMultiplyRenderer::_setRenderMode(const float mode)
        {
            if (_finalPassRenderMode != mode)
            {
                _finalPassRenderMode = mode;
                if (_finalPassProgram != 0)
                {
                    BOSS_GL(UseProgram, _finalPassProgram);
                    testGL();
                
                    BOSS_GL(Uniform1f, _finalPassRenderModeLocation, _finalPassRenderMode);
                    testGL();
                }
            }
        }

        //bx
        uint08 ForwardMultiplyRenderer::_getVersion()
        {
            static uint08 Version = 0x00;
            if(Version != 0x00) return Version;

            chars VendorString = (chars) glGetString(GL_VENDOR);
            chars RendererString = (chars) glGetString(GL_RENDERER);
            chars VersionString = (chars) glGetString(GL_VERSION);
            // 예시1: OpenGL ES 2.0 IMGSGX543-124.1
            // 예시2: OpenGL ES 3.0 APPLE-12.0.38
            // 예시3: 2.1 ATI-1.51.8
            // 예시4: 4.0.0 - Build 10.18.10.4303

            if(!boss_strncmp(VersionString, "OpenGL ES ", 10))
            {
                const uint32 VersionMajor = VersionString[10] - '0';
                const uint32 VersionMinor = VersionString[12] - '0';
                Version = ((VersionMajor & 0xF) << 4) | (VersionMinor & 0xF);
            }
            else if(!boss_strncmp(VersionString, "2.1 ATI-1.51.8", 7)) Version = 0x20;
            else if(!boss_strncmp(VersionString, "4.0.0 - Build 10.18.10.4303", 13)) Version = 0x40;
            else BOSS_ASSERT(String::Format("알 수 없는 버전정보(%s)입니다", VersionString), false);
            return Version;
        }

        void ForwardMultiplyRenderer::checkAndRemakeShaders()
        {
#ifdef  USE_BUFFER_OBJECT
            if (_vbo == 0 ||
                glIsBuffer(_vbo) == GL_FALSE)
            {
                glGenBuffers(1, &_vbo);
                glBindBuffer(GL_ARRAY_BUFFER, _vbo);
                glBufferData(GL_ARRAY_BUFFER, _verticesCapacity * sizeof(ZAY_Vertice), NULL, GL_DYNAMIC_DRAW);
            }
            else
            {
                glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            }

            if (_ibo == 0 ||
                glIsBuffer(_ibo) == GL_FALSE)
            {
                glGenBuffers(1, &_ibo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indicesCapacity * sizeof(GLushort), NULL, GL_DYNAMIC_DRAW);
            }
            else
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
            }
#endif//USE_BUFFER_OBJECT

            if (_finalPassVertexShader == 0 ||
                BOSS_GL(IsShader, _finalPassVertexShader) == GL_FALSE)
            {
                const char* s_vshFinalPassVertexShaderSourceCode20 =
                    "uniform float u_renderMode;\n"
                    "uniform mat4 u_mvpMat;\n"
                    "attribute vec4 a_position;\n"
                    "attribute vec2 a_texCoord;\n"
                    "attribute vec4 a_color;\n"
                    "varying vec2 v_texCoord;\n"
                    "varying vec4 v_fragmentColor;\n"
                    "\n"
                    "void main()\n"
                    "{\n"
                    "    v_texCoord = a_texCoord;\n"
                    "    if(u_renderMode == 0.0)\n" // 일반모드
                    "    {\n"
                    "        v_fragmentColor = a_color;\n"
                    "        gl_Position = u_mvpMat * vec4(a_position.x, a_position.y, 0.0, 1.0);\n"
                    "    }\n"
                    "    else if(u_renderMode < 1.0)\n" // 그림자모드
                    "    {\n"
                    #if BOSS_ANDROID
                        "        v_fragmentColor = vec4(0.0, 0.0, 0.0, 0.25);\n"
                    #else
                        "        v_fragmentColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
                    #endif
                    "        gl_Position = u_mvpMat * vec4(a_position.x + a_position.y * (u_renderMode - 0.5), a_position.y * -0.25, 0.0, 1.0);\n"
                    "    }\n"
                    "    else\n" // 외곽선모드
                    "    {\n"
                    "        if(a_color.w < 0.8)\n"
                    "            v_fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
                    "        else v_fragmentColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
                    "        vec2 n = normalize(a_position.zw);\n"
                    "        vec4 n_pos = u_mvpMat * vec4(n.x, n.y, 0.0, 1.0);\n"
                    "        vec4 a_pos = u_mvpMat * vec4(a_position.x, a_position.y, 0.0, 1.0);\n"
                    "        gl_Position = vec4(a_pos.x + n_pos.x * 17.5, a_pos.y + n_pos.y * 17.5, a_pos.z, a_pos.w);\n"
                    "    }\n"
                    "}\n";

                //if(_getVersion() == 0x20)
                {
                    _finalPassVertexShader = BOSS_GL(CreateShader, GL_VERTEX_SHADER);
                    testGL();
                    BOSS_GL(ShaderSource, _finalPassVertexShader, 1,
                        (const char**) &s_vshFinalPassVertexShaderSourceCode20, NULL);
                    testGL();
                    BOSS_GL(CompileShader, _finalPassVertexShader);
                    testShader(_finalPassVertexShader);
                }
                //else BOSS_ASSERT("지원되는 OpenGLES버전이 아닙니다", false);
            }

            if (_finalPassFragmentShader == 0 ||
                BOSS_GL(IsShader, _finalPassFragmentShader) == GL_FALSE)
            {
                const char* s_vshFinalPassFragmentShaderSourceCode20 =
                    "#ifdef GL_ES\n"
                    "    precision mediump float;\n"
                    "    precision mediump int;\n"
                    "#endif\n"
                    "uniform sampler2D u_colorTexture;\n"
                    "varying vec2 v_texCoord;\n"
                    "varying vec4 v_fragmentColor;\n"
                    "\n"
                    "void main()\n"
                    "{\n"
                    "    gl_FragColor = v_fragmentColor * texture2D(u_colorTexture, v_texCoord);\n"
                    "}\n";

                //if(_getVersion() == 0x20)
                {
                    _finalPassFragmentShader = BOSS_GL(CreateShader, GL_FRAGMENT_SHADER);
                    testGL();
                    BOSS_GL(ShaderSource, _finalPassFragmentShader, 1,
                        (const char**) &s_vshFinalPassFragmentShaderSourceCode20, NULL);
                    testGL();
                    BOSS_GL(CompileShader, _finalPassFragmentShader);
                    testShader(_finalPassFragmentShader);
                }
                //else BOSS_ASSERT("지원되는 OpenGLES버전이 아닙니다", false);
            }
            
            if (_finalPassProgram == 0 ||
                BOSS_GL(IsProgram, _finalPassProgram) == GL_FALSE)
            {
                _finalPassProgram = BOSS_GL(CreateProgram);
                testGL();

                BOSS_GL(AttachShader, _finalPassProgram, _finalPassVertexShader);
                testShader(_finalPassVertexShader);

                BOSS_GL(AttachShader, _finalPassProgram, _finalPassFragmentShader);
                testShader(_finalPassFragmentShader);

#ifdef  USE_BUFFER_OBJECT
                if (_finalPassVAO == 0 ||
                    glIsVertexArray(_finalPassVAO) == GL_FALSE)
                {
                    glGenVertexArrays(1, &_finalPassVAO);
                    glBindVertexArray(_finalPassVAO);
                }
#endif//USE_BUFFER_OBJECT
                
                BOSS_GL(EnableVertexAttribArray, VertexAttrib::Position);
                testGL();
                
                BOSS_GL(EnableVertexAttribArray, VertexAttrib::Color);
                testGL();
                
                BOSS_GL(EnableVertexAttribArray, VertexAttrib::TexCoord);
                testGL();

                #if ZAY_USE_NORMAL_FOR_VERTICE
                BOSS_GL(EnableVertexAttribArray, VertexAttrib::Normal); //bx
                testGL();
                #endif
                
                BOSS_GL(BindAttribLocation, _finalPassProgram, VertexAttrib::Position, "a_position");
                testGL();
                
                BOSS_GL(BindAttribLocation, _finalPassProgram, VertexAttrib::Color, "a_color");
                testGL();
                
                BOSS_GL(BindAttribLocation, _finalPassProgram, VertexAttrib::TexCoord, "a_texCoord");
                testGL();

                #if ZAY_USE_NORMAL_FOR_VERTICE
                BOSS_GL(BindAttribLocation, _finalPassProgram, VertexAttrib::Normal, "a_normal"); //bx
                testGL();
                #endif
                
                BOSS_GL(VertexAttribPointer, VertexAttrib::Position, 4, GL_FLOAT, GL_FALSE, sizeof(ZAY_Vertice), &_vertices[0].vertices[0]); //bx
                testGL();
                
                BOSS_GL(VertexAttribPointer, VertexAttrib::Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ZAY_Vertice), &_vertices[0].colors); //bx
                testGL();
                
                BOSS_GL(VertexAttribPointer, VertexAttrib::TexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(ZAY_Vertice), &_vertices[0].texCoords[0]); //bx
                testGL();

                #if ZAY_USE_NORMAL_FOR_VERTICE
                BOSS_GL(VertexAttribPointer, VertexAttrib::Normal, 2, GL_FLOAT, GL_FALSE, sizeof(ZAY_Vertice), &_vertices[0].normals[0]); //bx
                testGL();
                #endif
                
                
                
                BOSS_GL(LinkProgram, _finalPassProgram);
                testProgram(_finalPassProgram);
                
                BOSS_GL(ValidateProgram, _finalPassProgram);
                testProgram(_finalPassProgram);
                
                
                
                
                
                
                BOSS_GL(UseProgram, _finalPassProgram);
                testProgram(_finalPassProgram);
                
                //_finalPassLocationMultiplyMapTexture = BOSS_GL(GetUniformLocation, _finalPassProgram, "u_multiplyMapTexture");
                //testGL();
                
                //BOSS_GL(Uniform1i, _finalPassLocationMultiplyMapTexture, 0);
                //testGL();
                
                //_finalPassLocationMultiplyMapMultiplier = BOSS_GL(GetUniformLocation, _finalPassProgram, "u_multiplyMapMultiplier");
                //testGL();
                
                //BOSS_GL(Uniform1f, _finalPassLocationMultiplyMapMultiplier, _currentMultiplyMapMultiplier);
                //testGL();

                _finalPassLocationColorTexture = BOSS_GL(GetUniformLocation, _finalPassProgram, "u_colorTexture");
                testGL();
                
                BOSS_GL(Uniform1i, _finalPassLocationColorTexture, 0);
                testGL();
                
                _finalPassLocationMVPMatrix = BOSS_GL(GetUniformLocation, _finalPassProgram, "u_mvpMat");
                testGL();
                
                BOSS_GL(UniformMatrix4fv, _finalPassLocationMVPMatrix, 1, GL_FALSE, (const GLfloat*)getMVPMatrix()._getRaws());
                testGL();

                //bx
                _finalPassRenderModeLocation = BOSS_GL(GetUniformLocation, _finalPassProgram, "u_renderMode");
                testGL();

                //bx
                BOSS_GL(Uniform1f, _finalPassRenderModeLocation, _finalPassRenderMode);
                testGL();
            }
        }

        void ForwardMultiplyRenderer::render()
        {
            if (_renderPriorities[static_cast<int>(RenderType::TrianglesMesh)].left.size() > 0)
            {
                /*
                BOSS_GL(BindFramebuffer, GL_FRAMEBUFFER, _multiplyPassFBO);
                testGL();

                glViewport(0,
                           0,
                           _multiplyMapWidth,
                           _multiplyMapHeight);
                testGL();

                //glClearColor(0.3f, 0.0f, 0.0f, 1.0f);
                //testGL();
                //glClear(GL_COLOR_BUFFER_BIT);
                //testGL();
                
                if (_renderPriorities[static_cast<int>(RenderType::MultiplyMap)].left.size() > 0)
                {
                    glViewport(_multiplyMapBorder,
                               _multiplyMapBorder,
                               _multiplyMapWidth - _multiplyMapBorder*2,
                               _multiplyMapHeight - _multiplyMapBorder*2);
                    testGL();

                    
                    
                    BOSS_GL(UseProgram, _multiplyMapPassProgram);
                    testGL();

                    
                    
                    glEnable(GL_BLEND);
                    testGL();
                    
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    testGL();
                    
                    
                    
                    _multiplyRenderStart();
                    
                    for (auto it : _renderPriorities[static_cast<int>(RenderType::MultiplyMap)].left)
                    {
                        assert(dynamic_cast<MultiplyMapRenderCommand*>(it.second));
                        
                        if (it.second->getVisible())
                        {
                            auto renderCommand = static_cast<MultiplyMapRenderCommand*>(it.second);
                            
                            _renderMultiplyMap(renderCommand);
                        }
                    }
                    
                    _multiplyRenderEnd();
                }*/

                
                
                
                
                BOSS_GL(BindFramebuffer, GL_FRAMEBUFFER, _currentFBO);
                testGL();
                
                glViewport(getViewportX(),
                           getViewportY(),
                           getViewportWidth(),
                           getViewportHeight());
                testGL();
                
                
                //glClearColor(0.3f, 0.1f, 0.1f, 0.3f);
                //testGL();
                //glClear(GL_COLOR_BUFFER_BIT);
                //testGL();

                BOSS_GL(UseProgram, _finalPassProgram);
                testGL();

#ifdef  USE_BUFFER_OBJECT
                glBindVertexArray(_finalPassVAO);
                testGL();
#endif//USE_BUFFER_OBJECT


                
                BOSS_GL(BindBuffer, GL_ARRAY_BUFFER, _vbo);
                testGL();
                
                BOSS_GL(BindBuffer, GL_ELEMENT_ARRAY_BUFFER, _ibo);
                testGL();
                
                
                
                BOSS_GL(EnableVertexAttribArray, VertexAttrib::Position);
                testGL();
                
                BOSS_GL(EnableVertexAttribArray, VertexAttrib::Color);
                testGL();
                
                BOSS_GL(EnableVertexAttribArray, VertexAttrib::TexCoord);
                testGL();

                #if ZAY_USE_NORMAL_FOR_VERTICE
                BOSS_GL(EnableVertexAttribArray, VertexAttrib::Normal); //bx
                testGL();
                #endif

                
                
#ifdef  USE_BUFFER_OBJECT
                BOSS_GL(VertexAttribPointer, VertexAttrib::Position, 4, GL_FLOAT, GL_FALSE, sizeof(ZAY_Vertice), (const GLvoid*)(((char*)&_vertices[0].vertices[0]) - ((char*)_vertices)));
                testGL();
                
                BOSS_GL(VertexAttribPointer, VertexAttrib::Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ZAY_Vertice), (const GLvoid*)(((char*)&_vertices[0].colors) - ((char*)_vertices)));
                testGL();
                
                BOSS_GL(VertexAttribPointer, VertexAttrib::TexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(ZAY_Vertice), (const GLvoid*)(((char*)&_vertices[0].texCoords[0]) - ((char*)_vertices)));
                testGL();

                #if ZAY_USE_NORMAL_FOR_VERTICE
                BOSS_GL(VertexAttribPointer, VertexAttrib::Normal, 2, GL_FLOAT, GL_FALSE, sizeof(ZAY_Vertice), (const GLvoid*)(((char*)&_vertices[0].normals[0]) - ((char*)_vertices)));
                testGL();
                #endif
#else //USE_BUFFER_OBJECT
                BOSS_GL(VertexAttribPointer, VertexAttrib::Position, 4, GL_FLOAT, GL_FALSE, sizeof(ZAY_Vertice), &_vertices[0].vertices[0]); //bx
                testGL();
                
                BOSS_GL(VertexAttribPointer, VertexAttrib::Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ZAY_Vertice), &_vertices[0].colors); //bx
                testGL();
                
                BOSS_GL(VertexAttribPointer, VertexAttrib::TexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(ZAY_Vertice), &_vertices[0].texCoords[0]); //bx
                testGL();

                #if ZAY_USE_NORMAL_FOR_VERTICE
                BOSS_GL(VertexAttribPointer, VertexAttrib::Normal, 2, GL_FLOAT, GL_FALSE, sizeof(ZAY_Vertice), &_vertices[0].normals[0]); //bx
                testGL();
                #endif
#endif//USE_BUFFER_OBJECT
                
                glEnable(GL_BLEND);
                testGL();
                
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                testGL();
                
                
                /*
                BOSS_GL(ActiveTexture, GL_TEXTURE0);
                testGL();
                
                glBindTexture(GL_TEXTURE_2D, _multiplyMapRenderTexture);
                testGL();
                */

                //bx
                if(_finalPassRenderMode == 0.0) // 일반모드
                    _forcedBlendType = BlendType::DEFAULT;
                else if(_finalPassRenderMode == 1.0) // 외곽선모드
                    _forcedBlendType = BlendType::Outline;
                else _forcedBlendType = BlendType::Shadow; // 그림자모드

                _trianglesMeshRenderStart();
                
                for (auto it : _renderPriorities[static_cast<int>(RenderType::TrianglesMesh)].left)
                {
                    assert(dynamic_cast<TrianglesMeshRenderCommand*>(it.second));

                    if (it.second->getVisible())
                    {
                        auto renderCommand = static_cast<TrianglesMeshRenderCommand*>(it.second);
                        
                        _renderTrianglesMesh(renderCommand);
                    }
                }
                
                _trianglesMeshRenderEnd();

                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                testGL();
            }
        }

        void ForwardMultiplyRenderer::update()
        {
            updateRenderPriorities();
        }
        
        void ForwardMultiplyRenderer::_multiplyRenderStart()
        {
            if (_currentTexture)
            {
                _currentTexture->release();
                _currentTexture = nullptr;
            }
        }
        
        void ForwardMultiplyRenderer::_renderMultiplyMap(MultiplyMapRenderCommand* renderCommand)
        {
            assert(renderCommand);
            
            Texture2D* texture = renderCommand->getTexture();

            _setMultiplyMapTexture(texture);
            _addMultiplyMapVerticesAndIndices(renderCommand->getVerticesBuffer().getBufferPointer(),
                                              renderCommand->getVerticesBuffer().getBufferSize(),
                                              renderCommand->getIndicesBuffer().getBufferPointer(),
                                              renderCommand->getIndicesBuffer().getBufferSize());
        }
        
        void ForwardMultiplyRenderer::_renderCurrentMultiplyMapContext()
        {
            if (_currentTexture &&
                _verticesCount > 0 &&
                _indicesCount > 0)
            {
                BOSS_GL(ActiveTexture, GL_TEXTURE0);
                testGL();

                glBindTexture(GL_TEXTURE_2D, _currentTexture->getTextureID());
                testGL();

                glDrawElements(GL_TRIANGLES, _indicesCount, GL_UNSIGNED_SHORT, _indices);
                testGL();

                _verticesCount = 0;
                _indicesCount = 0;
            }
        }

        void ForwardMultiplyRenderer::_multiplyRenderEnd()
        {
            _renderCurrentMultiplyMapContext();

            if (_currentTexture)
            {
                _currentTexture->release();
                _currentTexture = nullptr;
            }
        }

        void ForwardMultiplyRenderer::_setMultiplyMapTexture(Texture2D* texture)
        {
            if (_currentTexture != texture)
            {
                _renderCurrentMultiplyMapContext();

                if (_currentTexture)
                {
                    _currentTexture->release();
                }
                
                _currentTexture = texture;
                
                if (_currentTexture)
                {
                    _currentTexture->retain();
                }
            }
        }

        void ForwardMultiplyRenderer::_addMultiplyMapVerticesAndIndices(ZAY_Vertice* vertices,
                                                                        int32_t verticesCount,
                                                                        GLushort* indices,
                                                                        int32_t indicesCount)
        {
            if ((_verticesCount + verticesCount) > _verticesCapacity ||
                (_indicesCount + indicesCount) > _indicesCapacity)
            {
                _renderCurrentMultiplyMapContext();
            }
            
            if ((_verticesCount + verticesCount) <= _verticesCapacity &&
                (_indicesCount + indicesCount) <= _indicesCapacity)
            {
                memcpy(_vertices+_verticesCount, vertices, sizeof(ZAY_Vertice) * verticesCount);
                
                auto dstIndices = _indices + _indicesCount;
                auto srcIndices = indices;
                for (auto i=0 ; i<indicesCount ; i++)
                {
                    *(dstIndices + i) = _verticesCount + *(srcIndices + i);
                }
                
                _verticesCount += verticesCount;
                _indicesCount += indicesCount;
            }
        }

        void ForwardMultiplyRenderer::_trianglesMeshRenderStart()
        {
            if (_currentTexture)
            {
                _currentTexture->release();
                _currentTexture = nullptr;
            }

            _currentBlendType = BlendType::DEFAULT;
            //_currentMultiplyMapMultiplier = 0.0f;
        }
        
        void ForwardMultiplyRenderer::_renderTrianglesMesh(TrianglesMeshRenderCommand* renderCommand)
        {
            if (renderCommand->getIndicesBuffer().getBufferSize() > 0)
            {
                Texture2D* texture = renderCommand->getTexture();

                _setTrianglesMeshTexture(texture);
                _setTrianglesMeshBlendType((_forcedBlendType != BlendType::DEFAULT)? _forcedBlendType : renderCommand->getBlendType()); //bx
                _setTrianglesMeshMultiplyMapMultiplier(renderCommand->getMultiplyMapMultiplier());
                _addTrianglesMeshVerticesAndIndices(renderCommand->getVerticesBuffer().getBufferPointer(),
                                                    renderCommand->getVerticesBuffer().getBufferSize(),
                                                    renderCommand->getIndicesBuffer().getBufferPointer(),
                                                    renderCommand->getIndicesBuffer().getBufferSize());
            }
        }
        
        void ForwardMultiplyRenderer::_renderCurrentTrianglesMeshContext()
        {
            if (_currentTexture &&
                _verticesCount > 0 &&
                _indicesCount > 0 &&
                _currentBlendType != BlendType::DEFAULT)
            {
#ifdef  USE_BUFFER_OBJECT
                glActiveTexture(GL_TEXTURE0);
                testGL();
                
                glBindTexture(GL_TEXTURE_2D, _currentTexture->getTextureID());
                testGL();

                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(ZAY_Vertice) * _verticesCount, _vertices);
                testGL();
                
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLushort) * _indicesCount, _indices);
                testGL();

                glDrawElements(GL_TRIANGLES, _indicesCount, GL_UNSIGNED_SHORT, 0);
                testGL();
#else //USE_BUFFER_OBJECT
                BOSS_GL(ActiveTexture, GL_TEXTURE0);
                testGL();

                glBindTexture(GL_TEXTURE_2D, _currentTexture->getTextureID());
                testGL();

                glDrawElements(GL_TRIANGLES, _indicesCount, GL_UNSIGNED_SHORT, _indices);
                testGL();
#endif//USE_BUFFER_OBJECT
                
                _verticesCount = 0;
                _indicesCount = 0;
            }
        }
        
        void ForwardMultiplyRenderer::_trianglesMeshRenderEnd()
        {
            _renderCurrentTrianglesMeshContext();
            
            if (_currentTexture)
            {
                _currentTexture->release();
                _currentTexture = nullptr;
            }

            _currentBlendType = BlendType::DEFAULT;
            //_currentMultiplyMapMultiplier = 0.0f;
        }

        void ForwardMultiplyRenderer::_setTrianglesMeshTexture(Texture2D* texture)
        {
            if (_currentTexture != texture)
            {
                _renderCurrentTrianglesMeshContext();

                if (_currentTexture)
                {
                    _currentTexture->release();
                }
                
                _currentTexture = texture;
                
                if (_currentTexture)
                {
                    _currentTexture->retain();
                }
            }
        }
        
        void ForwardMultiplyRenderer::_setTrianglesMeshBlendType(BlendType blendType)
        {
            if (_currentBlendType != blendType)
            {
                _renderCurrentTrianglesMeshContext();

                _currentBlendType = blendType;
                
                switch (_currentBlendType)
                {
                    case BlendType::Alpha:
                        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                        testGL();
                        break;
                    case BlendType::Additive:
                        glBlendFunc(GL_ONE, GL_ONE);
                        testGL();
                        break;
                    case BlendType::Multiply:
                        glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
                        testGL();
                        break;
                    case BlendType::Screen:
                        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
                        testGL();
                        break;
                    case BlendType::Outline:
                        glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
                        testGL();
                        break;
                    case BlendType::Shadow:
                        #if BOSS_ANDROID
                            glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
                        #else
                            glBlendFunc(GL_ONE, GL_ONE);
                        #endif
                        testGL();
                        break;
                    default:
                        break;
                }
            }
        }
        
        void ForwardMultiplyRenderer::_setTrianglesMeshMultiplyMapMultiplier(float multiplier)
        {
        }

        void ForwardMultiplyRenderer::_addTrianglesMeshVerticesAndIndices(ZAY_Vertice* vertices,
                                                                          int32_t verticesCount,
                                                                          GLushort* indices,
                                                                          int32_t indicesCount)
        {
            if ((_verticesCount + verticesCount) > _verticesCapacity ||
                (_indicesCount + indicesCount) > _indicesCapacity)
            {
                _renderCurrentTrianglesMeshContext();
            }
            
            if ((_verticesCount + verticesCount) <= _verticesCapacity &&
                (_indicesCount + indicesCount) <= _indicesCapacity)
            {
                memcpy(_vertices+_verticesCount, vertices, sizeof(ZAY_Vertice) * verticesCount);

                //bx
                for(int i = 0; i < verticesCount; ++i)
                {
                    ZAY_Vertice& CurVertice = _vertices[_verticesCount + i];
                    const ZAY_Vertice& PrevVertice = _vertices[_verticesCount + (i + verticesCount - 1) % verticesCount];
                    const float prev_ox = PrevVertice.vertices[0] - CurVertice.vertices[0];
                    const float prev_oy = PrevVertice.vertices[1] - CurVertice.vertices[1];
                    const float prev_dist = BOSS::Math::Sqrt(prev_ox * prev_ox + prev_oy * prev_oy);
                    const ZAY_Vertice& NextVertice = _vertices[_verticesCount + (i + 1) % verticesCount];
                    const float next_ox = NextVertice.vertices[0] - CurVertice.vertices[0];
                    const float next_oy = NextVertice.vertices[1] - CurVertice.vertices[1];
                    const float next_dist = BOSS::Math::Sqrt(next_ox * next_ox + next_oy * next_oy);
                    CurVertice.vertices[2] = (prev_ox / prev_dist + next_ox / next_dist) / -3;
                    CurVertice.normal = (prev_oy / prev_dist + next_oy / next_dist) / -3;
                }

                auto dstIndices = _indices + _indicesCount;
                auto srcIndices = indices;
                for (auto i=0 ; i<indicesCount ; i++)
                {
                    assert( *(srcIndices + i) < verticesCount );

                    *(dstIndices + i) = _verticesCount + *(srcIndices + i);
                }
                
                _verticesCount += verticesCount;
                _indicesCount += indicesCount;
            }
        }

        
        
        
        
        void ForwardMultiplyRenderer::updateRenderPriorities()
        {
            for (auto renderCommand : _modifiedRenderCommands)
            {
                auto renderType = static_cast<int>(renderCommand->getRenderType());
                
                auto it = _renderPriorities[renderType].right.find(renderCommand);
                
                if (it != _renderPriorities[renderType].right.end())
                {
                    _renderPriorities[renderType].right.erase(it);
                    _renderPriorities[renderType].insert(boost::bimap<RenderPriority, ForwardMultiplyRenderCommand*>::value_type(renderCommand->getRenderPriority(), renderCommand));
                }
            }
            
            _modifiedRenderCommands.clear();
        }

        void ForwardMultiplyRenderer::_addRenderCommand(RenderCommand* _renderCommand)
        {
            assert(_renderCommand);
            assert(dynamic_cast<ForwardMultiplyRenderCommand*>(_renderCommand));
            
            if (auto renderCommand = static_cast<ForwardMultiplyRenderCommand*>(_renderCommand))
            {
                auto renderType = static_cast<int>(renderCommand->getRenderType());
                
                assert(_renderPriorities[renderType].right.find(renderCommand) == _renderPriorities[renderType].right.end());
                
                _renderPriorities[renderType].insert(RenderCommandMap::value_type(renderCommand->getRenderPriority(), renderCommand));
                
                renderCommand->retain();
            }
        }
        
        void ForwardMultiplyRenderer::_removeRenderCommand(RenderCommand* _renderCommand)
        {
            assert(_renderCommand);
            assert(dynamic_cast<ForwardMultiplyRenderCommand*>(_renderCommand));
            
            if (auto renderCommand = static_cast<ForwardMultiplyRenderCommand*>(_renderCommand))
            {
                auto renderType = static_cast<int>(renderCommand->getRenderType());
                
                auto it = _renderPriorities[renderType].right.find(renderCommand);
                
                if (it != _renderPriorities[renderType].right.end())
                {
                    _renderPriorities[renderType].right.erase(it);
                    renderCommand->release();
                }
            }
        }
        
        void ForwardMultiplyRenderer::_notifyModifyRenderPriority(RenderCommand* _renderCommand)
        {
            assert(_renderCommand);
            assert(dynamic_cast<ForwardMultiplyRenderCommand*>(_renderCommand));
            
            if (auto renderCommand = static_cast<ForwardMultiplyRenderCommand*>(_renderCommand))
            {
                _modifiedRenderCommands.insert(renderCommand);
            }
        }

        void ForwardMultiplyRenderer::clearRenderCommands()
        {
            for (auto i=0 ; i<static_cast<int>(RenderType::COUNT) ; i++)
            {
                for (auto it : _renderPriorities[i])
                {
                    it.right->release();
                }
                _renderPriorities[i].clear();
            }
            
            _modifiedRenderCommands.clear();
        }
        
        const ForwardMultiplyRenderer::RenderCommandMap& ForwardMultiplyRenderer::getRenderPriorityWithType(RenderType type) const
        {
            return _renderPriorities[static_cast<int>(type)];
        }
        
        const std::array<ForwardMultiplyRenderer::RenderCommandMap, static_cast<int>(RenderType::COUNT)>& ForwardMultiplyRenderer::getRenderPrioritiesAll() const
        {
            return _renderPriorities;
        }
    }
}

