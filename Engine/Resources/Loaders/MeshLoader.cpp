﻿#include "MeshLoader.h"

#include "Core/DMemory.hpp"
#include "Core/EngineLogger.hpp"

#include "Containers/TString.hpp"
#include "Platform/FileSystem.hpp"
#include "Systems/ResourceSystem.h"
#include "Systems/GeometrySystem.h"
#include "Math/GeometryUtils.hpp"

#include <vector>
#include <stdio.h>	//sscanf

MeshLoader::MeshLoader() {
	Type = ResourceType::eResource_type_Static_Mesh;
	TypePath = "Models";
}

bool MeshLoader::Load(const std::string& name, void* params, Resource* resource) {
	if (name.length() == 0 || resource == nullptr) {
		return false;
	}

	const char* FormatStr = "%s/%s/%s%s";
	FileHandle f;

#define SUPPORTED_FILETYPE_COUNT 3
	SupportedMeshFileType SupportedFileTypes[SUPPORTED_FILETYPE_COUNT];
	SupportedFileTypes[0] = SupportedMeshFileType{ ".dsm", MeshFileType::eMesh_File_Type_DSM, true };
	SupportedFileTypes[1] = SupportedMeshFileType{ ".obj", MeshFileType::eMesh_File_Type_OBJ, false };
	SupportedFileTypes[2] = SupportedMeshFileType{ ".gltf", MeshFileType::eMesh_File_Type_GLTF, false };

	char FullFilePath[512];
	MeshFileType Type = MeshFileType::eMesh_File_Type_Not_Found;
	// Try each supported extension.
	for (uint32_t i = 0; i < SUPPORTED_FILETYPE_COUNT; ++i) {
		StringFormat(FullFilePath, 512, FormatStr, ResourceSystem::GetRootPath(), TypePath.c_str(), name.c_str(), SupportedFileTypes[i].extension);
		// If the file exists, open it and stop finding.
		if (FileSystemExists(FullFilePath)) {
			if (FileSystemOpen(FullFilePath, FileMode::eFile_Mode_Read, SupportedFileTypes[i].is_binary, &f)) {
				Type = SupportedFileTypes[i].type;
				break;
			}
		}
	}

	if (Type == MeshFileType::eMesh_File_Type_Not_Found) {
		LOG_ERROR("Unable to find mesh of supported type called '%s'.", name.c_str());
		return false;
	}

	resource->FullPath = std::string(FullFilePath);
	resource->Name = std::move(name);

	// The resource data is just an array of configs.
	std::vector<SGeometryConfig> ResourceDatas;
	ResourceDatas.reserve(25968);
	bool Result = false;
	switch (Type) {
	case MeshFileType::eMesh_File_Type_GLTF:
	{
		// Generate the dsm filename.
		char DsmFileName[512];
		StringFormat(DsmFileName, 512, "%s/%s/%s%s", ResourceSystem::GetRootPath(), TypePath.c_str(), name.c_str(), ".dsm");
		Result = ImportGltfFile(FullFilePath, DsmFileName, ResourceDatas);
	}break;
	case MeshFileType::eMesh_File_Type_OBJ:
	{
		// Generate the dsm filename.
		char DsmFileName[512];
		StringFormat(DsmFileName, 512, "%s/%s/%s%s", ResourceSystem::GetRootPath(), TypePath.c_str(), name.c_str(), ".dsm");
		Result = ImportObjFile(&f, DsmFileName, ResourceDatas);
	}break;
	case MeshFileType::eMesh_File_Type_DSM:
		Result = LoadDsmFile(&f, ResourceDatas);
		break;
	case MeshFileType::eMesh_File_Type_Not_Found:
		LOG_ERROR("Unable to find mesh of supported type called '%s'.", name.c_str());
		Result = false;
		break;
	}

	FileSystemClose(&f);

	if (!Result) {
		LOG_ERROR("Failed to process mesh file '%s'.", FullFilePath);
		ResourceDatas.clear();
		resource->Data = nullptr;
		resource->DataSize = 0;
		return false;
	}

	resource->Data = Memory::Allocate(sizeof(SGeometryConfig) * ResourceDatas.size(), MemoryType::eMemory_Type_Array);
	for (size_t i = 0; i < ResourceDatas.size(); ++i) {
		new (static_cast<SGeometryConfig*>(resource->Data) + i) SGeometryConfig(ResourceDatas[i]); // 使用 placement new
	}
	resource->DataSize = sizeof(SGeometryConfig);
	resource->DataCount = ResourceDatas.size();
	std::vector<SGeometryConfig>().swap(ResourceDatas);

	return true;
}

