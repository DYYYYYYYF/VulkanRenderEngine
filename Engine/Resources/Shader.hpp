#pragma once

#define GlobalShaderType ShaderType::eGLSL

#include "Resource.hpp"
#include "Math/MathTypes.hpp"
#include "Containers/THashTable.hpp"
#include <unordered_map>

// Shader compiler
#include <shaderc/shaderc.hpp>

struct TextureMap;

enum class ShaderType {
	eHLSL,
	eGLSL
};

enum ShaderRenderMode {
	eShader_Render_Mode_Default,
	eShader_Render_Mode_Lighting,
	eShader_Render_Mode_Normals,
	eShader_Render_Mode_Depth
};

enum ShaderState {
	eShader_State_Not_Created,
	eShader_State_Uninitialized,
	eShader_State_Initialized,
	eShader_State_Reloading
};

enum ShaderStage {
	eShader_Stage_Vertex = 0x00000001,
	eShader_Stage_Geometry = 0x00000002,
	eShader_Stage_Fragment = 0x00000004,
	eShader_Stage_Compute = 0x00000008
};

enum ShaderAttributeType {
	eShader_Attribute_Type_Float	= 0U,
	eShader_Attribute_Type_Float_2	= 1U,
	eShader_Attribute_Type_Float_3	= 2U,
	eShader_Attribute_Type_Float_4	= 3U,
	eShader_Attribute_Type_Matrix	= 4U,
	eShader_Attribute_Type_Int8		= 5U,
	eShader_Attribute_Type_UInt8	= 6U,
	eShader_Attribute_Type_Int16	= 7U,
	eShader_Attribute_Type_UInt16	= 8U,
	eShader_Attribute_Type_Int32	= 9U,
	eShader_Attribute_Type_UInt32	= 10U
};

enum ShaderUniformType {
	eShader_Uniform_Type_Float		= 0U,
	eShader_Uniform_Type_Float_2	= 1U,
	eShader_Uniform_Type_Float_3	= 2U,
	eShader_Uniform_Type_Float_4	= 3U,
	eShader_Uniform_Type_Matrix		= 4U,
	eShader_Uniform_Type_Int8		= 5U,
	eShader_Uniform_Type_UInt8		= 6U,
	eShader_Uniform_Type_Int16		= 7U,
	eShader_Uniform_Type_UInt16		= 8U,
	eShader_Uniform_Type_Int32		= 9U,
	eShader_Uniform_Type_UInt32		= 10U,
	eShader_Uniform_Type_Sampler	= 11U,
	eShader_Uniform_Type_Custom		= 12U
};

enum FaceCullMode {
	eFace_Cull_Mode_None = 0x0,
	eFace_Cull_Mode_Front = 0x1,
	eFace_Cull_Mode_Back = 0x2,
	eFace_Cull_Mode_Front_And_Back = 0x3,
};

enum PolygonMode {
	ePology_Mode_Fill = 0x0,
	ePology_Mode_Line = 0x1,
};

enum ShaderFlags {
	eShader_Flag_None = 0x0,
	eShader_Flag_DepthTest = 0x1,
	eShader_Flag_DepthWrite = 0x2,
};

typedef unsigned int ShaderFlagBits;

struct MaterialShaderUniformLocations {
	unsigned short projection = INVALID_ID_U16;
	unsigned short view = INVALID_ID_U16;
	unsigned short ambient_color = INVALID_ID_U16;
	unsigned short diffuse_color = INVALID_ID_U16;
	unsigned short diffuse_texture = INVALID_ID_U16;
	unsigned short normal_texture = INVALID_ID_U16;
	unsigned short specular_texture = INVALID_ID_U16;
	unsigned short shininess = INVALID_ID_U16;
	unsigned short view_position = INVALID_ID_U16;
	unsigned short model = INVALID_ID_U16;
	unsigned short render_mode = INVALID_ID_U16;
	unsigned short metallic = INVALID_ID_U16;
	unsigned short roughness = INVALID_ID_U16;
	unsigned short ambient_occlusion = INVALID_ID_U16;
};

