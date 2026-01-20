//
// Created by intwi on 2026/01/16.
//

#include "wistlib/audio_engine.h"
#include <iostream>
#include <cmath>
#include <fstream>

using namespace wwist;
using namespace wwist::audio_engine;

AudioEngine::AudioEngine(const AudioStreamMode& mode)
	: running_(false)
	, device_(nullptr)
	, device_enum_(nullptr)
	, audio_client_(nullptr)
	, render_client_(nullptr)
	, render_devices_(nullptr)
	, stream_format_(new WAVEFORMATEX)
	, stream_mode_(mode)
	, next_buffer_(1)
	, event_handle_()
	, render_thread_() {
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	_ASSERT(SUCCEEDED(hr));

	void* start_event	  = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
	void* terminate_event = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
	event_handle_[0] = start_event;
	event_handle_[1] = terminate_event;

	EnumerateRenderDevice();
}

HRESULT AudioEngine::Initialize() {
	auto hr = S_OK;

	// Default Device assignment.
	hr = AssignRenderDevice();
	if (!SUCCEEDED(hr)) return hr;

	// Activate
	if (stream_mode_ == AudioStreamMode::RENDER_EXCLUSIVE) {
		ActivateExclusiveStream();
	} else {
		ActivateSharedStream();
	}

	// Create Audio Renderer.
	hr = CreateRenderer();
	if (!SUCCEEDED(hr)) return hr;

	// Audio render thread join.
	if (render_thread_.joinable() == false) {
		render_thread_ = std::thread(&AudioEngine::RenderStream, this);
	}

	return hr;
}

AudioEngine::~AudioEngine() {
}

void AudioEngine::Terminate() {
	// Terminate render thread.
	if (render_thread_.joinable() == true) {
		SetEvent(event_handle_[1]);
		render_thread_.join();
	}

	// Release handles.
	for (void*& i : event_handle_) {
		if (i != nullptr) CloseHandle(i);
	}
}

HRESULT AudioEngine::Start() const {
	auto hr = audio_client_->Start();
	_ASSERT(SUCCEEDED(hr));
	return hr;
}

HRESULT AudioEngine::Stop() const {
	auto hr = audio_client_->Stop();
	_ASSERT(SUCCEEDED(hr));
	return hr;
}

void AudioEngine::RenderStream() {
	// Retrieve initial frame.
	uint32_t frames = 0;
	auto hr = audio_client_->GetBufferSize(&frames);
	_ASSERT(SUCCEEDED(hr));

	uint32_t	padding = 0;
	BYTE*		buffer;

	while (true) {
		// Exit loop when set terminal_event.
		hr = WaitForMultipleObjects(event_handle_.size()
			, event_handle_.data()
			, false
			, INFINITE
		);
		if (hr == WAIT_OBJECT_0 + 1) break;

		// Retrieve current padding. Not working on RENDER_EXCLUSIVE.
		if (stream_mode_ == AudioStreamMode::RENDER_SHARED) {
			hr = audio_client_->GetCurrentPadding(&padding);
			_ASSERT(SUCCEEDED(hr));
		}

	 	// Filling the buffer.
		hr = render_client_->GetBuffer(frames - padding, &buffer);
		_ASSERT(SUCCEEDED(hr));

		// Set an initial value to all data.
		std::fill_n(buffer, frames - padding, 0);

		// Release current buffer.
		hr = render_client_->ReleaseBuffer(frames - padding, 0);
		_ASSERT(SUCCEEDED(hr));
	}
}

void AudioEngine::Update() {
	// Handling audio sources.
}

void AudioEngine::ActivateSharedStream() {
	auto hr = device_->Activate(__uuidof(IAudioClient)
									, CLSCTX_ALL
									, nullptr
									, &audio_client_
	);
	_ASSERT(SUCCEEDED(hr));

	// Retrieve the device period for latency from the audio client.
	REFERENCE_TIME def_period, min_period = 0;
	hr = audio_client_->GetDevicePeriod(&def_period, &min_period);
	_ASSERT(SUCCEEDED(hr));

	// Check whether the stream format is supported.
	stream_format_				  = static_cast<WAVEFORMATEX*>(GetFormat());
	WAVEFORMATEXTENSIBLE* closest = nullptr;

	hr = audio_client_->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED
									, stream_format_
									, reinterpret_cast<WAVEFORMATEX**>(&closest)
	);
	if (hr == S_FALSE && closest) {
		// Restore closest in StreamFormat structure.
		*stream_format_ = closest->Format;
	}

	// Initialize the audio client.
	hr = audio_client_->Initialize(AUDCLNT_SHAREMODE_SHARED
		, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST
		, def_period
		, 0
		, stream_format_
		, nullptr
	);

	// If the requested buffer size is not aligned...
	if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED)
	{
		// Get the next aligned frame.
		uint32_t n_frames = 0;
		hr = audio_client_->GetBufferSize(&n_frames);
		_ASSERT(SUCCEEDED(hr));

		def_period = static_cast<REFERENCE_TIME>(
			std::round(10000.0 * 1000 / stream_format_->nSamplesPerSec * n_frames)
		);

		// Release.
		audio_client_->Release();

		// Re-Activate the audio client.
		hr = device_->Activate(__uuidof(IAudioClient)
							, CLSCTX_ALL
							, nullptr
							, &audio_client_
		);
		_ASSERT(SUCCEEDED(hr));

		// Restore format.
		stream_format_ = static_cast<WAVEFORMATEX*>(GetFormat());
		closest = nullptr;

		hr = audio_client_->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED
										, stream_format_
										, reinterpret_cast<WAVEFORMATEX**>(&closest)
		);
		if (hr == S_FALSE && closest) {
			// Restore closest in StreamFormat structure.
			*stream_format_ = closest->Format;
		}

		// Re-Initialize.
		hr = audio_client_->Initialize(AUDCLNT_SHAREMODE_SHARED
			, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST
			, def_period
			, 0
			, stream_format_
			, nullptr
		);

		_ASSERT(SUCCEEDED(hr));
	}

	hr = audio_client_->SetEventHandle(event_handle_[0]);
	_ASSERT(SUCCEEDED(hr));

}

