/***************************************************************************
* SUtils.cpp
*/

#include "Core/SUtils.h"
#include "Core/SException.h"
#include "Core/SMath.h"
#include "SConfig.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <ranges>
#include <limits>
#include <png.h>
#include <ufbx.h>

namespace SConst
{
	static const std::uint32_t MaxPngSize = 4096u;
	static const std::uint32_t MaxFbxWeights = 16u;
	static const std::uint32_t Max16bitIndex = std::numeric_limits<std::uint16_t>::max();
}

static_assert(SConst::MaxWeightsPerVertex <= SConst::MaxFbxWeights, "SUtils.cpp: Wrong weights per vertex");

std::string_view GetEngineVersion()
{
	static const char Version[] = {
		'0' + LC_ENGINE_MAJOR_VERSION,
		'.',
		'0' + LC_ENGINE_MINOR_VERSION,
		'\0'
	};
	return Version;
}

std::string GetTimeStamp(const std::chrono::system_clock::time_point& currentTime)
{
	const std::time_t startTime = std::chrono::system_clock::to_time_t(currentTime);
	std::tm* tm = std::localtime(&startTime);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()) % 1000;
	std::uint32_t msValue = static_cast<std::uint32_t>(ms.count());
	std::ostringstream oss;
	oss << std::put_time(tm, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << msValue;
	return oss.str().c_str();
}

std::string ReadTextFile(const std::filesystem::path& filePath)
{
	std::string utfPath = ToUtf8(filePath.c_str());
	std::string result;

	S_TRY

	std::ifstream stream(filePath, std::ios::in | std::ios::binary);

	const auto sz = std::filesystem::file_size(filePath);
	result = std::string(sz + 1, '\0');
	stream.read(result.data(), sz);

	S_CATCH{ S_THROW_EX("ReadTextFile('", utfPath.c_str(), "')");}

	return result;
}

SBytes ReadBinaryFile(const std::filesystem::path& filePath)
{
	std::string utfPath = ToUtf8(filePath.c_str());
	SBytes result;

	S_TRY

	std::ifstream stream(filePath, std::ios::in | std::ios::binary);

	const auto sz = std::filesystem::file_size(filePath);
	result = SBytes(sz);
	stream.read(reinterpret_cast<char*>(result.data()), sz);

	S_CATCH{ S_THROW_EX("ReadBinaryFile('", utfPath.c_str(), "')"); }

	return result;
}

void WriteTextFile(const std::filesystem::path& filePath, const std::string& text)
{
	std::string utfPath = ToUtf8(filePath.c_str());

	S_TRY

	std::ofstream stream(filePath, std::ios::out);

	stream.write(text.c_str(), text.length());

	S_CATCH{ S_THROW_EX("WriteTextFile('", utfPath.c_str(), "')");}
}