void MeshLoader::Unload(Resource* resource) {
	for (uint32_t i = 0; i < resource->DataCount; ++i) {
		SGeometryConfig* Config = &((SGeometryConfig*)resource->Data)[i];
		GeometrySystem::ConfigDispose(Config);
		//static_cast<SGeometryConfig*>(resource->Data)[i].~SGeometryConfig();
	}

	if (resource->Data) {
		Memory::Free(resource->Data, resource->DataSize * resource->DataCount, MemoryType::eMemory_Type_Array);
		resource->Data = nullptr;
		resource->DataSize = 0;
		resource->DataCount = 0;
		resource->LoaderID = INVALID_ID;
	}

	resource = nullptr;
}

bool MeshLoader::ImportObjFile(FileHandle* obj_file, const char* out_dsm_filename, std::vector<SGeometryConfig>& out_geometries) {
	// Positions
	std::vector<Vector3> Positions;
	// Normals
	std::vector<Vector3> Normals;
	// Texcoords
	std::vector<Vector2f> Texcoords;

	//Groups
	std::vector<MeshGroupData> Groups;

	// Default name is filename.
	char name[512] = "";

	char MaterialFileName[512] = "";
	unsigned short CurrentMatNameCount = 0;
	char MaterialNames[32][64] = {0};

	char LineBuf[512] = "";
	char* p = &LineBuf[0];
	size_t LineLength = 0;

	// index 0 is previous, 1 is previous before that.
	char PrevFirstChars[2] = { '\0', '\0'};
	while (true) {
		if (!FileSystemReadLine(obj_file, 511, &p, &LineLength)) {
			break;
		}

		// Skip blank lines.
		if (LineLength < 1) {
			continue;
		}

		char FirstChar = LineBuf[0];

		switch (FirstChar)
		{
		case '#':
			continue;
		case 'v':
		{
			char SecondChar = LineBuf[1];
			switch (SecondChar)
			{
			case ' ':{
				// Vertex position
				Vector3 Pos;
				char t[2];
				int Result = sscanf(LineBuf, "%s %f %f %f", t, &Pos.x, &Pos.y, &Pos.z);
				if (Result == -1) {
					LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
				}

				Positions.push_back(Pos);
			}break;
			case 'n': {
				// Vertex normal
				Vector3 Norm;
				char t[3];
				int Result = sscanf(LineBuf, "%s %f %f %f", t, &Norm.x, &Norm.y, &Norm.z);
				if (Result == -1) {
					LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
				}

				Normals.push_back(Norm);
			}break;
			case 't': {
				// Vertex texcoords
				Vector2f Texcoord;
				char t[3];
				int Result = sscanf(LineBuf, "%s %f %f", t, &Texcoord.x, &Texcoord.y);
				if (Result == -1) {
					LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
				}

				Texcoords.push_back(Texcoord);
			}break;
			}
		}break;
		case 's': {

		}break;
		case 'f': {
			// face
			// f 1/2/3
			// f 1/1/1 2/2/2 3/3/3 = pos/tex/norm pos/tex/norm pos/tex/norm
			MeshFaceData Face;
			char t[3];

			size_t NormalCount = Normals.size();
			size_t TexcoordCount = Texcoords.size();
			if (NormalCount == 0 && TexcoordCount == 0) {
				int Result = sscanf(LineBuf, "%s %d %d %d", 
					t, 
					&Face.vertices[0].position_index, 
					&Face.vertices[1].position_index,
					&Face.vertices[2].position_index);
				if (Result == -1) {
					LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
				}
			}
			else if (NormalCount == 0) {
				int Result = sscanf(LineBuf, "%s %d/%d %d/%d %d/%d",
					t,
					&Face.vertices[0].position_index,
					&Face.vertices[0].texcoord_index,
					&Face.vertices[1].position_index,
					&Face.vertices[1].texcoord_index,
					&Face.vertices[2].position_index,
					&Face.vertices[2].texcoord_index);
				if (Result == -1) {
					LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
				}
			}
			else if (TexcoordCount == 0) {
				int Result = sscanf(LineBuf, "%s %d/%*d/%d %d/%*d/%d %d/%*d/%d",
					t,
					&Face.vertices[0].position_index,
					&Face.vertices[0].normal_index,
					&Face.vertices[1].position_index,
					&Face.vertices[1].normal_index,
					&Face.vertices[2].position_index,
					&Face.vertices[2].normal_index);
				if (Result == -1) {
					LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
				}
			}
			else {
				int Result = sscanf(LineBuf, "%s %d/%d/%d %d/%d/%d %d/%d/%d",
					t, 
					&Face.vertices[0].position_index, 
					&Face.vertices[0].texcoord_index, 
					&Face.vertices[0].normal_index,

					&Face.vertices[1].position_index, 
					&Face.vertices[1].texcoord_index, 
					&Face.vertices[1].normal_index,

					&Face.vertices[2].position_index, 
					&Face.vertices[2].texcoord_index, 
					&Face.vertices[2].normal_index);
				if (Result == -1) {
					LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
				}
			}

			// 0 - max_position_count
			Face.vertices[0].position_index -= 1;
			Face.vertices[0].texcoord_index -= 1;
			Face.vertices[0].normal_index -= 1;
			Face.vertices[1].position_index -= 1;
			Face.vertices[1].texcoord_index -= 1;
			Face.vertices[1].normal_index -= 1;
			Face.vertices[2].position_index -= 1;
			Face.vertices[2].texcoord_index -= 1;
			Face.vertices[2].normal_index -= 1;

			size_t GroupSize = Groups.size();
			if (GroupSize == 0) {
				Groups.resize(1);
			}
			size_t GroupIndex = Groups.size() - 1;
			Groups[GroupIndex].Faces.push_back(Face);
		}break;
		case 'm': {
			// Material library file.
			char SubStr[8];
			int Result = sscanf(LineBuf, "%s %s", SubStr, MaterialFileName);
			if (Result == -1) {
				LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
			}

			// If found, save off the material file name.
			if (StringNequali(SubStr, "mtllib", 6)) {
				// TODO verification
			}
		}break;
		case 'u': {
			// Any time there is a usemtl, assume a new group.
			// New named group or smoothing group, all faces coming after should be added to it.
			/*MeshGroupData NewGroup;
			NewGroup.Faces.reserve(16384);
			Groups.push_back(NewGroup);*/

			// usemtl
			// Read the material name.
			char t[8];
			int Result = sscanf(LineBuf, "%s %s", t, MaterialNames[CurrentMatNameCount]);
			if (Result == -1) {
				LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
			}

			strncpy(name, MaterialNames[CurrentMatNameCount], 255);
			CurrentMatNameCount++;
		} break;
		case 'g': {
			size_t GroupCount = Groups.size();
			// Process each group as a subobject.
			for (size_t i = 0; i < GroupCount; ++i) {
				SGeometryConfig NewData;
				NewData.name = name;

				if (i > 0) {
					NewData.name += (int)i;
				}
				NewData.material_name = MaterialNames[i];

				ProcessSubobject(Positions, Normals, Texcoords, Groups[i].Faces, &NewData);
				out_geometries.push_back(NewData);

				// Increment the number of objects.
				Groups[i].Faces.clear();
				Memory::Zero(MaterialNames[i], 64);
			}

			CurrentMatNameCount = 0;
			Groups.clear();
			Memory::Zero(name, 512);

			// Read the name.
			char t[3];
			int Result = sscanf(LineBuf, "%s %s", t, name);
			if (Result == -1) {
				LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
			}
		}break;
		}

		PrevFirstChars[1] = PrevFirstChars[0];
		PrevFirstChars[0] = FirstChar;
	}	// Each line

	// Process the remaining group since the last one will not have been trigged
	// by the finding of a new name.
	// Process each group as a subobject.
	size_t GroupCount = Groups.size();
	SGeometryConfig NewData;
	for (size_t i = 0; i < GroupCount; ++i) {
		NewData.name = name;

		if (i > 0) {
			NewData.name += (int)i;
		}
		NewData.material_name = MaterialNames[i];

		ProcessSubobject(Positions, Normals, Texcoords, Groups[i].Faces, &NewData);
		out_geometries.push_back(NewData);

		// Increment the number of objects.
		Groups[i].Faces.clear();
	}

	Groups.clear();
	Positions.clear();
	Normals.clear();
	Texcoords.clear();

	if (strlen(MaterialFileName) > 0) {
		// Load up the material file.
		char FullMtlPath[512];
		Memory::Zero(FullMtlPath, sizeof(char) * 512);
		StringDirectoryFromPath(FullMtlPath, out_dsm_filename);
		String::Append(FullMtlPath, 512, FullMtlPath, MaterialFileName);

		// Process material library file.
		if (!ImportObjMaterialLibraryFile(FullMtlPath)) {
			LOG_ERROR("Error reading obj material file.");
		}
	}

	// De-duplicate geometry.
	DeduplicateGeometry(out_geometries);

	std::vector<Vector3>().swap(Positions);
	std::vector<Vector3>().swap(Normals);
	std::vector<Vector2f>().swap(Texcoords);
	std::vector<MeshGroupData>().swap(Groups);

	// Output a .dsm file, which will be loaded in the future.
	return WriteDsmFile(out_dsm_filename, name, out_geometries);
}

