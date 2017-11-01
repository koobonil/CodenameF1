#pragma once

#include "zay_renderer.h"
#include "zay_render_command.h"
#include "zay_array_buffer.h"

namespace ZAY
{
    namespace ForwardMultiplyRender
    {
        class ForwardMultiplyRenderCommand
        : public RenderCommand
        {
        public:
            ForwardMultiplyRenderCommand();
            virtual ~ForwardMultiplyRenderCommand();

        public:
            virtual RenderType getRenderType() const = 0;
        };

        class TrianglesMeshRenderCommand
        : public ForwardMultiplyRenderCommand
        {
        public:
            TrianglesMeshRenderCommand();
            virtual ~TrianglesMeshRenderCommand();
            
        public:
            virtual RenderType getRenderType() const override { return RenderType::TrianglesMesh; }

        public:
            void setTexture(Texture2D* texture);
            Texture2D* getTexture() const;
        private:
            Texture2D* _texture;

        public:
            void setBlendType(BlendType blendType);
            BlendType getBlendType() const;
        private:
            BlendType _blendType;

        public:
            ArrayBuffer<ZAY_Vertice>& getVerticesBuffer(); //bx
            const ArrayBuffer<ZAY_Vertice>& getVerticesBuffer() const; //bx
        private:
            ArrayBuffer<ZAY_Vertice> _verticesBuffer; //bx

        public:
            ArrayBuffer<GLushort>& getIndicesBuffer();
            const ArrayBuffer<GLushort>& getIndicesBuffer() const;
        private:
            ArrayBuffer<GLushort> _indicesBuffer;
            
        public:
            void setMultiplyMapMultiplier(float multiplier);
            float getMultiplyMapMultiplier() const;
        private:
            float _multiplyMapMultiplier;
        };

        
        
        
        
        
        
        
        class MultiplyMapRenderCommand
        : public ForwardMultiplyRenderCommand
        {
        public:
            MultiplyMapRenderCommand();
            virtual ~MultiplyMapRenderCommand();
            
        public:
            virtual RenderType getRenderType() const override { return RenderType::MultiplyMap; }
            
        public:
            void setTexture(Texture2D* texture);
            Texture2D* getTexture() const;
        private:
            Texture2D* _texture;
            
        public:
            ArrayBuffer<ZAY_Vertice>& getVerticesBuffer(); //bx
        private:
            ArrayBuffer<ZAY_Vertice> _verticesBuffer; //bx
            
        public:
            ArrayBuffer<GLushort>& getIndicesBuffer();
        private:
            ArrayBuffer<GLushort> _indicesBuffer;
        };

        
        
        