void ReadPngFile(const std::filesystem::path& filePath, std::uint32_t* outWidth, std::uint32_t* outHeight,
	std::uint32_t* outBPP, std::uint32_t* outRowBytes, void* outData)
{
	struct PngRAII
	{
		PngRAII(png_structp& in_png_ptr, png_infop& in_info_ptr, png_infop& in_end_info)
			: png_ptr(in_png_ptr)
			, info_ptr(in_info_ptr)
			, end_info(in_end_info)
		{
		}
		~PngRAII() { png_destroy_read_struct(&png_ptr, &info_ptr, &end_info); }
		png_structp& png_ptr;
		png_infop& info_ptr;
		png_infop& end_info;
	};

	S_TRY

	SFileRAII fp(filePath.string().c_str());
	if (!fp)
	{
		throw std::exception("Failed to read file");
	}

	const int sig_bytes = 8;
	png_byte png_header[sig_bytes];
	fread(png_header, 1, sig_bytes, fp);
	bool is_png = !png_sig_cmp(png_header, 0, sig_bytes);
	if (!is_png)
	{
		throw std::exception("Invalid png header");
	}

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr)
	{
		throw std::exception("Failed to create read struct");
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, 0, 0);
		throw std::exception("Failed to create info struct");
	}

	png_infop end_info = png_create_info_struct(png_ptr);
	if (!end_info)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		throw std::exception("Failed to create end struct");
	}

	PngRAII png(png_ptr, info_ptr, end_info);

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_bytes);
	png_read_info(png_ptr, info_ptr);

	std::uint32_t width = png_get_image_width(png_ptr, info_ptr);
	std::uint32_t height = png_get_image_height(png_ptr, info_ptr);
	if (!InRange(width, 0u, SConst::MaxPngSize) ||
		!InRange(height, 0u, SConst::MaxPngSize))
	{
		throw std::exception("Invalid image size");
	}

	auto row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	if (row_bytes == 0)
	{
		throw std::exception("Invalid row bytes");
	}

	auto color_type = png_get_color_type(png_ptr, info_ptr);
	if (color_type != PNG_COLOR_TYPE_RGB &&
		color_type != PNG_COLOR_TYPE_RGBA)
	{
		throw std::exception("Invalid color type");
	}

	auto bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	if (bit_depth != 8)
	{
		throw std::exception("Invalid bit depth");
	}

	if (outHeight) *outHeight = height;
	if (outWidth) *outWidth = width;
	if (outRowBytes) *outRowBytes = static_cast<std::uint32_t>(width * 4);
	if (outBPP) *outBPP = 4u; // Always 4 bytes per pixel
	if (outData)
	{
		std::unique_ptr<png_byte[]> tmp_row;
		if (color_type == PNG_COLOR_TYPE_RGB)
		{
			// Need temp row for BPP == 3
			tmp_row.reset(new png_byte[row_bytes]);
		}

		png_byte* rows = reinterpret_cast<png_byte*>(outData);
		for (int i = 0; i < height; i++)
		{
			if (color_type == PNG_COLOR_TYPE_RGBA)
			{
				// Read 4 bytes per pixel
				png_read_row(png_ptr, rows, 0);
			}
			else
			{
				// Read 3 bytes per pixel
				png_byte* row_pixels = tmp_row.get();
				png_read_row(png_ptr, row_pixels, 0);

				// Convert to 4 bytes
				for (int j = 0; j < width; j++)
				{
					memcpy(&rows[j * 4], &row_pixels[j * 3], 3);
					rows[j * 4 + 3] = 255;
				}
			}

			rows += width * 4;
		}
	}

	S_CATCH{ S_THROW_EX("ReadPngFile('", filePath.string().c_str(), "')"); }
}

namespace SConvert
{
	inline SVector4 ToQuat(const ufbx_quat& v)
	{
		return SVector4{
			static_cast<float>(v.x),
			static_cast<float>(v.y),
			static_cast<float>(v.z),
			static_cast<float>(v.w)
		};
	}

	inline SVector3 ToVector3(const ufbx_vec3& v)
	{
		return SVector3{
			static_cast<float>(v.x),
			static_cast<float>(v.y),
			static_cast<float>(v.z)
		};
	}

	inline SVector2 ToVector2(const ufbx_vec2& v)
	{
		return SVector2{
			static_cast<float>(v.x),
			static_cast<float>(v.y)
		};
	}
}

