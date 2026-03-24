/***************************************************************************
* SDXShaderManager.cpp
*/

#ifdef WIN32

#include "RenderSystem/Windows/SDXShaderManager.h"
#include "Core/SUtils.h"

#pragma comment(lib, "d3dcompiler.lib")


namespace SConst
{
	static const std::uint32_t MaxShaders = 64u;
}

void SDXShaderManager::Init(IThreadPool* inThreadPool)
{
	threadPool = inThreadPool;
	compiledShaders = std::make_shared<TCircularFIFOShaderQueue>(SConst::MaxShaders);
}

void SDXShaderManager::Update()
{
	if (!compiledShaders->empty())
	{
		SCompiledShaderData data;
		// read in game thread space
		while (compiledShaders->pop(data))
		{
			if (!data.psCode.Get() || !data.vsCode.Get())
			{
				throw std::exception("SDXShaderManager::Update(): cannot load or compile shader");
			}

			data.delegate(data);
		}
	}
}

void SDXShaderManager::LoadShader(const std::filesystem::path& path, TOnLoadedDelegate delegate)
{
	std::vector<std::filesystem::path> paths{ path };
	LoadShaders(paths, delegate);
}

void SDXShaderManager::LoadShaders(const std::vector<std::filesystem::path>& paths, TOnLoadedDelegate delegate)
{
	if (!threadPool || !compiledShaders)
	{
		throw std::exception("SDXShaderManager::LoadShader(): manager is not initialized");
	}

	auto LoadShadersTask = [this, paths, delegate]()
	{
		for (auto path : paths)
		{
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
				DebugMsg("[%s] SDXShaderManager::LoadShaders('%s'): cannot read file\n",
					GetTimeStamp(std::chrono::system_clock::now()).c_str(), name.c_str());
			}
			else
			{
				// compile
				if (SUCCEEDED(D3DCompile(code.c_str(), code.length(), NULL, NULL, NULL, "VShader", "vs_4_0", 0, 0, vsCode.GetAddressOf(), compileError.GetAddressOf())))
				{
					if (threadPool->IsDebugLogsEnabled())
					{
						DebugMsg("[%s] SDXShaderManager::LoadShaders('%s'): vertex shader compiled\n",
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
							DebugMsg("[%s] SDXShaderManager::LoadShaders('%s'): vertex shader compilation error '%s'\n",
								GetTimeStamp(std::chrono::system_clock::now()).c_str(), name.c_str(), errorMsg);
						}
						else
						{
							DebugMsg("[%s] SDXShaderManager::LoadShaders('%s'): vertex shader compilation error\n",
								GetTimeStamp(std::chrono::system_clock::now()).c_str(), name.c_str());
						}
					}

					compileError.Reset();
				}

				if (SUCCEEDED(D3DCompile(code.c_str(), code.length(), NULL, NULL, NULL, "PShader", "ps_4_0", 0, 0, psCode.GetAddressOf(), compileError.GetAddressOf())))
				{
					if (threadPool->IsDebugLogsEnabled())
					{
						DebugMsg("[%s] SDXShaderManager::LoadShaders('%s'): pixel shader compiled\n",
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
							DebugMsg("[%s] SDXShaderManager::LoadShaders('%s'): pixel shader compilation error '%s'\n",
								GetTimeStamp(std::chrono::system_clock::now()).c_str(), name.c_str(), errorMsg);
						}
						else
						{
							DebugMsg("[%s] SDXShaderManager::LoadShaders('%s'): pixel shader compilation error\n",
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
