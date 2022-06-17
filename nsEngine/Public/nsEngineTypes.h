#pragma once

#ifdef __NS_ENGINE_BUILD__
#define NS_ENGINE_API __declspec(dllexport)
#else
#define NS_ENGINE_API __declspec(dllimport)
#endif // __NS_ENGINE_BUILD__


#define NS_ENGINE_FRAME_BUFFERING									(3)

#define NS_ENGINE_MESH_MAX_LODs										(4)

#define NS_ENGINE_TEXTURE_MAX_MIPs									(16)
#define NS_ENGINE_TEXTURE_DYNAMIC_INDEXING_BINDING_COUNT			(64)
#define NS_ENGINE_TEXTURE_DEFAULT_WHITE_NAME						"tex_default_white"
#define NS_ENGINE_TEXTURE_DEFAULT_BLACK_NAME						"tex_default_black"

#define NS_ENGINE_MATERIAL_UNIFORM_SIZE								(256)
#define NS_ENGINE_MATERIAL_DEFAULT_PHONG_NAME						"mat_default_phong"
#define NS_ENGINE_MATERIAL_DEFAULT_PHONG_CHECKER_NAME				"mat_default_phong_checker"
#define NS_ENGINE_MATERIAL_DEFAULT_GUI_NAME							"mat_default_gui"
#define NS_ENGINE_MATERIAL_DEFAULT_FONT_NAME						"mat_default_font"

#define NS_ENGINE_ASSET_FILE_SIGNATURE								(0x0000734E) // Ns
#define NS_ENGINE_ASSET_FILE_VERSION								(1)
#define NS_ENGINE_ASSET_FILE_EXTENSION								".nsbin"
#define NS_ENGINE_ASSET_MATERIAL_MAX_TEXTURES						(8)
#define NS_ENGINE_ASSET_MODEL_MAX_MESHES							(4)
#define NS_ENGINE_ASSET_MODEL_DEFAULT_FLOOR_NAME					"mdl_default_floor"
#define NS_ENGINE_ASSET_MODEL_DEFAULT_WALL_NAME						"mdl_default_wall"
#define NS_ENGINE_ASSET_MODEL_DEFAULT_BOX_NAME						"mdl_default_box"
#define NS_ENGINE_GAME_MODULE_HOTRELOAD								(1)


#define NS_VENDOR_ID_AMD				(0x1002)
#define NS_VENDOR_ID_NVIDIA				(0x10DE)
#define NS_VENDOR_ID_INTEL				(0x8086)

constexpr const char* ns_VendorName(int vendorId)
{
	if (vendorId == NS_VENDOR_ID_AMD) return "AMD";
	if (vendorId == NS_VENDOR_ID_NVIDIA) return "NVIDIA";
	if (vendorId == NS_VENDOR_ID_INTEL) return "Intel";

	return "Others";
}


#define NS_ENGINE_DECLARE_HANDLE(type, managerClass) \
class type \
{ \
private: \
	int Id; \
public: \
	static NS_ENGINE_API type INVALID; \
private: \
	type(int id) noexcept : Id(id) {} \
public: \
	type() noexcept : Id(-1) {} \
	NS_NODISCARD_INLINE bool IsValid() const noexcept { return Id != -1; } \
	NS_NODISCARD_INLINE uint64 GetHash() const noexcept { return static_cast<uint64>(Id); } \
	NS_INLINE bool operator==(const type& rhs) const noexcept { return Id == rhs.Id; } \
	NS_INLINE bool operator!=(const type& rhs) const noexcept { return Id != rhs.Id; } \
	NS_INLINE bool operator<(const type& rhs) const noexcept { return Id < rhs.Id; } \
	NS_INLINE bool operator>(const type& rhs) const noexcept { return Id > rhs.Id; } \
	friend class managerClass; \
};

#define NS_ENGINE_DEFINE_HANDLE(type) type type::INVALID



#include "nsMemory.h"
#include "nsMath.h"
#include "nsLogger.h"
#include "nsDelegate.h"
#include "nsThreadPool.h"
#include "nsStream.h"


#ifdef NS_PLATFORM_WINDOWS
#define NS_VK_MODULE_NAME			"vulkan-1.dll"
#define VK_USE_PLATFORM_WIN32_KHR
#endif // NS_PLATFORM_WINDOWS

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vk_mem_alloc.h>



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
	float BoneWeights[4];
	int BoneIds[4];
	int BoneCount;
};



namespace nsERenderPass
{
	enum Flag
	{
		None			= (0),
		Depth			= (1 << 0),
		Shadow			= (1 << 1),
		Forward			= (1 << 2),
		Transparency	= (1 << 3),
		Final			= (1 << 4),
	};
};

typedef uint8 nsRenderPassFlags;




extern NS_ENGINE_API nsMemory g_EngineDefaultMemory;


template<typename T, typename...TConstructorArgs>
NS_NODISCARD_INLINE T* ns_CreateObject(TConstructorArgs&&... args) noexcept
{
	return g_EngineDefaultMemory.AllocateConstruct<T>(std::forward<TConstructorArgs>(args)...);
}


template<typename T>
NS_INLINE void ns_DestroyObject(T*& obj) noexcept
{
	g_EngineDefaultMemory.DeallocateDestruct<T>(obj);
}
