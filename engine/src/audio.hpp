// audio.hpp

#pragma once

#include "category.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace harpy
{
	class cContext;
	class iSoundAPI;
	class cOpenALSoundAPI;

	struct sWAVStructure
	{
		types::u8 _type[5] = {};
		types::u8 _format[5] = {};
		types::u8 _subchunk1ID[5] = {};
		types::u8 _subchunk2ID[5] = {};
		types::u32 _chunkSize = 0;
		types::u32 _subchunk1Size = 0;
		types::u32 _sampleRate = 0;
		types::u32 _byteRate = 0;
		types::u32 _subchunk2Size = 0;
		types::u16 _audioFormat = 0;
		types::u16 _numChannels = 0;
		types::u16 _blockAlign = 0;
		types::u16 _bitsPerSample = 0;
		types::u32 _numSamples = 0;
	};

	class cSound : public cObject
	{
		REALWARE_OBJECT(cSound)

		friend class cOpenALSoundAPI;

	public:
		enum class eFormat
		{
			NONE = 0,
			WAV
		};

	public:
		explicit cSound(cContext* context, eFormat format, const std::string& path);
		virtual ~cSound() override final;

		inline eFormat GetFormat() const { return _format; }
		inline types::u16 GetChannelCount() const { return _channelCount; }
		inline types::u16 GetBitsPerSample() const { return _bitsPerSample; }
		inline types::u32 GetSampleRate() const { return _sampleRate; }
		inline types::u16* GetData() const { return _data; }
		inline types::usize GetDataByteSize() const { return _dataByteSize; }
		inline types::u32 GetSource() const { return _source; }
		inline types::u32 GetBuffer() const { return _buffer; }
		
	private:
		inline void SetSource(types::u32 source) { _source = source; }
		inline void SetBuffer(types::u32 buffer) { _buffer = buffer; }

	private:
		iSoundAPI* _audioBackend = nullptr;
		eFormat _format = eFormat::NONE;
		types::u16 _channelCount = 0;
		types::u16 _bitsPerSample = 0;
		types::u32 _sampleRate = 0;
		types::u16* _data = nullptr;
		types::usize _dataByteSize = 0;
		types::u32 _source = 0;
		types::u32 _buffer = 0;
	};

	class cAudio : public cObject
	{
		REALWARE_OBJECT(cAudio)

	public:
		enum class API
		{
			NONE = 0,
			OAL,
		};

	public:
		explicit cAudio(cContext* context);
		virtual ~cAudio() override final;

		cSound* CreateSound(const std::string& id, cSound::eFormat format, const std::string& path);
		cSound* FindSound(const std::string& id);
		void DestroySound(const std::string& id);

		inline iSoundAPI* GetAPI() const { return _audioBackend; }
		void SetAPI(API api);

	private:
		API _audioBackendAPI = API::NONE;
		iSoundAPI* _audioBackend = nullptr;
		cIdVector<cSound>* _sounds;
	};
}