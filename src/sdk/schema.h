#pragma once

#include <cstdint>
#include <utldelegate.h>
#include <entityinstance.h>

struct SchemaKey {
	int32_t offset;
	bool networked;
};

struct CNetworkVarChainer {
public:
	CEntityInstance* GetObj() {
		return m_pObj;
	}

private:
	CEntityInstance* m_pObj;

public:
	struct ChainUpdatePropagationLL_t {
		ChainUpdatePropagationLL_t* pNext;
		CUtlDelegate<void(const CNetworkVarChainer&)> updateDelegate;
	};

	uint8_t unk[24];
	ChangeAccessorFieldPathIndex_t m_PathIndex;
	ChainUpdatePropagationLL_t* m_pPropagationList;
};

static_assert(offsetof(CNetworkVarChainer, m_PathIndex) == 32);

namespace schema {
	int16_t FindChainOffset(const char* className);
	SchemaKey GetOffset(const char* className, const char* memberName);
	SchemaKey GetOffset(const char* className, uint32_t classKey, const char* memberName, uint32_t memberKey);
	void NetworkStateChanged(int64 chainEntity, uint32 nLocalOffset, uint32 nArrayIndex);
	size_t GetClassSize(const char* className);
} // namespace schema

constexpr uint32_t val_32_const = 0x811c9dc5;
constexpr uint32_t prime_32_const = 0x1000193;
constexpr uint64_t val_64_const = 0xcbf29ce484222325;
constexpr uint64_t prime_64_const = 0x100000001b3;

inline constexpr uint32_t hash_32_fnv1a_const(const char* const str, const uint32_t value = val_32_const) noexcept {
	return (str[0] == '\0') ? value : hash_32_fnv1a_const(&str[1], (value ^ uint32_t(str[0])) * prime_32_const);
}

inline constexpr uint64_t hash_64_fnv1a_const(const char* const str, const uint64_t value = val_64_const) noexcept {
	return (str[0] == '\0') ? value : hash_64_fnv1a_const(&str[1], (value ^ uint64_t(str[0])) * prime_64_const);
}