bool MeshLoader::ImportObjMaterialLibraryFile(const char* mtl_file_path) {
	LOG_DEBUG("Importing obj .mtl file '%s'.", mtl_file_path);

	// Grab the .mtl file, if it exists, and read the material information.
	FileHandle MtlFile;
	if (!FileSystemOpen(mtl_file_path, FileMode::eFile_Mode_Read, false, &MtlFile)) {
		LOG_ERROR("Unable to open mtl file: '%s'", mtl_file_path);
		return false;
	}

	SMaterialConfig CurrentConfig;
	bool HitName = false;
	char* Line = nullptr;
	char LineBuf[512];
	char* p = &LineBuf[0];
	size_t LineLength = 0;
	while (true) {
		if (!FileSystemReadLine(&MtlFile, 512, &p, &LineLength)) {
			break;
		}

		// Trim the line first.
		Line = Strtrim(LineBuf);
		LineLength = strlen(Line);

		// Skip blank lines.
		if (LineLength < 1) {
			continue;
		}

		char FirstChar = Line[0];
		switch (FirstChar)
		{
		case '#':
			continue;
		case 'K': {
			char SecondChar = Line[1];
			switch (SecondChar)
			{
			case 'a':
			case 'd':
			{
				// Ambient/Diffuse color are treated the same at this level.
				// ambient color is determined by the level.
				char t[3];
				int Result = sscanf(Line, "%s %f %f %f", t, &CurrentConfig.diffuse_color.r, &CurrentConfig.diffuse_color.g, &CurrentConfig.diffuse_color.b);
				if (Result == -1) {
					LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
				}

				// NOTE: This is only used by the color shader, and will set to max_norm by default.
				// Transparency could be added as a material property all its own at a later time.
				CurrentConfig.diffuse_color.a = 1.0f;
			}break;
			case 's':
			{
				// Specular color
				char t[3];

				// NOTE: Not using this for now.
				float SpecRubbish = 0.0f;
				int Result = sscanf(Line, "%s %f %f %f", t, &SpecRubbish, &SpecRubbish, &SpecRubbish);
				if (Result == -1) {
					LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
				}
			} break;
			}
		}break;
		case 'N':
		{
			char SecondChar = Line[1];
			switch (SecondChar)
			{
			case 's':
			{
				// Specular exponent.
				char t[3];
				int Result = sscanf(Line, "%s %f", t, &CurrentConfig.shininess);
				if (Result == -1) {
					LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
				}
			}break;
			}
		} break;
		case 'm':
		{
			// Map
			char SubStr[10];
			char TextureFileName[512];

			int Result = sscanf(Line, "%s %s", SubStr, TextureFileName);
			if (Result == -1) {
				LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
			}

			if (StringNequali(SubStr, "map_Kd", 6)) {
				// Is a diffuse texture map.
				StringFilenameNoExtensionFromPath(CurrentConfig.diffuse_map_name, TextureFileName);
			}
			else if (StringNequali(SubStr, "map_Ks", 6)) {
				// Is a specular texture map.
				StringFilenameNoExtensionFromPath(CurrentConfig.specular_map_name, TextureFileName);
			}
			else if (StringNequali(SubStr, "map_bump", 8)) {
				// Is a normal texture map.
				StringFilenameNoExtensionFromPath(CurrentConfig.normal_map_name, TextureFileName);
			}
		}break;
		case 'b':
		{
			// Some implementations use 'bump' instead of 'map_bump'.
			char SubStr[10];
			char TextureFileName[512];

			int Result = sscanf(Line, "%s %s", SubStr, TextureFileName);
			if (Result == -1) {
				LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
			}

			if (StringNequali(SubStr, "bump", 4)) {
				// Is a bump(normal) texture map.
				StringFilenameNoExtensionFromPath(CurrentConfig.normal_map_name, TextureFileName);
			}
		} break;
		case 'n':
		{
			// Some implementations use 'bump' instead of 'map_bump'.
			char SubStr[10];
			char MaterialName[512];

			int Result = sscanf(Line, "%s %s", SubStr, MaterialName);
			if (Result == -1) {
				LOG_ERROR("Mesh loader ImportObjFile: sscanf failed. succeed num: %i  Line: %s", Result, LineBuf);
			}

			if (StringNequali(SubStr, "newmtl", 6)) {
				// Is a material name.

				// NOTE: Hardcoding default material shader name because all objects imported this way
				// will be treated same.
				CurrentConfig.shader_name = "Shader.Builtin.World";
				// NOTE:Shininess of 0 will cause problems in the shader. Use default.
				if (CurrentConfig.shininess == 0.0f) {
					CurrentConfig.shininess = 8.0f;
				}

				if (HitName) {
					// Write out a dmt file and move on.
					if (!WriteDmtFile(mtl_file_path, &CurrentConfig)) {
						LOG_ERROR("Unable to write dmt file.");
						return false;
					}

					// Reset material for the next round.
					Memory::Zero(&CurrentConfig, sizeof(SMaterialConfig));
				}

				CurrentConfig.name = std::string(MaterialName);
				HitName = true;
			}
		}
		}
	}	// Each line

	// Write out the remaining dmt file.
	// NOTE: Hardcoding default material shader name because all objects imported this way
	// will be treated same.
	CurrentConfig.shader_name = "Shader.Builtin.World";
	// NOTE:Shininess of 0 will cause problems in the shader. Use default.
	if (CurrentConfig.shininess == 0.0f) {
		CurrentConfig.shininess = 8.0f;
	}

	if (!WriteDmtFile(mtl_file_path, &CurrentConfig)) {
		LOG_ERROR("Unable to write dmt file.");
		return false;
	}

	FileSystemClose(&MtlFile);
	return true;
}

