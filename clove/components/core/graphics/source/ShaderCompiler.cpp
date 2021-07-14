#include "Clove/Graphics/ShaderCompiler.hpp"

#include "Clove/Graphics/Metal/MetalGlobals.hpp"

#include <Clove/Definitions.hpp>
#include <Clove/Log/Log.hpp>
#include <glslc/file_includer.h>
#include <fstream>
#include <libshaderc_util/file_finder.h>
#include <shaderc/shaderc.hpp>
#include <spirv.hpp>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>
#include <sstream>

namespace garlic::clove::ShaderCompiler {
    namespace {
        /**
         * @brief Keeps a map for shader source strings to provide as included files.
         */
        class EmbeddedSourceIncluder : public shaderc::CompileOptions::IncluderInterface {
            //VARIABLES
        private:
            std::unordered_map<std::string, std::string> includeSources;

            //FUNCTIONS
        public:
            EmbeddedSourceIncluder() = delete;
            EmbeddedSourceIncluder(std::unordered_map<std::string, std::string> includeSources)
                : includeSources{ std::move(includeSources) } {
            }

            EmbeddedSourceIncluder(EmbeddedSourceIncluder const &other)     = default;
            EmbeddedSourceIncluder(EmbeddedSourceIncluder &&other) noexcept = default;

            EmbeddedSourceIncluder &operator=(EmbeddedSourceIncluder const &other) = default;
            EmbeddedSourceIncluder &operator=(EmbeddedSourceIncluder &&other) noexcept = default;

            ~EmbeddedSourceIncluder() override = default;

            shaderc_include_result *GetInclude(const char *requested_source, shaderc_include_type type, const char *requesting_source, size_t include_depth) override {
                auto *result{ new shaderc_include_result };

                if(includeSources.find(requested_source) != includeSources.end()) {
                    std::string &source{ includeSources[requested_source] };

                    result->source_name        = requested_source;
                    result->source_name_length = strlen(requested_source);
                    result->content            = source.c_str();
                    result->content_length     = source.length();
                } else {
                    CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "{0}: Requested source {1} was not found.", CLOVE_FUNCTION_NAME_PRETTY, requested_source);

                    std::string error{ "Source not available in map" };

                    result->content        = error.c_str();
                    result->content_length = error.length();
                }

                return result;
            }

            void ReleaseInclude(shaderc_include_result *data) override {
                delete data;
            }
        };

        shaderc_shader_kind getShadercStage(GhaShader::Stage stage) {
            switch(stage) {
                case GhaShader::Stage::Vertex:
                    return shaderc_vertex_shader;
                case GhaShader::Stage::Pixel:
                    return shaderc_fragment_shader;
                case GhaShader::Stage::Compute:
                    return shaderc_compute_shader;
                default:
                    CLOVE_ASSERT("Unsupported shader stage {0}", CLOVE_FUNCTION_NAME);
                    return shaderc_vertex_shader;
            }
        }

        std::vector<char> readFile(std::filesystem::path const &filePath) {
            //Start at the end so we can get the file size
            std::ifstream file(filePath.c_str(), std::ios::ate | std::ios::binary);

            if(!file.is_open()) {
                CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "{0}: Failed to open file", CLOVE_FUNCTION_NAME);
                return {};
            }

            auto const fileSize{ file.tellg() };
            std::vector<char> buffer(static_cast<size_t>(fileSize));

            file.seekg(0);
            file.read(buffer.data(), static_cast<std::streamsize>(fileSize));

            file.close();

            return buffer;
        }

        Expected<std::vector<uint32_t>, std::runtime_error> compile(std::string_view source, std::unique_ptr<shaderc::CompileOptions::IncluderInterface> includer, std::string_view shaderName, GhaShader::Stage shaderStage) {
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Trace, "Compiling shader {0}...", shaderName);

            shaderc::CompileOptions options{};
            options.SetIncluder(std::move(includer));
