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
#include <string>
#include <unordered_map>

int main(int argc, char * * argv) {
	
	argagg::parser argp {{
		{ "help",     { "-h", "--help" }, "Show this help message", 0 },
		{ "info",     { "-i", "--info" }, "Print general info", 0 },
		{ "ents",     { "-e", "--ents" }, "Print the entity string", 0 },
		{ "shaders",  { "-s", "--shaders" }, "Print the shaders used", 0 },
		{ "shaders+", { "-S", "--shaders-extra" }, "Print the shaders used plus extra information", 0 },
	}};
	
	argagg::parser_results args;
	
	try {
		args = argp.parse(argc, argv);
	} catch (std::exception const & e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	
	if (args["help"] || !args.count()) {
		std::cerr << "Usage: bsptool [options] <path to bsp>" << std::endl << argp;
		return 0;
	}
	
	std::string bsp_path = args.as<std::string>(0);
	auto fd = open( bsp_path.c_str(), O_RDONLY );
	
	if (fd == -1) {
		std::cerr << "File not found!" << std::endl;
		return 1;
	}
	
	struct stat sb;
	fstat(fd, &sb);
	
	if (sb.st_size < (ssize_t)sizeof(BSP::Header)) {
		std::cerr << "File too small to be a BSP file!" << std::endl;
		return 1;
	}

	auto ptr = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	
	if (memcmp(ptr, BSP::IDENT, 4)) {
		std::cerr << "File does not appear to be a BSP file!" << std::endl;
		return 1;
	}
	
	BSP::Reader bspr { reinterpret_cast<uint8_t const *> (ptr) };
	
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
	
	if (args["ents"]) {
		std::cout << bspr.entities();
		std::flush(std::cout);
	}
	
	if (args["shaders"]) {
		for (auto const & shad : bspr.shaders()) std::cout << shad.shader << std::endl;
	}
	
	if (args["shaders+"]) {
		
		auto shads = bspr.shaders();
		
		std::unordered_map<int32_t, uint32_t> brush_usage;
		for (auto const & v : bspr.brushes()) brush_usage[v.shader]++;
		
		std::unordered_map<int32_t, uint32_t> brushside_usage;
		for (auto const & v : bspr.brushsides()) brushside_usage[v.shader]++;
		
		std::unordered_map<int32_t, uint32_t> surface_usage;
		for (auto const & v : bspr.surfaces()) surface_usage[v.shader]++;
		
		for (ssize_t i = 0; i < shads.size(); i++) {
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
	
	return 0;
}