void MeshLoader::ProcessSubobject(std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<Vector2f>& texcoords, std::vector<MeshFaceData>& faces, SGeometryConfig* out_data) {
	std::vector<uint32_t> Indices;
	std::vector<Vertex> Vertices;
	Indices.reserve(65535);
	Vertices.reserve(65535);
	
	bool ExtentSet = false;
	Memory::Zero(&out_data->min_extents, sizeof(Vector3));
	Memory::Zero(&out_data->max_extents, sizeof(Vector3));
	
	size_t FaceCount = faces.size();
	size_t NormalCount = normals.size();
	size_t TexcoordCount = texcoords.size();

	bool SkipNormal = false;
	bool SkipTexcoord = false;

	if (NormalCount == 0) {
		LOG_WARN("No normals are present in this model. Generating normals...");
		SkipNormal = true;
	}

	if (TexcoordCount == 0) {
		LOG_WARN("No tex-coord are present in this model.");
		SkipTexcoord = true;
	}

	for (size_t f = 0; f < FaceCount; f++) {
		// 确保法线存在
		Vector3 DefaultNormal = Vector3(0, 0, 1);
		if (SkipNormal) {
			MeshVertexIndexData IndexData1 = faces[f].vertices[0];
			MeshVertexIndexData IndexData2 = faces[f].vertices[1];
			MeshVertexIndexData IndexData3 = faces[f].vertices[2];

			Vector3 Pos1 = positions[IndexData1.position_index];
			Vector3 Pos2 = positions[IndexData2.position_index];
			Vector3 Pos3 = positions[IndexData3.position_index];

			Vector3 Edge1 = Pos2 - Pos1;
			Vector3 Edge2 = Pos3 - Pos2;

			DefaultNormal = (Edge1.Cross(Edge2)).Normalize();
		}

		// Each vertex
		for (size_t i = 0; i < 3; ++i) {
			MeshVertexIndexData IndexData = faces[f].vertices[i];
			Indices.push_back((uint32_t)(i + (f * 3)));

			Vertex Vert;
			Vector3 Pos = positions[IndexData.position_index];
			Vert.position = Pos;

			// Check extents - min
			if (Pos.x < out_data->min_extents.x || !ExtentSet) {
				out_data->min_extents.x = Pos.x;
			}
			if (Pos.y < out_data->min_extents.y || !ExtentSet) {
				out_data->min_extents.y = Pos.y;
			}
			if (Pos.z < out_data->min_extents.z || !ExtentSet) {
				out_data->min_extents.z = Pos.z;
			}

			// Check extents - max
			if (Pos.x > out_data->max_extents.x || !ExtentSet) {
				out_data->max_extents.x = Pos.x;
			}
			if (Pos.y > out_data->max_extents.y || !ExtentSet) {
				out_data->max_extents.y = Pos.y;
			}
			if (Pos.z > out_data->max_extents.z || !ExtentSet) {
				out_data->max_extents.z = Pos.z;
			}

			ExtentSet = true;

			if (SkipNormal) {
				Vert.normal = DefaultNormal;
			}
			else {
				Vert.normal = normals[IndexData.normal_index];
			}

			if (SkipTexcoord) {
				Vert.texcoord = Vector2f(0, 0);
			}
			else {
				Vert.texcoord = texcoords[IndexData.texcoord_index];
			}

			// TODO: Color
			Vert.color = Vector4(1, 1, 1, 1);
			Vertices.push_back(Vert);
		}
	}

	// Calculate the center based on the extents.
	for (unsigned short i = 0; i < 3; ++i) {
		out_data->center.elements[i] = (out_data->min_extents.elements[i] + out_data->max_extents.elements[i]) / 2.0f;
	}

	out_data->vertex_count = (uint32_t)Vertices.size();
	out_data->vertex_size = sizeof(Vertex);
	out_data->vertices = Memory::Allocate(out_data->vertex_count * out_data->vertex_size, MemoryType::eMemory_Type_Array);
	Memory::Copy(out_data->vertices, Vertices.data(), out_data->vertex_count * out_data->vertex_size);

	out_data->index_count = (uint32_t)Indices.size();
	out_data->index_size = sizeof(uint32_t);
	out_data->indices = Memory::Allocate(out_data->index_count * out_data->index_size, MemoryType::eMemory_Type_Array);
	Memory::Copy(out_data->indices, Indices.data(), out_data->index_count * out_data->index_size);

	std::vector<uint32_t>().swap(Indices);
	std::vector<Vertex>().swap(Vertices);
}	

