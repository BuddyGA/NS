#pragma once

#include "nsVulkan.h"



class nsShaderCompileTask : public nsIThreadTask
{
private:
	nsAtomic bDone;

public:
	nsName Name;
	nsString ShaderFile;
	VkShaderStageFlagBits ShaderType;
	nsTArray<nsName> MacroDefinitions;
	nsTArray<uint32> ShaderCodes;
	bool bCompiling;
	bool bCompileSuccess;


public:
	nsShaderCompileTask() noexcept;
	virtual void Reset() noexcept;
	virtual void Execute() noexcept override;
	virtual bool IsIdle() const noexcept override;
	virtual bool IsRunning() const noexcept;
	virtual bool IsDone() const noexcept override;

#ifdef _DEBUG
	virtual nsString GetDebugName() const noexcept override;
#endif // _DEBUG

};



class NS_ENGINE_API nsShaderManager
{
private:
	bool bInitialized;

	enum Flags
	{
		Flag_None			= (0),
		Flag_Dirty			= (1 << 0),
		Flag_Compiling		= (1 << 1),
		Flag_Compiled		= (1 << 2),
		Flag_HotReload		= (1 << 3),
	};

	nsTArray<nsName> ShaderNames;
	nsTArray<uint32> ShaderFlags;
	nsTArray<nsShaderCompileTask> ShaderCompileTasks;
	nsTArray<nsVulkanShader*> ShaderResources;


public:
	void Initialize() noexcept;
	void AddShader(nsName name, const nsString& glslFile, VkShaderStageFlagBits shaderType, nsTArray<nsName> optionalCompileMacros = nsTArray<nsName>()) noexcept;
	void CompileShaders(bool bWaitUntilFinished) noexcept;
	nsVulkanShader* GetShaderModule(const nsName& name) const noexcept;


	NS_DECLARE_SINGLETON(nsShaderManager)

};
