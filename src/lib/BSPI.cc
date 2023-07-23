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
// LEAFS
// ================================================================

BSPI::LeafArray::LeafArray(BSP::Reader::LeafArray const & in) {
	reserve(in.size());
	for (auto const & out : in) {
		emplace_back( out );
	}
}

BSPI::ByteArray BSPI::LeafArray::serialize() const {
	BSPI::ByteArray bytes;
	bytes.resize(size() * sizeof(BSP::Leaf));
	for (size_t i = 0; i < size(); i++) {
		BSP::Leaf const & in = at(i);
		BSP::Leaf & out = *reinterpret_cast<BSP::Leaf *>(bytes.data() + i * sizeof(BSP::Leaf));
		out = in;
	}
	return bytes;
}

// ================================================================
// LEAFSURFACES
// ================================================================

BSPI::LeafSurfaceArray::LeafSurfaceArray(BSP::Reader::LeafSurfaceArray const & in) {
	reserve(in.size());
	for (auto const & out : in) {
		emplace_back( out );
	}
}

BSPI::ByteArray BSPI::LeafSurfaceArray::serialize() const {
	BSPI::ByteArray bytes;
	bytes.resize(size() * sizeof(int32_t));
	for (size_t i = 0; i < size(); i++) {
		int32_t const & in = at(i);
		int32_t & out = *reinterpret_cast<int32_t *>(bytes.data() + i * sizeof(int32_t));
		out = in;
	}
	return bytes;
}

// ================================================================
// MODELS
// ================================================================

BSPI::ModelArray::ModelArray(BSP::Reader::ModelArray const & in) {
	reserve(in.size());
	for (auto const & out : in) {
		emplace_back( out );
	}
}

BSPI::ByteArray BSPI::ModelArray::serialize() const {
	BSPI::ByteArray bytes;
	bytes.resize(size() * sizeof(BSP::Model));
	for (size_t i = 0; i < size(); i++) {
		BSP::Model const & in = at(i);
		BSP::Model & out = *reinterpret_cast<BSP::Model *>(bytes.data() + i * sizeof(BSP::Model));
		out = in;
	}
	return bytes;
}

// ================================================================
// BRUSHES
// ================================================================

BSPI::BrushArray::BrushArray(BSP::Reader::BrushArray const & in) {
	reserve(in.size());
	for (auto const & out : in) {
		emplace_back( out );
	}
}

BSPI::ByteArray BSPI::BrushArray::serialize() const {
	BSPI::ByteArray bytes;
	bytes.resize(size() * sizeof(BSP::Brush));
	for (size_t i = 0; i < size(); i++) {
		BSP::Brush const & in = at(i);
		BSP::Brush & out = *reinterpret_cast<BSP::Brush *>(bytes.data() + i * sizeof(BSP::Brush));
		out = in;
	}
	return bytes;
}

// ================================================================
// BRUSHSIDES
// ================================================================

BSPI::BrushSideArray::BrushSideArray(BSP::Reader::BrushSideArray const & vin) {
	reserve(vin.size());
	for (auto const & v : vin) {
		emplace_back( v );
	}
}

BSPI::ByteArray BSPI::BrushSideArray::serialize() const {
	BSPI::ByteArray bytes;
	bytes.resize(size() * sizeof(BSP::BrushSide));
	for (size_t i = 0; i < size(); i++) {
		BSP::BrushSide const & vin = at(i);
		BSP::BrushSide & vout = *reinterpret_cast<BSP::BrushSide *>(bytes.data() + i * sizeof(BSP::BrushSide));
		vout = vin;
	} 
	return bytes;
}

// ================================================================
// DRAWVERT
// ================================================================

BSPI::VertexArray::VertexArray(BSP::Reader::VertexArray const & vertin) {
	reserve(vertin.size());
	for (auto const & vert : vertin) {
		emplace_back( vert );
	}
}

BSPI::ByteArray BSPI::VertexArray::serialize() const {
	BSPI::ByteArray bytes;
	bytes.resize(size() * sizeof(BSP::DrawVert));
	for (size_t i = 0; i < size(); i++) {
		BSP::DrawVert const & vertin = at(i);
		BSP::DrawVert & vertout = *reinterpret_cast<BSP::DrawVert *>(bytes.data() + i * sizeof(BSP::DrawVert));
		vertout = vertin;
	}
	return bytes;
}

// ================================================================
// DRAWINDEX
// ================================================================

BSPI::IndexArray::IndexArray(BSP::Reader::IndexArray const & idxin) {
	reserve(idxin.size());
	for (auto const & idx : idxin) {
		emplace_back( idx );
	}
}

BSPI::ByteArray BSPI::IndexArray::serialize() const {
	BSPI::ByteArray bytes;
	bytes.resize(size() * sizeof(int32_t));
	for (size_t i = 0; i < size(); i++) {
		int32_t const & idxin = at(i);
		int32_t & idxout = *reinterpret_cast<int32_t *>(bytes.data() + i * sizeof(int32_t));
		idxout = idxin;
	}
	return bytes;
}

// ================================================================
// SURFACE
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

// ================================================================
// LIGHTMAP
// ================================================================

BSPI::LightmapArray::LightmapArray(BSP::Reader::LightmapArray const & lmin) {
	reserve(lmin.size());
	for (auto const & lm : lmin) {
		emplace_back( lm );
	}
}

BSPI::ByteArray BSPI::LightmapArray::serialize() const {
	BSPI::ByteArray bytes;
	bytes.resize(size() * sizeof(BSP::Lightmap));
	for (size_t i = 0; i < size(); i++) {
		BSP::Lightmap const & lmin = at(i);
		BSP::Lightmap & lmout = *reinterpret_cast<BSP::Lightmap *>(bytes.data() + i * sizeof(BSP::Lightmap));
		lmout = lmin;
	}
	return bytes;
}
