#include "nsShaderManager.h"
#include "nsFileSystem.h"
#include <shaderc/shaderc.hpp>



static nsLogCategory ShaderLog("nsShaderLog", nsELogVerbosity::LV_DEBUG);



nsShaderCompileTask::nsShaderCompileTask() noexcept
{
	Reset();
}


void nsShaderCompileTask::Reset() noexcept
{
	bDone.Set(0);
	Name = "";
	ShaderFile = "";
	MacroDefinitions.Clear();
	ShaderCodes.Clear();
	bCompiling = false;
	bCompileSuccess = false;
}


void nsShaderCompileTask::Execute() noexcept
{
	shaderc_shader_kind shaderKind = shaderc_vertex_shader;

	switch (ShaderType)
	{
		case VK_SHADER_STAGE_VERTEX_BIT: shaderKind = shaderc_vertex_shader; break;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: shaderKind = shaderc_tess_control_shader; break;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: shaderKind = shaderc_tess_evaluation_shader; break;
		case VK_SHADER_STAGE_GEOMETRY_BIT: shaderKind = shaderc_geometry_shader; break;
		case VK_SHADER_STAGE_FRAGMENT_BIT: shaderKind = shaderc_fragment_shader; break;
		case VK_SHADER_STAGE_COMPUTE_BIT: shaderKind = shaderc_compute_shader; break;
		default: break;
	}

	nsString glslString;
	nsFileSystem::FileReadText(ShaderFile, glslString);

	shaderc::CompileOptions options;
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);

	for (int i = 0; i < MacroDefinitions.GetCount(); ++i)
	{
		options.AddMacroDefinition(*MacroDefinitions[i]);
	}

	shaderc::Compiler compiler;
	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(*glslString, glslString.GetLength(), shaderKind, *Name, "main", options);

	if (result.GetCompilationStatus() == shaderc_compilation_status_success)
	{
		NS_LogDebug(ShaderLog, "Compile shader [%s] SUCCESS [Errors: %i, Warnings: %i]", *Name, result.GetNumErrors(), result.GetNumWarnings());
		const int wordCount = static_cast<int>(result.cend() - result.cbegin());
		NS_Assert((sizeof(uint32) * wordCount) % 4 == 0);
		ShaderCodes.Clear();
		ShaderCodes.InsertAt(result.cbegin(), wordCount);
		bCompileSuccess = true;
	}
	else
	{
		NS_LogError(ShaderLog, "Compile shader [%s] FAILED [Errors: %i, Warnings: %i]\n%s", *Name, result.GetNumErrors(), result.GetNumWarnings(), result.GetErrorMessage().c_str());
	}

	bDone.Set(1);
}


bool nsShaderCompileTask::IsIdle() const noexcept
{
	return !bCompiling;
}


bool nsShaderCompileTask::IsRunning() const noexcept
{
	return bCompiling;
}


bool nsShaderCompileTask::IsDone() const noexcept
{
	return bDone.Get() == 1;
}


#ifdef _DEBUG

nsString nsShaderCompileTask::GetDebugName() const noexcept
{
	return nsString::Format("nsShaderCompileTask:%s", *Name);
}

#endif // _DEBUG



nsShaderManager::nsShaderManager() noexcept
	: bInitialized(false)
{
	ShaderNames.Reserve(64);
	ShaderFlags.Reserve(64);
	ShaderCompileTasks.Reserve(64);
	ShaderResources.Reserve(64);
}


