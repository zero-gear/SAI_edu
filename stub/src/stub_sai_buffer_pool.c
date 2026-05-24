#include "sai.h"
#include "assert.h"
#include "stub_sai.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

static const char *stub_buffer_pool_type_str(sai_buffer_pool_type_t type)
{
    switch (type) {
    case SAI_BUFFER_POOL_INGRESS:
        return "ingress";
    case SAI_BUFFER_POOL_EGRESS:
        return "egress";
    default:
        return "unknown";
    }
}

static const char *stub_buffer_th_mode_str(sai_buffer_threshold_mode_t mode)
{
    switch (mode) {
    case SAI_BUFFER_THRESHOLD_MODE_STATIC:
        return "static";
    case SAI_BUFFER_THRESHOLD_MODE_DYNAMIC:
        return "dynamic";
    default:
        return "unknown";
    }
}

#define MAX_NUMBER_OF_BUFFER_POOLS 4
#define MAX_NUMBER_OF_BUFFER_PROFILES 8

typedef struct _buffer_pool_db_entry_t {
    bool                       is_used;
    sai_object_id_t            buffer_pool_oid;
    sai_buffer_pool_type_t     type;
    sai_uint32_t               size;
    sai_buffer_threshold_mode_t th_mode;
} buffer_pool_db_entry_t;

typedef struct _buffer_profile_db_entry_t {
    bool            is_used;
    sai_object_id_t buffer_profile_oid;
    sai_object_id_t pool_oid;
    sai_uint32_t    buffer_size;
    sai_int8_t      shared_dynamic_th;
    sai_uint32_t    shared_static_th;
} buffer_profile_db_entry_t;

struct buffer_pool_db_t {
    buffer_pool_db_entry_t pools[MAX_NUMBER_OF_BUFFER_POOLS];
    buffer_profile_db_entry_t profiles[MAX_NUMBER_OF_BUFFER_PROFILES];
} buffer_pool_db;