bool MeshLoader::WriteDmtFile(const char* mtl_file_path, SMaterialConfig* config) {
	// NOTE: The .obj file this came from (and resulting .mtl file) sit in the
	// models directory. This moves up a level and back into the materials folder.
	// TODO: Read from config and get an avsolute path for output.
	const char* FormatStr = "%s../Materials/%s%s";
	FileHandle f;
	char Directory[320];
	StringDirectoryFromPath(Directory, mtl_file_path);

	char FullFilePath[512];
	StringFormat(FullFilePath, 512, FormatStr, Directory, config->name.c_str(), ".dmt");
	if (!FileSystemOpen(FullFilePath, FileMode::eFile_Mode_Write, false, &f)) {
		LOG_ERROR("Error opening material file for writing: '%s'.", FullFilePath);
		return false;
	}
	LOG_DEBUG("Writing .dmt file '%s'.", FullFilePath);

	char LineBuf[512];
	FileSystemWriteLine(&f, "#material file");
	FileSystemWriteLine(&f, "");
	FileSystemWriteLine(&f, "version=0.1");	// TODO: hardcoded version.
	StringFormat(LineBuf, 512, "name=%s", config->name.c_str());
	// BlinnPhong
	FileSystemWriteLine(&f, LineBuf);
	StringFormat(LineBuf, 512, "diffuse_color=%.6f %.6f %.6f %.6f", config->diffuse_color.r, config->diffuse_color.g, config->diffuse_color.b, config->diffuse_color.a);
	FileSystemWriteLine(&f, LineBuf);
	StringFormat(LineBuf, 512, "shininess=%.6f", config->shininess);

	// PBR
	FileSystemWriteLine(&f, LineBuf);
	StringFormat(LineBuf, 512, "metallic=%.6f", config->Metallic);
	FileSystemWriteLine(&f, LineBuf);
	StringFormat(LineBuf, 512, "roughness=%.6f", config->Roughness);
	FileSystemWriteLine(&f, LineBuf);
	StringFormat(LineBuf, 512, "ambient_occlusion=%.6f", config->AmbientOcclusion);
	FileSystemWriteLine(&f, LineBuf);

	// Textures
	if (config->diffuse_map_name[0]) {
		StringFormat(LineBuf, 512, "diffuse_map_name=%s", config->diffuse_map_name);
		FileSystemWriteLine(&f, LineBuf);
	}
	if (config->specular_map_name[0]) {
		StringFormat(LineBuf, 512, "specular_map_name=%s", config->specular_map_name);
		FileSystemWriteLine(&f, LineBuf);
	}
	if (config->normal_map_name[0]) {
		StringFormat(LineBuf, 512, "normal_map_name=%s", config->normal_map_name);
		FileSystemWriteLine(&f, LineBuf);
	}
	if (!config->MetallicRoughnessTexName.empty()) {
		StringFormat(LineBuf, 512, "roughness_metallic_map_name=%s", config->MetallicRoughnessTexName.c_str());
		FileSystemWriteLine(&f, LineBuf);
	}

	StringFormat(LineBuf, 512, "shader=%s", config->shader_name.c_str());
	FileSystemWriteLine(&f, LineBuf);

	FileSystemClose(&f);
	return true;
}

