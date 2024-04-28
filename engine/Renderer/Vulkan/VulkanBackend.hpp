#pragma once

#include "Renderer/RendererBackend.hpp"
#include "Core/DMemory.hpp"
#include "VulkanContext.hpp"

class VulkanBuffer;

class VulkanBackend : public IRendererBackend {
public:
	VulkanBackend();
	virtual ~VulkanBackend();

public:
	virtual bool Initialize(const char* application_name, struct SPlatformState* plat_state) override;
	virtual void Shutdown() override;

	virtual bool BeginFrame(double delta_time) override;
	virtual void UpdateGlobalState(Matrix4 projection, Matrix4 view, Vec3 view_position, Vec4 ambient_color, int mode) override;
	virtual void UpdateObject(Matrix4 model_trans) override;
	virtual bool EndFrame(double delta_time) override;


	virtual void Resize(unsigned short width, unsigned short height) override;

	virtual void CreateCommandBuffer();
	virtual void RegenerateFrameBuffers();
	virtual bool RecreateSwapchain();

	virtual bool CreateBuffers();

	virtual void UploadDataRange(VulkanBuffer* buffer, size_t offset, size_t size, void* data);

protected:
	VulkanContext Context;

};