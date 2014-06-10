//
//  TiledMapBase.h
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

#pragma once
#include <cstdint>
#include <functional>

namespace Tiled
{
	static constexpr uint32_t fnv32_prime = 0x01000193;
	static constexpr uint32_t fnv32_start = 0x811C9DC5;
	static constexpr uint32_t consthash_fnv1a (const char * const string, uint32_t hash = fnv32_start)
	{
		return string[0] == 0 ? hash : consthash_fnv1a (string + 1, fnv32_prime * (hash ^ string[0]));
	}
	
	
	//enum class TileMask : uint32_t
	//{
	const uint32_t FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
	const uint32_t FLIPPED_VERTICALLY_FLAG   = 0x40000000;
	const uint32_t FLIPPED_DIAGONALLY_FLAG   = 0x20000000;
	const uint32_t TILESET_MASK			  	 = 0x0FF00000;
	const uint32_t TILESET_SHIFT			 = 20;
	const uint32_t TILE_MASK				 = 0x000FFFFF;
    //};
    
	//enum class DrawStyle : uint32_t
	//{
	const uint32_t TiledScrH = 0x00000001;
	const uint32_t TiledScrV = 0x00000002;
	const uint32_t TiledWrpH = 0x00000004;
	const uint32_t TiledWrpV = 0x00000008;
	//};
	
	template <typename T>
	union Relocator
	{
	    uint64_t offset;
	    T *ptr;
	};
	
	struct Point
	{
		float x, y;
	};
	
	struct TileSet
	{
		int32_t         imageWidth, imageHeight;
		uint32_t        tileWidth, tileHeight;
		uint32_t		firstTileID, lastTileID;
		Relocator<char> imageFileName;
	};
	
	struct TileData
	{
		int32_t			tileSet;
		float			u0, v0, u1, v1;
	};
	
	struct MapLayer
	{
		int32_t             layerWidth, layerHeight;
		float               layerOpacity;
	    int32_t             pad; // 64bit alignment
		Relocator<char>     layerName;
		Relocator<uint32_t> layerData;
	};
	
	struct Property
	{
		uint32_t		propertyType;
		union
	    {
			uint32_t	_hash;
			float		_number;
		}	propertyData;
	    Relocator<char> propertyName;
	};
	
	struct Entity
	{
		uint32_t               objectTypeHash;
		int32_t                tiledID;
		int32_t                numProperties;
		int32_t                numPoints;
		Relocator<char>        objectName;
		Relocator<Property>    properties;
		Relocator<Point>       points;
	};
	
	struct MapHeader
	{
		float				version;
		//TiledMapDrawStyle	mapFlags;
		int32_t				blocksWide,		blocksHigh;
		int32_t				tileWidth,		tileHeight;
		float				pixelsWidth,	pixelsHeight;
		int32_t				numTileSets,	numLayers;
		int32_t				numObjects;
		
		Relocator<TileSet>	tileSets;
		Relocator<TileData>	tileMetrics;
		
		Relocator<MapLayer>	mapLayers;
		Relocator<uint32_t>	mapData;
		
		Relocator<Entity>	entities;
		Relocator<Point>	points;
		
		Relocator<Property>	properties;
		Relocator<char>		strings;
		
	    void FixupPointers();
	};
	
	////////////////////////////////////////////////////////////////////////////////////
	
	class Map
	{
	public:
		Map( void *convertedMapData, int32_t screenPixelsWide, int32_t screenPixelsHigh, float mapX = 0.0f, float mapY = 0.0f )
		: mapData(static_cast<MapHeader*>(convertedMapData))
		, screenPixelsWide(screenPixelsWide)
		, screenPixelsHigh(screenPixelsHigh)
		, mapX(mapX)
		, mapY(mapY)
		{
			if (mapData)
			{
				mapData->FixupPointers();
				numTilesWide = ((screenPixelsWide + (mapData->tileWidth-1) ) / mapData->tileWidth ) + 1;
				numTilesHigh = ((screenPixelsHigh + (mapData->tileHeight-1)) / mapData->tileHeight) + 1;
				// Decide on map options, fit, wrap, scroll etc.
				//if (numTilesWide > mapData->blocksWide || numTilesHigh > mapData->blocksHigh) {
				//numTilesWide = mapData->blocksWide;
				//numTilesHigh = mapData->blocksHigh;
				//}
				maxTilesToRender = numTilesWide * numTilesHigh;
			}
		}
		
		virtual ~Map() {}
		virtual void Render( const std::function<void(uint32_t,bool)> &textureSetter ) const = 0;
		const MapHeader *MapData() const
		{
			return mapData;
		}
		
		float mapX, mapY;
		
		int32_t ScreenPixelsWide() const { return screenPixelsWide; }
		int32_t ScreenPixelsHigh() const { return screenPixelsHigh; }
		
	protected:
		MapHeader *mapData;
		int32_t screenPixelsWide;
		int32_t	screenPixelsHigh;
		int32_t numTilesHigh;
		int32_t numTilesWide;
		int32_t maxTilesToRender;
		
 	private:
		Map();
    };
	
	
}
