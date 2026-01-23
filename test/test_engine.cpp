//
// Created by intwi on 2026/01/20.
//

#include "wistlib/audio_engine.h"
#include <gtest/gtest.h>

using namespace wwist::audio_engine;

TEST(EngineTest, EngineInitializeSharedMode) {
	std::unique_ptr<AudioEngine> engine = std::make_unique<AudioEngine>(AudioEngine::AudioStreamMode::RENDER_SHARED);
	engine.reset();
}