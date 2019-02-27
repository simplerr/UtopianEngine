#pragma once

/*
	The two lookup tables used in the marching cubes algorithm.
	They are copied to 1D textures and transfered to the GPU.
*/

extern int edgeTable[256];
extern int triTable[256][16];