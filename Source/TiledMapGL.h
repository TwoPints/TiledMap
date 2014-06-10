//
//  TiledMapGL.h
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
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <TiledMapBase.h>
#include <functional>
#include <algorithm>
#include <vector>

const uint32_t NUM_VERTEX_VBO_BUFFERS = 2;

class TiledGL : public	Tiled::Map
{
public:
    
	TiledGL( const std::function<void(uint32_t,GLuint&,GLuint&,GLuint&)> &attribIndices, void *convertedMapData, int32_t screenPixelsWide, int32_t screenPixelsHigh, float mapX = 0.0f, float mapY = 0.0f )
	: Map( convertedMapData, screenPixelsWide, screenPixelsHigh, mapX, mapY )
	{
		if(mapData)
		{
			GenerateGLData(attribIndices);
		}
	}
	
    void Update();
	void Render( const std::function<void(uint32_t,bool)> &textureSetter ) const;
    int32_t VertexBufferIndex() const { return vertexBufferIdx; }
    
private:
	
	void GenerateGLData( const std::function<void(uint32_t,GLuint&,GLuint&,GLuint&)> &attribIndices );
	
    
    struct VertexDef
    {
        GLshort x, y;
        GLuint	color;
        GLfloat u, v;
    };
	
    
	GLuint		indexVBO;
	struct
	{
    	GLuint					vertexVBO;
		GLuint					vao;
		int32_t					numQuads;
		int32_t					numVertices;
		int32_t					numIndices;
		std::vector<uint32_t>	textureBatches;
	}
	buffers[NUM_VERTEX_VBO_BUFFERS];
		
	int32_t		vertexBufferIdx;
    VertexDef	*vertexBuffer;
};
