#pragma once

#include "intermediate.hh"
#include "reader.hh"

#include <array>
#include <cstring>
#include <memory>
#include <stdexcept>

namespace BSP {
	
	// ================================
	// BASE
	
	struct UnprovidableLumpException : public std::exception {
		UnprovidableLumpException(LumpIndex i) : index(i) {}
		LumpIndex const index;
	};
	
	struct LumpProvider {
		virtual ~LumpProvider() = default;
		virtual BSPI::ByteArray generate_lump(LumpIndex) = 0;
	};
	
	using LumpProviderPtr = std::shared_ptr<LumpProvider>;
	
	struct Assembler {
		inline Assembler() = default;
		inline Assembler(LumpProviderPtr const & ptr) : providers { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr } {}
		inline LumpProviderPtr & operator [] (LumpIndex idx) { return providers[static_cast<size_t>(idx)]; }
		inline void set_all(LumpProviderPtr const & ptr) { providers.fill(ptr); }
		BSPI::ByteArray assemble();
	private:
		std::array<LumpProviderPtr, 18> providers;
	};
	
	struct BSPReaderLumpProvider : public LumpProvider {
		BSPReaderLumpProvider() = delete;
		inline BSPReaderLumpProvider(BSP::Reader const & bspr) : bspr(bspr) {}
		BSPI::ByteArray generate_lump(LumpIndex idx) override {
			auto data = bspr.get_data_span<uint8_t const>(idx);
			return BSPI::ByteArray { data.begin(), data.end() };
		}
	private:
		BSP::Reader bspr;
	};
	
	// ================================
	// INTERMEDIATE
	
	struct BSPIEntityArrayLumpProvider : public LumpProvider {
		BSPIEntityArrayLumpProvider() = delete;
		inline BSPIEntityArrayLumpProvider(std::shared_ptr<BSPI::EntityArray> const & ents) : ents(ents) {}
		inline BSPI::ByteArray generate_lump(LumpIndex idx) override {
			if (idx != LumpIndex::ENTITIES) throw UnprovidableLumpException(idx);
			auto ent_str = ents->stringify();
			BSPI::ByteArray ret (ent_str.size() + 1);
			memcpy(ret.data(), ent_str.data(), ent_str.size());
			ret[ent_str.size()] = 0;
			return ret;
		}
	private:
		std::shared_ptr<BSPI::EntityArray> ents;
	};
	
	template <typename T, LumpIndex IDX>
	struct BSPIGenericLumpProvider : public LumpProvider {
		BSPIGenericLumpProvider() = delete;
		inline BSPIGenericLumpProvider(std::shared_ptr<T> const & value) : value(value) {}
		inline BSPI::ByteArray generate_lump(LumpIndex idx) override {
			if (idx != IDX) throw UnprovidableLumpException(idx);
			return value->serialize();
		}
	private:
		std::shared_ptr<T> value;
	};
	
	using BSPIShaderArrayLumpProvider = BSPIGenericLumpProvider<BSPI::ShaderArray, LumpIndex::SHADERS>;
	using BSPILeafArrayLumpProvider = BSPIGenericLumpProvider<BSPI::LeafArray, LumpIndex::LEAFS>;
	using BSPILeafSurfacesArrayLumpProvider = BSPIGenericLumpProvider<BSPI::LeafSurfaceArray, LumpIndex::LEAFSURFACES>;
	using BSPIModelArrayLumpProvider = BSPIGenericLumpProvider<BSPI::ModelArray, LumpIndex::MODELS>;
	using BSPIBrushArrayLumpProvider = BSPIGenericLumpProvider<BSPI::BrushArray, LumpIndex::BRUSHES>;
	using BSPIBrushSidesArrayLumpProvider = BSPIGenericLumpProvider<BSPI::BrushSideArray, LumpIndex::BRUSHSIDES>;
	
	struct BSPIVertexArrayLumpProvider : public LumpProvider {
		BSPIVertexArrayLumpProvider() = delete;
		inline BSPIVertexArrayLumpProvider(std::shared_ptr<BSPI::VertexArray> const & vertices) : vertices(vertices) {}
		inline BSPI::ByteArray generate_lump(LumpIndex idx) override {
			if (idx != LumpIndex::DRAWVERTS) throw UnprovidableLumpException(idx);
			return vertices->serialize();
		}
	private:
		std::shared_ptr<BSPI::VertexArray> vertices;
	};
	
	struct BSPIIndexArrayLumpProvider : public LumpProvider {
		BSPIIndexArrayLumpProvider() = delete;
		inline BSPIIndexArrayLumpProvider(std::shared_ptr<BSPI::IndexArray> const & indices) : indices(indices) {}
		inline BSPI::ByteArray generate_lump(LumpIndex idx) override {
			if (idx != LumpIndex::DRAWINDEXES) throw UnprovidableLumpException(idx);
			return indices->serialize();
		}
	private:
		std::shared_ptr<BSPI::IndexArray> indices;
	};
	
	struct BSPISurfaceArrayLumpProvider : public LumpProvider {
		BSPISurfaceArrayLumpProvider() = delete;
		inline BSPISurfaceArrayLumpProvider(std::shared_ptr<BSPI::SurfaceArray> const & surfaces) : surfaces(surfaces) {}
		inline BSPI::ByteArray generate_lump(LumpIndex idx) override {
			if (idx != LumpIndex::SURFACES) throw UnprovidableLumpException(idx);
			return surfaces->serialize();
		}
	private:
		std::shared_ptr<BSPI::SurfaceArray> surfaces;
	};
	
	struct BSPILightmapArrayLumpProvider : public LumpProvider {
		BSPILightmapArrayLumpProvider() = delete;
		inline BSPILightmapArrayLumpProvider(std::shared_ptr<BSPI::LightmapArray> const & lightmaps) : lightmaps(lightmaps) {}
		inline BSPI::ByteArray generate_lump(LumpIndex idx) override {
			if (idx != LumpIndex::LIGHTMAPS) throw UnprovidableLumpException(idx);
			return lightmaps->serialize();
		}
	private:
		std::shared_ptr<BSPI::LightmapArray> lightmaps;
	};
}
