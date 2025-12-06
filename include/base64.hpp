#ifndef BASE64_HPP
#define BASE64_HPP

#include <string>
#include <stdexcept>
#include <stdint.h>

/// A small library for Base64 encoding and decoding
/// (c) Trin Wasinger 2025
/// https://en.wikipedia.org/wiki/Base64
namespace base64 {
	namespace {
		const char ALPHABET[64] = {
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'L', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' 
		};
	
		const char PADDING_CHAR = '=';
		
		uint8_t decode(const char c) {
			if ('A' <= c && c <= 'Z') {
				return c - 'A';
			} else if ('a' <= c && c <= 'z') {
				return c - 'a' + 26;
			} else if ('0' <= c && c <= '9') {
				return c - '0' + 52;
			} else if ('+' == c) {
				return 62;
			} else if ('/' == c) {
				return 63;
			} else {
				throw std::runtime_error("Invalid Base-64 digit");
			}
		}
	}

    /// Decodes a base64 string into text
	/// Based on https://rosettacode.org/wiki/Base64_encode_data
	std::string encode(const std::string& text) {
		std::string buffer;
		buffer.reserve((text.size() + 2)/3*4);

		for(size_t n = 0; n < text.size(); n += 3) {
			const bool p3 = n + 1 < text.size(), p4 = n + 2 < text.size();
			const uint32_t acc = (((uint32_t) text[n]) << 16) | (p3 ? ((uint32_t) text[n + 1]) << 8 : 0) | (p4 ? (uint32_t) text[n + 2] : 0);
			buffer.push_back(ALPHABET[(acc >> 18) & 63]);
			buffer.push_back(ALPHABET[(acc >> 12) & 63]);
			buffer.push_back(p3 ? ALPHABET[(acc >> 6) & 63] : PADDING_CHAR);
			buffer.push_back(p4 ? ALPHABET[acc & 63] : PADDING_CHAR);
		}

		return buffer;
	}

    /// Encodes text to a base64 string
	/// Based on https://rosettacode.org/wiki/Base64_decode_data
	std::string decode(const std::string& text) {
		if(text.size() % 4) {
			throw std::runtime_error("Invalid Base-64 data");
		}

		std::string buffer;
		buffer.reserve(text.length()/4*3);

		for(size_t n = 0; n < text.size(); n += 4) {
			const uint8_t i1 = decode(text[n]), i2 = decode(text[n + 1]);
			buffer.push_back((i1 << 2) | (i2 >> 4));

			if(text[n + 2] != PADDING_CHAR) {
				const uint8_t i3 = decode(text[n + 2]);
				buffer.push_back(((i2 & 0xf) << 4) | (i3 >> 2));

				if(text[n + 3] != PADDING_CHAR) {
					const uint8_t i4 = decode(text[n + 3]);
					buffer.push_back(((i3 & 0x3) << 6) | i4);
				}
			}
		}

		return buffer;
	}
}
#endif