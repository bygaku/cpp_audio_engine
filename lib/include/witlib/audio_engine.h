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
	 * @class AudioEngine
	 * @brief A class designed to manage audio processing, including a playback and streaming functionality etc.
	 *
	 * @note Entry point must call CoInitializeEx(nullptr, COINIT_MULTITHREADED) before construction
	 *       and CoUninitialize() after AudioEngine::Terminate() and AudioEngine::~AudioEngine() for proper COM initialization.
	 */
	class AudioEngine {
	public:
		/**
		 * @enum AudioStreamMode
		 * @brief Define the stream modes for a playback.
		 */
		enum class AudioStreamMode {
			RENDER_SHARED,
			RENDER_EXCLUSIVE
		};

		/**
		 * @brief Constructs the AudioEngine instance with the specified audio stream mode.
		 *
		 * @param mode The audio stream mode to be used (e.g., AudioStreamMode::RENDER_SHARED(default)
		 * or AudioStreamMode::RENDER_EXCLUSIVE).
		 *
		 * @attention Calling after CoInitializeEx(nullptr, COINIT_MULTITHREADED).
		 */
		explicit AudioEngine(const AudioStreamMode& mode = AudioStreamMode::RENDER_SHARED);

		/**
		 * @brief Destructs the AudioEngine instance by stopping
		 * the audio rendering process and releasing associated resources.
		 *
		 * @attention Calling explicitly before CoUninitialize().
		 */
		~AudioEngine();

		/**
		 * @brief Initializes the AudioEngine instance and sets up the necessary components
		 * for audio rendering based on the specified stream mode.
		 *
		 * @return HRESULT indicating the success or failure of the initialization process.
		 * Possible return values include S_OK on success or an error code on failure.
		 */
		HRESULT Initialize();

		/* TODO: There is scope for extension.
		HRESULT CreateClip();


		HRESULT CreateSource();
		*/

		/**
		 * @brief Updates the state of the AudioEngine, managing ongoing audio processes and handling active audio sources.
		 *
		 * @note Ensure that the AudioEngine instance has been properly initialized by calling Initialize()
		 * before invoking this method.
		 */
		void Update();

		/**
		 * @brief Assigns a render device for audio output based on the provided device index.
		 *
		 * @param device_idx The index of the render device to assign. Defaults to 0.
		 * @return HRESULT indicating the success or failure of the operation.
		 *
		 * @note Ensure that the AudioEngine instance has been properly initialized by calling Initialize()
		 * before invoking this method.
		 */
		HRESULT AssignRenderDevice(const uint32_t& device_idx = 0);

		/**
		 * @brief Starts the audio playback by triggering the audio client's rendering process.
		 *
		 * @return HRESULT indicating the success or failure of the start operation.
		 * Possible return values include S_OK on success or an error code on failure.
		 *
		 * @note Ensure that the AudioEngine instance has been properly initialized by calling Initialize()
		 * before invoking this method.
		 */
		HRESULT Start() const;

		/**
		 * @brief Stops the audio playback by halting the audio client's rendering process.
		 *
		 * @return HRESULT indicating the success or failure of the stop operation.
		 * Possible return values include S_OK on success or an error code on failure.
		 *
		 * @note Ensure that the audio playback has been started by calling Start() before invoking this method.
		 */
		HRESULT Stop() const;

		/**
		 * @brief Provides access to the collection of audio render devices available on the system.
		 *
		 * @return A pointer to an IMMDeviceCollection containing all available audio render endpoints,
		 * such as headphones or speakers.
		 *
		 * @note The returned pointer is managed internally. Ensure proper initialization
		 * of the AudioEngine instance before calling this method.
		 */
		IMMDeviceCollection* GetRenderDevices() const { return render_devices_.Get(); }

	private:
		void RenderStream();

		HRESULT ActivateSharedStream();

		HRESULT ActivateExclusiveStream();

		void EnumerateRenderDevice();

		HRESULT CreateRenderer();

	private:
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