bool MeshLoader::LoadDsmFile(FileHandle* dsm_file, std::vector<SGeometryConfig>& out_geometries) {
	// Version
	size_t BytesRead = 0;
	unsigned short Version = 0;
	FileSystemRead(dsm_file, sizeof(unsigned short), &Version, &BytesRead);

	// Name length
	uint32_t NameLength = 0;
	FileSystemRead(dsm_file, sizeof(uint32_t), &NameLength, &BytesRead);
	// Name + terminator
	char name[256];
	FileSystemRead(dsm_file, sizeof(char) * NameLength, name, &BytesRead);

	//Geometry count.
	uint32_t GeometryCount = 0;
	FileSystemRead(dsm_file, sizeof(uint32_t), &GeometryCount, &BytesRead);

	// Each geometry.
	for (uint32_t i = 0; i < GeometryCount; ++i) {
		SGeometryConfig g;

		// Vertices (size/count/array)
		FileSystemRead(dsm_file, sizeof(uint32_t), &g.vertex_size, &BytesRead);
		FileSystemRead(dsm_file, sizeof(uint32_t), &g.vertex_count, &BytesRead);
		g.vertices = Memory::Allocate(g.vertex_count * g.vertex_size, MemoryType::eMemory_Type_Array);
		FileSystemRead(dsm_file, g.vertex_count * g.vertex_size, g.vertices, &BytesRead);

		// Indices (size/count/array)
		FileSystemRead(dsm_file, sizeof(uint32_t), &g.index_size, &BytesRead);
		FileSystemRead(dsm_file, sizeof(uint32_t), &g.index_count, &BytesRead);
		g.indices = Memory::Allocate(g.index_count * g.index_size, MemoryType::eMemory_Type_Array);
		FileSystemRead(dsm_file, g.index_count * g.index_size, g.indices, &BytesRead);

		// Name
		uint32_t GNameLength = 0;
		FileSystemRead(dsm_file, sizeof(uint32_t), &GNameLength, &BytesRead);
		char* gn = (char*)Memory::Allocate(sizeof(char) * GNameLength, MemoryType::eMemory_Type_String);
		FileSystemRead(dsm_file, sizeof(char) * GNameLength, gn, &BytesRead);
		g.name = std::string(gn);
		Memory::Free(gn, sizeof(char) * GNameLength, MemoryType::eMemory_Type_String);

		// Material name.
		uint32_t MNameLength = 0;
		FileSystemRead(dsm_file, sizeof(uint32_t), &MNameLength, &BytesRead);
		char* mn = (char*)Memory::Allocate(sizeof(char) * MNameLength, MemoryType::eMemory_Type_String);
		FileSystemRead(dsm_file, sizeof(char) * MNameLength, mn, &BytesRead);
		g.material_name = std::string(mn);
		Memory::Free(mn, sizeof(char) * MNameLength, MemoryType::eMemory_Type_String);

		// Center
		FileSystemRead(dsm_file, sizeof(Vector3), &g.center, &BytesRead);

		// Extents (min/max)
		FileSystemRead(dsm_file, sizeof(Vector3), &g.min_extents, &BytesRead);
		FileSystemRead(dsm_file, sizeof(Vector3), &g.max_extents, &BytesRead);

		// Add to the output array.
		out_geometries.push_back(g);
	}

	FileSystemClose(dsm_file);
	return true;
}

