//
// Created by intwi on 2026/01/02.
//

#ifndef AUDIO_DSP_DATA_TYPE_H
#define AUDIO_DSP_DATA_TYPE_H

#include <cstdint>

namespace wwist {
	typedef unsigned char  	MY_BYTE;
	typedef unsigned short 	MY_WORD;
	typedef unsigned long  	MY_DWORD;
	typedef float          	BUF_TYPE;

}

namespace wwist::audio_engine {
	typedef MY_DWORD		CHUNK_ID;	// FOURcc

	/**
	 * @brief creates a new 4-character code from four characters.
	 * @code
	 *  constexpr CHUNK_ID YOU_ID = CHUNK_ID('Y', 'O', 'U', ' ');
	 * @endcode
	*/
#define Chunk_ID(a, b, c, d) (a + (b << 8) + (c << 16) + (d << 24))
	constexpr CHUNK_ID RIFF_ID 		= Chunk_ID('R', 'I', 'F', 'F');
	constexpr CHUNK_ID WAVE_ID 		= Chunk_ID('W', 'A', 'V', 'E');
	constexpr CHUNK_ID FMT_ID  		= Chunk_ID('f', 'm', 't', ' ');
	constexpr CHUNK_ID LIST_ID 		= Chunk_ID('L', 'I', 'S', 'T');
	constexpr CHUNK_ID DATA_ID 		= Chunk_ID('d', 'a', 't', 'a');
	constexpr MY_DWORD PCM     		= 16;
	constexpr MY_DWORD IEEE_FLOAT	= 32;

	/**
	 * @brief Stores activated AudioMetaData.
	 */
	struct StreamFormat {
		MY_WORD  format_type;	// PCM: (0x0100), IEEE float: (0x0300)
		MY_WORD  num_channels;	// mono: (0x0100), stereo: (0x0200)
		MY_DWORD sample_rate;	// 8kHz: (0x401F0000), 44.1kHz: (0x44AC0000)
		MY_WORD  bit_depth;		// bit depth (0x0800 or 0x1000)
		MY_WORD  block_size;	// channel * bit depth / 8
		MY_DWORD data_rate;		// sample_rate * block_size

	public:
		/**
		 * @brief StreamFormat structure.
		 */
		explicit StreamFormat(const MY_WORD	 fmt_type	= PCM
						  	, const MY_WORD	 chs		= 2
						  	, const MY_DWORD s_rate		= 44100
						  	, const MY_WORD	 b_depth	= 16)
			: format_type(fmt_type)
			, num_channels(chs)
			, sample_rate(s_rate)
			, bit_depth(b_depth)
			, block_size(chs * b_depth / 8)
			, data_rate(s_rate * block_size) {
		}

		/**
		 * @brief Get the block size of the audio data.
		 * @return The block size in bytes.
		 */
		const MY_WORD& GetBlockSize() const { return block_size; }

		/**
		 * @brief Get the data rate of the audio data.
		 * @return The data rate in bytes per second.
		 */
		const MY_DWORD& GetDataRate() const { return data_rate; }
	};
}

#endif //AUDIO_DSP_DATA_TYPE_H