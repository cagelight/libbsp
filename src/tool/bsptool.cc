#include "libbsp.hh"
#include "argagg.hh"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <filesystem>

int main(int argc, char * * argv) {
	
	argagg::parser argp {{
		{ "help",      { "-h", "--help" }, "Show this help message", 0 },
		{ "info",      { "-i", "--info" }, "Print general info", 0 },
		{ "entstr",    { "-e", "--entstring" }, "Print the entity string", 0 },
		{ "ents",      { "-E", "--ents" }, "Print information about the entities", 0 },
		{ "shaders",   { "-s", "--shaders" }, "Print the shaders used", 0 },
		{ "shaders+",  { "-S", "--shaders-extra" }, "Print the shaders used plus extra information", 0 },
		{ "reprocess", { "-r", "--reprocess" }, "Load the BSP and resave it", 0 },
		{ "output",    { "-o", "--output" }, "Output path for saving operations", 1 },
		
		{ "shsurfs",   { "--shader-surfaces" }, "<shader>", 0 },
		{ "remap",     { "--remap" }, "<idx to remap>, requires --src, --dst, and -o to be specified", 1 },
		
		{ "src",       { "--src" }, "<source shader name>", 1 },
		{ "dst",       { "--dst" }, "<dest shader name>", 1 },
	}};
	
	argagg::parser_results args;
	
	try {
		args = argp.parse(argc, argv);
	} catch (std::exception const & e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	
	// ================================
	// HELP
	// ================================
	
	if (args["help"] || !args.count()) {
		std::cerr << "Usage: bsptool [options] <path to bsp>" << std::endl << argp;
		return 0;
	}
	
	// ================================
	// SETUP
	// ================================
	
	std::string bsp_path = args.as<std::string>(0);
	
	std::string output_path;
	if (args["output"])
		output_path = args["output"].as<std::string>();
	
	auto fdr = open( bsp_path.c_str(), O_RDONLY );
	
	if (fdr == -1) {
		std::cerr << "File not found!" << std::endl;
		return 1;
	}
	
	struct stat sb;
	fstat(fdr, &sb);
	
	if (sb.st_size < (ssize_t)sizeof(BSP::Header)) {
		std::cerr << "File too small to be a BSP file!" << std::endl;
		return 1;
	}

	BSP::Reader bspr;
	std::vector<uint8_t> rdat;
	
	if (!std::filesystem::equivalent(bsp_path, output_path))
		bspr.rebase(reinterpret_cast<uint8_t const *> (mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fdr, 0)));
	else {
		rdat.resize(sb.st_size);
		read(fdr, rdat.data(), sb.st_size);
		close(fdr);
		bspr.rebase(rdat.data());
	}
	
	if (bspr.header().ident != BSP::IDENT) {
		std::cerr << "File does not appear to be a BSP file!" << std::endl;
		return 1;
	}
	
	// ================================
	// INFO
	// ================================
	
	if (args["info"]) {
		
		std::unordered_map<BSP::SurfaceType, uint_fast32_t> surface_type_uage;
		for (auto const & surf : bspr.surfaces()) surface_type_uage[surf.type]++;
		
		std::cout
			<< sb.st_size << " byte BSP file" << std::endl
			<< bspr.entities().size() << " entity string bytes" << std::endl
			
			<< bspr.shaders().size() << " shaders"
			<< " (" << bspr.shaders().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.planes().size() << " planes"
			<< " (" << bspr.planes().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.nodes().size() << " nodes" 
			<< " (" << bspr.nodes().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.leafs().size() << " leafs"
			<< " (" << bspr.leafs().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.leafsurfaces().size() << " leafsurfaces"
			<< " (" << bspr.leafsurfaces().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.leafbrushes().size() << " leafbrushes"
			<< " (" << bspr.leafbrushes().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.models().size() << " models"
			<< " (" << bspr.models().size_bytes() << " bytes" << ")" 
			<< std::endl;
			
		uint64_t total_side_refs = 0;
		int32_t highest_side_count = 0;
		
		for (auto b : bspr.brushes()) {
			total_side_refs += b.num_sides;
			if (b.num_sides > highest_side_count) highest_side_count = b.num_sides;
		}
		
		std::cout
			<< bspr.brushes().size() << " brushes"
			<< " (" << bspr.brushes().size_bytes() << " bytes" << ")" 
			<< std::endl
			<< "    "
			<< "highest number of sides: "
			<< highest_side_count
			<< ", average side count: "
			<< std::setprecision(4)
			<< static_cast<double>(total_side_refs) / bspr.brushes().size()
			<< std::endl;
			
		std::cout
			<< bspr.brushsides().size() << " brushsides"
			<< " (" << bspr.brushsides().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.drawverts().size() << " drawverts"
			<< " (" << bspr.drawverts().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.drawindices().size() << " drawindexes"
			<< " (" << bspr.drawindices().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.fogs().size() << " fogs"
			<< " (" << bspr.fogs().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.surfaces().size() << " surfaces"
			<< " (" << bspr.surfaces().size_bytes() << " bytes" << ")" 
			<< std::endl
			<< "    "
			<< surface_type_uage[BSP::SurfaceType::PLANAR] << " planars, "
			<< surface_type_uage[BSP::SurfaceType::PATCH] << " patches, "
			<< surface_type_uage[BSP::SurfaceType::TRISOUP] << " trisoups, "
			<< surface_type_uage[BSP::SurfaceType::FLARE] << " flares"
			<< std::endl
			
			<< bspr.lightmaps().size() << " lightmaps"
			<< " (" << bspr.lightmaps().size_bytes() << " bytes" << ")" 
			<< std::endl
			
			<< bspr.lightgrids().size() << " lightgrid elements"
			<< " (" << bspr.lightgrids().size_bytes() << " bytes" << ")" 
			<< std::endl
		;
		
		if (bspr.has_visibility()) {
			std::cout
				<< bspr.visibility().data.size() << " bytes of visibility data"
				<< std::endl
				<< "    "
				<< bspr.visibility().header.clusters << " clusters, "
				<< bspr.visibility().header.cluster_bytes << " bytes per cluster"
				<< std::endl
			;
		} else {
			std::cout << "no visibility data" << std::endl;
		}
			
		std::cout
			<< bspr.lightarray().size() << " lightarray elements"
			<< " (" << bspr.lightarray().size_bytes() << " bytes" << ")" 
			<< std::endl
		;
	}
	
	// ================================
	// ENTSTR
	// ================================
	
	if (args["entstr"]) {
		std::cout << bspr.entities();
		std::flush(std::cout);
	}
	
	// ================================
	// ENTS
	// ================================
	
	if (args["ents"]) {
		auto ents = bspr.entities_parsed();
		
		std::cout << ents.size() << " entities" << std::endl;
		
		size_t classless = 0;
		std::map<meadow::istring_view, std::vector<BSP::Reader::Entity const *>> classnames;
		for (auto const & ent : ents) {
			auto iter = ent.find(meadow::istring_view("classname"));
			if (iter == ent.end()) {
				classless++;
				continue;
			} else {
				classnames[iter->second].emplace_back(&ent);
			}
		}
		
		std::cout << "classes:" << std::endl;
		for (auto const & cl : classnames) std::cout << "    " << cl.first << ": " << cl.second.size() << std::endl;
		if (classless) {
			std::cout << "    " << classless << " classless entities" << std::endl;
		}
	}
	
	// ================================
	// SHADERS
	// ================================
	
	if (args["shaders"]) {
		for (auto const & shad : bspr.shaders()) std::cout << shad.shader << std::endl;
	}
	
	// ================================
	// SHADERS+
	// ================================
	
	if (args["shaders+"]) {
		
		auto shads = bspr.shaders();
		
		std::unordered_map<int32_t, uint32_t> brush_usage;
		for (auto const & v : bspr.brushes()) brush_usage[v.shader]++;
		
		std::unordered_map<int32_t, uint32_t> brushside_usage;
		for (auto const & v : bspr.brushsides()) brushside_usage[v.shader]++;
		
		std::unordered_map<int32_t, uint32_t> surface_usage;
		for (auto const & v : bspr.surfaces()) surface_usage[v.shader]++;
		
		for (size_t i = 0; i < shads.size(); i++) {
			auto const & shad = shads[i];
			std::cout 
				<< "Shader Path:     " << shad.shader 
				<< std::endl
				<< "Brush Usage:     " << brush_usage[i]
				<< std::endl
				<< "Brushside Usage: " << brushside_usage[i]
				<< std::endl
				<< "Surface Usage:   " << surface_usage[i]
				<< std::endl
				<< "Content Flags:   " 
				<< std::bitset<32> {static_cast<size_t>(shad.content_flags)} 
				<< " ( " << static_cast<size_t>(shad.content_flags) << " )"
				<< std::endl
				<< "Surface Flags:   "
				<< std::bitset<32> {static_cast<size_t>(shad.surface_flags)}
				<< " ( " << static_cast<size_t>(shad.surface_flags) << " )"
				<< std::endl
				<< std::endl;
		}
	}
	
	// ================================
	// REPROCESS
	// ================================
	
	if (args["reprocess"]) {
		BSP::LumpProviderPtr pprov = std::make_shared<BSP::BSPReaderLumpProvider>(bspr);
		BSP::Assembler bspa { pprov };
		auto bytes = bspa.assemble();
		
		std::ofstream f { (std::string) args["reprocess"][0], std::ios_base::binary | std::ios_base::out };
		if (!f.good()) {
			// TODO error
			return 1;
		}
		f.write( reinterpret_cast<char const *>(bytes.data()), bytes.size());
		f.close();
	}
	
	// ================================
	// SHSURFS
	// ================================
	
	if (args["shsurfs"]) {
		meadow::istring src = meadow::s2i(args["src"].as<std::string>());
		std::vector<BSP::Surface const *> matchsurfs;
		for (BSP::Surface const & surf : bspr.surfaces())
			if (bspr.shaders()[surf.shader].shader == src) matchsurfs.emplace_back(&surf);
		for (size_t i = 0; i < matchsurfs.size(); i++) {
			BSP::Surface const & surf = *matchsurfs[i];
			std::cout
				<< "[" << i << "] "
				<< "Vertices: " << surf.vert_count
			<< std::endl;
		}
	}
	
	// ================================
	// REMAP
	// ================================
	
	if (args["remap"]) {
		meadow::istring src = meadow::s2i(args["src"].as<std::string>());
		meadow::istring dst = meadow::s2i(args["dst"].as<std::string>());
		int32_t idx = args["remap"].as<int32_t>();
		
		if (!output_path.size()) {
			return 1;
		}
		
		if (idx < -1) return 1;
		
		BSP::LumpProviderPtr pprov = std::make_shared<BSP::BSPReaderLumpProvider>(bspr);
		BSP::Assembler bspa { pprov };
		
		std::shared_ptr<BSPI::ShaderArray> shaders = std::make_shared<BSPI::ShaderArray>(bspr.shaders());
		bspa[BSP::LumpIndex::SHADERS] = std::make_shared<BSP::BSPIShaderArrayLumpProvider>(shaders);
		std::shared_ptr<BSPI::SurfaceArray> surfaces = std::make_shared<BSPI::SurfaceArray>(bspr.surfaces());
		bspa[BSP::LumpIndex::SURFACES] = std::make_shared<BSP::BSPISurfaceArrayLumpProvider>(surfaces);
		
		auto shiter = std::find_if(shaders->begin(), shaders->end(), [&](BSPI::Shader const & v){ return src == v.path; });
		if (shiter == shaders->end()) {
			std::cerr << "source shader not found in BSP" << std::endl;
			return 1;
		}
		
		if (idx == -1) {
			
			shiter->path = dst;
			
		} else {
		
			int32_t shsrc = std::distance(shaders->begin(), shiter);
			int32_t shdst = 0;
			
			shiter = std::find_if(shaders->begin(), shaders->end(), [&](BSPI::Shader const & v){ return dst == v.path; });
			if (shiter == shaders->end()) {
				shdst = shaders->size();
				shaders->emplace_back( BSPI::Shader { dst, shaders->at(shsrc).surface_flags, shaders->at(shsrc).content_flags } );
			}
			else
				shdst = std::distance(shaders->begin(), shiter);
			
			std::cout << "Source Shader Index: " << shsrc << ", Dest Shader Index: " << shdst << std::endl;
			
			std::vector<BSP::Surface *> matchsurfs;
			for (BSP::Surface & surf : *surfaces)
				if (surf.shader == shsrc) matchsurfs.emplace_back(&surf);
				
			if (idx >= (int32_t)matchsurfs.size()) {
				std::cerr << "remap index greater than match surfaces" << std::endl;
				return 1;
			}
			
			matchsurfs[idx]->shader = shdst;
		
		}
		
		auto bytes = bspa.assemble();
		
		std::ofstream fout { output_path, std::ios_base::binary | std::ios_base::out };
		if (!fout.good()) {
			std::cerr << "failed to open output for writing" << std::endl;
			return 1;
		}
		fout.write( reinterpret_cast<char const *>(bytes.data()), bytes.size());
		fout.close();
	}
	
	return 0;
}
