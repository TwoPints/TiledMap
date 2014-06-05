//
//  TiledMapBase.cpp
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

#include "TiledMapBase.h"

void Tiled::MapHeader::FixupPointers()
{
	uint64_t mapDataIndex = reinterpret_cast<uint64_t>(this);
	tileSets.offset		+= mapDataIndex;
	tileMetrics.offset	+= mapDataIndex;
	mapLayers.offset	+= mapDataIndex;
	mapData.offset		+= mapDataIndex;
	entities.offset		+= mapDataIndex;
	points.offset		+= mapDataIndex;
	properties.offset	+= mapDataIndex;
	strings.offset		+= mapDataIndex;
	
	for( int layerIdx(0); layerIdx<numLayers; ++layerIdx )
    {
        MapLayer &pLayer = mapLayers.ptr[layerIdx];
	    pLayer.layerName.offset += strings.offset;
	    pLayer.layerData.offset += mapData.offset;
	}
	
	for( int objectIdx(0); objectIdx<numObjects; ++objectIdx )
    {
	    Entity &entity = entities.ptr[objectIdx];
	    entity.objectName.offset += strings.offset;
	    entity.properties.offset += properties.offset;
	    entity.points.offset += points.offset;
    	for( int propertyIdx(0); propertyIdx<entity.numProperties; ++propertyIdx )
        {
    	    entity.properties.ptr[propertyIdx].propertyName.offset += strings.offset;
    	}
	}
	
	for( int textureIdx(0); textureIdx<numTileSets; ++textureIdx )
	{
	    TileSet &pTileSet = tileSets.ptr[textureIdx];
	    pTileSet.imageFileName.offset += strings.offset;
	}
}