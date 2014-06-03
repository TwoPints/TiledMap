//
//  TiledMapParser.cpp
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

#include "TiledMapParser.h"

bool TMXConverter::ProcessMap( const char *TMXXmlData )
{
	tinyxml2::XMLDocument _document;
	_document.Parse(TMXXmlData);
        
	tinyxml2::XMLElement *mapElement = _document.FirstChildElement("map");
    if (!mapElement)
    {
		return false;
	}
        
    m_header.version = atof(mapElement->Attribute("version"));
    m_header.blocksWide = atoi(mapElement->Attribute("width"));
    m_header.blocksHigh = atoi(mapElement->Attribute("height"));
    m_header.tileWidth = atoi(mapElement->Attribute("tilewidth"));
    m_header.tileHeight  = atoi(mapElement->Attribute("tileheight"));
    m_header.pixelsWidth = m_header.blocksWide * m_header.tileWidth;
    m_header.pixelsHeight = m_header.blocksHigh * m_header.tileHeight;
        
    tinyxml2::XMLElement *tileSetElement = mapElement->FirstChildElement("tileset");
    while (tileSetElement)
    {
        if ( tinyxml2::XMLElement *imageElement = tileSetElement->FirstChildElement("image"))
        {
            Tiled::TileSet tileSet;
            Tiled::TileData tileData;
            
            tileData.tileSet			 = static_cast<uint32_t>(m_tileSets.size());
            tileSet.imageFileName.offset = AddString(imageElement->Attribute("source"));
            tileSet.imageWidth			 = atoi(imageElement->Attribute("width"));
            tileSet.imageHeight			 = atoi(imageElement->Attribute("height"));
            tileSet.tileWidth			 = atoi(tileSetElement->Attribute("tilewidth"));
            tileSet.tileHeight			 = atoi(tileSetElement->Attribute("tileheight"));
            int32_t _spacing			 = tileSetElement->Attribute("spacing") ? atoi(tileSetElement->Attribute("spacing")) : 0;
            int32_t _margin				 = tileSetElement->Attribute("margin")  ? atoi(tileSetElement->Attribute("margin"))  : 0;
            int32_t _firstGid			 = atoi(tileSetElement->Attribute("firstgid")) - 1;
                
            uint32_t _tilesWide			 = (tileSet.imageWidth  - _margin * 2 + _spacing) / (tileSet.tileWidth  + _spacing);
            uint32_t _tilesHigh			 = (tileSet.imageHeight - _margin * 2 + _spacing) / (tileSet.tileHeight + _spacing);
            int32_t  _lastGid			 = _firstGid + _tilesWide * _tilesHigh;
                
            m_tileMetrics.resize(_lastGid);
                
            float _nw = 1.0f / static_cast<float>(tileSet.imageWidth);
            float _nh = 1.0f / static_cast<float>(tileSet.imageHeight);
            float _n2w = static_cast<float>(tileSet.imageWidth)  / static_cast<float>(NextPOT(tileSet.imageWidth));
            float _n2h = static_cast<float>(tileSet.imageHeight) / static_cast<float>(NextPOT(tileSet.imageHeight));
                
            for (uint32_t y = 0; y < _tilesHigh; ++y)
            {
                for (uint32_t x = 0; x < _tilesWide; ++x)
                {
                    tileData.u0 = static_cast<float>(x * (tileSet.tileWidth  + _spacing) + _margin) * _nw;
                    tileData.v0 = static_cast<float>(y * (tileSet.tileHeight + _spacing) + _margin) * _nh;
                    tileData.u1 = tileData.u0 + (static_cast<float>(tileSet.tileWidth) * _nw);
                    tileData.v1 = tileData.v0 + (static_cast<float>(tileSet.tileWidth) * _nh);
                        
                    tileData.u0 *= _n2w;
                    tileData.v0 *= _n2h;
                    tileData.u1 *= _n2w;
                    tileData.v1 *= _n2h;
                        
                    m_tileMetrics[_firstGid++] = tileData;
                }
            }
            m_tileSets.push_back(tileSet);
        }
        tileSetElement = tileSetElement->NextSiblingElement("tileset");
    }
        
    tinyxml2::XMLElement *layerSetElement = mapElement->FirstChildElement("layer");
    while (layerSetElement)
    {
        if ( tinyxml2::XMLElement *dataElement = layerSetElement->FirstChildElement("data")) {
            Tiled::MapLayer layerSet;
                
            layerSet.layerName.offset = AddString(layerSetElement->Attribute("name"));
            layerSet.layerWidth = atoi(layerSetElement->Attribute("width"));
            layerSet.layerHeight = atoi(layerSetElement->Attribute("height"));
            layerSet.layerData.offset = static_cast<uint64_t>(m_mapLayers.size());
                
            uint64_t startIdx = layerSet.layerData.offset;
            uint64_t endIdx = startIdx + layerSet.layerWidth * layerSet.layerHeight;
                
            m_mapData.resize(endIdx);
                
            const char *data = dataElement->GetText();
            char *dataCopy = new char[strlen(data)+1];
            memccpy(dataCopy, data, 0, strlen(data)+1);
                
            static const char seperators[] = " \t\n\r,";
            char *token = strtok(dataCopy, seperators);
            for (uint64_t i=startIdx; i<endIdx; ++i)
            {
                if (token)
                {
                    m_mapData[i] = atoi(token);
                    token = strtok(NULL, seperators);
            	}
                else
                {
                    m_mapData[i] = 0;
                }
            }
                
            delete[] dataCopy;
            m_mapLayers.push_back(layerSet);
                
        }
        layerSetElement = layerSetElement->NextSiblingElement("layer");
    }
        
    tinyxml2::XMLElement *objectGroupElement = mapElement->FirstChildElement("objectgroup");
    while (objectGroupElement)
    {
        tinyxml2::XMLElement *objectElement = objectGroupElement->FirstChildElement("object");
        while (objectElement)
        {
            float x = atof(objectElement->Attribute("x"));
            float y = atof(objectElement->Attribute("y"));
                
            Tiled::Entity object;
            Tiled::Point point{x,y};
                
            object.tiledID = objectElement->Attribute("gid") ? atoi(objectElement->Attribute("gid"))-1 : 0;
                
            object.objectName.offset = AddString(objectElement->Attribute("name"));
            object.objectTypeHash = Tiled::consthash_fnv1a(objectElement->Attribute("type"));
            object.points.offset = static_cast<uint64_t>(m_points.size() * sizeof(Tiled::Point));
            object.numPoints = 0;
            object.numProperties = 0;
            if ( tinyxml2::XMLElement *polylineElement = objectElement->FirstChildElement("polyline"))
            {
                const char *data = polylineElement->Attribute("points");
                char *dataCopy = new char[strlen(data)+1];
                memccpy(dataCopy, data, 0, strlen(data)+1);
                    
                static const char seperators[] = " ";
                char *token = strtok(dataCopy, seperators);
                while (token)
                {
                    sscanf(token, "%f,%f", &point.x, &point.y);
                    point.x += x;
                    point.y += y;
                    ++object.numPoints;
                    m_points.push_back(point);
                    token = strtok(NULL, seperators);
                }
            }
            else
            {
                point.x = x;
                point.y = y;
                ++object.numPoints;
                m_points.push_back(point);
            }
                
            object.numProperties = AddProperties(objectElement, object.properties.offset);
                
            m_objects.push_back(object);
            objectElement = objectElement->NextSiblingElement("object");
        }
        objectGroupElement = objectGroupElement->NextSiblingElement("objectgroup");
    }
        
    return true;
}

