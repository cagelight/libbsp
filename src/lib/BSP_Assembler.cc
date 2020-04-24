#include "libbsp/assembler.hh"

BSPI::ByteArray BSP::Assembler::assemble() {
	
	for (auto const & ptr : providers) if (!ptr) throw std::logic_error {"a provider cannot be null"};
	
	BSPI::ByteArray bytes;
	
	BSP::Header header;
	header.ident = BSP::IDENT;
	header.version = BSP::VERSION;
	
	bytes.resize(sizeof(BSP::Header));
	for (size_t l = 0; l < 18; l++) {
		BSP::LumpIndex li = static_cast<BSP::LumpIndex>(l);
		auto lump_bytes = providers[l]->generate_lump(li);
		header.lumps[l].offs = bytes.size();
		bytes.insert(bytes.end(), lump_bytes.begin(), lump_bytes.end());
		header.lumps[l].size = lump_bytes.size();
	}
	memcpy(bytes.data(), &header, sizeof(BSP::Header));
	
	return bytes;
}
