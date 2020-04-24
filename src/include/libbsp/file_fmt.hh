#pragma once

#include "constants.hh"

#include <array>
#include <limits>

namespace BSP {
	
	struct alignas(8) Lump {
		int32_t offs; // pointer offset from the BSP header
		int32_t size; // size of lump
	};
	static_assert(sizeof(Lump) == 8);
	struct alignas(8) Header {

		ident_t              ident;   // see IDENT, checked to make sure file is a BSP file
		int32_t              version; // see VERSION, no other versions are known
		std::array<Lump, 18> lumps;   // lumps, see the LumpIndex enum
	};
	static_assert(sizeof(Header) == 152);
	
	struct alignas(8) Shader {
		char    shader[PATH_LENGTH]; // path of shader relative to the Q3 VFS
		int32_t surface_flags;       // surface flags, usually taken from the shader
		int32_t content_flags;       // content flags, usually taken from the shader
	};
	static_assert(sizeof(Shader) == 72);
	
	struct alignas(16) Plane {
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
	
	struct alignas(16) Leaf {
		int32_t cluster;                     // LumpIndex::VISIBILITY (if present)
		int32_t area;                        // area index
		int32_t mins[3], maxs[3];            // bounds
		int32_t	first_surface, num_surfaces; // LumpIndex::LEAFSURFACES
		int32_t	first_brush, num_brushes;    // LumpIndex::LEAFBRUSHES
	};
	static_assert(sizeof(Leaf) == 48);
	
	struct alignas(8) Model {
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
		int32_t surface; // UNUSED -- writen by q3map2 but unused by games, possibly refers to LumpIndex::SURFACES
	};
	static_assert(sizeof(BrushSide) == 12);
	
	struct alignas(16) DrawVert {
		float   pos[3];                   // XYZ coordinates
		float   uv[2];                    // UV coordinates, shader
		float   lightmap[LIGHTSTYLES][2]; // UV coordinates, lightmap, per-style
		float   normal[3];                // normal vector
		uint8_t color[LIGHTSTYLES][4];    // vertex colors, per-style
	};
	static_assert(sizeof(DrawVert) == 80);
	
	struct alignas(8) Fog {
		char    shader[PATH_LENGTH]; // path of shader relative to the Q3 VFS
		int32_t brush;               // LumpIndex::BRUSH
		int32_t visible_side;        // LumpIndex::BRUSHSIDE or -1 if none
	};
	static_assert(sizeof(Fog) == 72);
	
	struct alignas(4) Surface {
		int32_t     shader;                                           // LumpIndex::SHADERS
		int32_t     fog;                                              // LumpIndex::FOGS
		SurfaceType type;                                             // see the SurfaceType enum
		int32_t     vert_idx, vert_count;                             // LumpIndex::DRAWVERTS
		int32_t     index_idx, index_count;                           // LumpIndex::DRAWINDEXES
		uint8_t     lightmap_styles[LIGHTSTYLES];                     // lightmap style type, per-style
		uint8_t     vertex_styles[LIGHTSTYLES];                       // vertex style type, per-style
		int32_t     lightmap[LIGHTSTYLES];                            // LumpIndex::LIGHTMAPS, per-style
		int32_t     lightmap_x[LIGHTSTYLES], lightmap_y[LIGHTSTYLES]; // UNUSED -- written by q3map2 but unused by games, unknown purpose
		int32_t     lightmap_width, lightmap_height;                  // UNUSED -- written by q3map2 but unused by games, unknown purpose
		float       lightmap_origin[3];                               // if surface is FLARE, the origin of the flare, unused otherwise
		// if surface is a FLARE, first and third vectors are color and normal, respectively
		// if surface is a PATCH, first and second vectors represent the bounding box, and the third vector is used in normal calculation (???)
		// otherwise ignored
		float       lightmap_vectors[3][3];
		int32_t     patch_width, patch_height;                        // if surface is PATCH, the number of verts in each dimension
	};
	static_assert(sizeof(Surface) == 148);
	
	struct alignas(1) Color {
		uint8_t r, g, b;
	};
	static_assert(sizeof(Color) == 3);
	
	struct alignas(16384) Lightmap {
		Color pixels[LIGHTMAP_DIM][LIGHTMAP_DIM];
	};
	static_assert(sizeof(Lightmap) == LIGHTMAP_BYTES);
	
	struct alignas(2) Lightgrid {
		Color   ambient[LIGHTSTYLES]; // ambient lighting
		Color   direct[LIGHTSTYLES];  // direct lighting
		uint8_t styles[LIGHTSTYLES];  // additive color styles
		uint8_t latitude, longitude;  // 2-component normal vector of direct lighting
	};
	static_assert(sizeof(Lightgrid) == 30);
	
	struct alignas(8) VisibilityHeader {
		int32_t clusters;      // number of clusters
		int32_t cluster_bytes; // number of bytes per cluster, should be (clusters / 8) rounded up to the next 8-byte alignment
		// (clusters * cluster_bytes) should be the same number of bytes in the visibility data
	};
	static_assert(sizeof(VisibilityHeader) == 8);
	
	// refuse to compile on platforms where float is not an IEEE 32-bit
	static_assert(std::numeric_limits<float>::is_iec559);
}
