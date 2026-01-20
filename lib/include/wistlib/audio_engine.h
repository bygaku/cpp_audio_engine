//
// Created by intwi on 2026/01/15.
//

#ifndef AUDIO_DSP_AUDIO_ENGINE_H
#define AUDIO_DSP_AUDIO_ENGINE_H

#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <wrl.h>
#include <memory>
#include <array>
#include <thread>
#include <atomic>
#include "data/data_type.h"

using namespace Microsoft::WRL;

struct IMMDevice;
struct IAudioClient3;
struct IAudioRenderClient;


namespace wwist::audio_engine {
	/**
	 * @brief Represents an audio engine for rendering audio streams.
	 */
	class AudioEngine {
	public:
		// Stream mode.
		enum class AudioStreamMode {
			RENDER_SHARED,
			RENDER_EXCLUSIVE
		};

	public:

		explicit AudioEngine(const AudioStreamMode& mode = AudioStreamMode::RENDER_SHARED);


		~AudioEngine();


		HRESULT Initialize();


		void Terminate();

		/* TODO: There is scope for extension.
		HRESULT CreateClip();


		HRESULT CreateSource();
		*/

		void Update();


		HRESULT AssignRenderDevice(const uint32_t& device_idx = 0);


		HRESULT Start() const;


		HRESULT Stop() const;

	private:

		void RenderStream();


		void ActivateSharedStream();


		void ActivateExclusiveStream();


		void EnumerateRenderDevice();


		HRESULT CreateRenderer();

		/**
		 * @brief
		 * @attention Always perform a cast to WAVEFORMATEX or WAVEFORMATEXTENSIBLE when using this method.
		 * @return
		 */
		[[nodiscard]] void* GetFormat();

	private:
		std::atomic<bool>	  			running_;

		ComPtr<IMMDevice>				device_;
		ComPtr<IMMDeviceEnumerator> 	device_enum_;
		ComPtr<IAudioClient3>			audio_client_;
		ComPtr<IAudioRenderClient>		render_client_;		// Get the buffet from Client. for sending to render Endpoints, Subscribe to IAudioClient.
		ComPtr<IMMDeviceCollection> 	render_devices_;	// Collection of All audio render endpoints (e.g., Headphones, Speakers).

		WAVEFORMATEX*					stream_format_;
		AudioStreamMode					stream_mode_;

		std::atomic<uint32_t> 			next_buffer_;
		std::array<void*, 2>			event_handle_;
		std::thread			  			render_thread_;
	};

}

#endif //AUDIO_DSP_AUDIO_ENGINE_H
