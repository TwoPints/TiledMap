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

void TiledGL::GenerateGLData( const std::function<void(uint32_t,GLuint&,GLuint&,GLuint&)> &attribIndices )
{
	int32_t _numQuads    = maxTilesToRender;
	int32_t _numVertices = _numQuads * 4;
	int32_t _numIndices  = _numVertices + (_numVertices>>1);
	
	//////////////////////////////////////////////////////////////
	// Indices
	//////////////////////////////////////////////////////////////
	GLushort *indices = new GLushort[_numIndices];
	GLushort *indexPtr = indices;
	for (int i=0; i<_numQuads; ++i)
	{
#if !defined(USE_TRIANGLE_STRIPS)
	    *indexPtr++ = (i << 2) + 0;
	    *indexPtr++ = (i << 2) + 1;
	    *indexPtr++ = (i << 2) + 2;
	    *indexPtr++ = (i << 2) + 1;
	    *indexPtr++ = (i << 2) + 3;
	    *indexPtr++ = (i << 2) + 2;
#else
	    *indexPtr++ = (i << 2);
	    *indexPtr++ = (i << 2);
	    *indexPtr++ = (i << 2) + 1;
	    *indexPtr++ = (i << 2) + 2;
	    *indexPtr++ = (i << 2) + 3;
	    *indexPtr++ = (i << 2) + 3;
#endif
	}
	glGenBuffers(1, &indexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _numIndices * sizeof(GLushort), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	delete[] indices;
	
	//////////////////////////////////////////////////////////////
	// Vertices
	//////////////////////////////////////////////////////////////
	int32_t bufferIndexSize = _numIndices * sizeof(VertexDef);
	vertexBufferIdx = 0;
	vertexBuffer = new VertexDef[_numIndices];
	memset(vertexBuffer, 0, bufferIndexSize);
	
	for (int i=0; i<NUM_VERTEX_VBO_BUFFERS; ++i)
    {
		GLuint attribPosition = 0;
		GLuint attribColor = 0;
		GLuint attribTexCoords = 0;
		attribIndices(i, attribPosition, attribColor, attribTexCoords);
		
    	// Generate dynamic vbo so we can change the data later, copy the prefab data to stop any bitching.
	    glGenBuffers(1, &vertexVBO[i]);
    	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO[i]);
    	glBufferData(GL_ARRAY_BUFFER, bufferIndexSize, vertexBuffer, GL_DYNAMIC_DRAW);
    	glBindBuffer(GL_ARRAY_BUFFER, 0);
		
    	// Create vertex array object for speed, not sure if it helps in the scheme of things as presentRenderBuffer takes more time (unless this is idle time?)
	    glGenVertexArraysOES(1, &vao[i]);
    	glBindVertexArrayOES(vao[i]);
    	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO[i]);
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO);
		
    	glVertexAttribPointer(attribPosition, 2, GL_SHORT, GL_FALSE, sizeof(VertexDef), (void *)(offsetof(VertexDef, x)));
    	glEnableVertexAttribArray(attribPosition);
    	glVertexAttribPointer(attribColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexDef), (void *)(offsetof(VertexDef, color)));
    	glEnableVertexAttribArray(attribColor);
    	glVertexAttribPointer(attribTexCoords, 2, GL_FLOAT, GL_TRUE, sizeof(VertexDef), (void *)(offsetof(VertexDef, u)));
    	glEnableVertexAttribArray(attribTexCoords);
		
    	glBindVertexArrayOES(0);
    	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    	glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
}

void TiledGL::Update()
{
	//////////////////////////////////////////////////////////////
	// Vertices
	//////////////////////////////////////////////////////////////
	// Build the initial map
	VertexDef *pVertex = vertexBuffer;
	
	float idxX = (static_cast<int32_t>(mapX) / mapData->tileWidth)  % mapData->blocksWide;
	float idxY = (static_cast<int32_t>(mapY) / mapData->tileHeight) % mapData->blocksHigh;
	int32_t activeQuads = 0;
	for( int y=0; y<numTilesHigh * mapData->tileHeight; y+=mapData->tileHeight ) {
		int32_t sX = idxX;
		for (int x=0; x<numTilesWide * mapData->tileWidth; x+=mapData->tileWidth) {
			int32_t	mapIdx = (idxY * mapData->blocksWide) + sX;
			uint32_t tileInfo = mapData->mapLayers.ptr[0].layerData.ptr[mapIdx];
			
			if (tileInfo&Tiled::TILE_MASK) {
				Tiled::TileData	*tileMetric = &mapData->tileMetrics.ptr[(tileInfo&Tiled::TILE_MASK)-1];
				
				pVertex->x = static_cast<GLshort>(x);
				pVertex->y = static_cast<GLshort>(y + mapData->tileHeight);
				pVertex->u = static_cast<GLfloat>(tileMetric->u0);
				pVertex->v = static_cast<GLfloat>(tileMetric->v1);
				pVertex->color = 0xffffffff;
				++pVertex;
				
				pVertex->x = static_cast<GLshort>(x);
				pVertex->y = static_cast<GLshort>(y);
				pVertex->u = static_cast<GLfloat>(tileMetric->u0);
				pVertex->v = static_cast<GLfloat>(tileMetric->v0);
				pVertex->color = 0xffffffff;
				++pVertex;
				
				pVertex->x = static_cast<GLshort>(x + mapData->tileWidth);
				pVertex->y = static_cast<GLshort>(y + mapData->tileHeight);
				pVertex->u = static_cast<GLfloat>(tileMetric->u1);
				pVertex->v = static_cast<GLfloat>(tileMetric->v1);
				pVertex->color = 0xffffffff;
				++pVertex;
				
				pVertex->x = static_cast<GLshort>(x + mapData->tileWidth);
				pVertex->y = static_cast<GLshort>(y);
				pVertex->u = static_cast<GLfloat>(tileMetric->u1);
				pVertex->v = static_cast<GLfloat>(tileMetric->v0);
				pVertex->color = 0xffffffff;
				++pVertex;
				
				++activeQuads;
			}
			if (++sX >= mapData->blocksWide) {
				sX -= mapData->blocksWide;
			}
		}
		if (++idxY >= mapData->blocksHigh) {
			idxY -= mapData->blocksHigh;
		}
	}
	
	//////////////////////////////////////////////////////////////
	
	if (++vertexBufferIdx>=NUM_VERTEX_VBO_BUFFERS) {
		vertexBufferIdx = 0;
	}
	
	numQuads[vertexBufferIdx] = activeQuads;
	numVertices[vertexBufferIdx] = activeQuads * 4;
	numIndices[vertexBufferIdx] = activeQuads * 6;
	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO[vertexBufferIdx]);
	void *ptr = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
	memcpy(ptr, vertexBuffer, numVertices[vertexBufferIdx] * sizeof(VertexDef));
	glUnmapBufferOES(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TiledGL::Render() const
{
	glBindVertexArrayOES(vao[vertexBufferIdx]);
#if !defined(USE_TRIANGLE_STRIPS)
	glDrawElements(GL_TRIANGLES, numIndices[vertexBufferIdx], GL_UNSIGNED_SHORT, 0);
#else
	glDrawElements(GL_TRIANGLE_STRIP, numIndices[vertexBufferIdx], GL_UNSIGNED_SHORT, 0);
#endif
	glBindVertexArrayOES(0);
}