bool MeshLoader::WriteDsmFile(const char* path, const char* name, std::vector<SGeometryConfig>& geometries) {
	if (FileSystemExists(path)) {
		LOG_INFO("File '%s' already exists and will be overwritten.", path);
	}

	FileHandle f;
	if (!FileSystemOpen(path, FileMode::eFile_Mode_Write, true, &f)) {
		LOG_INFO("Unable to open file '%s'. Dsm file write failed.", path);
		return false;
	}

	uint32_t geometry_count = (uint32_t)geometries.size();

	// Version.
	size_t Written = 0;
	unsigned short Version = 0x0001U;
	FileSystemWrite(&f, sizeof(unsigned short), &Version, &Written);

	// Name length.
	uint32_t NameLength = (uint32_t)strlen(name) + 1;
	FileSystemWrite(&f, sizeof(uint32_t), &NameLength, &Written);
	// Name + terminator
	FileSystemWrite(&f, sizeof(char) * NameLength, &name, &Written);

	// Geometry count.
	FileSystemWrite(&f, sizeof(uint32_t), &geometry_count, &Written);

	// Each geometry.
	for (uint32_t i = 0; i < geometry_count; ++i) {
		SGeometryConfig* g = &geometries[i];

		// Vertices (size/count/array)
		FileSystemWrite(&f, sizeof(uint32_t), &g->vertex_size, &Written);
		FileSystemWrite(&f, sizeof(uint32_t), &g->vertex_count, &Written);
		FileSystemWrite(&f, g->vertex_size * g->vertex_count, g->vertices, &Written);

		// Indices (size/count/array)
		FileSystemWrite(&f, sizeof(uint32_t), &g->index_size, &Written);
		FileSystemWrite(&f, sizeof(uint32_t), &g->index_count, &Written);
		FileSystemWrite(&f, g->index_size * g->index_count, g->indices, &Written);

		// Name
		uint32_t GNameLength = (uint32_t)g->name.length() + 1;
		FileSystemWrite(&f, sizeof(uint32_t), &GNameLength, &Written);
		FileSystemWrite(&f, sizeof(char) * GNameLength, (void*)g->name.c_str(), &Written);

		// Material Name
		uint32_t MNameLength = (uint32_t)g->material_name.length() + 1;
		FileSystemWrite(&f, sizeof(uint32_t), &MNameLength, &Written);
		FileSystemWrite(&f, sizeof(char) * MNameLength, (void*)g->material_name.c_str(), &Written);

		// Center
		FileSystemWrite(&f, sizeof(Vector3), &g->center, &Written);

		// Extents (min/ max)
		FileSystemWrite(&f, sizeof(Vector3), &g->min_extents, &Written);
		FileSystemWrite(&f, sizeof(Vector3), &g->max_extents, &Written);
	}

	FileSystemClose(&f);
	return true;
}

