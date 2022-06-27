#pragma once

#ifdef __NS_ENGINE_BUILD__
#define NS_ENGINE_API __declspec(dllexport)
#else
#define NS_ENGINE_API __declspec(dllimport)
#endif // __NS_ENGINE_BUILD__


#define NS_ENGINE_FRAME_BUFFERING									(3)

#define NS_ENGINE_PHYSICS_MAX_HIT_RESULT							(8)

#define NS_ENGINE_MESH_MAX_LOD										(4)

#define NS_ENGINE_TEXTURE_MAX_MIP									(16)
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
#define NS_ENGINE_ASSET_MATERIAL_MAX_TEXTURE						(8)
#define NS_ENGINE_ASSET_MODEL_MAX_MESH								(4)
#define NS_ENGINE_ASSET_MODEL_DEFAULT_FLOOR_NAME					"mdl_default_floor"
#define NS_ENGINE_ASSET_MODEL_DEFAULT_WALL_NAME						"mdl_default_wall"
#define NS_ENGINE_ASSET_MODEL_DEFAULT_BOX_NAME						"mdl_default_box"
#define NS_ENGINE_ASSET_MODEL_DEFAULT_PLATFORM_NAME					"mdl_default_platform"

#define NS_ENGINE_TRANSFORM_MAX_CHILDREN							(8)

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
NS_ENGINE_DECLARE_HANDLE(nsSkeletonID, nsAnimationManager)
NS_ENGINE_DECLARE_HANDLE(nsAnimationSequenceID, nsAnimationManager)
NS_ENGINE_DECLARE_HANDLE(nsRenderContextMeshID, nsRenderContextWorld)
NS_ENGINE_DECLARE_HANDLE(nsRenderContextPointLightID, nsRenderContextWorld)
NS_ENGINE_DECLARE_HANDLE(nsMaterialID, nsMaterialManager)


NS_NODISCARD_INLINE uint64 ns_GetHash(nsMaterialID material) noexcept
{
	return material.GetHash();
}



namespace physx
{
	class PxPhysics;
	class PxMaterial;
	class PxCooking;
	class PxScene;
	class PxRigidActor;
	class PxShape;
};


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



enum class nsEPhysicsShape : uint8
{
	NONE = 0,
	BOX,
	SPHERE,
	CAPSULE,
	CONVEX_MESH,
	TRIANGLE_MESH,
	HEIGHTFIELD
};



namespace nsEPhysicsCollisionChannel
{
	enum Type : uint32
	{
		NONE			= (0),
		Default			= (1 << 0),
		Character		= (1 << 1),
		Camera			= (1 << 2),
		MousePicking	= (1 << 3),
	};
};

typedef uint32 nsPhysicsCollisionChannels;



enum class nsEPhysicsCollisionTest : uint8
{
	NONE = 0,
	COLLISION_ONLY,
	QUERY_ONLY,
	COLLISION_AND_QUERY
};


typedef nsTArrayInline<nsActor*, 8> nsPhysicsQueryIgnoredActors;


struct nsPhysicsQueryParams
{
	nsEPhysicsCollisionChannel::Type Channel;
	nsPhysicsQueryIgnoredActors IgnoredActors;


public:
	nsPhysicsQueryParams()
		: Channel(nsEPhysicsCollisionChannel::Default)
		, IgnoredActors()
	{
	}

};



struct nsPhysicsHitResult
{
	nsActor* Actor;
	nsActorComponent* Component;
	nsVector3 WorldPosition;
	nsVector3 WorldNormal;
	float Distance;
	bool bIsBlock;


public:
	nsPhysicsHitResult()
		: Actor(nullptr)
		, Component(nullptr)
		, WorldPosition()
		, WorldNormal()
		, Distance(0.0f)
		, bIsBlock(false)
	{
	}

};

typedef nsTArrayInline<nsPhysicsHitResult, NS_ENGINE_PHYSICS_MAX_HIT_RESULT> nsPhysicsHitResultMany;



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
	NS_Validate_IsMainThread();
	return g_EngineDefaultMemory.AllocateConstruct<T>(std::forward<TConstructorArgs>(args)...);
}


template<typename T>
NS_INLINE void ns_DestroyObject(T*& obj) noexcept
{
	NS_Validate_IsMainThread();
	g_EngineDefaultMemory.DeallocateDestruct<T>(obj);
}
