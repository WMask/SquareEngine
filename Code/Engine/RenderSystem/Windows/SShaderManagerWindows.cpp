/***************************************************************************
* SShaderManagerWindows.cpp
*/

#ifdef WIN32

#include "RenderSystem/Windows/SShaderManagerWindows.h"
#include "Core/SUtils.h"

#pragma comment(lib, "d3dcompiler.lib")

namespace SConst
{
	constexpr std::uint32_t MaxShaders = 64u;
	constexpr std::string_view ShaderIncludesExt = ".hlsli";
}


class SShaderIncludeHandler : public ID3DInclude
{
public:
	SShaderIncludeHandler(const std::unordered_map<std::string, SBytes>& inIncludeShaders)
		: includeShaders(inIncludeShaders) {}
	//
	STDMETHOD(Open)(D3D_INCLUDE_TYPE Type, LPCSTR pFileName, LPCVOID pParentData,
		LPCVOID* ppData, UINT* pBytes) override
	{
		for (auto& entry : includeShaders)
		{
			if (entry.first.ends_with(pFileName))
			{
				*ppData = entry.second.data();
				*pBytes = entry.second.size();
				return S_OK;
			}
		}
		return E_FAIL;
	}
	//
	STDMETHOD(Close)(LPCVOID pData) override
	{
		return S_OK;
	}

private:
	//
	const std::unordered_map<std::string, SBytes>& includeShaders;
};

SShaderManagerWindows::~SShaderManagerWindows()
{
	Shutdown();
}

void SShaderManagerWindows::Shutdown()
{
	compiledShaders.reset();
	threadPool = nullptr;
}

void SShaderManagerWindows::Init(IThreadPool* inThreadPool)
{
	threadPool = inThreadPool;
	compiledShaders = std::make_shared<TCircularFIFOShaderQueue>(SConst::MaxShaders);
}

void SShaderManagerWindows::Update()
{
	if (compiledShaders)
	{
		SCompiledShaderData data;
		// read in game thread space
		if (compiledShaders->pop(data))
		{
			if (!data.psCode.Get() || !data.vsCode.Get())
			{
				throw std::exception("SShaderManagerWindows::Update(): cannot load or compile shader");
			}

			data.delegate(data);
		}
	}
}

void SShaderManagerWindows::LoadShader(const SPath& path, TOnLoadedDelegate delegate)
{
	SPathList paths{ path };
	LoadShaders(paths, delegate);
}

void SShaderManagerWindows::LoadShaders(const SPathList& paths, TOnLoadedDelegate delegate)
{
	if (!threadPool || !compiledShaders)
	{
		throw std::exception("SShaderManagerWindows::LoadShader(): manager is not initialized");
	}

	auto LoadShadersTask = [this, paths, delegate]()
	{
		std::unordered_map<std::string, SBytes> includes;
		includes.reserve(paths.size());

		// load include files
		for (auto path : paths)
		{
			if (path.extension().string() == SConst::ShaderIncludesExt)
			{
				SBytes data;
				try
				{
					data = ReadBinaryFile(path);
				}
				catch (std::exception&)
				{
					DebugMsg("[%s] SShaderManagerWindows::LoadShaders(): cannot load '%s' include file\n",
						GetTimeStamp(std::chrono::system_clock::now()).c_str(), path.c_str());
				}

				if (!data.empty())
				{
					includes.emplace(path.string(), data);
				}
			}
		}

		SShaderIncludeHandler includeHandler(includes);

		// load and compile shaders
		for (auto path : paths)
		{
			if (path.extension().string() == SConst::ShaderIncludesExt)
			{
				continue;
			}

			std::string name = ToUtf8(path.c_str());
			size_t pos = name.find("Shaders");
			if (pos != std::string::npos)
			{
				name = name.substr(pos);
			}

			ComPtr<ID3DBlob> vsCode;
			ComPtr<ID3DBlob> psCode;
			ComPtr<ID3DBlob> compileError;

			// read file
			std::string code;
			try
			{
				code = ReadTextFile(path);
			}
			catch (std::exception&) {}

			if (code.empty())
			{
				DebugMsg("[%s] SShaderManagerWindows::LoadShaders('%s'): cannot read file\n",
					GetTimeStamp(std::chrono::system_clock::now()).c_str(), name.c_str());
			}
			else
			{
				// compile
				if (SUCCEEDED(D3DCompile(code.c_str(), code.length(), NULL, NULL, &includeHandler, "VShader", "vs_4_0", 0, 0, vsCode.GetAddressOf(), compileError.GetAddressOf())))
				{
					if (threadPool->IsDebugLogsEnabled())
					{
						DebugMsg("[%s] SShaderManagerWindows::LoadShaders('%s'): vertex shader compiled\n",
							GetTimeStamp(std::chrono::system_clock::now()).c_str(), name.c_str());
					}
				}
				else
				{
					if (threadPool->IsDebugLogsEnabled())
					{
						auto errorMsg = static_cast<const char*>(compileError ? compileError->GetBufferPointer() : nullptr);
						if (errorMsg)
						{
							DebugMsg("[%s] SShaderManagerWindows::LoadShaders('%s'): vertex shader compilation error '%s'\n",
								GetTimeStamp(std::chrono::system_clock::now()).c_str(), name.c_str(), errorMsg);
						}
						else
						{
							DebugMsg("[%s] SShaderManagerWindows::LoadShaders('%s'): vertex shader compilation error\n",
								GetTimeStamp(std::chrono::system_clock::now()).c_str(), name.c_str());
						}
					}

					compileError.Reset();
				}

				if (SUCCEEDED(D3DCompile(code.c_str(), code.length(), NULL, NULL, &includeHandler, "PShader", "ps_4_0", 0, 0, psCode.GetAddressOf(), compileError.GetAddressOf())))
				{
					if (threadPool->IsDebugLogsEnabled())
					{
						DebugMsg("[%s] SShaderManagerWindows::LoadShaders('%s'): pixel shader compiled\n",
							GetTimeStamp(std::chrono::system_clock::now()).c_str(), name.c_str());
					}
				}
				else
				{
					if (threadPool->IsDebugLogsEnabled())
					{
						auto errorMsg = static_cast<const char*>(compileError ? compileError->GetBufferPointer() : nullptr);
						if (errorMsg)
						{
							DebugMsg("[%s] SShaderManagerWindows::LoadShaders('%s'): pixel shader compilation error '%s'\n",
								GetTimeStamp(std::chrono::system_clock::now()).c_str(), name.c_str(), errorMsg);
						}
						else
						{
							DebugMsg("[%s] SShaderManagerWindows::LoadShaders('%s'): pixel shader compilation error\n",
								GetTimeStamp(std::chrono::system_clock::now()).c_str(), name.c_str());
						}
					}

					compileError.Reset();
				}
			}

			// write in thread pool space
			compiledShaders->push({ name, vsCode, psCode, delegate });
		}
	};

	// send task to thread pool
	threadPool->AddTask(LoadShadersTask, "Load and compile shaders");
}

#endif // WIN32
