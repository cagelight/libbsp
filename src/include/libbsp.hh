#pragma once

#include "libbsp_version.hh"
#include "libbsp/file_fmt.hh"

// FIXME -- replace with std::span when C++20 rolls around
#include <gsl/span>

#include <memory>
#include <string_view>
#include <vector>

namespace BSP {
	
	struct Visibility {
		VisibilityHeader const & header;
		gsl::span<uint8_t const> data;
	};
	
	struct Reader {
		
		inline Reader() = default;
		inline Reader(uint8_t const * base) { rebase(base); }
		inline Reader(Reader const &) = default;
		inline Reader(Reader &&) = default;
		
		inline ~Reader() = default;
		
		inline void rebase(uint8_t const * base) {
			m_base = reinterpret_cast<Header const *> (base);
		}
		
		inline Lump const & get_lump(LumpIndex lump_num) const {
			return m_base->lumps[static_cast<size_t>(lump_num)];
		}
		
		template <typename T> T const * get_data(LumpIndex lump_num) const {
			Lump const & lump = get_lump(lump_num);
			return reinterpret_cast<T const *>(reinterpret_cast<uint8_t const *>(m_base) + lump.offs);
		}
		
		inline std::string_view get_string_view(LumpIndex lump_num) const {
			Lump const & lump = get_lump(lump_num);
			return std::string_view {
				reinterpret_cast<char const *>(reinterpret_cast<uint8_t const *>(m_base) + lump.offs),
				static_cast<size_t>(lump.size)
			};
		}
		
		template <typename T> gsl::span<T> get_data_span(LumpIndex lump_num, size_t offset = 0) const {
			Lump const & lump = get_lump(lump_num);
			return gsl::span<T> {
				reinterpret_cast<T const *>(reinterpret_cast<uint8_t const *>(m_base) + lump.offs + offset),
				static_cast<ssize_t>(lump.size / sizeof(T) - offset)
			};
		}
		
		// ================================
		// CHECKS
		
		inline bool has_visibility() const {
			return get_lump(LumpIndex::VISIBILITY).size;
		}
		
		// ================================
		// RETRIEVAL
		
		inline std::string_view entities() const {
			return get_string_view(LumpIndex::ENTITIES);
		}
		
		inline gsl::span<Shader const> shaders() const {
			return get_data_span<Shader const>(LumpIndex::SHADERS);
		}

		inline gsl::span<Plane const> planes() const {
			return get_data_span<Plane const>(LumpIndex::PLANES);
		}

		inline gsl::span<Node const> nodes() const {
			return get_data_span<Node const>(LumpIndex::NODES);
		}
		
		inline gsl::span<Leaf const> leafs() const {
			return get_data_span<Leaf const>(LumpIndex::LEAFS);
		}
		
		inline gsl::span<int32_t const> leafsurfaces() const {
			return get_data_span<int32_t const>(LumpIndex::LEAFSURFACES);
		}
		
		inline gsl::span<int32_t const> leafbrushes() const {
			return get_data_span<int32_t const>(LumpIndex::LEAFBRUSHES);
		}
		
		inline gsl::span<Model const> models() const {
			return get_data_span<Model const>(LumpIndex::MODELS);
		}
		
		inline gsl::span<Brush const> brushes() const {
			return get_data_span<Brush const>(LumpIndex::BRUSHES);
		}
		
		inline gsl::span<BrushSide const> brushsides() const {
			return get_data_span<BrushSide const>(LumpIndex::BRUSHSIDES);
		}
		
		inline gsl::span<DrawVert const> drawverts() const {
			return get_data_span<DrawVert const>(LumpIndex::DRAWVERTS);
		}
		
		inline gsl::span<int32_t const> drawindices() const {
			return get_data_span<int32_t const>(LumpIndex::DRAWINDEXES);
		}
		
		inline gsl::span<Fog const> fogs() const {
			return get_data_span<Fog const>(LumpIndex::FOGS);
		}
		
		inline gsl::span<Surface const> surfaces() const {
			return get_data_span<Surface const>(LumpIndex::SURFACES);
		}
		
		inline gsl::span<Lightmap const> lightmaps() const {
			return get_data_span<Lightmap const>(LumpIndex::LIGHTMAPS);
		}
		
		inline gsl::span<Lightgrid const> lightgrids() const {
			return get_data_span<Lightgrid const>(LumpIndex::LIGHTGRID);
		}
		
		// ================================
		// VISIBILITY
		
		struct VisibilityCluster {
			gsl::span<uint8_t const> data;
			
			inline bool can_see(int32_t other_cluster) {
				// (other_cluster >> 3) determines which byte that cluster is in
				// (1 << (other_cluster & 7)) determines which bit of that byte is the vis for that cluster
				// this function is basically a faster way to do the following:
				//     size_t cluster_byte = other_cluster / 8;
				//     size_t cluster_bit  = other_cluster % 8;
				//     return data[cluster_byte] & (1 << cluster_bit);
				return data[other_cluster >> 3] & (1 << (other_cluster & 7));
			}
		};
		
		inline Visibility visibility() const {
			return Visibility {
				*get_data<VisibilityHeader const>(LumpIndex::VISIBILITY),
				get_data_span<uint8_t const>(LumpIndex::VISIBILITY, sizeof(VisibilityHeader))
			};
		}
		
		inline VisibilityCluster visibility_cluster(int32_t idx) const {
			auto vis = visibility();
			return VisibilityCluster { gsl::span<uint8_t const> { vis.data.data() + vis.header.cluster_bytes * idx, vis.header.cluster_bytes }};
		}
		
		// ================================
		
		inline gsl::span<uint16_t const> lightarray() const {
			return get_data_span<uint16_t const>(LumpIndex::LIGHTARRAY);
		}
		
	private:
		
		Header const * m_base = nullptr;
		
	};
	
}
