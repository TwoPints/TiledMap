//
//  TiledMapParser.h
//
//  Created by KISS Projekt on 30/05/2014.
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

#pragma once
#include "TiledMapBase.h"
#include "tinyxml2.h"
#include <vector>

class TMXConverter {
public:
    
    inline uint32_t NextPOT(uint32_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }
	
	bool ProcessMap( const char *TMXXmlData );
 	uint64_t SaveMap( char* &buffer );
	
private:
    uint32_t AddString( const char *string );
    uint32_t AddProperties( tinyxml2::XMLElement *parentElement, uint64_t &propertiesOffset );
	
	// I don't like this.
	void *Copy(void * &destination, const void * source, size_t num)
	{
		void * retVal = static_cast<uint8_t*>(memcpy(destination, source, num)) + num;
		destination = retVal;
		return retVal;
	}
	
	Tiled::MapHeader 				m_header;
    std::vector<Tiled::TileSet> 	m_tileSets;
    std::vector<Tiled::TileData> 	m_tileMetrics;
    std::vector<Tiled::MapLayer> 	m_mapLayers;
    std::vector<uint32_t> 			m_mapData;
    std::vector<Tiled::Entity> 		m_objects;
    std::vector<Tiled::Point> 		m_points;
    std::vector<Tiled::Property>	m_properties;
    std::vector<char> 				m_stringBuffer;
};
