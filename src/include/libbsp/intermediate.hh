#pragma once

#include "constants.hh"
#include "reader.hh"

#include <meadow/istring.hh>

#include <map>
#include <vector>

namespace BSP::Intermediate {
	
	using ByteArray = std::vector<uint8_t>;
	
	// ================================
	// ENTITY
	
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
	// SHADER
	
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
	// SURFACE
	
	struct SurfaceArray : public std::vector<BSP::Surface> {
		
		using std::vector<BSP::Surface>::vector;
		using std::vector<BSP::Surface>::operator [];
		
		SurfaceArray() = default;
		explicit SurfaceArray(BSP::Reader::SurfaceArray const &);
		~SurfaceArray() = default;
		
		ByteArray serialize() const;
	};
}

namespace BSPI = BSP::Intermediate;
