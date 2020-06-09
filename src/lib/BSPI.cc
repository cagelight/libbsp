#include "libbsp/intermediate.hh"

#include <cstring>
#include <iomanip>
#include <sstream>

// ================================================================
// ENTITY
// ================================================================

BSPI::EntityArray::EntityArray(BSP::Reader::EntityArray const & ents_in) {
	reserve(ents_in.size());
	for (auto const & ent_in : ents_in) {
		auto & ent = emplace_back();
		for (auto const & kvp : ent_in) {
			ent[ meadow::istring {kvp.first} ] = meadow::istring {kvp.second};
		}
	}
}

std::string BSPI::EntityArray::stringify() const {
	std::ostringstream ss;
	for (auto const & ent : *this) {
		ss << "{\n";
		for (auto const & kvp : ent) {
			ss << "\"" << kvp.first << "\" \"" << kvp.second << "\"\n";
		}
		ss << "}\n";
	}
	return ss.str();
}

// ================================================================
// SHADER
// ================================================================

BSPI::ShaderArray::ShaderArray(BSP::Reader::ShaderArray const & shin) {
	for (auto const & sh : shin) {
		emplace_back( Shader { sh.shader, sh.surface_flags, sh.content_flags } );
	}
}

BSPI::ByteArray BSPI::ShaderArray::serialize() const {
	BSPI::ByteArray bytes;
	bytes.resize(size() * sizeof(BSP::Shader));
	for (size_t i = 0; i < size(); i++) {
		BSPI::Shader const & shin = at(i);
		BSP::Shader & shout = *reinterpret_cast<BSP::Shader *>(bytes.data() + i * sizeof(BSP::Shader));
		std::strncpy(shout.shader, shin.path.data(), 64);
		shout.surface_flags = shin.surface_flags;
		shout.content_flags = shin.content_flags;
	}
	return bytes;
}

// ================================================================
// SHADER
// ================================================================

BSPI::SurfaceArray::SurfaceArray(BSP::Reader::SurfaceArray const & surfin) {
	for (auto const & surf : surfin) {
		emplace_back( surf );
	}
}

BSPI::ByteArray BSPI::SurfaceArray::serialize() const {
	BSPI::ByteArray bytes;
	bytes.resize(size() * sizeof(BSP::Surface));
	for (size_t i = 0; i < size(); i++) {
		BSP::Surface const & surfin = at(i);
		BSP::Surface & surfout = *reinterpret_cast<BSP::Surface *>(bytes.data() + i * sizeof(BSP::Surface));
		surfout = surfin;
	}
	return bytes;
}
