#pragma once

#include "constants.hh"
#include "reader.hh"

#include <meadow/istring.hh>

#include <map>
#include <vector>

namespace BSP::Intermediate {
	
	using ByteArray = std::vector<uint8_t>;
	
	using Entity = std::map<meadow::istring, meadow::istring, std::less<>>;
	
	struct EntityArray : public std::vector<Entity> {
		
		using std::vector<Entity>::vector;
		using std::vector<Entity>::operator [];
		
		EntityArray() = default;
		explicit EntityArray(BSP::Reader::EntityArray const &);
		~EntityArray() = default;
		
		std::string stringify() const;
	};
	
}

namespace BSPI = BSP::Intermediate;