void AudioEngine::ActivateExclusiveStream() {
	// Activate the audio client.
	auto hr = device_->Activate(__uuidof(IAudioClient)
										, CLSCTX_ALL
										, nullptr
										, &audio_client_
	);
	_ASSERT(SUCCEEDED(hr));

	// Check whether the stream format is supported.
	stream_format_ = static_cast<WAVEFORMATEX*>(GetFormat());

	hr = audio_client_->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE
										, stream_format_
										, nullptr
	);
	_ASSERT(hr == S_OK);	// S_FALSE and AUDCLNT_E_UNSUPPORTED_FORMAT are not allowed.

	// Retrieve the device period for latency from the audio client.
	REFERENCE_TIME def_period, min_period = 0;
	hr = audio_client_->GetDevicePeriod(&def_period, &min_period);
	_ASSERT(SUCCEEDED(hr));

	// Initialize the audio client.
	hr = audio_client_->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE
		, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST
		, def_period
		, def_period
		, stream_format_
		, nullptr
	);

	// If the requested buffer size is not aligned...
	if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
		// Get the next aligned frame.
		uint32_t n_frames = 0;
		hr = audio_client_->GetBufferSize(&n_frames);
		_ASSERT(SUCCEEDED(hr));

		def_period = static_cast<REFERENCE_TIME>(
			std::round(10000.0 * 1000 / stream_format_->nSamplesPerSec * n_frames)
		);

		// Release.
		audio_client_->Release();

		// Re-Activate the audio client.
		hr = device_->Activate(__uuidof(IAudioClient)
							, CLSCTX_ALL
							, nullptr
							, &audio_client_
		);
		_ASSERT(SUCCEEDED(hr));

		// Restore format.
		hr = audio_client_->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE
										, stream_format_
										, nullptr
			);
		_ASSERT(hr == S_OK);	// S_FALSE and AUDCLNT_E_UNSUPPORTED_FORMAT are not allowed.

		// Re-Initialize.
		hr = audio_client_->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE
			, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST
			, def_period
			, def_period
			, stream_format_
			, nullptr
		);
		_ASSERT(SUCCEEDED(hr));
	}

	hr = audio_client_->SetEventHandle(event_handle_[0]);
	_ASSERT(SUCCEEDED(hr));
}

void AudioEngine::EnumerateRenderDevice() {
	// Initialize Device enumerator.
	auto hr = CoCreateInstance(__uuidof(MMDeviceEnumerator)
					, nullptr
					, CLSCTX_ALL
					, __uuidof(IMMDeviceEnumerator)
					, &device_enum_
	);
	_ASSERT(SUCCEEDED(hr));

	// Assign the render device to endpoints_ pointer.
	hr = device_enum_->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &render_devices_);
	_ASSERT(SUCCEEDED(hr));
}

HRESULT AudioEngine::CreateRenderer() {
	// Activate the audio render client (renderer).
	auto hr = audio_client_->GetService(__uuidof(IAudioRenderClient), &render_client_);
	if (!SUCCEEDED(hr)) return hr;

	// Clear the initial buffer.
	uint32_t		frame	= 0;
	unsigned char*	buf		= nullptr;

	hr = audio_client_->GetBufferSize(&frame);
	if (!SUCCEEDED(hr)) return hr;

	hr = render_client_->GetBuffer(frame, &buf);
	if (!SUCCEEDED(hr)) return hr;

	hr = render_client_->ReleaseBuffer(frame, AUDCLNT_BUFFERFLAGS_SILENT);
	if (!SUCCEEDED(hr)) return hr;

	return hr;
}

HRESULT AudioEngine::AssignRenderDevice(const uint32_t &device_idx) {
	// Count the number of devices.
	unsigned int num_devices = 0;
	auto hr = render_devices_->GetCount(&num_devices);
	_ASSERT(SUCCEEDED(hr));

	// Error
	if (device_idx > num_devices) return hr;

	// Activate.
	if (device_idx == 0) {
		// Using a default audio render device.
		hr = device_enum_->GetDefaultAudioEndpoint(eRender, eConsole, &device_);
		_ASSERT(SUCCEEDED(hr));
	} else {
		// A selected device.
		hr = render_devices_->Item(device_idx, &device_);
		_ASSERT(SUCCEEDED(hr));
	}

	return true;
}

void* AudioEngine::GetFormat() {
	WAVEFORMATEXTENSIBLE* fmt;
	auto hr = S_OK;

	// Mode::Shared
	if (stream_mode_ == AudioStreamMode::RENDER_SHARED) {
		hr = audio_client_->GetMixFormat(reinterpret_cast<WAVEFORMATEX**>(&fmt));
		_ASSERT(SUCCEEDED(hr));
	}
	// Mode::Exclusive
	else {
		ComPtr<IPropertyStore> store = nullptr;
		hr = device_->OpenPropertyStore(STGM_READ, &store);
		_ASSERT(SUCCEEDED(hr));

		PROPVARIANT var{};
		hr = store->GetValue(PKEY_AudioEngine_DeviceFormat, &var);
		_ASSERT(SUCCEEDED(hr));

		fmt = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(var.blob.pBlobData);
	}

	return fmt;
}