uint64_t TMXConverter::SaveMap( char* &buffer )
{
	m_header.numTileSets		= static_cast<uint32_t>(m_tileSets.size());
	m_header.numLayers			= static_cast<uint32_t>(m_mapLayers.size());
	m_header.numObjects			= static_cast<uint32_t>(m_objects.size());
	m_header.tileSets.offset 	= sizeof(Tiled::MapHeader);
	m_header.tileMetrics.offset	= m_header.tileSets.offset + sizeof(Tiled::TileSet) * m_tileSets.size();
	m_header.mapLayers.offset 	= m_header.tileMetrics.offset + sizeof(Tiled::TileData) * m_tileMetrics.size();
	m_header.mapData.offset		= m_header.mapLayers.offset + sizeof(Tiled::MapLayer) * m_mapLayers.size();
	m_header.entities.offset	= m_header.mapData.offset + sizeof(uint32_t) * m_mapData.size();
	m_header.points.offset		= m_header.entities.offset + sizeof(Tiled::Entity) * m_objects.size();
	m_header.properties.offset	= m_header.points.offset + sizeof(Tiled::Point) * m_points.size();
	m_header.strings.offset		= m_header.properties.offset + sizeof(Tiled::Property) * m_properties.size();

    
	// The lack of std::memory_stream can bite my big shiny metal....
	uint64_t bufferSize = m_header.strings.offset + m_stringBuffer.size();
	buffer = new char[bufferSize];
	void *bufferPtr = buffer;
	Copy(bufferPtr, reinterpret_cast<char*>(&m_header), 		sizeof(Tiled::MapHeader)						);
	Copy(bufferPtr, reinterpret_cast<char*>(&m_tileSets[0]), 	sizeof(Tiled::TileSet) * m_tileSets.size() 		);
	Copy(bufferPtr, reinterpret_cast<char*>(&m_tileMetrics[0]), sizeof(Tiled::TileData) * m_tileMetrics.size() 	);
	Copy(bufferPtr, reinterpret_cast<char*>(&m_mapLayers[0]), 	sizeof(Tiled::MapLayer) * m_mapLayers.size() 	);
	Copy(bufferPtr, reinterpret_cast<char*>(&m_mapData[0]), 	sizeof(uint32_t) * m_mapData.size() 			);
	Copy(bufferPtr, reinterpret_cast<char*>(&m_objects[0]), 	sizeof(Tiled::Entity) * m_objects.size() 		);
	Copy(bufferPtr, reinterpret_cast<char*>(&m_points[0]), 		sizeof(Tiled::Point) * m_points.size() 			);
	Copy(bufferPtr, reinterpret_cast<char*>(&m_properties[0]), 	sizeof(Tiled::Property) * m_properties.size() 	);
	Copy(bufferPtr, reinterpret_cast<char*>(&m_stringBuffer[0]),sizeof(char) * m_stringBuffer.size() 			);
			
	return bufferSize;
}