bool LoadFbxStaticMeshes(const std::filesystem::path& filePath, SGroupID groupId,
	std::forward_list<SMeshID>& cachedMeshesIds, std::vector<SMesh>& outMeshes, std::vector<SMeshInstance>& outInstances)
{
	S_TRY

	static std::string meshMsg;
	ufbx_error error{};
	ufbx_scene* scene = ufbx_load_file(filePath.string().c_str(), nullptr, &error);
	if (!scene)
	{
		throw std::exception(error.description.data ? error.description.data : "Cannot read fbx file");
	}

	std::vector<SMeshID> instAdded;
	outMeshes.clear();
	outInstances.clear();

	for (auto i = 0; i < scene->nodes.count; i++)
	{
		ufbx_node* node = scene->nodes.data[i];
		if (node->is_root || !node->mesh) continue;

		SMesh mesh;
		mesh.name = node->mesh->name.data;
		mesh.id = ResourceID<SMeshID>(mesh.name);

		// skip if added
		const bool bAlreadyAdded = std::find(instAdded.begin(), instAdded.end(), mesh.id) != instAdded.end();
		if (bAlreadyAdded) continue;

		// skip duplicate mesh
		bool bDuplicate = false;
		for (auto& otherMesh : outMeshes)
		{
			if (mesh.id == otherMesh.id)
			{
				bDuplicate = true;
				break;
			}
		}
		if (bDuplicate) continue;

		// skip cashed mesh
		bool bSkipCached = false;
		for (auto otherId : cachedMeshesIds)
		{
			if (mesh.id == otherId)
			{
				bSkipCached = true;
				break;
			}
		}

		for (auto instance : node->mesh->instances)
		{
			STransform transform {
				SConvert::ToVector3(instance->local_transform.translation),
				SConvert::ToQuat(instance->local_transform.rotation),
				SConvert::ToVector3(instance->local_transform.scale)
			};
			outInstances.emplace_back(mesh.id, groupId, transform);
		}

		instAdded.push_back(mesh.id);

		if (bSkipCached) continue;

		// add new mesh and instances
		std::uint32_t indexOffset = 0;
		for (const ufbx_mesh_part& part : node->mesh->material_parts)
		{
			std::vector<SVertex> vertices;
			std::vector<std::uint32_t> triIndices;

			const size_t numTriangles = part.num_triangles;
			vertices.resize(numTriangles * 3);
			size_t numVertices = 0;

			const size_t numTriIndices = node->mesh->max_face_triangles * 3;
			triIndices.resize(numTriIndices);

			// collect vertices
			for (size_t faceId = 0; faceId < part.num_faces; faceId++)
			{
				ufbx_face face = node->mesh->faces.data[part.face_indices.data[faceId]];
				uint32_t numTris = ufbx_triangulate_face(triIndices.data(), numTriIndices, node->mesh, face);

				for (size_t i = 0; i < numTris * 3; i++)
				{
					uint32_t index = triIndices[i];
					SVertex* v = &vertices[numVertices++];
					v->pos = SConvert::ToVector3(ufbx_get_vertex_vec3(&node->mesh->vertex_position, index));
					v->norm = SConvert::ToVector3(ufbx_get_vertex_vec3(&node->mesh->vertex_normal, index));
					if (node->mesh->vertex_tangent.exists)
					{
						v->tangent = SConvert::ToVector3(ufbx_get_vertex_vec3(&node->mesh->vertex_tangent, index));
					}
					v->uv = SConvert::ToVector2(ufbx_get_vertex_vec2(&node->mesh->vertex_uv, index));
					v->uv.y = 1.0f - v->uv.y;
				}
			}

			triIndices.clear();
			if (numVertices != (numTriangles * 3))
			{
				meshMsg = "Wrong mesh structure: ";
				meshMsg += node->mesh->name.data;
				throw std::exception(meshMsg.c_str());
			}

			// generate indices
			std::vector<std::uint32_t> indices;
			ufbx_vertex_stream streams[1] = {
				{ vertices.data(), numVertices, sizeof(SVertex)},
			};
			const size_t numIndices = numTriangles * 3;
			indices.resize(numIndices);

			numVertices = ufbx_generate_indices(streams, 1, indices.data(), numIndices, NULL, NULL);

			// add new material part to the mesh
			const ufbx_material* material = node->mesh->materials[part.index];
			std::string baseTexture, normTexture, rmaTexture, emiTexture;
			if (material->textures.count > 0)
			{
				baseTexture = material->pbr.base_color.texture ? material->pbr.base_color.texture->filename.data : "";
				normTexture = material->pbr.normal_map.texture ? material->pbr.normal_map.texture->filename.data : "";
				if (material->pbr.base_color.texture)
				{
					baseTexture = material->pbr.base_color.texture->filename.data;
					std::replace(baseTexture.begin(), baseTexture.end(), '\\', '/');
				}
				if (material->pbr.normal_map.texture)
				{
					normTexture = material->pbr.normal_map.texture->filename.data;
					std::replace(normTexture.begin(), normTexture.end(), '\\', '/');
				}
				if (material->pbr.roughness.texture)
				{
					rmaTexture = material->pbr.roughness.texture->filename.data;
					std::replace(rmaTexture.begin(), rmaTexture.end(), '\\', '/');
				}
				if (material->pbr.emission_color.texture)
				{
					emiTexture = material->pbr.emission_color.texture->filename.data;
					std::replace(emiTexture.begin(), emiTexture.end(), '\\', '/');
				}
			}

			const bool bUseIndices16bit = (numIndices < SConst::Max16bitIndex);
			if (bUseIndices16bit)
			{
				auto view16 = indices | std::views::transform([](auto index) {
					return static_cast<std::uint16_t>(index);
				});
				mesh.indices16.assign(view16.begin(), view16.end());
			}
			else
			{
				mesh.indices32 = std::move(indices);
			}

			const SMaterial meshMaterial {
				indexOffset, static_cast<std::uint32_t>(numVertices), static_cast<std::uint32_t>(numIndices),
				baseTexture.c_str(), normTexture.c_str(), rmaTexture.c_str(), emiTexture.c_str()
			};
			mesh.materials.push_back(meshMaterial);
			mesh.vertices.insert(mesh.vertices.end(), vertices.begin(), vertices.begin() + numVertices);

			indexOffset += numIndices;

			// load only one skeletal mesh part
			break;
		}

		outMeshes.push_back(mesh);
	}

	ufbx_free_scene(scene);

	return !outMeshes.empty() || !outInstances.empty();

	S_CATCH{ S_THROW_EX("LoadFbxStaticMeshes('", filePath.string().c_str(), "')"); }
}

