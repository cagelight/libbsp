#include "libbsp.hh"

using namespace libbsp;
using namespace libbsp::fmt;

BSP_Reader::BSP_Reader(uint8_t const * base) {
	rebase(base);
}

void BSP_Reader::rebase(uint8_t const * base) {
	m_base = reinterpret_cast<BSP_Header const *> (base);
}

/*
struct Lump_Data {
	uint8_t const * ptr;
	uint_fast32_t size;
};

static Lump_Data get_data(uint8_t const * base, Lump lump_num) {
	BSP_Lump const & lump = reinterpret_cast<BSP_Header const *>(base)->get_lump(lump_num);
	return {
		base + lump.offs,
		static_cast<uint_fast32_t>(lump.filelen > 0 ? lump.filelen : 0)
	};
}
*/

std::string_view BSP_Reader::entities() const {
	return m_base->get_data<char>(Lump::ENTITIES);
}

gsl::span<BSP_Shader const> BSP_Reader::shaders() const {
	return m_base->get_data_span<BSP_Shader const>(Lump::SHADERS);
}

gsl::span<fmt::BSP_Plane const> BSP_Reader::planes() const {
	return m_base->get_data_span<BSP_Plane const>(Lump::PLANES);
}

gsl::span<fmt::BSP_Node const> BSP_Reader::nodes() const {
	return m_base->get_data_span<BSP_Node const>(Lump::NODES);
}
