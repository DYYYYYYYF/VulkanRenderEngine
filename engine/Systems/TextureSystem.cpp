#include "TextureSystem.h"

#include "Core/EngineLogger.hpp"
#include "Core/Application.hpp"

// TODO: resource loader.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Renderer/RendererFrontend.hpp"

STextureSystemConfig TextureSystem::TextureSystemConfig;
Texture TextureSystem::DefaultTexture;
Texture* TextureSystem::RegisteredTextures = nullptr;
STextureReference* TextureSystem::TableMemory = nullptr;
HashTable TextureSystem::RegisteredTextureTable;
bool TextureSystem::Initilized = false;
IRenderer* TextureSystem::Renderer = nullptr;

bool TextureSystem::Initialize(IRenderer* renderer, STextureSystemConfig config) {
	if (config.max_texture_count == 0) {
		UL_FATAL("Texture system init failed. TextureSystemConfig.max_texture_count should > 0");
		return false;
	}

	if (renderer == nullptr) {
		UL_FATAL("Texture system init failed. Renderer is nullptr.");
		return false;
	}

	if (Initilized) {
		return true;
	}

	TextureSystemConfig = config;
	Renderer = renderer;

	// Block of memory will block for array, then block for hashtable.
	size_t ArraryRequirement = sizeof(Texture) * TextureSystemConfig.max_texture_count;
	size_t HashtableRequirement = sizeof(STextureReference) * TextureSystemConfig.max_texture_count;

	// The array block is after the state. Already allocated, so just set the pointer.
	RegisteredTextures = (Texture*)Memory::Allocate(ArraryRequirement, MemoryType::eMemory_Type_Array);

	// Hashtable block is after array.
	void* HashtableBlock = (size_t*)ArraryRequirement;

	// Create a hashtable for texture lookups.
	TableMemory = (STextureReference*)Memory::Allocate(HashtableRequirement, MemoryType::eMemory_Type_Array);
	RegisteredTextureTable.Create(sizeof(STextureReference), TextureSystemConfig.max_texture_count, TableMemory, false);

	// Fill the hashtable with invalid references to use as a default.
	STextureReference InvalidRef;
	InvalidRef.auto_release = false;
	InvalidRef.handle = INVALID_ID;
	InvalidRef.reference_count = 0;
	RegisteredTextureTable.Fill(&InvalidRef);

	// Invalidate all textures in the array.
	uint32_t Count = TextureSystemConfig.max_texture_count;
	for (uint32_t i = 0; i < Count; ++i) {
		RegisteredTextures[i].Id = INVALID_ID;
		RegisteredTextures[i].Generation = INVALID_ID;
	}

	// Create default textures for use in the system.
	CreateDefaultTexture();

	Initilized = true;
	return true;
}

void TextureSystem::Shutdown() {
	// Destroy all loaded textures.
	for (uint32_t i = 0; i < TextureSystemConfig.max_texture_count; ++i) {
		Texture* t = &RegisteredTextures[i];
		if (t->Generation != INVALID_ID) {
			Renderer->DestroyTexture(t);
		}
	}

	DestroyDefaultTexture();
}

Texture* TextureSystem::Acquire(const char* name, bool auto_release) {
	// Return default texture, but warn about it since this should be returned via GetDefaultTexture()
	if (strcmp(name, DEFAULT_TEXTURE_NAME) == 0) {
		UL_WARN("Texture acquire return default texture. Use GetDefaultTexture() for texture 'default'");
		return &DefaultTexture;
	}

	STextureReference Ref;
	if (RegisteredTextureTable.Get(name, &Ref)) {
		// This can only be changed the first time a texture is loaded.
		if (Ref.reference_count == 0) {
			Ref.auto_release = auto_release;
		}

		Ref.reference_count++;
		if (Ref.handle == INVALID_ID) {
			// This mean no texture exists here. Fina a free index first.
			uint32_t Count = TextureSystemConfig.max_texture_count;
			Texture* t = nullptr;
			for (uint32_t i = 0; i < Count; ++i) {
				if (RegisteredTextures[i].Id == INVALID_ID) {
					// A free slot has been found. Use it index as the handle.
					Ref.handle = i;
					t = &RegisteredTextures[i];
				}
				break;
			}

			// Make sure an empty slot was actually found.
			if (t == nullptr || Ref.handle == INVALID_ID) {
				UL_FATAL("Texture acquire failed. Texture system cannot hold anymore textures. Adjust configuration to allow more.");
				return nullptr;
			}

			// Create new texture.
			if (!LoadTexture(name, t)) {
				UL_ERROR("Load %s texture failed.", name);
				return nullptr;
			}

			// Also use the handle as the texture id.
			t->Id = Ref.handle;
			UL_INFO("Texture '%s' does not yet exist. Created and RefCount is now %i.", name, Ref.reference_count);
		}
		else {
			UL_INFO("Texture '%s' already exist. RefCount increased to %i.", name, Ref.reference_count);
		}

		// Update the entry.
		RegisteredTextureTable.Set(name, &Ref);
		return &RegisteredTextures[Ref.handle];
	}

	// NOTO: This can only happen in the event something went wrong with the state.
	UL_ERROR("Texture acquire failed to acquire texture % s, nullptr will be returned.", name);
	return nullptr;
}

