#pragma once

#include "constants.hh"
#include "reader.hh"

#include <meadow/istring.hh>

#include <map>
#include <vector>

namespace BSP::Intermediate {
	
	using ByteArray = std::vector<uint8_t>;
	
	// ================================
	// ENTITIES
	
	using Entity = std::map<meadow::istring, meadow::istring, std::less<>>;
	
	struct EntityArray : public std::vector<Entity> {
		
		using std::vector<Entity>::vector;
		using std::vector<Entity>::operator [];
		
		EntityArray() = default;
		explicit EntityArray(BSP::Reader::EntityArray const &);
		~EntityArray() = default;
		
		std::string stringify() const;
	};
	
	// ================================
	// SHADERS
	
	struct Shader {
		meadow::istring path;
		int32_t surface_flags;
		int32_t content_flags;
	};
	
	struct ShaderArray : public std::vector<Shader> {
		
		using std::vector<Shader>::vector;
		using std::vector<Shader>::operator [];
		
		ShaderArray() = default;
		explicit ShaderArray(BSP::Reader::ShaderArray const &);
		~ShaderArray() = default;
		
		ByteArray serialize() const;
	};
	
	// ================================
	// LEAFS
	
	struct LeafArray : public std::vector<BSP::Leaf> {
		
		using std::vector<BSP::Leaf>::vector;
		using std::vector<BSP::Leaf>::operator [];
		
		LeafArray() = default;
		explicit LeafArray(BSP::Reader::LeafArray const &);
		~LeafArray() = default;
		
		ByteArray serialize() const;
	};
	
	// ================================
	// LEAFSURFACES
	
	struct LeafSurfaceArray : public std::vector<int32_t> {
		
		using std::vector<int32_t>::vector;
		using std::vector<int32_t>::operator [];
		
		LeafSurfaceArray() = default;
		explicit LeafSurfaceArray(BSP::Reader::LeafSurfaceArray const &);
		~LeafSurfaceArray() = default;
		
		ByteArray serialize() const;
	};
	
	// ================================
	// MODELS
	
	struct ModelArray : public std::vector<BSP::Model> {
		
		using std::vector<BSP::Model>::vector;
		using std::vector<BSP::Model>::operator [];
		
		ModelArray() = default;
		explicit ModelArray(BSP::Reader::ModelArray const &);
		~ModelArray() = default;
		
		ByteArray serialize() const;
	};
	
	// ================================
	// BRUSHES
	
	struct BrushArray : public std::vector<BSP::Brush> {
		
		using std::vector<BSP::Brush>::vector;
		using std::vector<BSP::Brush>::operator [];
		
		BrushArray() = default;
		explicit BrushArray(BSP::Reader::BrushArray const &);
		~BrushArray() = default;
		
		ByteArray serialize() const;
	};
	
	// ================================
	// BRUSHSIDES
	
	struct BrushSideArray : public std::vector<BSP::BrushSide> {
		
		using std::vector<BSP::BrushSide>::vector;
		using std::vector<BSP::BrushSide>::operator [];
		
		BrushSideArray() = default;
		explicit BrushSideArray(BSP::Reader::BrushSideArray const &);
		~BrushSideArray() = default;
		
		ByteArray serialize() const;
	};
	
	// ================================
	// DRAWVERT
	
	struct VertexArray : public std::vector<BSP::DrawVert> {
		
		using std::vector<BSP::DrawVert>::vector;
		using std::vector<BSP::DrawVert>::operator [];
		
		VertexArray() = default;
		explicit VertexArray(BSP::Reader::VertexArray const &);
		~VertexArray() = default;
		
		ByteArray serialize() const;
	};
	
	// ================================
	// DRAWINDEX
	
	struct IndexArray : public std::vector<int32_t> {
		
		using std::vector<int32_t>::vector;
		using std::vector<int32_t>::operator [];
		
		IndexArray() = default;
		explicit IndexArray(BSP::Reader::IndexArray const &);
		~IndexArray() = default;
		
		ByteArray serialize() const;
	};
	
	// ================================
	// SURFACE
	
	struct SurfaceArray : public std::vector<BSP::Surface> {
		
		using std::vector<BSP::Surface>::vector;
		using std::vector<BSP::Surface>::operator [];
		
		SurfaceArray() = default;
		explicit SurfaceArray(BSP::Reader::SurfaceArray const &);
		~SurfaceArray() = default;
		
		ByteArray serialize() const;
	};
	
	// ================================
	// LIGHTMAP
	
	struct LightmapArray : public std::vector<BSP::Lightmap> {
		
		using std::vector<BSP::Lightmap>::vector;
		using std::vector<BSP::Lightmap>::operator [];
		
		LightmapArray() = default;
		explicit LightmapArray(BSP::Reader::LightmapArray const &);
		~LightmapArray() = default;
		
		ByteArray serialize() const;
	};
}

namespace BSPI = BSP::Intermediate;
