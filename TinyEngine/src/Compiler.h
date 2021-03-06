// https://github.com/google/shaderc/blob/master/examples/online-compile/main.cc

#ifndef COMPILER_H_
#define COMPILER_H_

#include <map>
#include <string>

#include <shaderc/shaderc.hpp>

class Compiler
{
public:
	static bool buildShader(std::vector<uint32_t>& spirv, const std::string& source, const std::map<std::string, std::string>& macros, shaderc_shader_kind shaderKind, shaderc_optimization_level optimizationLevel = shaderc_optimization_level_zero);
};

#endif /* COMPILER_H_ */
