#pragma once

#ifdef __NS_ENGINE_BUILD__
#define NS_ENGINE_API __declspec(dllexport)
#else
#define NS_ENGINE_API __declspec(dllimport)
#endif // __NS_ENGINE_BUILD__


#ifndef __NS_ENGINE_SHIPPING__
#define NS_ENGINE_DEBUG_DRAW
#endif // !__NS_ENGINE_SHIPPING__


// Frame buffering count
#define NS_ENGINE_FRAME_BUFFERING									(3)

// Maximum children count in transform hierarchy
#define NS_ENGINE_TRANSFORM_MAX_CHILDREN							(8)

// Maximum hits on physics raycast, sweep, overlap
#define NS_ENGINE_PHYSICS_MAX_HIT_RESULT							(8)

// Maximum mesh LODs
#define NS_ENGINE_MESH_MAX_LOD										(4)

// Maximum texture mips
#define NS_ENGINE_TEXTURE_MAX_MIP									(16)

// Maximum descriptor indexing for texture (bindless)
#define NS_ENGINE_TEXTURE_DYNAMIC_INDEXING_BINDING_COUNT			(64)

// Default texture name (white)
#define NS_ENGINE_TEXTURE_DEFAULT_WHITE_NAME						"tex_default_white"

// Default texture name (black)
#define NS_ENGINE_TEXTURE_DEFAULT_BLACK_NAME						"tex_default_black"

// Maximum material uniform buffer size in bytes
#define NS_ENGINE_MATERIAL_UNIFORM_SIZE								(256)

// Default material name (phong)
#define NS_ENGINE_MATERIAL_DEFAULT_PHONG_NAME						"mat_default_phong"

// Default material name (phong-checker)
#define NS_ENGINE_MATERIAL_DEFAULT_PHONG_CHECKER_NAME				"mat_default_phong_checker"

// Default material name (GUI)
#define NS_ENGINE_MATERIAL_DEFAULT_GUI_NAME							"mat_default_gui"

// Default material name (font)
#define NS_ENGINE_MATERIAL_DEFAULT_FONT_NAME						"mat_default_font"

// Maximum bones per skeleton
#define NS_ENGINE_ANIMATION_SKELETON_MAX_BONE						(64)

// Asset file signature (magic number) 
#define NS_ENGINE_ASSET_FILE_SIGNATURE								(0x0000734E) // Ns

// Asset file version
#define NS_ENGINE_ASSET_FILE_VERSION								(1)

// Asset file extension
#define NS_ENGINE_ASSET_FILE_EXTENSION								".nsbin"

// Maximum texture count in material asset
#define NS_ENGINE_ASSET_MATERIAL_MAX_TEXTURE						(8)

// Maximum mesh count in model asset
#define NS_ENGINE_ASSET_MODEL_MAX_MESH								(4)

// Default model name (floor)
#define NS_ENGINE_ASSET_MODEL_DEFAULT_FLOOR_NAME					"mdl_default_floor"

// Default model name (wall)
#define NS_ENGINE_ASSET_MODEL_DEFAULT_WALL_NAME						"mdl_default_wall"

// Default model name (box)
#define NS_ENGINE_ASSET_MODEL_DEFAULT_BOX_NAME						"mdl_default_box"

// Default model name (platform)
#define NS_ENGINE_ASSET_MODEL_DEFAULT_PLATFORM_NAME					"mdl_default_platform"

// Enable/disable game module hotreload
#define NS_ENGINE_GAME_MODULE_HOTRELOAD								(1)

// Vendor ID (AMD)
#define NS_VENDOR_ID_AMD											(0x1002)

// Vendor ID (NVIDIA)
#define NS_VENDOR_ID_NVIDIA											(0x10DE)

// Vendor ID (INTEL)
#define NS_VENDOR_ID_INTEL											(0x8086)

constexpr const char* ns_VendorName(int vendorId)
{
	if (vendorId == NS_VENDOR_ID_AMD) return "AMD";
	if (vendorId == NS_VENDOR_ID_NVIDIA) return "NVIDIA";
	if (vendorId == NS_VENDOR_ID_INTEL) return "Intel";

	return "Others";
}



