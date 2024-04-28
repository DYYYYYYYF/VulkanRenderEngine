#pragma once

#include "Defines.hpp"
#include "Math/MathTypes.hpp"

enum RendererBackendType {
	eRenderer_Backend_Type_Vulkan,
	eRenderer_Backend_Type_OpenGL,
	eRenderer_Backend_Type_DirecX
};

struct SRenderPacket {
	double delta_time;
};

// Uniform Buffer Object
struct SGlobalUBO {
	Matrix4 projection;	// 64 bytes
	Matrix4 view;		// 64 bytes
	Matrix4 reserved0;	// 64 bytes, reserved for future use
	Matrix4 reserved1;	// 64 bytes, reserved for future use
};

// Object Material
struct ObjectUniformObject {
	Vec4 diffuse_color;	// 16 Bytes
	Vec4 v_reserved0;	// 16 Bytes,reserved for future use
	Vec4 v_reserved1;	// 16 Bytes,reserved for future use
	Vec4 v_reserved2;	// 16 Bytes,reserved for future use
};

struct GeometryRenderData {
	uint32_t object_id;
	Matrix4 model;
	class Texture* textures[16];
};