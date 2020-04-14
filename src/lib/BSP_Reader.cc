#include "libbsp.hh"

using namespace libbsp;

struct alignas(4) BSP_Lump {
	int32_t offs;
	int32_t filelen;
};

struct alignas(4) BSP_Header {
	int32_t ident;
	int32_t version;
	BSP_Lump lumps[18];
};

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

BSP_Reader::BSP_Reader(uint8_t const * base) {
	rebase(base);
}

void BSP_Reader::rebase(uint8_t const * base) {
	m_base = base;
}

struct Lump_Data {
	uint8_t const * ptr;
	uint_fast32_t size;
};

static Lump_Data get_data(uint8_t const * base, Lump lump_num) {
	BSP_Lump const & lump = reinterpret_cast<BSP_Header const *>(base)->lumps[static_cast<uint8_t>(lump_num)];
	return {
		base + lump.offs,
		static_cast<uint_fast32_t>(lump.filelen > 0 ? lump.filelen : 0)
	};
}

std::string_view BSP_Reader::entities() const {
	Lump_Data data = get_data(m_base, Lump::ENTITIES);
	return {
		reinterpret_cast<char const *>(data.ptr),
		data.size
	};
}
