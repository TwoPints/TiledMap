//
//  TiledMap.m
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

#import "TiledMap.h"
#import "TiledMapGL.h"
#import "TiledMapParser.h"

//#define USE_TRIANGLE_STRIPS

@interface TiledMap ()
{
    NSMutableData	*mTiledMapData;		// The raw binary data for the map - needs relocation of the pointers
    NSMutableArray	*mTextures;			// These are the textures that the map wants us to load in
}

@end

@implementation TiledMap

+ (TiledMap*)initWithContentsOfFile:(NSString *)path
{
	return [[TiledMap alloc] initWithContentsOfFile:path];
}

- (TiledMap*)initWithContentsOfFile:(NSString *)path
{
    if(self = [super init])
	{
		float scaleFactor = [Sparrow contentScaleFactor];
		NSString *fixedUpPath = [SPUtils absolutePathToFile:path withScaleFactor:scaleFactor];
		if (!fixedUpPath)
		{
			[NSException raise:SP_EXC_FILE_NOT_FOUND format:@"file not found: %@", path];
		}
		else
		{
			mTiledMapData = [[NSMutableData alloc] initWithContentsOfFile:fixedUpPath];
			
			TMXConverter tmxConverter;
			char *convertedMapBuffer = nullptr;
			tmxConverter.ProcessMap(static_cast<const char*>([mTiledMapData bytes]));
			uint64_t convertedMapBufferSize = tmxConverter.SaveMap(convertedMapBuffer);
			mTiledMapData = [[NSMutableData alloc] initWithBytes:convertedMapBuffer length:convertedMapBufferSize];
			delete[] convertedMapBuffer;
			
			auto programs = [&](uint32_t index, GLuint &attribPosition, GLuint &attribColor, GLuint &attribTexCoords)
			{
				SPBaseEffect	*effect = [[SPBaseEffect alloc]init];
				effect.useTinting = NO;
				effect.texture = [mTextures lastObject];
				[effect prepareToDraw];
				
				attribPosition = effect.attribPosition;
				attribColor = effect.attribColor;
				attribTexCoords = effect.attribTexCoords;
				
				baseEffect[index] = effect;
			};
			
			tiledEngine = new TiledGL(programs, [mTiledMapData mutableBytes], [Sparrow stage].width, [Sparrow stage].height);
			
			self.blendMode = SP_BLEND_MODE_NONE;
			self.touchable = NO;
			
			mTextures = [[NSMutableArray alloc] init];
			for( int textureIdx=0; textureIdx<tiledEngine->MapData()->numTileSets; ++textureIdx ) {
				SPTexture *texture = [SPTexture textureWithContentsOfFile:[NSString stringWithUTF8String:tiledEngine->MapData()->tileSets.ptr[textureIdx].imageFileName.ptr]];
				[texture setSmoothing:SPTextureSmoothingNone];
				[mTextures addObject:texture];
			}
			dirty = true;
		}
	}
	return self;
}

-(void)update
{
	if(!dirty) return;
	tiledEngine->Update();
	dirty = false;
}


-(void)setX:(float)x
{
	float oldMapX = tiledEngine->mapX / tiledEngine->MapData()->pixelsWidth;
	tiledEngine->mapX = fmodf(-x, tiledEngine->MapData()->pixelsWidth);
	if (tiledEngine->mapX < 0.0f) {
		tiledEngine->mapX += tiledEngine->MapData()->pixelsWidth;
	}
	dirty = dirty || (oldMapX != tiledEngine->mapX / tiledEngine->MapData()->pixelsWidth);
	super.x = fmodf(-tiledEngine->mapX, tiledEngine->MapData()->tileWidth);

}
-(float)x
{
	return super.x;
}

-(void)setY:(float)y
{
	float oldMapY = tiledEngine->mapY / tiledEngine->MapData()->tileHeight;
	tiledEngine->mapY = fmodf(-y, tiledEngine->MapData()->pixelsHeight);
	if (tiledEngine->mapY < 0.0f) {
		tiledEngine->mapY += tiledEngine->MapData()->pixelsHeight;
	}
	dirty = dirty || (oldMapY != tiledEngine->mapY / tiledEngine->MapData()->tileHeight);
	super.y = fmodf(-tiledEngine->mapY, tiledEngine->MapData()->tileHeight);
}
-(float)y
{
	return super.y;
}

- (float)screenPixelsWide
{
	return static_cast<float>(tiledEngine->ScreenPixelsWide());
}
- (float)screenPixelsHigh
{
	return static_cast<float>(tiledEngine->ScreenPixelsHigh());
}
- (float)mapPixelsWide
{
	return static_cast<float>(tiledEngine->MapData()->pixelsWidth);
}
- (float)mapPixelsHigh;
{
	return static_cast<float>(tiledEngine->MapData()->pixelsHeight);
}
- (float)mapPixelOffsetX
{
	return tiledEngine->mapX;
}
- (float)mapPixelOffsetY
{
	return tiledEngine->mapY;
}


- (void)render:(SPRenderSupport *)support
{
	[support finishQuadBatch];
	
	[support addDrawCalls:1];
    SPBaseEffect *effect = baseEffect[tiledEngine->VertexBufferIndex()];
	effect.mvpMatrix = support.mvpMatrix;
	effect.alpha = support.alpha;
	effect.texture = [mTextures lastObject];
	[effect prepareToDraw];
	[SPBlendMode applyBlendFactorsForBlendMode:support.blendMode premultipliedAlpha:YES];

	tiledEngine->Render();
}

-(SPTexture*)textureWithIndex:(int)index
{
	return [mTextures objectAtIndex:index];
}

-(SPRectangle*)subTextureRectangleWithIndex:(int)index
{
	const Tiled::MapHeader *mapData = tiledEngine->MapData();
	uint32_t tileSet = mapData->tileMetrics.ptr[index].tileSet;
	int max_x = (mapData->tileSets.ptr[tileSet].imageWidth - 1*2 + 1) / (mapData->tileWidth + 2);
	float xs = (index % max_x) * (mapData->tileWidth + 2) + 1;
	float ys = (index / max_x) * (mapData->tileHeight + 2) + 1;
	return [SPRectangle rectangleWithX:xs y:ys width:mapData->tileWidth height:mapData->tileHeight];

}

- (int)tileAtX:(float)x y:(float)y
{
	const Tiled::MapHeader *mapData = tiledEngine->MapData();
	
	float xp = floorf(x/(float)mapData->tileWidth);
	float yp = floorf(y/(float)mapData->tileHeight);
	
	if( xp >= 0 && xp < (float)mapData->blocksWide && yp >= 0 && yp < (float)mapData->blocksHigh ) {
		return mapData->mapLayers.ptr[0].layerData.ptr[(int)(((int)yp * mapData->blocksWide) + xp)] - 1;
	}
	return -1;
}

@end