#if !CLOVE_DEBUG
            options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
            options.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

            shaderc::Compiler compiler{};
            shaderc::SpvCompilationResult spirvResult{ compiler.CompileGlslToSpv(source.data(), source.size(), getShadercStage(shaderStage), shaderName.data(), options) };
            
            if(spirvResult.GetCompilationStatus() != shaderc_compilation_status_success) {
                CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "Failed to compile shader {0}:\n\t{1}", shaderName, spirvResult.GetErrorMessage());
                return Unexpected{ std::runtime_error{ "Failed to compile shader. See output log for details." } };
            }

            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Debug, "Successfully compiled shader {0}!", shaderName);
            return std::vector<uint32_t>{ spirvResult.begin(), spirvResult.end() };
        }
    }

    Expected<std::vector<uint32_t>, std::runtime_error> compileFromFile(std::filesystem::path const &file, GhaShader::Stage shaderStage) {
        if(!file.has_filename()) {
            return Unexpected{ std::runtime_error{ "Path does not have a file name" } };
        }

        shaderc_util::FileFinder fileFinder{};
        fileFinder.search_path().emplace_back(std::filesystem::path{ file }.remove_filename().string());

        std::vector<char> source{ readFile(file) };
        auto fileIncluder{ std::make_unique<glslc::FileIncluder>(&fileFinder) };
        std::string shaderName{ file.stem().string() };

        return compile({ source.data(), source.size() }, std::move(fileIncluder), shaderName, shaderStage);
    }

    Expected<std::vector<uint32_t>, std::runtime_error> compileFromSource(std::string_view source, std::unordered_map<std::string, std::string> includeSources, std::string_view shaderName, GhaShader::Stage shaderStage) {
        return compile(source, std::make_unique<EmbeddedSourceIncluder>(std::move(includeSources)), shaderName, shaderStage);
    }

    Expected<std::string, std::runtime_error> spirvToHLSL(std::span<uint32_t> spirvSource) {
        return Unexpected{ std::runtime_error{ "SPIR-V to HLSL not supported!" } };
    }

    Expected<std::string, std::runtime_error> spirvToMSL(std::span<uint32_t> spirvSource) {
        spirv_cross::CompilerMSL msl{ spirvSource.data(), spirvSource.size() };
		msl.set_msl_options(spirv_cross::CompilerMSL::Options{
			.platform                  = spirv_cross::CompilerMSL::Options::Platform::macOS,
			.msl_version               = spirv_cross::CompilerMSL::Options::make_msl_version(2, 0),
			.argument_buffers          = true,
			.enable_decoration_binding = true,
		});

		spirv_cross::ShaderResources resources{ msl.get_shader_resources() };
		
		if(!resources.push_constant_buffers.empty()) {
			auto const &resource{ resources.push_constant_buffers.front() };
			spirv_cross::ID const bufferId{ resource.id };
			spirv_cross::TypeID const bufferTypeId{ resource.base_type_id };
			spirv_cross::SmallVector<spirv_cross::BufferRange> const bufferRanges{ msl.get_active_buffer_ranges(bufferId) };
			
			//Remove the padding added onto push constants when using an offset. The offset is not required in metal
			//as pushing small chunks of data works much like uploading a buffer.
			if(!bufferRanges.empty()) {
				size_t const initialOffset{ msl.get_member_decoration(bufferTypeId, bufferRanges.front().index, spv::Decoration::DecorationOffset) };
				for(auto const &range : bufferRanges) {
					//Unset decoration does not seem to work for msl so just manually override the decoration instead.
					msl.set_member_decoration(bufferTypeId, range.index, spv::Decoration::DecorationOffset, range.offset - initialOffset);
				}
			}
			
			//Remap the slot the push constant is in to stop overlap with descriptor sets.
			msl.set_decoration(bufferId, spv::Decoration::DecorationBinding, pushConstantSlot);
		}
		
		
        return msl.compile();
    }
}