bool LoadFbxSkeletalMesh(const std::filesystem::path& filePath, SSkeletalMesh& outMesh)
{
	S_TRY

	static std::string meshMsg;
	ufbx_error error{};
	ufbx_scene* scene = ufbx_load_file(filePath.string().c_str(), nullptr, &error);
	if (!scene)
	{
		throw std::exception(error.description.data ? error.description.data : "Cannot read fbx file");
	}

	ClearMesh(outMesh);

	for (auto i = 0; i < scene->nodes.count; i++)
	{
		ufbx_node* node = scene->nodes.data[i];
		if (node->is_root || !node->mesh) continue;

		outMesh.name = node->mesh->name.data;
		outMesh.id = ResourceID<SMeshID>(outMesh.name);

		// save transform
		for (auto instance : node->mesh->instances)
		{
			STransform transform {
				SConvert::ToVector3(instance->local_transform.translation),
				SConvert::ToQuat(instance->local_transform.rotation),
				SConvert::ToVector3(instance->local_transform.scale)
			};
			outMesh.transform = transform;
			break;
		}

		// add new skeletal mesh
		std::uint32_t indexOffset = 0;
		for (const ufbx_mesh_part& part : node->mesh->material_parts)
		{
			std::vector<SBlendVertex> vertices;
			std::vector<std::uint32_t> triIndices;

			const size_t numTriangles = part.num_triangles;
			vertices.resize(numTriangles * 3);
			size_t numVertices = 0;

			const size_t numTriIndices = node->mesh->max_face_triangles * 3;
			triIndices.resize(numTriIndices);

			// collect vertices
			for (size_t faceId = 0; faceId < part.num_faces; faceId++)
			{
				ufbx_face face = node->mesh->faces.data[part.face_indices.data[faceId]];
				uint32_t numTris = ufbx_triangulate_face(triIndices.data(), numTriIndices, node->mesh, face);

				for (size_t i = 0; i < numTris * 3; i++)
				{
					uint32_t index = triIndices[i];
					SBlendVertex* v = &vertices[numVertices++];
					v->pos = SConvert::ToVector3(ufbx_get_vertex_vec3(&node->mesh->vertex_position, index));
					v->norm = SConvert::ToVector3(ufbx_get_vertex_vec3(&node->mesh->vertex_normal, index));
					if (node->mesh->vertex_tangent.exists)
					{
						v->tangent = SConvert::ToVector3(ufbx_get_vertex_vec3(&node->mesh->vertex_tangent, index));
					}
					v->uv = SConvert::ToVector2(ufbx_get_vertex_vec2(&node->mesh->vertex_uv, index));
					v->uv.y = 1.0f - v->uv.y;
				}
			}

			triIndices.clear();
			if (numVertices != (numTriangles * 3))
			{
				meshMsg = "Wrong mesh structure: ";
				meshMsg += node->mesh->name.data;
				throw std::exception(meshMsg.c_str());
			}

			// generate indices
			std::vector<std::uint32_t> indices;
			ufbx_vertex_stream streams[1] = {
				{ vertices.data(), numVertices, sizeof(SBlendVertex)},
			};
			const size_t numIndices = numTriangles * 3;
			indices.resize(numIndices);

			numVertices = ufbx_generate_indices(streams, 1, indices.data(), numIndices, NULL, NULL);

			// add skinning data
			if (node->mesh->skin_deformers.count != 1 ||
				numVertices != node->mesh->skin_deformers[0]->vertices.count)
			{
				meshMsg = "Wrong skinning data: ";
				meshMsg += node->mesh->name.data;
				throw std::exception(meshMsg.c_str());
			}

			const auto skinData = node->mesh->skin_deformers[0];

			for (std::uint32_t v = 0; v < numVertices; v++)
			{
				const auto& vertexInfo = skinData->vertices[v];
				if (vertexInfo.num_weights > SConst::MaxFbxWeights)
				{
					meshMsg = "Too many weights per vertex: ";
					meshMsg += node->mesh->name.data;
					throw std::exception(meshMsg.c_str());
				}

				float overflow = 0.0f;
				for (std::uint32_t w = 0; w < vertexInfo.num_weights; w++)
				{
					if ((vertexInfo.weight_begin + w) >= skinData->weights.count)
					{
						meshMsg = "Wrong weights list size: ";
						meshMsg += node->mesh->name.data;
						throw std::exception(meshMsg.c_str());
					}

					const float weight = skinData->weights[vertexInfo.weight_begin + w].weight;
					if (w >= SConst::MaxWeightsPerVertex) overflow += weight;
				}

				auto& vertex = vertices[v];
				auto numWeights = std::min(vertexInfo.num_weights, SConst::MaxWeightsPerVertex);
				if (vertexInfo.num_weights > SConst::MaxWeightsPerVertex)
				{
					for (std::uint32_t w = 0; w < numWeights; w++)
					{
						const std::uint32_t boneId = skinData->weights[vertexInfo.weight_begin + w].cluster_index;
						if (boneId >= skinData->clusters.count)
						{
							meshMsg = "Wrong bone id: ";
							meshMsg += node->mesh->name.data;
							throw std::exception(meshMsg.c_str());
						}

						// add distributed overflow to previous weights
						const float weight = skinData->weights[vertexInfo.weight_begin + w].weight;
						const float overflowDistribution = overflow * weight;
						vertex.weights[w] = weight + overflowDistribution;
						vertex.indices[w] = boneId;
					}
				}
				else
				{
					for (std::uint32_t w = 0; w < numWeights; w++)
					{
						vertex.weights[w] = skinData->weights[vertexInfo.weight_begin + w].weight;
						vertex.indices[w] = skinData->weights[vertexInfo.weight_begin + w].cluster_index;
					}
				}
			}

			// add bones
			outMesh.bones.reserve(skinData->clusters.count);
			for (std::uint32_t b = 0; b < skinData->clusters.count; b++)
			{
				outMesh.bones.emplace(skinData->clusters[b]->name.data, b);
			}

			// add new material part to the mesh
			const ufbx_material* material = node->mesh->materials[part.index];
			std::string baseTexture, normTexture, rmaTexture, emiTexture;
			if (material->textures.count > 0)
			{
				baseTexture = material->pbr.base_color.texture ? material->pbr.base_color.texture->filename.data : "";
				normTexture = material->pbr.normal_map.texture ? material->pbr.normal_map.texture->filename.data : "";
				if (material->pbr.base_color.texture)
				{
					baseTexture = material->pbr.base_color.texture->filename.data;
					std::replace(baseTexture.begin(), baseTexture.end(), '\\', '/');
				}
				if (material->pbr.normal_map.texture)
				{
					normTexture = material->pbr.normal_map.texture->filename.data;
					std::replace(normTexture.begin(), normTexture.end(), '\\', '/');
				}
				if (material->pbr.roughness.texture)
				{
					rmaTexture = material->pbr.roughness.texture->filename.data;
					std::replace(rmaTexture.begin(), rmaTexture.end(), '\\', '/');
				}
				if (material->pbr.emission_color.texture)
				{
					emiTexture = material->pbr.emission_color.texture->filename.data;
					std::replace(emiTexture.begin(), emiTexture.end(), '\\', '/');
				}
			}

			const bool bUseIndices16bit = (numIndices < SConst::Max16bitIndex);
			if (bUseIndices16bit)
			{
				auto view16 = indices | std::views::transform([](auto index) {
					return static_cast<std::uint16_t>(index);
				});
				outMesh.indices16.assign(view16.begin(), view16.end());
			}
			else
			{
				outMesh.indices32 = std::move(indices);
			}

			const SMaterial meshMaterial {
				indexOffset, static_cast<std::uint32_t>(numVertices), static_cast<std::uint32_t>(numIndices),
				baseTexture.c_str(), normTexture.c_str(), rmaTexture.c_str(), emiTexture.c_str()
			};
			outMesh.materials.push_back(meshMaterial);
			outMesh.vertices.insert(outMesh.vertices.end(), vertices.begin(), vertices.begin() + numVertices);

			indexOffset += numIndices;

			// load only one skeletal mesh part
			break;
		}

		// load only one skeletal mesh
		break;
	}

	ufbx_free_scene(scene);

	return !outMesh.vertices.empty();

	S_CATCH{ S_THROW_EX("LoadFbxSkeletalMesh('", filePath.string().c_str(), "')"); }
}

