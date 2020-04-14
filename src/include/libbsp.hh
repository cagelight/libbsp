#pragma once

#include "libbsp_version.hh"

#include <cstdint>
#include <cstdlib>

#include <memory>
#include <string_view>
#include <vector>

namespace libbsp {
	
	struct BSP_Reader {
		
		BSP_Reader() = default;
		BSP_Reader(uint8_t const * base);
		
		~BSP_Reader() = default;
		
		void rebase(uint8_t const * base);
		
		// ================================
		// RETRIEVAL
		
		std::string_view entities() const;
		
	private:
		
		uint8_t const * m_base = nullptr;
		
	};
	
}
