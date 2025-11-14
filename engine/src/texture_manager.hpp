// texture_manager.hpp

#pragma once

#include <string>
#include <vector>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace render
    {
        class iRenderContext;
        struct sTexture;

        struct sTextureAtlasTexture : public utils::sIdVecObject
        {
            sTextureAtlasTexture(const types::boolean isNormalized, const glm::vec3& offset, const glm::vec2& size);
            ~sTextureAtlasTexture() = default;

            types::boolean IsNormalized = types::K_FALSE;
            glm::vec3 Offset = glm::vec3(0.0f);
            glm::vec2 Size = glm::vec2(0.0f);
        };

        struct sTextureAtlasTextureGPU
        {
            glm::vec4 TextureInfo = glm::vec4(0.0f);
            types::f32 TextureLayerInfo = 0.0f;
        };

        class mTexture
        {
        public:
            explicit mTexture(const app::cApplication* const app, const iRenderContext* const context);
            ~mTexture();

            sTextureAtlasTexture* CreateTexture(const std::string& id, const glm::vec2& size, const types::usize channels, const types::u8* data);
            sTextureAtlasTexture* CreateTexture(const std::string& id, const std::string& filename);
            sTextureAtlasTexture* FindTexture(const std::string& id);
            void DestroyTexture(const std::string& id);

            sTextureAtlasTexture CalculateNormalizedArea(const sTextureAtlasTexture& area);

            sTexture* GetAtlas();
            types::usize GetWidth() const;
            types::usize GetHeight() const;
            types::usize GetDepth() const;

        protected:
            app::cApplication* _app = nullptr;
            iRenderContext* _context = nullptr;
            sTexture* _atlas = nullptr;
            utils::cIdVec<sTextureAtlasTexture> _textures;
        };
    }
}