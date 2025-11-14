// sound_manager.hpp

#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace sound
    {
        class iSoundContext;

        struct sWAVStructure
        {
            types::u8 Type[5];
            types::u8 Format[5];
            types::u8 Subchunk1ID[5];
            types::u8 Subchunk2ID[5];
            types::u32 ChunkSize;
            types::u32 Subchunk1Size;
            types::u32 SampleRate;
            types::u32 ByteRate;
            types::u32 Subchunk2Size;
            types::u16 AudioFormat;
            types::u16 NumChannels;
            types::u16 BlockAlign;
            types::u16 BitsPerSample;
            types::u32 NumSamples;
            types::u32 DataByteSize;
            types::u16* Data;
        };

        struct sSound : public utils::sIdVecObject
        {
            explicit sSound(const app::cApplication* const app, const types::u32 source, const types::u32 buffer);
            ~sSound();

            app::cApplication* App = nullptr;
            game::Category Format = game::Category::SOUND_FORMAT_WAV;
            sWAVStructure* File = nullptr;
            types::u32 Source = 0;
            types::u32 Buffer = 0;
        };

        class mSound
        {
        public:
            mSound(const app::cApplication* const app, const iSoundContext* const context);
            ~mSound() = default;

            sSound* CreateSound(const std::string& id, const std::string& filename, const game::Category& format);
            sSound* FindSound(const std::string& id);
            void DestroySound(const std::string& id);

        private:
            app::cApplication* _app = nullptr;
            iSoundContext* _context = nullptr;
            utils::cIdVec<sSound> _sounds;
        };
    }
}