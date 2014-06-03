//
//  TiledMap.h
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

#import <UIKit/UIKit.h>
#import "TiledMapGL.h"

@interface TiledMap : SPDisplayObjectContainer
{
@public
	SPBaseEffect 	*baseEffect[NUM_VERTEX_VBO_BUFFERS];
	TiledGL			*tiledEngine;
    bool dirty;
}

+ (TiledMap*)initWithContentsOfFile:(NSString *)path;
- (TiledMap*)initWithContentsOfFile:(NSString *)path;
- (void)render:(SPRenderSupport *)support;

- (SPTexture*)textureWithIndex:(int)index;
- (SPRectangle*)subTextureRectangleWithIndex:(int)index;
- (int)tileAtX:(float)x y:(float)y;

- (float)screenPixelsWide;
- (float)screenPixelsHigh;
- (float)mapPixelsWide;
- (float)mapPixelsHigh;
- (float)mapPixelOffsetX;
- (float)mapPixelOffsetY;

-(void)update;


@end