struct UIShaderUniformLocations {
	unsigned short projection = INVALID_ID_U16;
	unsigned short view = INVALID_ID_U16;
	unsigned short diffuse_color = INVALID_ID_U16;
	unsigned short diffuse_texture = INVALID_ID_U16;
	unsigned short model = INVALID_ID_U16;
};

/**
 * @brief Defines shader scope, which indicates how
 * often it gets updated
 */
enum ShaderScope {
	eShader_Scope_Global = 0,
	eShader_Scope_Instance = 1,
	eShader_Scope_Local = 2
};

struct ShaderUniform {
	size_t offset;
	unsigned short location;
	unsigned short index;
	unsigned short size;
	unsigned short set_index;

	ShaderScope scope;
	ShaderUniformType type;
};

struct ShaderAttribute {
	char* name = nullptr;
	ShaderAttributeType type;
	uint32_t size;
};

struct ShaderAttributeConfig {
	unsigned short name_length;
	char* name = nullptr;
	unsigned short size;
	uint32_t location;
	ShaderAttributeType type;
};

struct ShaderUniformConfig {
	unsigned short name_length;
	char* name = nullptr;
	unsigned short size;
	uint32_t location;
	ShaderUniformType type;
	ShaderScope scope;
};

// Uniform Buffer Object
struct MaterialShaderGlobalUbo {
	Matrix4 projection;	// 64 bytes
	Matrix4 view;		// 64 bytes
	Matrix4 reserved0;	// 64 bytes, reserved for future use
	Matrix4 reserved1;	// 64 bytes, reserved for future use
};

// Object Material
struct MaterialShaderInstanceUbo {
	Vec4 diffuse_color;	// 16 Bytes
	Vec4 v_reserved0;	// 16 Bytes,reserved for future use
	Vec4 v_reserved1;	// 16 Bytes,reserved for future use
	Vec4 v_reserved2;	// 16 Bytes,reserved for future use
};

struct ShaderConfig {
	char* name = nullptr;
	FaceCullMode cull_mode;
	PolygonMode polygon_mode;

	std::vector<ShaderAttributeConfig> attributes;
	std::vector<ShaderUniformConfig> uniforms;
	std::vector<ShaderStage> stages;
	std::vector<char*> stage_names;
	std::vector<char*> stage_filenames;

	bool depthTest;
	bool depthWrite;
};

class Shader{
public:
	Shader() {
		ID = INVALID_ID;
		Name = nullptr;
		RenderFrameNumber = INVALID_ID_U64;
		BoundInstanceId = INVALID_ID;
		State = ShaderState::eShader_State_Uninitialized;
		Type = GlobalShaderType;
		Flags = 0;
		AttributeStride = 0;
		PushConstantsRangeCount = 0;
		RenderFrameNumber = 0;
		RequiredUboAlignment = 0;
		GlobalUboSize = 0;
		GlobalUboOffset = 0;
		GlobalUboStride = 0;
		UboSize = 0;
		UboStride = 0;
		PushConstantsSize = 0;
		PushConstantsStride = 0;
		InstanceTextureCount = 0;
		BoundScope = ShaderScope::eShader_Scope_Instance;
		BoundUboOffset = 0;
	}

	virtual ~Shader() {}

public:
	virtual std::vector<uint32_t> CompileShaderFile(const char* filename, shaderc_shader_kind shadercStage, bool writeToDisk = true) = 0;

public:
	ShaderFlagBits Flags;
	uint32_t ID;
	char* Name;
	ShaderType Type;

	size_t RenderFrameNumber;
	size_t RequiredUboAlignment;
	size_t GlobalUboSize;
	size_t GlobalUboOffset;
	size_t GlobalUboStride;
	size_t UboSize;
	size_t UboStride;
	size_t PushConstantsSize;
	size_t PushConstantsStride;
	uint32_t InstanceTextureCount;
	ShaderScope BoundScope;
	uint32_t BoundInstanceId;
	uint32_t BoundUboOffset;
	std::unordered_map<std::string, unsigned short> HashMap;
	ShaderState State;
	unsigned short PushConstantsRangeCount;
	Range PushConstantsRanges[32];
	unsigned short AttributeStride;

	std::vector<ShaderUniform> Uniforms;
	std::vector<ShaderAttribute> Attributes;
	std::vector<TextureMap*> GlobalTextureMaps;

};