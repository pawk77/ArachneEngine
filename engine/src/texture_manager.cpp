// texture_manager.cpp

#define STB_IMAGE_IMPLEMENTATION
#include "../../thirdparty/stb-master/stb_image.h"
#include "application.hpp"
#include "texture_manager.hpp"
#include "render_context.hpp"
#include "memory_pool.hpp"
#include "log.hpp"

using namespace types;

namespace realware
{
    using namespace app;
    using namespace game;
    using namespace log;
    using namespace utils;

    namespace render
    {
        sTextureAtlasTexture::sTextureAtlasTexture(const types::boolean isNormalized, const glm::vec3& offset, const glm::vec2& size) : IsNormalized(isNormalized), Offset(offset), Size(size)
        {
        }

        mTexture::mTexture(const cApplication* const app, const iRenderContext* const context) : _app((cApplication*)app), _textures((cApplication*)app, ((cApplication*)app)->GetDesc()->MaxTextureCount)
        {
            sApplicationDescriptor* desc = _app->GetDesc();

            _context = (iRenderContext*)context;
            _atlas = _context->CreateTexture(
                desc->TextureAtlasWidth,
                desc->TextureAtlasHeight,
                desc->TextureAtlasDepth,
                sTexture::eType::TEXTURE_2D_ARRAY,
                sTexture::eFormat::RGBA8_MIPS,
                nullptr
            );
            _atlas->Slot = 0;
        }

        mTexture::~mTexture()
        {
            _context->DestroyTexture(_atlas);
        }

        sTextureAtlasTexture* mTexture::CreateTexture(const std::string& id, const glm::vec2& size, const usize channels, const u8* data)
        {
            const usize width = size.x;
            const usize height = size.y;

            if (data == nullptr || channels != 4)
            {
                Print("Error: you can only create texture with 4 channels in RGBA format!");

                return nullptr;
            }

            const auto& textures = _textures.GetObjects();
            for (usize layer = 0; layer < _atlas->Depth; layer++)
            {
                for (usize y = 0; y < _atlas->Height; y++)
                {
                    for (usize x = 0; x < _atlas->Width; x++)
                    {
                        types::boolean isIntersecting = K_FALSE;

                        for (auto& area : textures)
                        {
                            if (area.IsNormalized == K_FALSE)
                            {
                                const glm::vec4 textureRect = glm::vec4(
                                    x, y, x + width, y + height
                                );
                                if ((area.Offset.z == layer &&
                                    area.Offset.x <= textureRect.z && area.Offset.x + area.Size.x >= textureRect.x &&
                                    area.Offset.y <= textureRect.w && area.Offset.y + area.Size.y >= textureRect.y) ||
                                    (x + width > _atlas->Width || y + height > _atlas->Height))
                                {
                                    isIntersecting = K_FALSE;
                                    break;
                                }
                            }
                            else if (area.IsNormalized == K_TRUE)
                            {
                                const glm::vec4 textureRectNorm = glm::vec4(
                                    (f32)x / (f32)_atlas->Width, (f32)y / (f32)_atlas->Height,
                                    ((f32)x + (f32)width) / (f32)_atlas->Width, ((f32)y + (f32)height) / (f32)_atlas->Height
                                );
                                if ((area.Offset.z == layer &&
                                    area.Offset.x <= textureRectNorm.z && area.Offset.x + area.Size.x >= textureRectNorm.x &&
                                    area.Offset.y <= textureRectNorm.w && area.Offset.y + area.Size.y >= textureRectNorm.y) ||
                                    (textureRectNorm.z > 1.0f || textureRectNorm.w > 1.0f))
                                {
                                    isIntersecting = true;
                                    break;
                                }
                            }
                        }

                        if (!isIntersecting)
                        {
                            const glm::vec3 offset = glm::vec3(x, y, layer);
                            const glm::vec2 size = glm::vec2(width, height);

                            _context->WriteTexture(_atlas, offset, size, data);
                            if (_atlas->Format == render::sTexture::eFormat::RGBA8_MIPS)
                                _context->GenerateTextureMips(_atlas);

                            sTextureAtlasTexture* newTex = _textures.Add(id, K_FALSE, offset, size);
                            *newTex = CalculateNormalizedArea(*newTex);

                            return newTex;
                        }
                    }
                }
            }

            return nullptr;
        }

        sTextureAtlasTexture* mTexture::CreateTexture(const std::string& id, const std::string& filename)
        {
            const usize channelsRequired = 4;

            s32 width = 0;
            s32 height = 0;
            s32 channels = 0;
            u8* data = nullptr;
            data = stbi_load(filename.c_str(), &width, &height, &channels, channelsRequired);

            return CreateTexture(id, glm::vec2(width, height), channelsRequired, data);
        }

        sTextureAtlasTexture* mTexture::FindTexture(const std::string& id)
        {
            return _textures.Find(id);
        }

        void mTexture::DestroyTexture(const std::string& id)
        {
            _textures.Delete(id);
        }

        sTextureAtlasTexture mTexture::CalculateNormalizedArea(const sTextureAtlasTexture& area)
        {
            sTextureAtlasTexture norm = sTextureAtlasTexture(
                types::K_TRUE,
                glm::vec3(area.Offset.x / _atlas->Width, area.Offset.y / _atlas->Height, area.Offset.z),
                glm::vec2(area.Size.x / _atlas->Width, area.Size.y / _atlas->Height)
            );

            return norm;
        }

        sTexture* mTexture::GetAtlas()
        {
            return _atlas;
        }

        usize mTexture::GetWidth() const
        {
            return _atlas->Width;
        }

        usize mTexture::GetHeight() const
        {
            return _atlas->Height;
        }

        usize mTexture::GetDepth() const
        {
            return _atlas->Depth;
        }
    }
}