std::uint32_t GetNumFrames(ufbx_baked_node& bakedBone, float duration)
{
	std::uint32_t frames = 0u;
	for (size_t i = 0; i < bakedBone.translation_keys.count; i++)
	{
		const ufbx_baked_vec3& trans = bakedBone.translation_keys[i];
		const float keyTime = static_cast<float>(trans.time);
		if (keyTime <= duration) frames++;
	}

	return frames;
}

bool LoadFbxBakedAnimation(const std::filesystem::path& filePath, const TBonesMap& bones,
	SBakedSkeletalAnimation& outAnim, std::uint32_t framesPerSecond)
{
	S_TRY

	static std::string animMsg;
	ufbx_error error{};
	ufbx_scene* scene = ufbx_load_file(filePath.string().c_str(), nullptr, &error);
	if (!scene)
	{
		throw std::exception(error.description.data ? error.description.data : "Cannot read fbx file");
	}

	for (size_t i = 0; i < scene->anim_stacks.count; i++)
	{
		ufbx_anim_stack* stack = scene->anim_stacks.data[i];
		ufbx_baked_anim* bakedAnim = ufbx_bake_anim(scene, stack->anim, NULL, NULL);
		if (!bakedAnim)
		{
			throw std::exception("Cannot bake animation");
		}

		if (bakedAnim->nodes.count < bones.size())
		{
			throw std::exception("Not enough bones in animation");
		}

		const std::uint32_t numBones = bones.size();
		const std::uint32_t numFrames = GetNumFrames(bakedAnim->nodes[0], bakedAnim->playback_duration);
		const std::uint32_t fps = numFrames / bakedAnim->playback_duration;
		if (fps != framesPerSecond)
		{
			throw std::exception("Wrong frames per second in animation");
		}

		outAnim.duration = bakedAnim->playback_duration;
		outAnim.frames.resize(numFrames);

		for (size_t b = 0; b < bakedAnim->nodes.count; b++)
		{
			ufbx_baked_node* bakedBone = &bakedAnim->nodes.data[b];
			if (bakedBone->translation_keys.count != bakedBone->rotation_keys.count ||
				bakedBone->translation_keys.count != bakedBone->scale_keys.count ||
				bakedBone->translation_keys.count < numFrames)
			{
				throw std::exception("Wrong animation keys data");
			}

			ufbx_node* sceneBone = scene->nodes.data[bakedBone->typed_id];
			auto boneIt = bones.find(sceneBone->name.data);
			if (boneIt == bones.end())
			{
				// skip not skinned bones
				continue;
			}

			const std::uint32_t boneId = boneIt->second;
			if (boneId >= numBones)
			{
				throw std::exception("Unexpected bone id");
			}

			for (size_t f = 0; f < numFrames; f++)
			{
				auto& frame = outAnim.frames[f];
				if (frame.trans.size() < numBones) frame.trans.resize(numBones);
				if (frame.rotation.size() < numBones) frame.rotation.resize(numBones);
				if (frame.scale.size() < numBones) frame.scale.resize(numBones);

				const ufbx_baked_vec3& trans = bakedBone->translation_keys[f];
				const ufbx_baked_quat& rotation = bakedBone->rotation_keys[f];
				const ufbx_baked_vec3& scale = bakedBone->scale_keys[f];
				const float keyTime = static_cast<float>(trans.time);
				if (keyTime <= bakedAnim->playback_duration)
				{
					frame.trans[boneId] = SConvert::ToVector3(trans.value);
					frame.rotation[boneId] = SConvert::ToQuat(rotation.value);
					frame.scale[boneId] = SConvert::ToVector3(scale.value);
					frame.time = keyTime;
				}
			}
		}

		ufbx_free_baked_anim(bakedAnim);
		break;
	}

	ufbx_free_scene(scene);

	std::string animName = MakeAnimationName(filePath);
	outAnim.id = ResourceID<SAnimID>(animName);
	outAnim.name = animName;
	outAnim.framesPerSecond = SConst::AnimationFramesPerSecond;

	return !outAnim.frames.empty();

	S_CATCH{ S_THROW_EX("LoadFbxBakedAnimation('", filePath.string().c_str(), "')"); }
}

