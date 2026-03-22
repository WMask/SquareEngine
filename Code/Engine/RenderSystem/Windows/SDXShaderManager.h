/***************************************************************************
* SDXShaderManager.h
*/

#pragma once

#include "Core/ThreadPool/SThreadPoolInterface.h"
#include "Core/ThreadPool/Fifo4.h"
#include "Core/SUtils.h"

#include <d3dcompiler.h>
#include <wrl.h>

using namespace Microsoft::WRL;


/***************************************************************************
* DirectX shader manager
*/
class SDXShaderManager
{
public:
	//
	struct SCompiledShader
	{
		std::string name;
		ComPtr<ID3DBlob> vsCode;
		ComPtr<ID3DBlob> psCode;
	};
	//
	using TOnLoadedDelegate = std::function<void(const SCompiledShader&)>;
	//
	struct SCompiledShaderData : public SCompiledShader
	{
		TOnLoadedDelegate delegate;
	};
	//
	using TCircularFIFOShaderQueue = Fifo4<SCompiledShaderData>;


public:
	//
	SDXShaderManager() : threadPool{} {}
	//
	~SDXShaderManager() { threadPool = nullptr; }
	//
	void Init(IThreadPool* threadPool);
	//
	void Update();
	//
	void LoadShader(const std::filesystem::path& path, TOnLoadedDelegate delegate);
	//
	void LoadShaders(const std::vector<std::filesystem::path>& paths, TOnLoadedDelegate delegate);


protected:
	//
	std::shared_ptr<TCircularFIFOShaderQueue> compiledShaders;
	//
	IThreadPool* threadPool;

};