void TextureSystem::Release(const char* name) {
	// Ignore release requests for the default texture.
	if (strcmp(name, DEFAULT_TEXTURE_NAME)) {
		return;
	}

	STextureReference Ref;
	if (RegisteredTextureTable.Get(name, &Ref)) {
		if (Ref.reference_count == 0) {
			UL_WARN("Tried to release non-existent texture: %s", name);
			return;
		}

		Ref.reference_count--;
		if (Ref.reference_count == 0 && Ref.auto_release) {
			Texture* t = &RegisteredTextures[Ref.handle];

			// Release texture.
			Renderer->DestroyTexture(t);

			// Reset the array entry, ensure invalid ids are set.
			Memory::Zero(t, sizeof(Texture));
			t->Id = INVALID_ID;
			t->Generation = INVALID_ID;

			// Reset the reference.
			Ref.handle = INVALID_ID;
			Ref.auto_release = false;
			UL_INFO("Released texture '%s'. Texture unloaded.", name);
		}
		else {
			UL_INFO("Release texture '%s'. Now has a reference count of '%i' (auto release = %s)", name,
				Ref.reference_count, Ref.auto_release ? "True" : "False");
		}

		// Update the entry.
		RegisteredTextureTable.Set(name, &Ref);
	}
	else {
		UL_ERROR("Texture release failed to release texture '%s'.", name);
	}
}

Texture* TextureSystem::GetDefaultTexture() {
	if (Initilized) {
		return &DefaultTexture;
	}
	
	return nullptr;
}

bool TextureSystem::CreateDefaultTexture() {

	// NOTE: create default texture, a 256x256 blue/white checkerboard pattern.
	// This is done in code to eliminate asset dependencies.
	UL_INFO("Createing default texture...");
	const uint32_t TexDimension = 256;
	const uint32_t bpp = 4;
	const uint32_t PixelCount = TexDimension * TexDimension;
	unsigned char Pixels[PixelCount * bpp];

	Memory::Set(Pixels, 255, sizeof(unsigned char) * PixelCount * bpp);

	// Each pixel.
	for (size_t row = 0; row < TexDimension; row++) {
		for (size_t col = 0; col < TexDimension; col++) {
			size_t Index = (row * TexDimension) + col;
			size_t IndexBpp = Index * bpp;
			if (row % 2) {
				if (col % 2) {
					Pixels[IndexBpp + 0] = 0;
					Pixels[IndexBpp + 1] = 0;
				}
			}
			else {
				if (!(col % 2)) {
					Pixels[IndexBpp + 0] = 0;
					Pixels[IndexBpp + 1] = 0;
				}
			}
		}
	}

	Renderer->CreateTexture(DEFAULT_TEXTURE_NAME, TexDimension, TexDimension, 4, Pixels, false, &DefaultTexture);
	UL_INFO("Default texture created.");

	// Manually set the texture generation to invalid since this is a default texture.
	DefaultTexture.Generation = INVALID_ID;

	return true;
}

void TextureSystem::DestroyDefaultTexture() {
	Renderer->DestroyTexture(&DefaultTexture);
}


bool TextureSystem::LoadTexture(const char* name, Texture* texture) {
	// TODO: Should be able to be located anywhere.
	char* FormatStr = "../Asset/Textures/%s.%s";
	const int RequiredChannelCount = 4;
	stbi_set_flip_vertically_on_load(true);
	char FullFilePath[512];

	// TODO: Try different extensions.
	sprintf_s(FullFilePath, FormatStr, name, "png");

	// Use a temporary texture to load into.
	Texture TempTexture;

	unsigned char* data = stbi_load(FullFilePath, (int*)&TempTexture.Width, (int*)&TempTexture.Height,
		(int*)&TempTexture.ChannelCount, RequiredChannelCount);

	TempTexture.ChannelCount = RequiredChannelCount;

	if (data != nullptr) {
		uint32_t CurrentGeneration = texture->Generation;
		texture->Generation = INVALID_ID;

		size_t TotalSize = TempTexture.Width * TempTexture.Height * RequiredChannelCount;
		// Check for transparency.
		bool HasTransparency = false;
		for (size_t i = 0; i < TotalSize; ++i) {
			unsigned char a = data[i + 3];
			if (a < 255) {
				HasTransparency = true;
				break;
			}
		}

		if (stbi_failure_reason() != nullptr) {
			UL_WARN("Load texture failed to load file %s : %s", FullFilePath, stbi_failure_reason());
		}

		//Acquire internal texture resources and upload to GPU.
		Renderer->CreateTexture(name, TempTexture.Width, TempTexture.Height, TempTexture.ChannelCount, data, HasTransparency, &TempTexture);

		// Take a copy of the old texture.
		Texture Old = *texture;

		// Assign the temp texture to the pointer.
		*texture = TempTexture;

		// Destroy the old texture.
		Renderer->DestroyTexture(&Old);

		if (CurrentGeneration == INVALID_ID) {
			texture->Generation = 0;
		}
		else {
			texture->Generation = CurrentGeneration + 1;
		}

		// Clean up data.
		stbi_image_free(data);
		return true;
	}
	else {
		if (stbi_failure_reason() != nullptr) {
			UL_WARN("Load texture failed to load file %s : %s", FullFilePath, stbi_failure_reason());
		}

		return false;
	}
}
