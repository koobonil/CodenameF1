#include "zay_types.h"
#include "zay_opengl.h"

#if !USE_QT_OPENGL
    #if BOSS_WINDOWS
        #include <windows.h>
        #include <wingdi.h>
        #pragma comment(lib, "User32.lib")
        #pragma comment(lib, "Opengl32.lib")
        int main(int argc, char* argv[]);
        int CALLBACK WinMain(
            _In_ HINSTANCE hInstance,
            _In_ HINSTANCE hPrevInstance,
            _In_ LPSTR     lpCmdLine,
            _In_ int       nCmdShow)
        #if BOSS_NEED_MAIN
            {return main(0, nullptr);}
        #else
            {return 0;}
        #endif

        # if BOSS_X64
        #  if BOSS_NDEBUG
        #   pragma comment(lib, "../../codename_fx/source/glew-1.12.0/lib/Release/x64/glew32.lib")
        #  else
        #   pragma comment(lib, "../../codename_fx/source/glew-1.12.0/lib/Release/x64/glew32.lib")
        #  endif
        # else
        #  if BOSS_NDEBUG
        #   pragma comment(lib, "../../codename_fx/source/glew-1.12.0/lib/Release/Win32/glew32.lib")
        #  else
        #   pragma comment(lib, "../../codename_fx/source/glew-1.12.0/lib/Release/Win32/glew32.lib")
        #  endif
        # endif

        #define SET_GL_FUNC(NAME) setGLEWFunction(__glew##NAME, wglGetProcAddress("gl"#NAME), #NAME)
        template<typename RValueType, typename LValueType>
        void setGLEWFunction(RValueType& rvalue, LValueType lvalue, const char* name)
        {rvalue = (RValueType) lvalue;}

        static bool initGLEWFunctions();
        static bool Run = initGLEWFunctions();

        bool initGLEWFunctions()
        {
            glewExperimental = GL_TRUE;
            glewInit();

            // OpenGL 1.2 core functions
            SET_GL_FUNC(CopyTexSubImage3D);
            SET_GL_FUNC(TexSubImage3D);
            SET_GL_FUNC(TexImage3D);
            SET_GL_FUNC(DrawRangeElements);

            // OpenGL 1.3 core functions
            SET_GL_FUNC(GetCompressedTexImage);
            SET_GL_FUNC(CompressedTexSubImage1D);
            SET_GL_FUNC(CompressedTexSubImage2D);
            SET_GL_FUNC(CompressedTexSubImage3D);
            SET_GL_FUNC(CompressedTexImage1D);
            SET_GL_FUNC(CompressedTexImage2D);
            SET_GL_FUNC(CompressedTexImage3D);
            SET_GL_FUNC(SampleCoverage);
            SET_GL_FUNC(ActiveTexture);

            // OpenGL 1.4 core functions
            SET_GL_FUNC(PointParameteriv);
            SET_GL_FUNC(PointParameteri);
            SET_GL_FUNC(PointParameterfv);
            SET_GL_FUNC(PointParameterf);
            SET_GL_FUNC(MultiDrawElements);
            SET_GL_FUNC(MultiDrawArrays);
            SET_GL_FUNC(BlendFuncSeparate);

            // OpenGL 1.5 core functions
            SET_GL_FUNC(GetBufferPointerv);
            SET_GL_FUNC(GetBufferParameteriv);
            SET_GL_FUNC(UnmapBuffer);
            SET_GL_FUNC(MapBuffer);
            SET_GL_FUNC(GetBufferSubData);
            SET_GL_FUNC(BufferSubData);
            SET_GL_FUNC(BufferData);
            SET_GL_FUNC(IsBuffer);
            SET_GL_FUNC(GenBuffers);
            SET_GL_FUNC(DeleteBuffers);
            SET_GL_FUNC(BindBuffer);
            SET_GL_FUNC(GetQueryObjectuiv);
            SET_GL_FUNC(GetQueryObjectiv);
            SET_GL_FUNC(GetQueryiv);
            SET_GL_FUNC(EndQuery);
            SET_GL_FUNC(BeginQuery);
            SET_GL_FUNC(IsQuery);
            SET_GL_FUNC(DeleteQueries);
            SET_GL_FUNC(GenQueries);

            // OpenGL 2.0 core functions
            SET_GL_FUNC(VertexAttribPointer);
            SET_GL_FUNC(ValidateProgram);
            SET_GL_FUNC(UniformMatrix4fv);
            SET_GL_FUNC(UniformMatrix3fv);
            SET_GL_FUNC(UniformMatrix2fv);
            SET_GL_FUNC(Uniform4iv);
            SET_GL_FUNC(Uniform3iv);
            SET_GL_FUNC(Uniform2iv);
            SET_GL_FUNC(Uniform1iv);
            SET_GL_FUNC(Uniform4fv);
            SET_GL_FUNC(Uniform3fv);
            SET_GL_FUNC(Uniform2fv);
            SET_GL_FUNC(Uniform1fv);
            SET_GL_FUNC(Uniform4i);
            SET_GL_FUNC(Uniform3i);
            SET_GL_FUNC(Uniform2i);
            SET_GL_FUNC(Uniform1i);
            SET_GL_FUNC(Uniform4f);
            SET_GL_FUNC(Uniform3f);
            SET_GL_FUNC(Uniform2f);
            SET_GL_FUNC(Uniform1f);
            SET_GL_FUNC(UseProgram);
            SET_GL_FUNC(ShaderSource);
            SET_GL_FUNC(LinkProgram);
            SET_GL_FUNC(IsShader);
            SET_GL_FUNC(IsProgram);
            SET_GL_FUNC(GetVertexAttribPointerv);
            SET_GL_FUNC(GetVertexAttribiv);
            SET_GL_FUNC(GetVertexAttribfv);
            SET_GL_FUNC(GetVertexAttribdv);
            SET_GL_FUNC(GetUniformiv);
            SET_GL_FUNC(GetUniformfv);
            SET_GL_FUNC(GetUniformLocation);
            SET_GL_FUNC(GetShaderSource);
            SET_GL_FUNC(GetShaderInfoLog);
            SET_GL_FUNC(GetShaderiv);
            SET_GL_FUNC(GetProgramInfoLog);
            SET_GL_FUNC(GetProgramiv);
            SET_GL_FUNC(GetAttribLocation);
            SET_GL_FUNC(GetAttachedShaders);
            SET_GL_FUNC(GetActiveUniform);
            SET_GL_FUNC(GetActiveAttrib);
            SET_GL_FUNC(EnableVertexAttribArray);
            SET_GL_FUNC(DisableVertexAttribArray);
            SET_GL_FUNC(DetachShader);
            SET_GL_FUNC(DeleteShader);
            SET_GL_FUNC(DeleteProgram);
            SET_GL_FUNC(CreateShader);
            SET_GL_FUNC(CreateProgram);
            SET_GL_FUNC(CompileShader);
            SET_GL_FUNC(BindAttribLocation);
            SET_GL_FUNC(AttachShader);
            SET_GL_FUNC(StencilMaskSeparate);
            SET_GL_FUNC(StencilFuncSeparate);
            SET_GL_FUNC(StencilOpSeparate);
            SET_GL_FUNC(DrawBuffers);
            SET_GL_FUNC(BlendEquationSeparate);

            // OpenGL 2.1 core functions
            SET_GL_FUNC(UniformMatrix4x3fv);
            SET_GL_FUNC(UniformMatrix3x4fv);
            SET_GL_FUNC(UniformMatrix4x2fv);
            SET_GL_FUNC(UniformMatrix2x4fv);
            SET_GL_FUNC(UniformMatrix3x2fv);
            SET_GL_FUNC(UniformMatrix2x3fv);

            // OpenGL 3.0 core functions
            SET_GL_FUNC(IsVertexArray);
            SET_GL_FUNC(GenVertexArrays);
            SET_GL_FUNC(DeleteVertexArrays);
            SET_GL_FUNC(BindVertexArray);
            SET_GL_FUNC(FlushMappedBufferRange);
            SET_GL_FUNC(MapBufferRange);
            SET_GL_FUNC(FramebufferTextureLayer);
            SET_GL_FUNC(RenderbufferStorageMultisample);
            SET_GL_FUNC(BlitFramebuffer);
            SET_GL_FUNC(GenerateMipmap);
            SET_GL_FUNC(GetFramebufferAttachmentParameteriv);
            SET_GL_FUNC(FramebufferRenderbuffer);
            SET_GL_FUNC(FramebufferTexture3D);
            SET_GL_FUNC(FramebufferTexture2D);
            SET_GL_FUNC(FramebufferTexture1D);
            SET_GL_FUNC(CheckFramebufferStatus);
            SET_GL_FUNC(GenFramebuffers);
            SET_GL_FUNC(DeleteFramebuffers);
            SET_GL_FUNC(BindFramebuffer);
            SET_GL_FUNC(IsFramebuffer);
            SET_GL_FUNC(GetRenderbufferParameteriv);
            SET_GL_FUNC(RenderbufferStorage);
            SET_GL_FUNC(GenRenderbuffers);
            SET_GL_FUNC(DeleteRenderbuffers);
            SET_GL_FUNC(BindRenderbuffer);
            SET_GL_FUNC(IsRenderbuffer);
            SET_GL_FUNC(GetStringi);
            SET_GL_FUNC(ClearBufferfi);
            SET_GL_FUNC(ClearBufferfv);
            SET_GL_FUNC(ClearBufferuiv);
            SET_GL_FUNC(ClearBufferiv);
            SET_GL_FUNC(GetTexParameterIuiv);
            SET_GL_FUNC(GetTexParameterIiv);
            SET_GL_FUNC(TexParameterIuiv);
            SET_GL_FUNC(TexParameterIiv);
            SET_GL_FUNC(Uniform4uiv);
            SET_GL_FUNC(Uniform3uiv);
            SET_GL_FUNC(Uniform2uiv);
            SET_GL_FUNC(Uniform1uiv);
            SET_GL_FUNC(Uniform4ui);
            SET_GL_FUNC(Uniform3ui);
            SET_GL_FUNC(Uniform2ui);
            SET_GL_FUNC(Uniform1ui);
            SET_GL_FUNC(GetFragDataLocation);
            SET_GL_FUNC(BindFragDataLocation);
            SET_GL_FUNC(GetUniformuiv);
            SET_GL_FUNC(GetVertexAttribIuiv);
            SET_GL_FUNC(GetVertexAttribIiv);
            SET_GL_FUNC(VertexAttribIPointer);
            SET_GL_FUNC(EndConditionalRender);
            SET_GL_FUNC(BeginConditionalRender);
            SET_GL_FUNC(ClampColor);
            SET_GL_FUNC(GetTransformFeedbackVarying);
            SET_GL_FUNC(TransformFeedbackVaryings);
            SET_GL_FUNC(BindBufferBase);
            SET_GL_FUNC(BindBufferRange);
            SET_GL_FUNC(EndTransformFeedback);
            SET_GL_FUNC(BeginTransformFeedback);
            SET_GL_FUNC(IsEnabledi);
            SET_GL_FUNC(Disablei);
            SET_GL_FUNC(Enablei);
            SET_GL_FUNC(GetIntegeri_v);
            SET_GL_FUNC(GetBooleani_v);
            SET_GL_FUNC(ColorMaski);

            // OpenGL 3.1 core functions
            SET_GL_FUNC(CopyBufferSubData);
            SET_GL_FUNC(UniformBlockBinding);
            SET_GL_FUNC(GetActiveUniformBlockName);
            SET_GL_FUNC(GetActiveUniformBlockiv);
            SET_GL_FUNC(GetUniformBlockIndex);
            SET_GL_FUNC(GetActiveUniformName);
            SET_GL_FUNC(GetActiveUniformsiv);
            SET_GL_FUNC(GetUniformIndices);
            SET_GL_FUNC(PrimitiveRestartIndex);
            SET_GL_FUNC(TexBuffer);
            SET_GL_FUNC(DrawElementsInstanced);
            SET_GL_FUNC(DrawArraysInstanced);

            // OpenGL 3.2 core functions
            SET_GL_FUNC(SampleMaski);
            SET_GL_FUNC(GetMultisamplefv);
            SET_GL_FUNC(TexImage3DMultisample);
            SET_GL_FUNC(TexImage2DMultisample);
            SET_GL_FUNC(GetSynciv);
            SET_GL_FUNC(GetInteger64v);
            SET_GL_FUNC(WaitSync);
            SET_GL_FUNC(ClientWaitSync);
            SET_GL_FUNC(DeleteSync);
            SET_GL_FUNC(IsSync);
            SET_GL_FUNC(FenceSync);
            SET_GL_FUNC(ProvokingVertex);
            SET_GL_FUNC(MultiDrawElementsBaseVertex);
            SET_GL_FUNC(DrawElementsInstancedBaseVertex);
            SET_GL_FUNC(DrawRangeElementsBaseVertex);
            SET_GL_FUNC(DrawElementsBaseVertex);
            SET_GL_FUNC(FramebufferTexture);
            SET_GL_FUNC(GetBufferParameteri64v);
            SET_GL_FUNC(GetInteger64i_v);

            // OpenGL 3.3 core functions
            SET_GL_FUNC(VertexAttribP4uiv);
            SET_GL_FUNC(VertexAttribP4ui);
            SET_GL_FUNC(VertexAttribP3uiv);
            SET_GL_FUNC(VertexAttribP3ui);
            SET_GL_FUNC(VertexAttribP2uiv);
            SET_GL_FUNC(VertexAttribP2ui);
            SET_GL_FUNC(VertexAttribP1uiv);
            SET_GL_FUNC(VertexAttribP1ui);
            SET_GL_FUNC(SecondaryColorP3uiv);
            SET_GL_FUNC(SecondaryColorP3ui);
            SET_GL_FUNC(ColorP4uiv);
            SET_GL_FUNC(ColorP4ui);
            SET_GL_FUNC(ColorP3uiv);
            SET_GL_FUNC(ColorP3ui);
            SET_GL_FUNC(NormalP3uiv);
            SET_GL_FUNC(NormalP3ui);
            SET_GL_FUNC(MultiTexCoordP4uiv);
            SET_GL_FUNC(MultiTexCoordP4ui);
            SET_GL_FUNC(MultiTexCoordP3uiv);
            SET_GL_FUNC(MultiTexCoordP3ui);
            SET_GL_FUNC(MultiTexCoordP2uiv);
            SET_GL_FUNC(MultiTexCoordP2ui);
            SET_GL_FUNC(MultiTexCoordP1uiv);
            SET_GL_FUNC(MultiTexCoordP1ui);
            SET_GL_FUNC(TexCoordP4uiv);
            SET_GL_FUNC(TexCoordP4ui);
            SET_GL_FUNC(TexCoordP3uiv);
            SET_GL_FUNC(TexCoordP3ui);
            SET_GL_FUNC(TexCoordP2uiv);
            SET_GL_FUNC(TexCoordP2ui);
            SET_GL_FUNC(TexCoordP1uiv);
            SET_GL_FUNC(TexCoordP1ui);
            SET_GL_FUNC(VertexP4uiv);
            SET_GL_FUNC(VertexP4ui);
            SET_GL_FUNC(VertexP3uiv);
            SET_GL_FUNC(VertexP3ui);
            SET_GL_FUNC(VertexP2uiv);
            SET_GL_FUNC(VertexP2ui);
            SET_GL_FUNC(GetQueryObjectui64v);
            SET_GL_FUNC(GetQueryObjecti64v);
            SET_GL_FUNC(QueryCounter);
            SET_GL_FUNC(GetSamplerParameterIuiv);
            SET_GL_FUNC(GetSamplerParameterfv);
            SET_GL_FUNC(GetSamplerParameterIiv);
            SET_GL_FUNC(GetSamplerParameteriv);
            SET_GL_FUNC(SamplerParameterIuiv);
            SET_GL_FUNC(SamplerParameterIiv);
            SET_GL_FUNC(SamplerParameterfv);
            SET_GL_FUNC(SamplerParameterf);
            SET_GL_FUNC(SamplerParameteriv);
            SET_GL_FUNC(SamplerParameteri);
            SET_GL_FUNC(BindSampler);
            SET_GL_FUNC(IsSampler);
            SET_GL_FUNC(DeleteSamplers);
            SET_GL_FUNC(GenSamplers);
            SET_GL_FUNC(GetFragDataIndex);
            SET_GL_FUNC(BindFragDataLocationIndexed);
            SET_GL_FUNC(VertexAttribDivisor);

            // OpenGL 4.0 core functions
            SET_GL_FUNC(GetQueryIndexediv);
            SET_GL_FUNC(EndQueryIndexed);
            SET_GL_FUNC(BeginQueryIndexed);
            SET_GL_FUNC(DrawTransformFeedbackStream);
            SET_GL_FUNC(DrawTransformFeedback);
            SET_GL_FUNC(ResumeTransformFeedback);
            SET_GL_FUNC(PauseTransformFeedback);
            SET_GL_FUNC(IsTransformFeedback);
            SET_GL_FUNC(GenTransformFeedbacks);
            SET_GL_FUNC(DeleteTransformFeedbacks);
            SET_GL_FUNC(BindTransformFeedback);
            SET_GL_FUNC(PatchParameterfv);
            SET_GL_FUNC(PatchParameteri);
            SET_GL_FUNC(GetProgramStageiv);
            SET_GL_FUNC(GetUniformSubroutineuiv);
            SET_GL_FUNC(UniformSubroutinesuiv);
            SET_GL_FUNC(GetActiveSubroutineName);
            SET_GL_FUNC(GetActiveSubroutineUniformName);
            SET_GL_FUNC(GetActiveSubroutineUniformiv);
            SET_GL_FUNC(GetSubroutineIndex);
            SET_GL_FUNC(GetSubroutineUniformLocation);
            SET_GL_FUNC(GetUniformdv);
            SET_GL_FUNC(UniformMatrix4x3dv);
            SET_GL_FUNC(UniformMatrix4x2dv);
            SET_GL_FUNC(UniformMatrix3x4dv);
            SET_GL_FUNC(UniformMatrix3x2dv);
            SET_GL_FUNC(UniformMatrix2x4dv);
            SET_GL_FUNC(UniformMatrix2x3dv);
            SET_GL_FUNC(UniformMatrix4dv);
            SET_GL_FUNC(UniformMatrix3dv);
            SET_GL_FUNC(UniformMatrix2dv);
            SET_GL_FUNC(Uniform4dv);
            SET_GL_FUNC(Uniform3dv);
            SET_GL_FUNC(Uniform2dv);
            SET_GL_FUNC(Uniform1dv);
            SET_GL_FUNC(Uniform4d);
            SET_GL_FUNC(Uniform3d);
            SET_GL_FUNC(Uniform2d);
            SET_GL_FUNC(Uniform1d);
            SET_GL_FUNC(DrawElementsIndirect);
            SET_GL_FUNC(DrawArraysIndirect);
            SET_GL_FUNC(BlendFuncSeparatei);
            SET_GL_FUNC(BlendFunci);
            SET_GL_FUNC(BlendEquationSeparatei);
            SET_GL_FUNC(BlendEquationi);
            SET_GL_FUNC(MinSampleShading);

            // OpenGL 4.1 core functions
            SET_GL_FUNC(GetDoublei_v);
            SET_GL_FUNC(GetFloati_v);
            SET_GL_FUNC(DepthRangeIndexed);
            SET_GL_FUNC(DepthRangeArrayv);
            SET_GL_FUNC(ScissorIndexedv);
            SET_GL_FUNC(ScissorIndexed);
            SET_GL_FUNC(ScissorArrayv);
            SET_GL_FUNC(ViewportIndexedfv);
            SET_GL_FUNC(ViewportIndexedf);
            SET_GL_FUNC(ViewportArrayv);
            SET_GL_FUNC(GetVertexAttribLdv);
            SET_GL_FUNC(VertexAttribLPointer);
            SET_GL_FUNC(VertexAttribL4dv);
            SET_GL_FUNC(VertexAttribL3dv);
            SET_GL_FUNC(VertexAttribL2dv);
            SET_GL_FUNC(VertexAttribL1dv);
            SET_GL_FUNC(VertexAttribL4d);
            SET_GL_FUNC(VertexAttribL3d);
            SET_GL_FUNC(VertexAttribL2d);
            SET_GL_FUNC(VertexAttribL1d);
            SET_GL_FUNC(GetProgramPipelineInfoLog);
            SET_GL_FUNC(ValidateProgramPipeline);
            SET_GL_FUNC(ProgramUniformMatrix4x3dv);
            SET_GL_FUNC(ProgramUniformMatrix3x4dv);
            SET_GL_FUNC(ProgramUniformMatrix4x2dv);
            SET_GL_FUNC(ProgramUniformMatrix2x4dv);
            SET_GL_FUNC(ProgramUniformMatrix3x2dv);
            SET_GL_FUNC(ProgramUniformMatrix2x3dv);
            SET_GL_FUNC(ProgramUniformMatrix4x3fv);
            SET_GL_FUNC(ProgramUniformMatrix3x4fv);
            SET_GL_FUNC(ProgramUniformMatrix4x2fv);
            SET_GL_FUNC(ProgramUniformMatrix2x4fv);
            SET_GL_FUNC(ProgramUniformMatrix3x2fv);
            SET_GL_FUNC(ProgramUniformMatrix2x3fv);
            SET_GL_FUNC(ProgramUniformMatrix4dv);
            SET_GL_FUNC(ProgramUniformMatrix3dv);
            SET_GL_FUNC(ProgramUniformMatrix2dv);
            SET_GL_FUNC(ProgramUniformMatrix4fv);
            SET_GL_FUNC(ProgramUniformMatrix3fv);
            SET_GL_FUNC(ProgramUniformMatrix2fv);
            SET_GL_FUNC(ProgramUniform4uiv);
            SET_GL_FUNC(ProgramUniform4ui);
            SET_GL_FUNC(ProgramUniform4dv);
            SET_GL_FUNC(ProgramUniform4d);
            SET_GL_FUNC(ProgramUniform4fv);
            SET_GL_FUNC(ProgramUniform4f);
            SET_GL_FUNC(ProgramUniform4iv);
            SET_GL_FUNC(ProgramUniform4i);
            SET_GL_FUNC(ProgramUniform3uiv);
            SET_GL_FUNC(ProgramUniform3ui);
            SET_GL_FUNC(ProgramUniform3dv);
            SET_GL_FUNC(ProgramUniform3d);
            SET_GL_FUNC(ProgramUniform3fv);
            SET_GL_FUNC(ProgramUniform3f);
            SET_GL_FUNC(ProgramUniform3iv);
            SET_GL_FUNC(ProgramUniform3i);
            SET_GL_FUNC(ProgramUniform2uiv);
            SET_GL_FUNC(ProgramUniform2ui);
            SET_GL_FUNC(ProgramUniform2dv);
            SET_GL_FUNC(ProgramUniform2d);
            SET_GL_FUNC(ProgramUniform2fv);
            SET_GL_FUNC(ProgramUniform2f);
            SET_GL_FUNC(ProgramUniform2iv);
            SET_GL_FUNC(ProgramUniform2i);
            SET_GL_FUNC(ProgramUniform1uiv);
            SET_GL_FUNC(ProgramUniform1ui);
            SET_GL_FUNC(ProgramUniform1dv);
            SET_GL_FUNC(ProgramUniform1d);
            SET_GL_FUNC(ProgramUniform1fv);
            SET_GL_FUNC(ProgramUniform1f);
            SET_GL_FUNC(ProgramUniform1iv);
            SET_GL_FUNC(ProgramUniform1i);
            SET_GL_FUNC(GetProgramPipelineiv);
            SET_GL_FUNC(IsProgramPipeline);
            SET_GL_FUNC(GenProgramPipelines);
            SET_GL_FUNC(DeleteProgramPipelines);
            SET_GL_FUNC(BindProgramPipeline);
            SET_GL_FUNC(CreateShaderProgramv);
            SET_GL_FUNC(ActiveShaderProgram);
            SET_GL_FUNC(UseProgramStages);
            SET_GL_FUNC(ProgramParameteri);
            SET_GL_FUNC(ProgramBinary);
            SET_GL_FUNC(GetProgramBinary);
            SET_GL_FUNC(ClearDepthf);
            SET_GL_FUNC(DepthRangef);
            SET_GL_FUNC(GetShaderPrecisionFormat);
            SET_GL_FUNC(ShaderBinary);
            SET_GL_FUNC(ReleaseShaderCompiler);
            return true;
        }
    #elif BOSS_MAC_OSX
    #elif BOSS_IPHONE
    #elif BOSS_ANDROID
        #include "platform/android/OpenGLES_Functions-android.cpp"
    #endif
#endif
