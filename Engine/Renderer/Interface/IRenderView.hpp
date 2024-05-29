#pragma once
#include "Math/MathTypes.hpp"

#include <vector>
#include <functional>

enum RenderViewKnownType {
	eRender_View_Known_Type_World = 0x01,
	eRender_View_Known_Type_UI = 0x02
};

enum RenderViewViewMatrixtSource {
	eRender_View_View_Matrix_Source_Scene_Camera = 0x01,
	eRender_View_View_Matrix_Source_UI_Camera = 0x02,
	eRender_View_View_Matrix_Source_Light_Camera = 0x03
};

enum RenderViewProjectionMatrixSource {
	eRender_View_Projection_Matrix_Source_Default_Perspective = 0x01,
	eRender_View_Projection_Matrix_Source_Default_Orthographic = 0x02,
};

struct RenderViewPassConfig {
	const char* name = nullptr;
};

struct RenderViewConfig {
	const char* name = nullptr;
	const char* custom_shader_name = nullptr;
	unsigned short width;
	unsigned short height;
	RenderViewKnownType type;
	RenderViewViewMatrixtSource view_matrix_source;
	RenderViewProjectionMatrixSource projection_matrix_source;
	unsigned char pass_count;
	std::vector<RenderViewPassConfig> passes;
};

struct RenderViewPacket;

class IRenderView {
public:
	virtual void OnCreate() = 0;
	virtual void OnDestroy() = 0;
	virtual void OnResize(uint32_t width, uint32_t height) = 0;
	virtual bool OnBuildPacket(void* data, struct RenderViewPacket* out_packet) const = 0;
	virtual bool OnRender(struct RenderViewPacket* packet, class IRendererBackend* back_renderer, size_t frame_number, size_t render_target_index) const = 0;

public:
	virtual unsigned short GetID() { return ID; }
	virtual void SetID(unsigned short id) { ID = id; }

public:
	unsigned short ID;
	const char* Name = nullptr;
	unsigned short Width;
	unsigned short Height;
	RenderViewKnownType Type;

	unsigned char RenderpassCount;
	std::vector<class IRenderpass*> Passes;

	const char* CustomShaderName = nullptr;
	void* InternalData = nullptr;

	/*
	std::function<bool(class IRenderView* self)> OnCreate;
	std::function<void(class IRenderView* self)> OnDestroy;
	std::function<void(class IRenderView* self, uint32_t width, uint32_t height)> OnResize;
	std::function<bool(class IRenderView* self, void* data, struct RenderViewPacket* out_packet)> OnBuildPacket;
	std::function<bool(class IRenderView* self, const struct RenderViewPacket* out_packet, size_t frame_number, size_t render_target_index)> OnRender;
	*/
};

struct RenderViewPacket {
	const IRenderView* view = nullptr;
	Matrix4 view_matrix;
	Matrix4 projection_matrix;
	Vec3 view_position;
	Vec4 ambient_color;
	uint32_t geometry_count;
	std::vector<struct GeometryRenderData> geometries;
	const char* custom_shader_name = nullptr;
	void* extended_data = nullptr;
};