std::string MakeAnimationName(const std::filesystem::path& path)
{
	return path.filename().replace_extension().string();
}

SFileRAII::SFileRAII(const std::filesystem::path& filePath, const char* mode) : file(nullptr)
{
#ifdef _WINDOWS
	fopen_s(&file, filePath.string().c_str(), mode);
#else
	file = fopen(filePath.string().c_str(), mode);
#endif
}

SFileRAII::~SFileRAII()
{
	if (file) fclose(file);
}

#ifdef _WINDOWS

#include <windows.h>
#include <stdio.h>

std::string ToUtf8(const std::wstring& str)
{
	int requiredSize = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0, NULL, NULL);
	if (requiredSize <= 0) throw std::exception("ToUtf8(): Convert error");

	std::string mbChars(requiredSize, ' ');
	int result = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), (int)str.length(), &mbChars[0], (int)mbChars.length(), NULL, NULL);
	if (result != requiredSize) throw std::exception("ToUtf8(): Cannot convert");

	return mbChars;
}

std::wstring FromUtf8(const std::string& str)
{
	int requiredSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
	if (requiredSize <= 0) throw std::exception("FromUtf8(): Convert error");

	std::wstring wideChars(requiredSize, ' ');
	int result = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wideChars[0], (int)wideChars.length());
	if (result != requiredSize) throw std::exception("FromUtf8(): Cannot convert");

	return wideChars;
}

