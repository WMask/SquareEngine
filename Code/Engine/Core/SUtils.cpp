/***************************************************************************
* SUtils.cpp
*/

#include "Core/SUtils.h"
#include "Core/SException.h"
#include "SConfig.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
#include <png.h>
#include <ufbx.h>

namespace SConst
{
	static const std::uint32_t MAX_PNG_SIZE = 4096u;
}


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
	if (!InRange(width, 0u, SConst::MAX_PNG_SIZE + 1) ||
		!InRange(height, 0u, SConst::MAX_PNG_SIZE + 1))
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
	inline SVector4 ToVector4(const ufbx_quat& v)
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

void LoadFbxStaticMeshes(const std::filesystem::path& filePath, SGroupID groupId,
	std::forward_list<SMeshID>& meshesCache, std::vector<SMesh>& outMeshes, std::vector<SMeshInstance>& outInstances)
{
	S_TRY

	static std::string staticMeshMsg;
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
		for (auto& otherId : meshesCache)
		{
			if (mesh.id == otherId)
			{
				bSkipCached = true;
				break;
			}
		}

		for (auto instance : node->mesh->instances)
		{
			ufbx_vec3 rotation = ufbx_find_vec3(&node->props, "Lcl Rotation", ufbx_zero_vec3);
			STransform transform {
				SConvert::ToVector3(instance->local_transform.translation),
				SConvert::ToQuat(rotation.x, rotation.y, rotation.z),
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
				staticMeshMsg = "Wrong mesh structure: ";
				staticMeshMsg += node->mesh->name.data;
				throw std::exception(staticMeshMsg.c_str());
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

			static const std::uint32_t max16bitIndex = std::numeric_limits<std::uint16_t>::max();
			const bool bUseIndices16bit = (numIndices < max16bitIndex);
			if (bUseIndices16bit)
			{
				mesh.indices16.resize(indices.size());
				for (auto j = 0; j < indices.size(); j++)
				{
					mesh.indices16[j] = static_cast<std::uint16_t>(indices[j]);
				}
			}
			else
			{
				mesh.indices32 = indices;
			}

			mesh.vertices.insert(mesh.vertices.end(), vertices.begin(), vertices.begin() + numVertices);
			mesh.materials.emplace_back(baseTexture.c_str(), normTexture.c_str(), rmaTexture.c_str(), emiTexture.c_str(),
				indexOffset, static_cast<std::uint32_t>(numVertices), static_cast<std::uint32_t>(numIndices));

			indexOffset += numIndices;
		}

		outMeshes.push_back(mesh);
	}

	ufbx_free_scene(scene);

	S_CATCH{ S_THROW_EX("LoadFbxStaticMeshes('", filePath.string().c_str(), "')"); }
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