bool MeshLoader::DeduplicateGeometry(std::vector<SGeometryConfig>& outGeometries) {
	size_t Count = outGeometries.size();
	uint32_t NewVertCount = 0;
	Vertex* UniqueVerts = nullptr;
	for (size_t i = 0; i < Count; ++i) {
		SGeometryConfig* g = &outGeometries[i];
		LOG_DEBUG("Geometry de-duplication process starting on geometry object named '%s'.", g->name.c_str());
		GeometryUtils::DeduplicateVertices(g->vertex_count, (Vertex*)g->vertices, g->index_count, (uint32_t*)g->indices, &NewVertCount, &UniqueVerts);

		// Destroy the old, large array.
		Memory::Free(g->vertices, g->vertex_count * g->vertex_size, MemoryType::eMemory_Type_Array);

		// And replace with the de-duplicated one.k
		g->vertex_count = NewVertCount;
		g->vertices = UniqueVerts;

		// Take a copy of the indices as a normal.
		uint32_t* Indices = (uint32_t*)Memory::Allocate(sizeof(uint32_t) * g->index_count, MemoryType::eMemory_Type_Array);
		Memory::Copy(Indices, g->indices, sizeof(uint32_t) * g->index_count);
		// Destroy.
		Memory::Free(g->indices, sizeof(uint32_t) * g->index_count, MemoryType::eMemory_Type_Array);
		g->indices = Indices;
	}

	return true;
}