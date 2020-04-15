#pragma once

#include "libbsp_version.hh"
#include "libbsp_fmt.hh"

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
		gsl::span<fmt::BSP_Shader const> shaders() const;
		gsl::span<fmt::BSP_Plane const> planes() const;
		
	private:
		
		fmt::BSP_Header const * m_base = nullptr;
		
	};
	
}