static const sai_attribute_entry_t buffer_pool_attribs[] = {
    { SAI_BUFFER_POOL_ATTR_TYPE, true, true, false, true,
      "Pool type", SAI_ATTR_VAL_TYPE_S32 },
    { SAI_BUFFER_POOL_ATTR_SIZE, true, true, true, true,
      "Pool size", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_BUFFER_POOL_ATTR_TH_MODE, false, true, false, true,
      "Threshold mode", SAI_ATTR_VAL_TYPE_S32 },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

sai_status_t get_buffer_pool_attribute(_In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg)
{
    sai_status_t status;
    uint32_t     pool_db_id;

    (void)attr_index;
    (void)cache;

    assert(SAI_BUFFER_POOL_ATTR_TYPE == (int64_t)arg ||
           SAI_BUFFER_POOL_ATTR_SIZE == (int64_t)arg ||
           SAI_BUFFER_POOL_ATTR_TH_MODE == (int64_t)arg);

    status = stub_object_to_type(key->object_id, SAI_OBJECT_TYPE_BUFFER_POOL, &pool_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get buffer pool DB index.\n");
        return status;
    }
    if (pool_db_id >= MAX_NUMBER_OF_BUFFER_POOLS || !buffer_pool_db.pools[pool_db_id].is_used) {
        printf("Cannot get buffer pool attribute: invalid or unused pool\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch ((int64_t)arg) {
    case SAI_BUFFER_POOL_ATTR_TYPE:
        value->s32 = buffer_pool_db.pools[pool_db_id].type;
        break;
    case SAI_BUFFER_POOL_ATTR_SIZE:
        value->u32 = buffer_pool_db.pools[pool_db_id].size;
        break;
    case SAI_BUFFER_POOL_ATTR_TH_MODE:
        value->s32 = buffer_pool_db.pools[pool_db_id].th_mode;
        break;
    default:
        printf("Got unexpected buffer pool attribute ID\n");
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static const sai_vendor_attribute_entry_t buffer_pool_vendor_attribs[] = {
    { SAI_BUFFER_POOL_ATTR_TYPE,
      { true, false, false, true },
      { true, false, false, true },
      get_buffer_pool_attribute, (void*) SAI_BUFFER_POOL_ATTR_TYPE,
      NULL, NULL },
    { SAI_BUFFER_POOL_ATTR_SIZE,
      { true, false, true, true },
      { true, false, true, true },
      get_buffer_pool_attribute, (void*) SAI_BUFFER_POOL_ATTR_SIZE,
      NULL, NULL },
    { SAI_BUFFER_POOL_ATTR_TH_MODE,
      { true, false, false, true },
      { true, false, false, true },
      get_buffer_pool_attribute, (void*) SAI_BUFFER_POOL_ATTR_TH_MODE,
      NULL, NULL },
};

// Buffer pool API
sai_status_t stub_create_buffer_pool(
    _Out_ sai_object_id_t* buffer_pool_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
) {
    sai_status_t status;
    uint32_t idx;
    uint32_t pool_db_id;
    const sai_attribute_value_t *type_attr;
    const sai_attribute_value_t *size_attr;
    const sai_attribute_value_t *th_mode_attr;
    uint32_t type_index;
    uint32_t size_index;
    uint32_t th_mode_index;

    if (NULL == buffer_pool_id) {
        printf("Cannot create buffer pool: NULL buffer_pool_id\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = check_attribs_metadata(attr_count, attr_list, buffer_pool_attribs,
                                    buffer_pool_vendor_attribs, SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed buffer pool attributes check\n");
        return status;
    }

    assert(SAI_STATUS_SUCCESS ==
           find_attrib_in_list(attr_count, attr_list, SAI_BUFFER_POOL_ATTR_TYPE,
                               &type_attr, &type_index));
    assert(SAI_STATUS_SUCCESS ==
           find_attrib_in_list(attr_count, attr_list, SAI_BUFFER_POOL_ATTR_SIZE,
                               &size_attr, &size_index));

    if (type_attr->s32 != SAI_BUFFER_POOL_INGRESS &&
        type_attr->s32 != SAI_BUFFER_POOL_EGRESS) {
        printf("Invalid buffer pool type %d\n", type_attr->s32);
        return SAI_STATUS_INVALID_ATTR_VALUE_0 + type_index;
    }

    if (SAI_STATUS_SUCCESS ==
        find_attrib_in_list(attr_count, attr_list, SAI_BUFFER_POOL_ATTR_TH_MODE,
                            &th_mode_attr, &th_mode_index)) {
        if (th_mode_attr->s32 != SAI_BUFFER_THRESHOLD_MODE_STATIC &&
            th_mode_attr->s32 != SAI_BUFFER_THRESHOLD_MODE_DYNAMIC) {
            printf("Invalid buffer pool threshold mode %d\n", th_mode_attr->s32);
            return SAI_STATUS_INVALID_ATTR_VALUE_0 + th_mode_index;
        }
    }

    for (idx = 0; idx < MAX_NUMBER_OF_BUFFER_POOLS; idx++) {
        if (!buffer_pool_db.pools[idx].is_used) {
            break;
        }
    }
    if (idx == MAX_NUMBER_OF_BUFFER_POOLS) {
        printf("Cannot create buffer pool: limit is reached\n");
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    pool_db_id = idx;

    buffer_pool_db.pools[pool_db_id].is_used = true;
    buffer_pool_db.pools[pool_db_id].type = (sai_buffer_pool_type_t)type_attr->s32;
    buffer_pool_db.pools[pool_db_id].size = size_attr->u32;
    buffer_pool_db.pools[pool_db_id].th_mode = SAI_BUFFER_THRESHOLD_MODE_DYNAMIC;

    if (SAI_STATUS_SUCCESS ==
        find_attrib_in_list(attr_count, attr_list, SAI_BUFFER_POOL_ATTR_TH_MODE,
                            &th_mode_attr, &th_mode_index)) {
        buffer_pool_db.pools[pool_db_id].th_mode = (sai_buffer_threshold_mode_t)th_mode_attr->s32;
    }

    status = stub_create_object(SAI_OBJECT_TYPE_BUFFER_POOL, pool_db_id, buffer_pool_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create buffer pool OID\n");
        memset(&buffer_pool_db.pools[pool_db_id], 0, sizeof(buffer_pool_db.pools[pool_db_id]));
        return status;
    }

    buffer_pool_db.pools[pool_db_id].buffer_pool_oid = *buffer_pool_id;

    printf("[STUB|new] BUFFER POOL[%u]  oid=0x%016" PRIx64 "  %s  size=%u  th_mode=%s\n",
           pool_db_id, (uint64_t)*buffer_pool_id,
           stub_buffer_pool_type_str(buffer_pool_db.pools[pool_db_id].type),
           buffer_pool_db.pools[pool_db_id].size,
           stub_buffer_th_mode_str(buffer_pool_db.pools[pool_db_id].th_mode));

    return SAI_STATUS_SUCCESS;
}
sai_status_t stub_remove_buffer_pool(
    _In_ sai_object_id_t buffer_pool_id
) {
    sai_status_t status;
    uint32_t pool_db_id;
    sai_object_id_t id = buffer_pool_id;

    status = stub_object_to_type(buffer_pool_id, SAI_OBJECT_TYPE_BUFFER_POOL, &pool_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }
    if (pool_db_id >= MAX_NUMBER_OF_BUFFER_POOLS || !buffer_pool_db.pools[pool_db_id].is_used) {
        printf("Cannot remove buffer pool: invalid or not allocated pool id\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t i = 0; i < MAX_NUMBER_OF_BUFFER_PROFILES; i++) {
        if (buffer_pool_db.profiles[i].is_used &&
            buffer_pool_db.profiles[i].pool_oid == buffer_pool_id) {
            printf("Cannot remove buffer pool: has active profiles\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    status = stub_destroy_object(SAI_OBJECT_TYPE_BUFFER_POOL, &id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }

    memset(&buffer_pool_db.pools[pool_db_id], 0, sizeof(buffer_pool_db.pools[pool_db_id]));
    printf("[STUB|del] BUFFER POOL[%u]  oid=0x%016" PRIx64 "\n",
           pool_db_id, (uint64_t)buffer_pool_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_buffer_pool_attribute(
    _In_ sai_object_id_t buffer_pool_id,
    _In_ const sai_attribute_t *attr
) {
    return SAI_STATUS_SUCCESS;
}
sai_status_t stub_get_buffer_pool_attribute(
    _In_ sai_object_id_t buffer_pool_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
) {
    const sai_object_key_t key = { .object_id = buffer_pool_id };
    return sai_get_attributes(&key, NULL, buffer_pool_attribs, buffer_pool_vendor_attribs,
                              attr_count, attr_list);
}
sai_status_t stub_get_buffer_pool_stats(
    _In_ sai_object_id_t pool_id,
    _In_ const sai_buffer_pool_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters,
    _Out_ uint64_t* counters
) {
    return SAI_STATUS_SUCCESS;
}

// Ingress priority group API
sai_status_t stub_set_ingress_priority_group_attribute(
    _In_ sai_object_id_t ingress_priority_group_id,
    _In_ const sai_attribute_t *attr
) {
    return SAI_STATUS_SUCCESS;
}
sai_status_t stub_get_ingress_priority_group_attribute(
    _In_ sai_object_id_t ingress_priority_group_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
) {
    return SAI_STATUS_SUCCESS;
}
sai_status_t stub_get_ingress_priority_group_stats(
    _In_ sai_object_id_t ingress_pg_id,
    _In_ const sai_ingress_priority_group_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters,
    _Out_ uint64_t* counters
) {
    return SAI_STATUS_SUCCESS;
}
sai_status_t stub_clear_ingress_priority_group_stats(
    _In_ sai_object_id_t ingress_pg_id,
    _In_ const sai_ingress_priority_group_stat_counter_t *counter_ids,
    _In_ uint32_t number_of_counters
) {
    return SAI_STATUS_SUCCESS;
}

static const sai_attribute_entry_t buffer_profile_attribs[] = {
    { SAI_BUFFER_PROFILE_ATTR_POOL_ID, true, true, true, true,
      "Pool ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE, true, true, true, true,
      "Buffer size", SAI_ATTR_VAL_TYPE_U32 },
    { SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH, false, true, true, true,
      "Shared dynamic threshold", SAI_ATTR_VAL_TYPE_S8 },
    { SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH, false, true, true, true,
      "Shared static threshold", SAI_ATTR_VAL_TYPE_U32 },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

sai_status_t get_buffer_profile_attribute(_In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg)
{
    sai_status_t status;
    uint32_t     profile_db_id;

    (void)attr_index;
    (void)cache;

    assert(SAI_BUFFER_PROFILE_ATTR_POOL_ID == (int64_t)arg ||
           SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE == (int64_t)arg ||
           SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH == (int64_t)arg ||
           SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH == (int64_t)arg);

    status = stub_object_to_type(key->object_id, SAI_OBJECT_TYPE_BUFFER_PROFILE, &profile_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get buffer profile DB index.\n");
        return status;
    }
    if (profile_db_id >= MAX_NUMBER_OF_BUFFER_PROFILES ||
        !buffer_pool_db.profiles[profile_db_id].is_used) {
        printf("Cannot get buffer profile attribute: invalid or unused profile\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch ((int64_t)arg) {
    case SAI_BUFFER_PROFILE_ATTR_POOL_ID:
        value->oid = buffer_pool_db.profiles[profile_db_id].pool_oid;
        break;
    case SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE:
        value->u32 = buffer_pool_db.profiles[profile_db_id].buffer_size;
        break;
    case SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH:
        value->s8 = buffer_pool_db.profiles[profile_db_id].shared_dynamic_th;
        break;
    case SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH:
        value->u32 = buffer_pool_db.profiles[profile_db_id].shared_static_th;
        break;
    default:
        printf("Got unexpected buffer profile attribute ID\n");
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static const sai_vendor_attribute_entry_t buffer_profile_vendor_attribs[] = {
    { SAI_BUFFER_PROFILE_ATTR_POOL_ID,
      { true, false, true, true },
      { true, false, true, true },
      get_buffer_profile_attribute, (void*) SAI_BUFFER_PROFILE_ATTR_POOL_ID,
      NULL, NULL },
    { SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE,
      { true, false, true, true },
      { true, false, true, true },
      get_buffer_profile_attribute, (void*) SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE,
      NULL, NULL },
    { SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH,
      { true, false, true, true },
      { true, false, true, true },
      get_buffer_profile_attribute, (void*) SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH,
      NULL, NULL },
    { SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH,
      { true, false, true, true },
      { true, false, true, true },
      get_buffer_profile_attribute, (void*) SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH,
      NULL, NULL },
};

static sai_status_t stub_get_buffer_pool_db_id(
    _In_ sai_object_id_t pool_oid,
    _Out_ uint32_t *pool_db_id)
{
    sai_status_t status;

    if (NULL == pool_db_id) {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (pool_oid == SAI_NULL_OBJECT_ID) {
        printf("Cannot use NULL buffer pool id for profile\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = stub_object_to_type(pool_oid, SAI_OBJECT_TYPE_BUFFER_POOL, pool_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }
    if (*pool_db_id >= MAX_NUMBER_OF_BUFFER_POOLS ||
        !buffer_pool_db.pools[*pool_db_id].is_used) {
        printf("Cannot use invalid or unused buffer pool for profile\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

// Buffer profile API
sai_status_t stub_create_buffer_profile(
    _Out_ sai_object_id_t* buffer_profile_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
) {
    sai_status_t status;
    uint32_t idx;
    uint32_t profile_db_id;
    uint32_t pool_db_id;
    const sai_attribute_value_t *pool_id_attr;
    const sai_attribute_value_t *buffer_size_attr;
    const sai_attribute_value_t *dynamic_th_attr;
    const sai_attribute_value_t *static_th_attr;
    uint32_t pool_id_index;
    uint32_t buffer_size_index;
    uint32_t dynamic_th_index;
    uint32_t static_th_index;

    if (NULL == buffer_profile_id) {
        printf("Cannot create buffer profile: NULL buffer_profile_id\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = check_attribs_metadata(attr_count, attr_list, buffer_profile_attribs,
                                    buffer_profile_vendor_attribs, SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed buffer profile attributes check\n");
        return status;
    }

    assert(SAI_STATUS_SUCCESS ==
           find_attrib_in_list(attr_count, attr_list, SAI_BUFFER_PROFILE_ATTR_POOL_ID,
                               &pool_id_attr, &pool_id_index));
    assert(SAI_STATUS_SUCCESS ==
           find_attrib_in_list(attr_count, attr_list, SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE,
                               &buffer_size_attr, &buffer_size_index));

    status = stub_get_buffer_pool_db_id(pool_id_attr->oid, &pool_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }

    if (buffer_pool_db.pools[pool_db_id].th_mode == SAI_BUFFER_THRESHOLD_MODE_DYNAMIC) {
        if (SAI_STATUS_SUCCESS !=
            find_attrib_in_list(attr_count, attr_list, SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH,
                                &dynamic_th_attr, &dynamic_th_index)) {
            printf("Missing SHARED_DYNAMIC_TH for dynamic buffer pool profile\n");
            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }
    } else if (buffer_pool_db.pools[pool_db_id].th_mode == SAI_BUFFER_THRESHOLD_MODE_STATIC) {
        if (SAI_STATUS_SUCCESS !=
            find_attrib_in_list(attr_count, attr_list, SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH,
                                &static_th_attr, &static_th_index)) {
            printf("Missing SHARED_STATIC_TH for static buffer pool profile\n");
            return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
        }
    } else {
        printf("Invalid buffer pool threshold mode for profile\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (idx = 0; idx < MAX_NUMBER_OF_BUFFER_PROFILES; idx++) {
        if (!buffer_pool_db.profiles[idx].is_used) {
            break;
        }
    }
    if (idx == MAX_NUMBER_OF_BUFFER_PROFILES) {
        printf("Cannot create buffer profile: limit is reached\n");
        return SAI_STATUS_INSUFFICIENT_RESOURCES;
    }
    profile_db_id = idx;

    buffer_pool_db.profiles[profile_db_id].is_used = true;
    buffer_pool_db.profiles[profile_db_id].pool_oid = pool_id_attr->oid;
    buffer_pool_db.profiles[profile_db_id].buffer_size = buffer_size_attr->u32;
    buffer_pool_db.profiles[profile_db_id].shared_dynamic_th = 0;
    buffer_pool_db.profiles[profile_db_id].shared_static_th = 0;

    if (buffer_pool_db.pools[pool_db_id].th_mode == SAI_BUFFER_THRESHOLD_MODE_DYNAMIC) {
        buffer_pool_db.profiles[profile_db_id].shared_dynamic_th = dynamic_th_attr->s8;
    } else if (buffer_pool_db.pools[pool_db_id].th_mode == SAI_BUFFER_THRESHOLD_MODE_STATIC) {
        buffer_pool_db.profiles[profile_db_id].shared_static_th = static_th_attr->u32;
    }

    status = stub_create_object(SAI_OBJECT_TYPE_BUFFER_PROFILE, profile_db_id, buffer_profile_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create buffer profile OID\n");
        memset(&buffer_pool_db.profiles[profile_db_id], 0,
               sizeof(buffer_pool_db.profiles[profile_db_id]));
        return status;
    }

    buffer_pool_db.profiles[profile_db_id].buffer_profile_oid = *buffer_profile_id;

    if (buffer_pool_db.pools[pool_db_id].th_mode == SAI_BUFFER_THRESHOLD_MODE_DYNAMIC) {
        printf("[STUB|new] BUFFER PROFILE[%u]  oid=0x%016" PRIx64 "  pool=0x%016" PRIx64
               "  size=%u  dynamic_th=%d\n",
               profile_db_id, (uint64_t)*buffer_profile_id, (uint64_t)pool_id_attr->oid,
               buffer_pool_db.profiles[profile_db_id].buffer_size,
               buffer_pool_db.profiles[profile_db_id].shared_dynamic_th);
    } else if (buffer_pool_db.pools[pool_db_id].th_mode == SAI_BUFFER_THRESHOLD_MODE_STATIC) {
        printf("[STUB|new] BUFFER PROFILE[%u]  oid=0x%016" PRIx64 "  pool=0x%016" PRIx64
               "  size=%u  static_th=%u\n",
               profile_db_id, (uint64_t)*buffer_profile_id, (uint64_t)pool_id_attr->oid,
               buffer_pool_db.profiles[profile_db_id].buffer_size,
               buffer_pool_db.profiles[profile_db_id].shared_static_th);
    }

    return SAI_STATUS_SUCCESS;
}
sai_status_t stub_remove_buffer_profile(
    _In_ sai_object_id_t buffer_profile_id
) {
    sai_status_t status;
    uint32_t profile_db_id;
    sai_object_id_t id = buffer_profile_id;

    status = stub_object_to_type(buffer_profile_id, SAI_OBJECT_TYPE_BUFFER_PROFILE, &profile_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }
    if (profile_db_id >= MAX_NUMBER_OF_BUFFER_PROFILES ||
        !buffer_pool_db.profiles[profile_db_id].is_used) {
        printf("Cannot remove buffer profile: invalid or not allocated profile id\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = stub_destroy_object(SAI_OBJECT_TYPE_BUFFER_PROFILE, &id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }

    memset(&buffer_pool_db.profiles[profile_db_id], 0,
           sizeof(buffer_pool_db.profiles[profile_db_id]));
    printf("[STUB|del] BUFFER PROFILE[%u]  oid=0x%016" PRIx64 "\n",
           profile_db_id, (uint64_t)buffer_profile_id);
    return SAI_STATUS_SUCCESS;
}
sai_status_t stub_set_buffer_profile_attribute(
    _In_ sai_object_id_t buffer_profile_id,
    _In_ const sai_attribute_t *attr
) {
    return SAI_STATUS_SUCCESS;
}
sai_status_t stub_get_buffer_profile_attribute(
    _In_ sai_object_id_t buffer_profile_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
) {
    const sai_object_key_t key = { .object_id = buffer_profile_id };
    return sai_get_attributes(&key, NULL, buffer_profile_attribs, buffer_profile_vendor_attribs,
                              attr_count, attr_list);
}

const sai_buffer_api_t buffer_api = {
    // Buffer pool API
    stub_create_buffer_pool,
    stub_remove_buffer_pool,
    stub_set_buffer_pool_attribute,
    stub_get_buffer_pool_attribute,
    stub_get_buffer_pool_stats,
    // Ingress priority group API
    stub_set_ingress_priority_group_attribute,
    stub_get_ingress_priority_group_attribute,
    stub_get_ingress_priority_group_stats,
    stub_clear_ingress_priority_group_stats,
    // Buffer profile API
    stub_create_buffer_profile,
    stub_remove_buffer_profile,
    stub_set_buffer_profile_attribute,
    stub_get_buffer_profile_attribute
};