std::string ToLower(const char* str)
{
	std::string src(str);
	std::string dst;
	dst.resize(src.size());

	std::transform(src.begin(), src.end(), dst.begin(), ::tolower);

	return dst;
}

std::string ToUpper(const char* str)
{
	std::string src(str);
	std::string dst;
	dst.resize(src.size());

	std::transform(src.begin(), src.end(), dst.begin(), ::toupper);

	return dst;
}

std::string ToString(std::int32_t value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

std::string ToString(float value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

std::wstring ToStringW(std::int32_t value)
{
	std::wstringstream ss;
	ss << value;
	return ss.str();
}

std::wstring ToStringW(float value)
{
	std::wstringstream ss;
	ss << value;
	return ss.str();
}

std::wstring Localize(const wchar_t* fmt, ...)
{
	static wchar_t localize_out[4096];
	va_list argp;
	va_start(argp, fmt);
	vswprintf_s(localize_out, fmt, argp);
	va_end(argp);
	return std::wstring(localize_out);
}

void DebugMsg(const char* fmt, ...)
{
	static char dbg_out[4096];
	va_list argp;
	va_start(argp, fmt);
	vsprintf_s(dbg_out, fmt, argp);
	va_end(argp);
	OutputDebugStringA(dbg_out);
}

void DebugMsgW(const wchar_t* fmt, ...)
{
	static wchar_t dbg_out[4096];
	va_list argp;
	va_start(argp, fmt);
	vswprintf_s(dbg_out, fmt, argp);
	va_end(argp);
	OutputDebugStringW(dbg_out);
}

void ShowMessageModal(const char* message, const char* title)
{
	MessageBoxA(NULL, message, title, MB_OK | MB_SERVICE_NOTIFICATION);
}

#else

void DebugMsg(const char* fmt, ...) {}
void DebugMsgW(const wchar_t* fmt, ...) {}

#endif
