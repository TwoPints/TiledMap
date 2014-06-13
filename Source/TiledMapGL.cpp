//
//  TiledMapGL.cpp
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

#include "TiledMapGL.h"

// On iOS and GLES2 at least and dealing with quads, I don't think there's really that much difference.
//#define USE_TRIANGLE_STRIPS

void TiledGL::GenerateGLData( const std::function<void(uint32_t,GLuint&,GLuint&,GLuint&)> &attribIndices )
{
	int32_t _numQuads    = maxTilesToRender;
	int32_t _numVertices = _numQuads * 4;
	int32_t _numIndices  = _numVertices + (_numVertices>>1);
	
	//////////////////////////////////////////////////////////////
	// Indices
	//////////////////////////////////////////////////////////////
#if !defined(USE_TRIANGLE_STRIPS)
	GLushort *indices = new GLushort[_numIndices];
	GLushort *indexPtr = indices;
	for (int i=0; i<_numQuads; ++i)
	{
	    *indexPtr++ = (i << 2) + 0;
	    *indexPtr++ = (i << 2) + 1;
	    *indexPtr++ = (i << 2) + 2;
	    *indexPtr++ = (i << 2) + 1;
	    *indexPtr++ = (i << 2) + 3;
	    *indexPtr++ = (i << 2) + 2;
	}
	glGenBuffers(1, &indexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _numIndices * sizeof(GLushort), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	delete[] indices;
#else
	_numVertices = _numIndices;
#endif
	
	//////////////////////////////////////////////////////////////
	// Vertices
	//////////////////////////////////////////////////////////////
	int32_t bufferIndexSize = _numIndices * sizeof(VertexDef);

	vertexBufferIdx = 0;
	vertexBuffer = new VertexDef[_numIndices];
	memset(vertexBuffer, 0, bufferIndexSize);
	
	for (int i=0; i<NUM_VERTEX_VBO_BUFFERS; ++i)
    {
		buffers[i].textureBatches.resize(mapData->numTileSets);
		
		GLuint attribPosition = 0;
		GLuint attribColor = 0;
		GLuint attribTexCoords = 0;
		attribIndices(i, attribPosition, attribColor, attribTexCoords);
		
    	// Generate dynamic vbo so we can change the data later, copy the prefab data to stop any bitching.
	    glGenBuffers(1, &buffers[i].vertexVBO);
    	glBindBuffer(GL_ARRAY_BUFFER, buffers[i].vertexVBO);
    	glBufferData(GL_ARRAY_BUFFER, bufferIndexSize, vertexBuffer, GL_DYNAMIC_DRAW);
    	glBindBuffer(GL_ARRAY_BUFFER, 0);
		
    	// Create vertex array object for speed, not sure if it helps in the scheme of things as presentRenderBuffer takes more time (unless this is idle time?)
	    glGenVertexArraysOES(1, &buffers[i].vao);
    	glBindVertexArrayOES(buffers[i].vao);
    	glBindBuffer(GL_ARRAY_BUFFER, buffers[i].vertexVBO);
#if !defined(USE_TRIANGLE_STRIPS)
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
#endif
    	glVertexAttribPointer(attribPosition, 2, GL_SHORT, GL_FALSE, sizeof(VertexDef), (void *)(offsetof(VertexDef, x)));
    	glEnableVertexAttribArray(attribPosition);
    	glVertexAttribPointer(attribColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexDef), (void *)(offsetof(VertexDef, color)));
    	glEnableVertexAttribArray(attribColor);
    	glVertexAttribPointer(attribTexCoords, 2, GL_FLOAT, GL_TRUE, sizeof(VertexDef), (void *)(offsetof(VertexDef, u)));
    	glEnableVertexAttribArray(attribTexCoords);
		
    	glBindVertexArrayOES(0);
#if !defined(USE_TRIANGLE_STRIPS)
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
    	glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
}

void TiledGL::Update()
{
	//////////////////////////////////////////////////////////////
	// Vertices
	//////////////////////////////////////////////////////////////
	
	uint32_t tileCache[numTilesHigh * numTilesWide];
	Tiled::Point positionCache[numTilesHigh * numTilesWide];

	std::vector<uint32_t> textureBatches(mapData->numTileSets);
	
	float idxX = (static_cast<int32_t>(mapX) / mapData->tileWidth)  % mapData->blocksWide;
	float idxY = (static_cast<int32_t>(mapY) / mapData->tileHeight) % mapData->blocksHigh;
	int32_t activeQuads = 0;
	for( int y=0; y<numTilesHigh * mapData->tileHeight; y+=mapData->tileHeight )
	{
		int32_t sX = idxX;
		for (int x=0; x<numTilesWide * mapData->tileWidth; x+=mapData->tileWidth)
		{
			int32_t	mapIdx = (idxY * mapData->blocksWide) + sX;
			uint32_t tileInfo = mapData->mapLayers.ptr[0].layerData.ptr[mapIdx];
			
			if (tileInfo&Tiled::TILE_MASK)
			{
				++textureBatches[(tileInfo&Tiled::TILESET_MASK) >> Tiled::TILESET_SHIFT];
				positionCache[activeQuads].x = x;
				positionCache[activeQuads].y = y;
				tileCache[activeQuads++] = tileInfo;
			}
			if (++sX >= mapData->blocksWide)
			{
				sX -= mapData->blocksWide;
			}
		}
		if (++idxY >= mapData->blocksHigh)
		{
			idxY -= mapData->blocksHigh;
		}
	}

	for (int i(1); i<mapData->numTileSets; ++i)
	{
		textureBatches[i] += textureBatches[i-1];
	}

	// Build the map
	VertexDef *pVertex = vertexBuffer;
	for (int tileSet(0); tileSet<mapData->numTileSets; ++tileSet)
	{
		for (int i(0); i<activeQuads; ++i)
		{
			const uint32_t tileData = tileCache[i];
			if ((tileData&Tiled::TILESET_MASK) >> Tiled::TILESET_SHIFT == tileSet)
			{
				const Tiled::TileData	&tileMetric = mapData->tileMetrics.ptr[(tileCache[i]&Tiled::TILE_MASK)-1];
				const Tiled::TileSet 	&tileSet    = mapData->tileSets.ptr[tileMetric.tileSet];
				
				const GLshort t(static_cast<GLshort>(positionCache[i].y));
				const GLshort l(static_cast<GLshort>(positionCache[i].x));
				const GLshort b(t + static_cast<GLshort>(tileSet.tileHeight));
				const GLshort r(l + static_cast<GLshort>(tileSet.tileWidth));
				
#if defined(USE_TRIANGLE_STRIPS)
				pVertex->x = l;
				pVertex->y = b;
				pVertex->u = static_cast<GLfloat>(tileMetric.u0);
				pVertex->v = static_cast<GLfloat>(tileMetric.v1);
				pVertex->color = 0xffffffff;
				++pVertex;
#endif
				pVertex->x = l;
				pVertex->y = b;
				pVertex->u = static_cast<GLfloat>(tileMetric.u0);
				pVertex->v = static_cast<GLfloat>(tileMetric.v1);
				pVertex->color = 0xffffffff;
				++pVertex;
				
				pVertex->x = l;
				pVertex->y = t;
				pVertex->u = static_cast<GLfloat>(tileMetric.u0);
				pVertex->v = static_cast<GLfloat>(tileMetric.v0);
				pVertex->color = 0xffffffff;
				++pVertex;
				
				pVertex->x = r;
				pVertex->y = b;
				pVertex->u = static_cast<GLfloat>(tileMetric.u1);
				pVertex->v = static_cast<GLfloat>(tileMetric.v1);
				pVertex->color = 0xffffffff;
				++pVertex;
				
				pVertex->x = r;
				pVertex->y = t;
				pVertex->u = static_cast<GLfloat>(tileMetric.u1);
				pVertex->v = static_cast<GLfloat>(tileMetric.v0);
				pVertex->color = 0xffffffff;
				++pVertex;
				
#if defined(USE_TRIANGLE_STRIPS)
				pVertex->x = r;
				pVertex->y = t;
				pVertex->u = static_cast<GLfloat>(tileMetric.u1);
				pVertex->v = static_cast<GLfloat>(tileMetric.v0);
				pVertex->color = 0xffffffff;
				++pVertex;
#endif
			}
		}
	}
	//////////////////////////////////////////////////////////////
	
	if (++vertexBufferIdx>=NUM_VERTEX_VBO_BUFFERS)
	{
		vertexBufferIdx = 0;
	}
	
	buffers[vertexBufferIdx].textureBatches = std::move(textureBatches);
	buffers[vertexBufferIdx].numQuads = activeQuads;
	buffers[vertexBufferIdx].numVertices = activeQuads * 4;
	buffers[vertexBufferIdx].numIndices = activeQuads * 6;
#if defined(USE_TRIANGLE_STRIPS)
	buffers[vertexBufferIdx].numVertices = buffers[vertexBufferIdx].numIndices;
#endif
	
	glBindBuffer(GL_ARRAY_BUFFER, buffers[vertexBufferIdx].vertexVBO);
	void *ptr = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
	memcpy(ptr, vertexBuffer, buffers[vertexBufferIdx].numVertices * sizeof(VertexDef));
	glUnmapBufferOES(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TiledGL::Render( const std::function<void(uint32_t,bool)> &textureSetter ) const
{
	uint32_t lastBatchSize = 0;
	for (int tileSet(0); tileSet<mapData->numTileSets; ++tileSet)
	{
		const uint32_t batchSize = buffers[vertexBufferIdx].textureBatches[tileSet];
		if(batchSize != lastBatchSize	)
		{
			textureSetter(tileSet, false);
			glBindVertexArrayOES(buffers[vertexBufferIdx].vao);
#if !defined(USE_TRIANGLE_STRIPS)
			glDrawElements(GL_TRIANGLES, batchSize * 6, GL_UNSIGNED_SHORT, (void *)(lastBatchSize * 6 * sizeof(GLshort)));
#else
			glDrawArrays(GL_TRIANGLE_STRIP, lastBatchSize * 6, batchSize * 6);
#endif
			glBindVertexArrayOES(0);
		}
		lastBatchSize = batchSize;
	}
}




