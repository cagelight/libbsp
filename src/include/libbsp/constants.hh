#pragma once

#include <cstdint>
#include <cstdlib>

namespace BSP {

	static constexpr char     IDENT[4] = { 'R', 'B', 'S', 'P' };
	static constexpr int32_t  VERSION = 1;
	static constexpr uint32_t PATH_LENGTH = 64;
	
	static constexpr uint32_t LIGHTSTYLES = 4; // usually referring to lightmap and vertex color usage
	static constexpr uint32_t LIGHTMAP_DIM = 128; // hardcoded width and height of a lightmap
	static constexpr uint32_t LIGHTMAP_PIXELS = LIGHTMAP_DIM * LIGHTMAP_DIM; // number of pixels per lightmap
	static constexpr uint32_t LIGHTMAP_CHANNELS = 3; // RGB
	static constexpr uint32_t LIGHTMAP_BYTES = LIGHTMAP_PIXELS * LIGHTMAP_CHANNELS; // fixed total number of bytes per lightmap
	
	static constexpr float    LIGHTGRID_SIZE[3] = { 64, 64, 128 };
	
	enum struct LumpIndex : size_t {
		ENTITIES = 0,     // the entity string
		SHADERS = 1,      // array of shaders used by brushes and surfaces in the map
		PLANES = 2,       // TODO
		NODES = 3,        // TODO
		LEAFS = 4,        // TODO
		LEAFSURFACES = 5, // TODO
		LEAFBRUSHES = 6,  // TODO
		MODELS = 7,       // array of brush models, the first of which is the static map, and the rest are used in things like doors and other movers
		BRUSHES = 8,      // array of brushes
		BRUSHSIDES = 9,   // array of the sides of brushes
		DRAWVERTS = 10,   // array of drawverts
		DRAWINDEXES = 11, // TODO
		FOGS = 12,        // array of fog volumes
		SURFACES = 13,    // array of surfaces
		LIGHTMAPS = 14,   // array of lightmaps
		LIGHTGRID = 15,   // TODO
		VISIBILITY = 16,  // am 8-byte header and then an array of visibility clusters
		LIGHTARRAY = 17   // TODO
	};
	
	enum struct SurfaceType : int32_t {
		BAD = 0,     // used by game, should never appear in a BSP file (unless something terrible happened)
		PLANAR = 1,  // generated from brush faces (and baked patches if compiled with -patchmeta)
		PATCH = 2,   // unsubdivided patch meshes
		TRISOUP = 3, // baked trisoup generated from imported models (OBJ, ASE, MD3, etc.), for example a model key on a misc_model entity
		FLARE = 4    // autosprites
	};
}
