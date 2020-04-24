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
	
}
