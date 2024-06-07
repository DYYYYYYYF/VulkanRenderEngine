#include "RenderViewSkybox.hpp"

#include "Core/EngineLogger.hpp"
#include "Core/DMemory.hpp"
#include "Core/Event.hpp"
#include "Math/DMath.hpp"
#include "Math/Transform.hpp"
#include "Containers/TArray.hpp"

#include "Systems/MaterialSystem.h"
#include "Systems/ShaderSystem.h"
#include "Systems/CameraSystem.h"

#include "Renderer/RendererFrontend.hpp"
#include "Renderer/Interface/IRenderpass.hpp"
#include "Renderer/Interface/IRendererBackend.hpp"

void RenderViewSkybox::OnCreate() {
	// Get either the custom shader override or the defined default.
	Shader* s = ShaderSystem::Get(CustomShaderName ? CustomShaderName : "Shader.Builtin.Skybox");
	ShaderID = s->ID;
	ProjectionLocation = ShaderSystem::GetUniformIndex(s, "projection");
	ViewLocation = ShaderSystem::GetUniformIndex(s, "view");
	CubeMapLocation = ShaderSystem::GetUniformIndex(s, "cube_texture");
	
	// TODO: Configurable.
	ReserveY = true;

	// TODO: Set from configurable.
	NearClip = 0.1f;
	FarClip = 1000.0f;
	Fov = Deg2Rad(45.0f);

	// Default
	ProjectionMatrix = Matrix4::Perspective(Fov, 1280.0f / 720.0f, NearClip, FarClip, ReserveY);
	WorldCamera = CameraSystem::GetDefault();
}

void RenderViewSkybox::OnDestroy() {

}

void RenderViewSkybox::OnResize(uint32_t width, uint32_t height) {
	// Check if different.
	if (width == Width && height == Height) {
		return;
	}

	Width = width;
	Height = height;
	ProjectionMatrix = Matrix4::Perspective(Fov, (float)Width / (float)Height, NearClip, FarClip, ReserveY);

	for (uint32_t i = 0; i < RenderpassCount; ++i) {
		Passes[i]->SetRenderArea(Vec4(0, 0, (float)Width, (float)Height));
	}
}

bool RenderViewSkybox::OnBuildPacket(void* data, struct RenderViewPacket* out_packet) const {
	if (data == nullptr || out_packet == nullptr) {
		LOG_WARN("RenderViewSkybox::OnBuildPacke() Requires valid pointer to packet and data.");
		return false;
	}

	SkyboxPacketData* SkyboxData = (SkyboxPacketData*)data;
	out_packet->view = this;

	// Set matrix, etc.
	out_packet->projection_matrix = ProjectionMatrix;
	out_packet->view_matrix = WorldCamera->GetViewMatrix();
	out_packet->view_position = WorldCamera->GetPosition();

	// Just set the extended data to the skybox data.
	out_packet->extended_data = SkyboxData;

	return true;
}

void RenderViewSkybox::OnDestroyPacket(struct RenderViewPacket* packet) const {
	// No much to do here, just zero mem.
	Memory::Zero(packet, sizeof(RenderViewPacket));
}

bool RenderViewSkybox::OnRender(struct RenderViewPacket* packet, IRendererBackend* back_renderer, size_t frame_number, size_t render_target_index) const {
	uint32_t SID = ShaderID;
	SkyboxPacketData* SkyboxData = (SkyboxPacketData*)packet->extended_data;
	for (uint32_t p = 0; p < RenderpassCount; ++p) {
		IRenderpass* Pass = Passes[p];
		Pass->Begin(&Pass->Targets[render_target_index]);

		if (!ShaderSystem::UseByID(SID)) {
			LOG_ERROR("RenderViewSkybox::OnRender() Failed to use material shader. Render frame failed.");
			return false;
		}

		// Get the view matrix, but zero out the position so the skybox stays put on screen.
		Matrix4 ViewMatrix = WorldCamera->GetViewMatrix();
		ViewMatrix.data[12] = 0.0f;
		ViewMatrix.data[13] = 0.0f;
		ViewMatrix.data[14] = 0.0f;

		// Apply globals
		// TODO: This is terrible
		back_renderer->BindGlobalsShader(ShaderSystem::GetByID(SID));
		if (!ShaderSystem::SetUniformByIndex(ProjectionLocation, &packet->projection_matrix)) {
			LOG_ERROR("RenderViewSkybox::OnRender() Failed to apply skybox projection uniform.");
			return false;
		}

		if (!ShaderSystem::SetUniformByIndex(ViewLocation, &ViewMatrix)) {
			LOG_ERROR("RenderViewSkybox::OnRender() Failed to apply skybox view uniform.");
			return false;
		}

		ShaderSystem::ApplyGlobal();

		// Instance.
		ShaderSystem::BindInstance(SkyboxData->sb->InstanceID);
		if (!ShaderSystem::SetUniformByIndex(CubeMapLocation, &SkyboxData->sb->Cubemap)) {
			LOG_ERROR("RenderViewSkybox::OnRender() Failed to apply cube map uniform.");
			return false;
		}

		bool NeedsUpdate = SkyboxData->sb->RenderFrameNumber != frame_number;
		ShaderSystem::ApplyInstance(NeedsUpdate);

		// Sync the frame num.
		SkyboxData->sb->RenderFrameNumber = frame_number;

		// Draw
		GeometryRenderData RenderData;
		RenderData.geometry = SkyboxData->sb->g;
		back_renderer->DrawGeometry(&RenderData);

		Pass->End();
	}

	return true;
}
