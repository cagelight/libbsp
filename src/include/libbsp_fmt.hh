#pragma once

#include <cstdint>
#include <cstdlib>

// FIXME -- replace with std::span when C++20 rolls around
#include <gsl/span>

namespace libbsp::fmt {
	
	static constexpr uint32_t BSP_PATH_LENGTH = 64;
	
	enum struct Lump : uint8_t {
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
	
	struct alignas(4) BSP_Lump {
		int32_t offs;
		int32_t filelen;
	};

	struct alignas(4) BSP_Header {
		
		// ================
		// DATA
		
		int32_t  ident;
		int32_t  version;
		BSP_Lump lumps[18];
		
		// ================
		// HELPERS
		
		inline BSP_Lump const & get_lump(Lump lump_num) const {
			return lumps[static_cast<uint8_t>(lump_num)];
		}
		
		template <typename T> T const * get_data(Lump lump_num) const {
			BSP_Lump const & lump = get_lump(lump_num);
			return reinterpret_cast<T const *>(reinterpret_cast<uint8_t const *>(this) + lump.offs);
		}
		
		template <typename T> gsl::span<T> get_data_span(Lump lump_num) const {
			BSP_Lump const & lump = get_lump(lump_num);
			return gsl::span<T> {
				reinterpret_cast<T const *>(reinterpret_cast<uint8_t const *>(this) + lump.offs),
				static_cast<ssize_t>(lump.filelen / sizeof(T))
			};
		}
		
		// ================
	};
	
	struct alignas(4) BSP_Shader {
		char    shader[BSP_PATH_LENGTH];
		int32_t surface_flags;
		int32_t content_flags;
	};
	
	struct alignas(4) BSP_Plane {
		float normal[3];
		float dist;
	};
	
	struct alignas(4) BSP_Node {
		int32_t plane_idx;
		int32_t children[2];
		int32_t mins[3];
		int32_t maxs[3];
	};
}