#define NS_ENGINE_DECLARE_HANDLE(type, managerClass)										\
class type																					\
{																							\
private:																					\
	int Id;																					\
public:																						\
	static NS_ENGINE_API type INVALID;														\
private:																					\
	type(int id) noexcept : Id(id) {}														\
public:																						\
	type() noexcept : Id(-1) {}																\
	NS_NODISCARD_INLINE bool IsValid() const noexcept { return Id != -1; }					\
	NS_NODISCARD_INLINE int GetId() const noexcept { return Id; }							\
	NS_NODISCARD_INLINE uint64 GetHash() const noexcept { return static_cast<uint64>(Id); } \
	NS_INLINE bool operator==(const type& rhs) const noexcept { return Id == rhs.Id; }		\
	NS_INLINE bool operator!=(const type& rhs) const noexcept { return Id != rhs.Id; }		\
	NS_INLINE bool operator<(const type& rhs) const noexcept { return Id < rhs.Id; }		\
	NS_INLINE bool operator>(const type& rhs) const noexcept { return Id > rhs.Id; }		\
	friend class managerClass;																\
};

#define NS_ENGINE_DEFINE_HANDLE(type) type type::INVALID



#include "nsMemory.h"
#include "nsMath.h"
#include "nsLogger.h"
#include "nsDelegate.h"
#include "nsThreadPool.h"
#include "nsStream.h"
#include "nsObject.h"


#ifdef NS_PLATFORM_WINDOWS
#define NS_VK_MODULE_NAME			"vulkan-1.dll"
#define VK_USE_PLATFORM_WIN32_KHR
#endif // NS_PLATFORM_WINDOWS

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vk_mem_alloc.h>


NS_ENGINE_DECLARE_HANDLE(nsMeshID, nsMeshManager)
NS_ENGINE_DECLARE_HANDLE(nsTextureID, nsTextureManager)
NS_ENGINE_DECLARE_HANDLE(nsMaterialID, nsMaterialManager)
NS_ENGINE_DECLARE_HANDLE(nsAnimationSkeletonID, nsAnimationManager)
NS_ENGINE_DECLARE_HANDLE(nsAnimationClipID, nsAnimationManager)
NS_ENGINE_DECLARE_HANDLE(nsAnimationInstanceID, nsAnimationManager)
NS_ENGINE_DECLARE_HANDLE(nsRenderMeshID, nsRenderContextWorld)
NS_ENGINE_DECLARE_HANDLE(nsRenderPointLightID, nsRenderContextWorld)


NS_NODISCARD_INLINE uint64 ns_GetHash(nsMaterialID material) noexcept
{
	return material.GetHash();
}



class nsViewport;
class nsActor;
class nsLevel;
class nsWorld;
class nsActorComponent;
class nsTransformComponent;



enum class nsEAxisType : uint8
{
	X_Axis = 0,
	Y_Axis,
	Z_Axis
};



enum class nsEBlendMode : uint8
{
	NONE = 0,
	TRANSPARENCY,
	ADDITIVE,
	MULTIPLY
};



struct nsVertexPrimitive2D
{
	nsVector2 Position;
	nsColor Color;
};



struct nsVertexGUI
{
	nsVector2 Position;
	nsVector2 TexCoord;
	nsColor Color;
};



struct nsVertexPrimitive
{
	nsVector4 Position;
	nsColor Color;
};



typedef nsVector3 nsVertexMeshPosition;

struct nsVertexMeshAttribute
{
	nsVector3 Normal;
	nsVector3 Tangent;
	nsVector2 TexCoord;
};


struct nsVertexMeshSkin
{
	nsVector4 Weights;
	uint32 Joints;
};




extern NS_ENGINE_API nsMemory g_EngineDefaultMemory;


template<typename T, typename...TConstructorArgs>
NS_NODISCARD_INLINE T* ns_CreateObject(TConstructorArgs&&... args) noexcept
{
	NS_Validate_IsMainThread();
	return g_EngineDefaultMemory.AllocateConstruct<T>(std::forward<TConstructorArgs>(args)...);
}


template<typename T>
NS_INLINE void ns_DestroyObject(T*& obj) noexcept
{
	NS_Validate_IsMainThread();
	g_EngineDefaultMemory.DeallocateDestruct<T>(obj);
}