void nsShaderManager::Initialize() noexcept
{
	if (bInitialized)
	{
		return;
	}

	// Compile default shaders
	NS_LogDebug(ShaderLog, "Compiling default shaders...");
	const nsString shaderDirPath = "../../../Shaders/";

	// vertex shaders
	AddShader("primitive_2d", "../../../Shaders/PrimitiveBatch2D.vert", VK_SHADER_STAGE_VERTEX_BIT);
	AddShader("gui", "../../../Shaders/GUI.vert", VK_SHADER_STAGE_VERTEX_BIT);
	AddShader("primitive", "../../../Shaders/PrimitiveBatch.vert", VK_SHADER_STAGE_VERTEX_BIT);
	AddShader("mesh", "../../../Shaders/Mesh.vert", VK_SHADER_STAGE_VERTEX_BIT);
	AddShader("mesh_wireframe", "../../../Shaders/MeshWireframe.vert", VK_SHADER_STAGE_VERTEX_BIT);
	AddShader("fullscreen", "../../../Shaders/Fullscreen.vert", VK_SHADER_STAGE_VERTEX_BIT);

	// fragment shaders
	AddShader("color", "../../../Shaders/Color.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
	AddShader("texture", "../../../Shaders/Texture.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
	AddShader("texture_depth", "../../../Shaders/Texture.frag", VK_SHADER_STAGE_FRAGMENT_BIT, { "TEXTURE_DEPTH" });
	AddShader("texture_color", "../../../Shaders/TextureColor.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
	AddShader("font", "../../../Shaders/TextureColor.frag", VK_SHADER_STAGE_FRAGMENT_BIT, { "FONT" });
	AddShader("forward_phong", "../../../Shaders/ForwardPhong.frag", VK_SHADER_STAGE_FRAGMENT_BIT);
	AddShader("forward_phong_checker", "../../../Shaders/ForwardPhong.frag", VK_SHADER_STAGE_FRAGMENT_BIT, { "CHECKER" });

	CompileShaders(true);

	bInitialized = true;
}


void nsShaderManager::AddShader(nsName name, const nsString& glslFile, VkShaderStageFlagBits shaderType, nsTArray<nsName> optionalCompileMacros) noexcept
{
	NS_AssertV(ShaderNames.Find(name) == NS_ARRAY_INDEX_INVALID, "Shader with name [%s] already exists!", *name);

	ShaderNames.Add(name);
	ShaderFlags.Add(Flag_Dirty);

	nsShaderCompileTask& task = ShaderCompileTasks.Add();
	task.Name = name;
	task.ShaderFile = glslFile;
	task.ShaderType = shaderType;
	task.MacroDefinitions = optionalCompileMacros;

	ShaderResources.Add(nullptr);
}


void nsShaderManager::CompileShaders(bool bWaitUntilFinished) noexcept
{
	nsTArrayInline<nsShaderCompileTask*, 32> submitTasks;

	for (int i = 0; i < ShaderFlags.GetCount(); ++i)
	{
		uint32& flags = ShaderFlags[i];

		if (flags & Flag_Dirty)
		{
			flags &= ~Flag_Dirty;
			flags |= Flag_Compiling;

			nsShaderCompileTask& task = ShaderCompileTasks[i];
			task.bCompiling = true;

			submitTasks.Add(&task);
		}
	}

	const int submitCount = submitTasks.GetCount();

	if (submitCount > 0)
	{
		nsThreadPool::SubmitTasks(reinterpret_cast<nsIThreadTask**>(submitTasks.GetData()), submitCount, bWaitUntilFinished ? nsEThreadAffinity::Thread_ALL : nsEThreadAffinity::Thread_ExcludeMain);
	}

	if (bWaitUntilFinished)
	{
		while (true)
		{
			bool bAllTasksFinished = true;

			for (int i = 0; i < ShaderCompileTasks.GetCount(); ++i)
			{
				nsShaderCompileTask& task = ShaderCompileTasks[i];

				if (task.bCompiling && !task.IsDone())
				{
					bAllTasksFinished = false;
					break;
				}
			}

			if (bAllTasksFinished)
			{
				break;
			}
		}
	}

	for (int i = 0; i < ShaderCompileTasks.GetCount(); ++i)
	{
		nsShaderCompileTask& task = ShaderCompileTasks[i];

		if (!task.bCompiling)
		{
			continue;
		}

		if (!task.IsDone())
		{
			continue;
		}

		if (task.bCompileSuccess)
		{
			nsVulkanShader*& shader = ShaderResources[i];
			nsVulkan::DestroyShaderModule(shader);
			shader = nsVulkan::CreateShaderModule(task.ShaderType, task.ShaderCodes.GetData(), sizeof(uint32) * task.ShaderCodes.GetCount(), task.Name);
			NS_LogDebug(ShaderLog, "Create shader module [%s]", *task.Name);

			task.ShaderCodes.Clear(true);
			task.bCompileSuccess = false;

			// TODO: Broadcast delegate shader module changed
		}

		task.bCompiling = false;
	}
}


nsVulkanShader* nsShaderManager::GetShaderModule(const nsName& name) const noexcept
{
	const int index = ShaderNames.Find(name);

	if (index == NS_ARRAY_INDEX_INVALID)
	{
		return nullptr;
	}

	return ShaderResources[index];
}