#define SCHEMA_GETTER(type, varName, varOffset) \
	{ \
		if (varOffset == 0) { \
			static constexpr auto datatable_hash = hash_32_fnv1a_const(ThisClassName); \
			static constexpr auto prop_hash = hash_32_fnv1a_const(#varName); \
			static const auto m_key = schema::GetOffset(ThisClassName, datatable_hash, #varName, prop_hash); \
			return *reinterpret_cast<std::add_pointer_t<type>>((uintptr_t)(this) + m_key.offset); \
		} \
		return *reinterpret_cast<std::add_pointer_t<type>>((uintptr_t)(this) + varOffset); \
	}

#define SCHEMA_SETTER(type, varName, varOffset, arrayIndex) \
	{ \
		if (varOffset == 0) { \
			static constexpr auto datatable_hash = hash_32_fnv1a_const(ThisClassName); \
			static constexpr auto prop_hash = hash_32_fnv1a_const(#varName); \
			static const auto m_key = schema::GetOffset(ThisClassName, datatable_hash, #varName, prop_hash); \
			static const auto m_chain = schema::FindChainOffset(ThisClassName); \
			if (m_key.networked) { \
				if (m_chain != 0) { \
					schema::NetworkStateChanged((uintptr_t)(this) + m_chain, m_key.offset, arrayIndex); \
				} else { \
					if (!IsStruct) { \
						((CEntityInstance*)this)->NetworkStateChanged(m_key.offset, arrayIndex, -1); \
					} else { \
					} \
				} \
			} \
			*reinterpret_cast<std::add_pointer_t<type>>((uintptr_t)(this) + m_key.offset) = const_cast<type&>(val); \
		} else { \
			*reinterpret_cast<std::add_pointer_t<type>>((uintptr_t)(this) + varOffset) = const_cast<type&>(val); \
		} \
	}

#define SCHEMA_FIELD_SETTER_IS_NOT_ENTITY(type, varName, varOffset) \
	void varName(type val, uint16_t arrayIndex = 0xFFFF) { \
		if (varOffset == 0) { \
			static constexpr auto datatable_hash = hash_32_fnv1a_const(ThisClassName); \
			static constexpr auto prop_hash = hash_32_fnv1a_const(#varName); \
			static const auto m_key = schema::GetOffset(ThisClassName, datatable_hash, #varName, prop_hash); \
			static const auto m_chain = schema::FindChainOffset(ThisClassName); \
			if (m_key.networked) { \
				if (m_chain != 0) { \
					schema::NetworkStateChanged((uintptr_t)(this) + m_chain, m_key.offset, arrayIndex); \
				} \
			} \
			*reinterpret_cast<std::add_pointer_t<type>>((uintptr_t)(this) + m_key.offset) = val; \
		} else { \
			*reinterpret_cast<std::add_pointer_t<type>>((uintptr_t)(this) + varOffset) = val; \
		} \
	}

#define SCHEMA_FIELD_GETTER(type, varName, varOffset) std::add_lvalue_reference_t<type> varName() SCHEMA_GETTER(type, varName, varOffset) std::add_lvalue_reference_t<type> varName() const SCHEMA_GETTER(type, varName, varOffset)

#define SCHEMA_FIELD_SETTER(type, varName, varOffset) void varName(type&& val, uint16_t arrayIndex = 0xFFFF) SCHEMA_SETTER(SCHEMA_VA(type), varName, varOffset, arrayIndex) void varName(const type& val, uint16_t arrayIndex = 0xFFFF) SCHEMA_SETTER(SCHEMA_VA(type), varName, varOffset, arrayIndex)

#define SCHEMA_FIELD_POINTER_GETTER(type, varName, varOffset) \
	type* varName() { \
		if (varOffset == 0) { \
			static constexpr auto datatable_hash = hash_32_fnv1a_const(ThisClassName); \
			static constexpr auto prop_hash = hash_32_fnv1a_const(#varName); \
			static const auto m_key = schema::GetOffset(ThisClassName, datatable_hash, #varName, prop_hash); \
			return reinterpret_cast<std::add_pointer_t<type>>((uintptr_t)(this) + m_key.offset); \
		} \
		return reinterpret_cast<std::add_pointer_t<type>>((uintptr_t)(this) + varOffset); \
	}

#define SCHEMA_FIELD_POINTER_SETTER(type, varName, varOffset) SCHEMA_FIELD_SETTER(SCHEMA_VA(type), varName, varOffset)

#define SCHEMA_FIELD_POINTER_ENTITY_SETTER(type, varName) \
	void varName(CEntityInstance* pEntity) { \
		type hdl; \
		if (pEntity) \
			hdl = pEntity->GetRefHandle(); \
		varName(hdl); \
	}

#define SCHEMA_DATAMAP_FUNC_SETTER(varName) \
	template<typename T> \
	void varName(T fn) { \
		static constexpr auto datatable_hash = hash_32_fnv1a_const(ThisClassName); \
		static constexpr auto prop_hash = hash_32_fnv1a_const(#varName); \
		static const auto m_key = schema::GetOffset(ThisClassName, datatable_hash, #varName, prop_hash); \
		*reinterpret_cast<std::add_pointer_t<T>>((uintptr_t)(this) + m_key.offset) = fn; \
	}

#define SCHEMA_VA(...) __VA_ARGS__

// Use this when you want the member's value itself, the type must be fully reversed.
#define SCHEMA_FIELD(type, varName) \
	SCHEMA_FIELD_GETTER(SCHEMA_VA(type), varName, 0) \
	SCHEMA_FIELD_SETTER(SCHEMA_VA(type), varName, 0)

#define SCHEMA_FIELD_IS_NOT_ENTITY(type, varName) \
	SCHEMA_FIELD_GETTER(SCHEMA_VA(type), varName, 0) \
	SCHEMA_FIELD_SETTER_IS_NOT_ENTITY(SCHEMA_VA(type), varName, 0)

// See SCHEMA_FIELD.
// Support custom offset and custom name.
#define SCHEMA_FIELD_CUSTOM(type, varName, offset) \
	SCHEMA_FIELD_GETTER(type, varName, offset) \
	SCHEMA_FIELD_SETTER(type, varName, offset)

// Reversed SCHEMA_FIELD.
// Support template with 2+ args
// e.g: SCHEMA_FIELD(m_perRoundStats, CUtlVectorEmbeddedNetworkVar<CSPerRoundStats_t, true>);
#define SCHEMA_FIELD_TEMPLATE(varName, ...) SCHEMA_FIELD(SCHEMA_VA(__VA_ARGS__), varName)

// Use this when you want a pointer to a member, especially the type is not fully reversed(like CNetworkOriginCellCoordQuantizedVector, CUtlVector...)
#define SCHEMA_FIELD_POINTER(type, varName) \
	SCHEMA_FIELD_POINTER_GETTER(SCHEMA_VA(type), varName, 0) \
	SCHEMA_FIELD_POINTER_SETTER(SCHEMA_VA(type), varName, 0)

// See SCHEMA_FIELD_POINTER.
// Support custom offset and custom name.
#define SCHEMA_FIELD_POINTER_CUSTOM(type, varName, offset) \
	SCHEMA_FIELD_POINTER_GETTER(type, varName, offset) \
	SCHEMA_FIELD_POINTER_SETTER(type, varName, offset)

// Reversed SCHEMA_FIELD_POINTER.
// Support template with 2+ args
// e.g: SCHEMA_FIELD_POINTER_TEMPLATE(m_perRoundStats, CUtlVectorEmbeddedNetworkVar<CSPerRoundStats_t, true>);
#define SCHEMA_FIELD_POINTER_TEMPLATE(varName, ...) SCHEMA_FIELD_POINTER(SCHEMA_VA(__VA_ARGS__), varName)

// Use this when the type is CHandle<CBaseEntity>.
#define SCHEMA_FIELD_POINTER_ENTITY(type, varName) \
	SCHEMA_FIELD_POINTER(type, varName) \
	SCHEMA_FIELD_POINTER_ENTITY_SETTER(type, varName)

#define SCHEMA_DATAMAP_FUNC(varName) \
	SCHEMA_DATAMAP_FUNC_SETTER(varName) \
	SCHEMA_FIELD_GETTER(void*, varName, 0)

#define DECLARE_SCHEMA_CLASS_BASE(className, schemaName, isStruct) \
	typedef className ThisClass; \
	static constexpr const char* ThisClassName = #schemaName; \
	static constexpr bool IsStruct = isStruct; \
	static size_t GetClassSize() { \
		return schema::GetClassSize(ThisClassName); \
	}

#define DECLARE_SCHEMA_CLASS(className) DECLARE_SCHEMA_CLASS_BASE(className, className, false)

#define DECLARE_SCHEMA_CLASS_CUSTOM(className, schemaName) DECLARE_SCHEMA_CLASS_BASE(className, schemaName, false)

// Use this if it not extends CEntityInstance, or not contains __m_pChainEntity.
#define DECLARE_SCHEMA_STRUCT(className) DECLARE_SCHEMA_CLASS_BASE(className, className, true)

// See DECLARE_SCHEMA_STRUCT
#define DECLARE_SCHEMA_CLASS_INLINE(className) DECLARE_SCHEMA_STRUCT(className)
