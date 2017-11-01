#pragma once

#include "zay_base.h"
#include "zay_opengl.h"
#include "zay_render_priority.h"
#include "zay_vector2.h"
#include "zay_vector3.h"
#include "zay_matrix4.h"
#include "zay_colour_value.h"

namespace ZAY
{
    #define ZAY_USE_NORMAL_FOR_VERTICE 0
    struct ZAY_V3F_T2F
    {
        Vector3 vertices;   // 12 bytes
        Vector2 texCoords;  // 8 bytes
    };
    struct ZAY_V4F_C4B_T2F
    {
        Vector3 vertices;   // 12 bytes
        float normal;       // 4 bytes
        RGBA colors;        // 4 bytes
        Vector2 texCoords;  // 8 bytes
    };
    struct ZAY_V3F_C4B_T2F_N3F
    {
        Vector3 vertices;   // 12 bytes
        RGBA colors;        // 4 bytes
        Vector2 texCoords;  // 8 bytes
        Vector3 normals;    // 12 bytes
    };
    #if ZAY_USE_NORMAL_FOR_VERTICE
    typedef ZAY_V3F_C4B_T2F_N3F ZAY_Vertice; //bx
    #else
    typedef ZAY_V4F_C4B_T2F ZAY_Vertice; //bx
    #endif

    struct ZAY_Point
    {
        Vector3 position;
        float color[4];
        float minSize;
        float maxSize;
    };
    
    class Renderer
    : public Base
    {
    public:
        Renderer();
        virtual ~Renderer();
        
    public:
        virtual void init() = 0;
        virtual void term() = 0;
        
    public:
        void setViewport(int32_t viewportX, int32_t viewportY, int32_t viewportWidth, int32_t viewportHeight);
        int32_t getViewportX() const;
        int32_t getViewportY() const;
        int32_t getViewportWidth() const;
        int32_t getViewportHeight() const;
    private:
        int32_t _viewportX;
        int32_t _viewportY;
        int32_t _viewportWidth;
        int32_t _viewportHeight;

    public:
        static void testGL();
        static void testShader(GLuint shader);
        static void testProgram(GLuint program);

    public:
        void setMVPMatrix(const Matrix4& matrix);
        const Matrix4& getMVPMatrix() const;
    protected:
        virtual void _notifyMVPMatrixChanged() = 0;
    private:
        Matrix4 _mvpMatrix;

    public: //bx
        void setRenderMode(const float mode);
    protected: //bx
        virtual void _setRenderMode(const float mode) = 0;

    public:
        virtual void render(bool shadow) = 0;
        virtual void update() = 0;

    public:
        virtual void updateRenderPriorities() = 0;
    public:
        virtual void _addRenderCommand(RenderCommand* renderCommand) = 0;
        virtual void _removeRenderCommand(RenderCommand* renderCommand) = 0;
        virtual void _notifyModifyRenderPriority(RenderCommand* renderCommand) = 0;
    };
}
