#include "RenderViewUI.hpp"

#include "Core/EngineLogger.hpp"
#include "Core/DMemory.hpp"
#include "Core/Event.hpp"
#include "Math/DMath.hpp"
#include "Math/Transform.hpp"
#include "Containers/TArray.hpp"
#include "Systems/MaterialSystem.h"
#include "Systems/ShaderSystem.h"
#include "Renderer/RendererFrontend.hpp"
#include "Renderer/Interface/IRenderpass.hpp"
#include "Renderer/Interface/IRendererBackend.hpp"
#include "Resources/UIText.hpp"

void RenderViewUI::OnCreate() {
	ShaderID = ShaderSystem::GetID(CustomShaderName ? CustomShaderName : "Shader.Builtin.UI");
	UsedShader = ShaderSystem::GetByID(ShaderID);
	DiffuseMapLocation = ShaderSystem::GetUniformIndex(UsedShader, "diffuse_texture");
	DiffuseColorLocation = ShaderSystem::GetUniformIndex(UsedShader, "diffuse_color");
	ModelLocation = ShaderSystem::GetUniformIndex(UsedShader, "model");

	// TODO: Set from configurable.
	NearClip = -100.0f;
	FarClip = 100.0f;

	// Default
	ProjectionMatrix = Matrix4::Matrix4::Orthographic(0, 1280.0f, 720.0f, 0.0f, NearClip, FarClip, true);
	ViewMatrix = Matrix4::Identity();
}

void RenderViewUI::OnDestroy() {

}

void RenderViewUI::OnResize(uint32_t width, uint32_t height) {
	// Check if different. If so, regenerate projection matrix.
	if (width == Width && height == Height) {
		return;
	}

	Width = width;
	Height = height;
	ProjectionMatrix = Matrix4::Orthographic(0.0f, (float)Width, (float)Height, 0.0f, NearClip, FarClip, true);

	for (uint32_t i = 0; i < RenderpassCount; ++i) {
		Passes[i]->SetRenderArea(Vec4(0, 0, (float)Width, (float)Height));
	}
}

bool RenderViewUI::OnBuildPacket(void* data, struct RenderViewPacket* out_packet) const{
	if (data == nullptr || out_packet == nullptr) {
		LOG_WARN("RenderViewUI::OnBuildPacke() Requires valid pointer to packet and data.");
		return false;
	}

	UIPacketData* PacketData = (UIPacketData*)data;
	out_packet->view = this;

	// Set matrix, etc.
	out_packet->projection_matrix = ProjectionMatrix;
	out_packet->view_matrix = ViewMatrix;

	// TODO: Temp set extended data to the test text objects for now.
	out_packet->extended_data = data;

	// Obtain all geometries from the current scene.
	// Iterate all meshes and them to the packet's geometries collection.
	for (uint32_t i = 0; i < PacketData->meshData.mesh_count; ++i) {
		Mesh* pMesh = PacketData->meshData.meshes[i];
		for (uint32_t j = 0; j < pMesh->geometry_count; j++) {
			GeometryRenderData RenderData;
			RenderData.geometry = pMesh->geometries[j];
			RenderData.model = pMesh->Transform.GetWorldTransform();
			out_packet->geometries.push_back(RenderData);
			out_packet->geometry_count++;
		}
	}

	return true;
}

void RenderViewUI::OnDestroyPacket(struct RenderViewPacket* packet) const {
	// No much to do here, just zero mem.
	packet->geometries.clear();
	std::vector<GeometryRenderData>().swap(packet->geometries);

	Memory::Zero(packet, sizeof(RenderViewPacket));
}

bool RenderViewUI::OnRender(struct RenderViewPacket* packet, IRendererBackend* back_renderer, size_t frame_number, size_t render_target_index) const {
	uint32_t SID = ShaderID;
	for (uint32_t p = 0; p < RenderpassCount; ++p) {
		IRenderpass* Pass = Passes[p];
		Pass->Begin(&Pass->Targets[render_target_index]);

		if (!ShaderSystem::UseByID(SID)) {
			LOG_ERROR("RenderViewUI::OnRender() Failed to use material shader. Render frame failed.");
			return false;
		}

		// Apply globals.
		if (!MaterialSystem::ApplyGlobal(SID, frame_number, packet->projection_matrix, packet->view_matrix, Vec4(0), Vec3(0))) {
			LOG_ERROR("RenderViewUI::OnRender() Failed to use global shader. Render frame failed.");
			return false;
		}

		// Draw geometries.
		uint32_t Count = packet->geometry_count;
		for (uint32_t i = 0; i < Count; ++i) {
			Material* Mat = nullptr;
			if (packet->geometries[i].geometry->Material) {
				Mat = packet->geometries[i].geometry->Material;
			}
			else {
				Mat = MaterialSystem::GetDefaultMaterial();
			}

			bool IsNeedUpdate = Mat->RenderFrameNumer != frame_number;
			if (!MaterialSystem::ApplyInstance(Mat, IsNeedUpdate)) {
				LOG_WARN("Failed to apply material '%s'. Skipping draw.", Mat->Name);
				continue;
			}
			else {
				// Sync the frame number.
				Mat->RenderFrameNumer = (uint32_t)frame_number;
			}

			// Apply local
			MaterialSystem::ApplyLocal(Mat, packet->geometries[i].model);

			// Draw
			back_renderer->DrawGeometry(&packet->geometries[i]);
		}

		// Draw bitmap text.
		UIPacketData* PacketData = (UIPacketData*)packet->extended_data;
		for (uint32_t i = 0; i < PacketData->textCount; ++i) {
			UIText* Text = PacketData->Textes[i];
			ShaderSystem::BindInstance(Text->InstanceID);

			if (!ShaderSystem::SetUniformByIndex(DiffuseMapLocation, &Text->Data->atlas)) {
				LOG_ERROR("Failed to apply bitmap font diffuse map uniform.");
				return false;
			}

			// TODO: font color
			static Vec4 WhiteColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
			if (!ShaderSystem::SetUniformByIndex(DiffuseColorLocation, &WhiteColor)) {
				LOG_ERROR("Failed to apply bitmap font diffuse color uniform.");
				return false;
			}

			bool NeedUpdate = Text->RenderFrameNumber != frame_number;
			ShaderSystem::ApplyInstance(NeedUpdate);

			// Sync frame number.
			Text->RenderFrameNumber = frame_number;

			// Apply the locals.
			Matrix4 Model = Text->Trans.GetWorldTransform();
			if (!ShaderSystem::SetUniformByIndex(ModelLocation, &Model)) {
				LOG_ERROR("Failde to apply model matrix for text.");
			}

			Text->Draw();
		}

		Pass->End();
	}

	return true;
}
