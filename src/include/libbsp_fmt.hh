#pragma once

#include <cstdint>
#include <cstdlib>

#include <array>
#include <limits>

namespace BSP {
	
	static constexpr char     IDENT[4] = { 'R', 'B', 'S', 'P' };
	static constexpr int32_t  VERSION = 1;
	static constexpr uint32_t PATH_LENGTH = 64;
	static constexpr uint32_t STYLES = 4; // usually referring to lightmap and vertex color usage
	
	static constexpr uint32_t LIGHTMAP_DIM = 128; // hardcoded width and height of a lightmap
	static constexpr uint32_t LIGHTMAP_PIXELS = LIGHTMAP_DIM * LIGHTMAP_DIM; // number of pixels per lightmap
	static constexpr uint32_t LIGHTMAP_CHANNELS = 3; // RGB
	static constexpr uint32_t LIGHTMAP_BYTES = LIGHTMAP_PIXELS * LIGHTMAP_CHANNELS; // fixed total number of bytes per lightmap
	
	enum struct LumpIndex : size_t {
		ENTITIES = 0,
		SHADERS = 1,
		PLANES = 2,
		NODES = 3,
		LEAFS = 4,
		LEAFSURFACES = 5,
		LEAFBRUSHES = 6,
		MODELS = 7,
		BRUSHES = 8,
		BRUSHSIDES = 9,
		DRAWVERTS = 10,
		DRAWINDEXES = 11,
		FOGS = 12,
		SURFACES = 13,
		LIGHTMAPS = 14,
		LIGHTGRID = 15,
		VISIBILITY = 16,
		LIGHTARRAY = 17
	};
	
	enum struct SurfaceType : int32_t {
		BAD = 0,
		PLANAR = 1,
		PATCH = 2,
		TRISOUP = 3,
		FLARE = 4
	};
	
	struct alignas(4) Lump {
		int32_t offs;
		int32_t filelen;
	};
	static_assert(sizeof(Lump) == 8);

	struct alignas(4) Header {
		int32_t              ident;   // see IDENT, checked to make sure file is a BSP file
		int32_t              version; // see VERSION, no other versions are known
		std::array<Lump, 18> lumps;   // lumps, see the Lump enum
	};
	static_assert(sizeof(Header) == 152);
	
	struct alignas(4) Shader {
		char    shader[PATH_LENGTH]; // path of shader relative to the Q3 VFS
		int32_t surface_flags;       // surface flags, usually taken from the shader
		int32_t content_flags;       // content flags, usually taken from the shader
	};
	static_assert(sizeof(Shader) == 72);
	
	struct alignas(4) Plane {
		float normal[3]; // normal vector of the plane
		float dist;      // distance from the origin
	};
	static_assert(sizeof(Plane) == 16);
	
	struct alignas(4) Node {
		int32_t plane;            // LumpIndex::PLANES 
		int32_t children[2];      // LumpIndex::NODES if >= 0, else LumpIndex::LEAFS (negate and subtract 1)
		int32_t mins[3], maxs[3]; // bounds
	};
	static_assert(sizeof(Node) == 36);
	
	struct alignas(4) Leaf {
		int32_t cluster;                     // TODO
		int32_t area;                        // TODO
		int32_t mins[3], maxs[3];            // bounds
		int32_t	first_surface, num_surfaces; // LumpIndex::LEAFSURFACES
		int32_t	first_brush, num_brushes;    // LumpIndex::LEAFBRUSHES
	};
	static_assert(sizeof(Leaf) == 48);
	
	struct alignas(4) Model {
		float   mins[3], maxs[3];            // bounds
		int32_t	first_surface, num_surfaces; // LumpIndex::SURFACES
		int32_t	first_brush, num_brushes;    // LumpIndex::BRUSHES
	};
	static_assert(sizeof(Model) == 40);

	struct alignas(4) Brush {
		int32_t first_side, num_sides; // LumpIndex::BRUSHSIDES
		int32_t shader;                // LumpIndex::SHADERS
	};
	static_assert(sizeof(Brush) == 12);
	
	struct alignas(4) BrushSide {
		int32_t plane;   // LumpIndex::PLANES 
		int32_t shader;  // LumpIndex::SHADERS
		int32_t surface; // writen by q3map2 but unused by games, possibly refers to LumpIndex::SURFACES -- TODO CHECK
	};
	static_assert(sizeof(BrushSide) == 12);
	
	struct alignas(4) DrawVert {
		float   pos[3];              // XYZ coordinates
		float   uv[2];               // UV coordinates, shader
		float   lightmap[STYLES][2]; // UV coordinates, lightmap, per-style
		float   normal[3];           // normal vector
		uint8_t color[STYLES][4];    // vertex colors, per-style
	};
	static_assert(sizeof(DrawVert) == 80);
	
	struct alignas(4) Fog {
		char    shader[PATH_LENGTH]; // path of shader relative to the Q3 VFS
		int32_t brush;               // LumpIndex::BRUSH
		int32_t visible_side;        // LumpIndex::BRUSHSIDE or -1 if none
	};
	static_assert(sizeof(Fog) == 72);
	
	struct alignas(4) Surface {
		int32_t     shader;                                 // LumpIndex::SHADERS
		int32_t     fog;                                    // LumpIndex::FOGS
		SurfaceType type;                                   // see the SurfaceType enum
		int32_t     vert_idx, vert_count;                   // LumpIndex::DRAWVERTS
		int32_t     index_idx, index_count;                 // LumpIndex::DRAWINDEXES
		uint8_t     lightmap_styles[STYLES];                // lightmap style type, per-style
		uint8_t     vertex_styles[STYLES];                  // vertex style type, per-style
		int32_t     lightmap[STYLES];                       // LumpIndex::LIGHTMAPS, per-style
		int32_t     lightmap_x[STYLES], lightmap_y[STYLES]; // TODO
		int32_t     lightmap_width, lightmap_height;        // always LIGHTMAP_DIM (128) unless something went terribly wrong
		float       lightmap_origin[3];                     // TODO
		float       lightmap_vectors[3][3];                 // TODO
		int32_t     patch_width, patch_height;              // TODO
	};
	static_assert(sizeof(Surface) == 148);
	
	struct alignas(4) Lightmap {
		struct alignas(1) Pixel {
			uint8_t r, g, b;
		};
		Pixel pixels[LIGHTMAP_DIM][LIGHTMAP_DIM];
	};
	static_assert(sizeof(Lightmap) == LIGHTMAP_BYTES);
	
	// refuse to compile on platforms where float is not an IEEE 32-bit
	static_assert(std::numeric_limits<float>::is_iec559);
}