        class ForwardMultiplyRenderer
        : public Renderer
        {
        public:
            typedef enum
            {
                Position, Color, TexCoord, Normal //bx
            } VertexAttrib;

        public:
            ForwardMultiplyRenderer();
        public:
            virtual ~ForwardMultiplyRenderer();
            
        public:
            void setMultiplyMapWidth(int32_t width);
            void setMultiplyMapHeight(int32_t height);
            void setMultiplyMapBorder(int32_t border);
            int32_t getMultiplyMapWidth() const;
            int32_t getMultiplyMapHeight() const;
            int32_t getMultiplyMapBorder() const;
        private:
            int32_t _multiplyMapWidth;
            int32_t _multiplyMapHeight;
            int32_t _multiplyMapBorder;
            bool _needToRegenerateMultiplyMap;

        public:
            virtual void init() override;
            virtual void term() override;

        public:
            virtual void _notifyMVPMatrixChanged() override;

            // frame buffer objects
        public:
            void setOriginalFBO(GLint originalFBO);
            GLint getOriginalFBO() const;
        private:
            GLint _originalFBO;

            GLuint _multiplyPassFBO;

            // render texture
        private:
            GLuint _multiplyMapRenderTexture;

            // white texture
        public:
            Texture2D* getWhiteTexture() const;
        private:
            Texture2D* _whiteTexture;

            // multiply map pass shader
        private:
            //GLuint _multiplyMapPassProgram;
            //GLuint _multiplyMapPassVertexShader;
            //GLuint _multiplyMapPassFragmentShader;
            //GLint _multiplyMapPassLocationTexture;
            //GLint _multiplyMapPassLocationMVPMatrix;
            //GLuint _multiplyMapPassVAO;

            // final(multiply) pass shader
        private:
            GLuint _finalPassProgram;
            GLuint _finalPassVertexShader;
            GLuint _finalPassFragmentShader;
            //GLint _finalPassLocationMultiplyMapTexture;
            //GLint _finalPassLocationMultiplyMapMultiplier;
            GLint _finalPassLocationColorTexture;
            GLint _finalPassLocationMVPMatrix;
            GLuint _finalPassVAO;

        public: //bx
            virtual void _setRenderMode(const float mode) override;

        private: //bx
            GLint _finalPassRenderModeLocation;
            GLfloat _finalPassRenderMode;

        public:
            void checkAndRemakeShaders();
        public:
            virtual void render(bool shadow) override;
            virtual void update() override;
            
            
            // multiply map render
        private:
            void _multiplyRenderStart();
            void _renderMultiplyMap(MultiplyMapRenderCommand* renderCommand);
            void _renderCurrentMultiplyMapContext();
            void _multiplyRenderEnd();
        private:
            void _setMultiplyMapTexture(Texture2D* texture);
            void _addMultiplyMapVerticesAndIndices(ZAY_Vertice* verticesPointer,
                                                   int32_t verticesCount,
                                                   GLushort* indices,
                                                   int32_t indicesCount);

            // triangles mesh render
        private:
            void _trianglesMeshRenderStart();
            void _renderTrianglesMesh(TrianglesMeshRenderCommand* renderCommand);
            void _renderCurrentTrianglesMeshContext();
            void _trianglesMeshRenderEnd();
        private:
            void _setTrianglesMeshTexture(Texture2D* texture);
            void _setTrianglesMeshBlendType(BlendType blendType);
            void _setTrianglesMeshMultiplyMapMultiplier(float multiplier);
            void _addTrianglesMeshVerticesAndIndices(ZAY_Vertice* vertices,
                                                     int32_t verticesCount,
                                                     GLushort* indices,
                                                     int32_t indicesCount);
            
        private:
            Texture2D* _currentTexture;
            BlendType _currentBlendType;
            //float _currentMultiplyMapMultiplier;
        private:
            BlendType _forcedBlendType; //bx
        private:
            int32_t _verticesCapacity;
            ZAY_Vertice* _vertices; //bx
            int32_t _verticesCount;
            GLuint _vbo;
        private:
            int32_t _indicesCapacity;
            GLushort* _indices;
            int32_t _indicesCount;
            GLuint _ibo;

        public:
            typedef boost::bimap<boost::bimaps::multiset_of<RenderPriority>, boost::bimaps::set_of<ForwardMultiplyRenderCommand*>> RenderCommandMap;
        public:
            virtual void updateRenderPriorities() override;
        public:
            virtual void _addRenderCommand(RenderCommand* renderCommand) override;
            virtual void _removeRenderCommand(RenderCommand* renderCommand) override;
            virtual void _notifyModifyRenderPriority(RenderCommand* renderCommand) override;
        public:
            void clearRenderCommands();
            const RenderCommandMap& getRenderPriorityWithType(RenderType type) const;
            const std::array<RenderCommandMap, static_cast<uint32_t>(RenderType::COUNT)>& getRenderPrioritiesAll() const;
        private:
            std::array<RenderCommandMap, static_cast<uint32_t>(RenderType::COUNT)> _renderPriorities;
        private:
            std::set<ForwardMultiplyRenderCommand*> _modifiedRenderCommands;
        };

        
    }
}
