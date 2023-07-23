#pragma once

#include "file_fmt.hh"

#include <meadow/istring.hh>

#include <map>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace BSP {
	
	struct Reader {
		
		inline Reader() = default;
		inline Reader(uint8_t const * base) { rebase(base); }
		inline Reader(Reader const &) = default;
		inline Reader(Reader &&) = default;
		
		inline ~Reader() = default;
		
		struct ReadException : public std::exception {
			ReadException(std::string what) : m_what(what) {}
			inline char const * what() const noexcept override { return m_what.data(); }
		private:
			std::string m_what;
		};
		
		inline void rebase(uint8_t const * base) {
			m_base = reinterpret_cast<Header const *> (base);
		}
		
		inline Lump const & get_lump(LumpIndex lump_num) const {
			return m_base->lumps[static_cast<size_t>(lump_num)];
		}
		
		template <typename T = uint8_t> T const * get_data(LumpIndex lump_num) const {
			Lump const & lump = get_lump(lump_num);
			return reinterpret_cast<T const *>(reinterpret_cast<uint8_t const *>(m_base) + lump.offs);
		}
		
		inline std::string_view get_string_view(LumpIndex lump_num) const {
			Lump const & lump = get_lump(lump_num);
			return std::string_view {
				reinterpret_cast<char const *>(reinterpret_cast<uint8_t const *>(m_base) + lump.offs),
				static_cast<size_t>(lump.size - 1)
			};
		}
		
		template <typename T = uint8_t> std::span<T> get_data_span(LumpIndex lump_num, size_t offset = 0) const {
			Lump const & lump = get_lump(lump_num);
			return std::span<T> {
				reinterpret_cast<T const *>(reinterpret_cast<uint8_t const *>(m_base) + lump.offs + offset),
				lump.size / sizeof(T) - offset
			};
		}
		
		inline BSP::Header const & header() { return *m_base; }
		
		// ================================
		// CHECKS
		
		inline bool has_visibility() const {
			return get_lump(LumpIndex::VISIBILITY).size;
		}
		
		// ================================
		// ENTITIES
		
		using Entity = std::map<meadow::istring_view, meadow::istring_view, std::less<>>;
		using EntityArray = std::vector<Entity>;
		
		inline std::string_view entities() const {
			return get_string_view(LumpIndex::ENTITIES);
		}
		
		static EntityArray parse_entities(std::string_view const &);
		EntityArray entities_parsed() const;
		
		// ================================
		// SHADERS
		
		using ShaderArray = std::span<Shader const>;
		
		inline ShaderArray shaders() const {
			return get_data_span<Shader const>(LumpIndex::SHADERS);
		}
		
		// ================================
		// PLANES
		
		using PlaneArray = std::span<Plane const>;

		inline PlaneArray planes() const {
			return get_data_span<Plane const>(LumpIndex::PLANES);
		}
		
		// ================================
		// NODES

		using NodeArray = std::span<Node const>;

		inline NodeArray nodes() const {
			return get_data_span<Node const>(LumpIndex::NODES);
		}
		
		// ================================
		// LEAFS
		
		using LeafArray = std::span<Leaf const>;
		
		inline LeafArray leafs() const {
			return get_data_span<Leaf const>(LumpIndex::LEAFS);
		}
		
		// ================================
		// LEAFSURFACES
		
		using LeafSurfaceArray = std::span<int32_t const>;
		
		inline LeafSurfaceArray leafsurfaces() const {
			return get_data_span<int32_t const>(LumpIndex::LEAFSURFACES);
		}
		
		// ================================
		// LEAFBRUSHES
		
		inline std::span<int32_t const> leafbrushes() const {
			return get_data_span<int32_t const>(LumpIndex::LEAFBRUSHES);
		}
		
		// ================================
		// MODELS
		
		using ModelArray = std::span<Model const>;

		inline ModelArray models() const {
			return get_data_span<Model const>(LumpIndex::MODELS);
		}
		
		// ================================
		// BRUSHES

		using BrushArray = std::span<Brush const>;
		
		inline BrushArray brushes() const {
			return get_data_span<Brush const>(LumpIndex::BRUSHES);
		}
		
		// ================================
		// BRUSHSIDES

		using BrushSideArray = std::span<BrushSide const>;
		
		inline BrushSideArray brushsides() const {
			return get_data_span<BrushSide const>(LumpIndex::BRUSHSIDES);
		}
		
		// ================================
		// DRAWVERTS
		
		using VertexArray = std::span<DrawVert const>;
		
		inline VertexArray drawverts() const {
			return get_data_span<DrawVert const>(LumpIndex::DRAWVERTS);
		}
		
		// ================================
		// DRAWINDEXES
		
		using IndexArray = std::span<int32_t const>;
		
		inline IndexArray drawindices() const {
			return get_data_span<int32_t const>(LumpIndex::DRAWINDEXES);
		}
		
		// ================================
		// FOGS
		
		using FogArray = std::span<Fog const>;
		
		inline FogArray fogs() const {
			return get_data_span<Fog const>(LumpIndex::FOGS);
		}
		
		// ================================
		// SURFACES
		
		using SurfaceArray = std::span<Surface const>;
		
		inline SurfaceArray surfaces() const {
			return get_data_span<Surface const>(LumpIndex::SURFACES);
		}
		
		// ================================
		// LIGHTMAPS
		
		using LightmapArray = std::span<Lightmap const>;
		
		inline LightmapArray lightmaps() const {
			return get_data_span<Lightmap const>(LumpIndex::LIGHTMAPS);
		}
		
		// ================================
		// LIGHTGRID

		using LightgridArray = std::span<Lightgrid const>;
		
		inline LightgridArray lightgrids() const {
			return get_data_span<Lightgrid const>(LumpIndex::LIGHTGRID);
		}
		
		// ================================
		// VISIBILITY
		
		struct Visibility {
			VisibilityHeader const & header;
			std::span<uint8_t const> data;

			struct Cluster {
				std::span<uint8_t const> data;

				inline bool can_see(int32_t other_cluster) const {
					// (other_cluster >> 3) determines which byte that cluster is in
					// (1 << (other_cluster & 7)) determines which bit of that byte is the vis for that cluster
					// this function is basically a faster way to do the following:
					//     size_t cluster_byte = other_cluster / 8;
					//     size_t cluster_bit  = other_cluster % 8;
					//     return data[cluster_byte] & (1 << cluster_bit);
					return data[other_cluster >> 3] & (1 << (other_cluster & 7));
				}
			};

			inline Cluster cluster(int32_t idx) const {
				return Cluster { std::span<uint8_t const> { data.data() + header.cluster_bytes * idx, static_cast<size_t>(header.cluster_bytes) }};
			}
		};
		
		inline Visibility visibility() const {
			return Visibility {
				*get_data<VisibilityHeader const>(LumpIndex::VISIBILITY),
				get_data_span<uint8_t const>(LumpIndex::VISIBILITY, sizeof(VisibilityHeader))
			};
		}
		
		// ================================
		// LIGHTARRAY

		using LightArray = std::span<uint16_t const>;
		
		inline LightArray lightarray() const {
			return get_data_span<uint16_t const>(LumpIndex::LIGHTARRAY);
		}
		
		// ================================
		
	private:
		
		Header const * m_base = nullptr;
		
	};
	
}