uint32_t TMXConverter::AddString( const char *string )
{
    uint32_t stringOffset = static_cast<uint32_t>(m_stringBuffer.size());
    if (string)
    {
        uint32_t stringLength = static_cast<uint32_t>(strlen(string));
        m_stringBuffer.resize(stringOffset + stringLength + 1);
        memccpy(&m_stringBuffer[stringOffset], string, 0, stringLength + 1);
    }
    return stringOffset;
}
	
uint32_t TMXConverter::AddProperties( tinyxml2::XMLElement *parentElement, uint64_t &propertiesOffset )
{
    Tiled::Property property;
    uint32_t numProperties = 0;
    propertiesOffset = static_cast<uint64_t>(m_properties.size() * sizeof(Tiled::Property));
    if (tinyxml2::XMLElement *propertiesElement = parentElement->FirstChildElement("properties"))
    {
        tinyxml2::XMLElement *propertyElement = propertiesElement->FirstChildElement("property");
        while (propertyElement)
        {
            property.propertyName.offset = AddString(propertyElement->Attribute("name"));
            property.propertyType = Tiled::consthash_fnv1a(propertyElement->Attribute("name"));
            const char *val = propertyElement->Attribute("value");
            if (strlen(val)>0 && val[0]=='#')
            {
            	property.propertyData._number = atof(val+1);
            }
            else
            {
                property.propertyData._hash = Tiled::consthash_fnv1a(val);
            }
            ++numProperties;
            m_properties.push_back(property);
            propertyElement = propertyElement->NextSiblingElement("property");
        }
    }
    return numProperties;
}
