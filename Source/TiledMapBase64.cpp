//
//  TiledMapBase64.cpp
//
//  (c)2014 KISS Projekt
//
//  KissProjekt@hotmail.com
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must
// not claim that you wrote the original software. If you use this
// software in a product, an acknowledgment in the product documentation
// would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and
// must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
// distribution.

/*
RFC 4648 tested - ftp://ftp.isi.edu/in-notes/rfc4648.txt
 
std::vector<uint8_t> decodedData;

Tiled::Base64Decoder::Decode(decodedData, "");
Tiled::Base64Decoder::Decode(decodedData, "Zg==");
Tiled::Base64Decoder::Decode(decodedData, "Zm8=");
Tiled::Base64Decoder::Decode(decodedData, "Zm9v");
Tiled::Base64Decoder::Decode(decodedData, "Zm9vYg==");
Tiled::Base64Decoder::Decode(decodedData, "Zm9vYmE=");
Tiled::Base64Decoder::Decode(decodedData, "Zm9vYmFy");
*/

#include "TiledMapBase64.h"

const uint8_t Tiled::Base64Decoder::Base64Decoder::decodeTable[256] =
{
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
	64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
	64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

int32_t Tiled::Base64Decoder::Decode( std::vector<uint8_t> &output, const std::string& input)
{
	output.clear();
	
	auto inputBegin= input.begin();
	while (inputBegin != input.end() && decodeTable[*inputBegin]==64)
	{
		++inputBegin;
	}
	auto inputEnd = inputBegin;
	while (inputEnd != input.end() && decodeTable[*inputEnd]!=64)
	{
		++inputEnd;
	}
	auto packetCount = std::distance(input.begin(), inputEnd);
	if(packetCount > 1)
	{
		uint32_t b24 = 0;
		
		output.resize((packetCount * 3) / 4);
		auto outputBegin= output.begin();
		
		auto remainder = packetCount % 4;
		inputEnd -= remainder;
		while (inputBegin!=inputEnd)
		{
			b24  = decodeTable[*inputBegin++] << 18;
			b24 |= decodeTable[*inputBegin++] << 12;
			b24 |= decodeTable[*inputBegin++] << 6;
			b24 |= decodeTable[*inputBegin++];
			*outputBegin++ = b24 >> 16;
			*outputBegin++ = (b24 >> 8) & 0xff;
			*outputBegin++ = (b24 & 0xff);
		}
		if(remainder>1)
		{
			b24  = decodeTable[*inputBegin++] << 18;
			b24 |= decodeTable[*inputBegin++] << 12;
			*outputBegin++ = b24 >> 16;
			if(remainder > 2)
			{
				b24 |= decodeTable[*inputBegin++] << 6;
				*outputBegin++ = (b24 >> 8) & 0xff;
			}
		}
	}
	return output.size();
}





