#include <stdio.h>
#include <inttypes.h>
#include "sai.h"

#define TEST_SECTION(title) printf("\n========== %s ==========\n", title)
#define OID_FMT "0x%016" PRIx64
#define MAX_BUFFER_PROFILES 8

static const char *pool_type_name(sai_buffer_pool_type_t type)
{
    switch (type) {
    case SAI_BUFFER_POOL_INGRESS:
        return "ingress";
    case SAI_BUFFER_POOL_EGRESS:
        return "egress";
    default:
        return "?";
    }
}

static const char *th_mode_name(sai_buffer_threshold_mode_t mode)
{
    switch (mode) {
    case SAI_BUFFER_THRESHOLD_MODE_STATIC:
        return "static";
    case SAI_BUFFER_THRESHOLD_MODE_DYNAMIC:
        return "dynamic";
    default:
        return "?";
    }
}

const char* test_profile_get_value(
    _In_ sai_switch_profile_id_t profile_id,
    _In_ const char* variable)
{
    return 0;
}

int test_profile_get_next_value(
    _In_ sai_switch_profile_id_t profile_id,
    _Out_ const char** variable,
    _Out_ const char** value)
{
    return -1;
}

const service_method_table_t test_services = {
    test_profile_get_value,
    test_profile_get_next_value
};

int main()
{
    sai_status_t status;
    sai_buffer_api_t *buffer_api;

    status = sai_api_initialize(0, &test_services);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] SAI init, status=%d\n", status);
        return 1;
    }

    status = sai_api_query(SAI_API_BUFFERS, (void**)&buffer_api);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] query BUFFERS API, status=%d\n", status);
        return 1;
    }

    TEST_SECTION("CREATING");

    sai_object_id_t pool1_oid;
    sai_attribute_t pool1_attrs[2];
    pool1_attrs[0].id = SAI_BUFFER_POOL_ATTR_TYPE;
    pool1_attrs[0].value.s32 = SAI_BUFFER_POOL_INGRESS;
    pool1_attrs[1].id = SAI_BUFFER_POOL_ATTR_SIZE;
    pool1_attrs[1].value.u32 = 1024 * 1024;
    status = buffer_api->create_buffer_pool(&pool1_oid, 2, pool1_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] create BUFFER POOL#1, status=%d\n", status);
        return 1;
    }
    printf("[TEST|add] BUFFER POOL#1  " OID_FMT "  %s  size=%u\n",
           (uint64_t)pool1_oid, pool_type_name(pool1_attrs[0].value.s32),
           pool1_attrs[1].value.u32);

    sai_object_id_t pool2_oid;
    sai_attribute_t pool2_attrs[3];
    pool2_attrs[0].id = SAI_BUFFER_POOL_ATTR_TYPE;
    pool2_attrs[0].value.s32 = SAI_BUFFER_POOL_EGRESS;
    pool2_attrs[1].id = SAI_BUFFER_POOL_ATTR_SIZE;
    pool2_attrs[1].value.u32 = 512 * 1024;
    pool2_attrs[2].id = SAI_BUFFER_POOL_ATTR_TH_MODE;
    pool2_attrs[2].value.s32 = SAI_BUFFER_THRESHOLD_MODE_STATIC;
    status = buffer_api->create_buffer_pool(&pool2_oid, 3, pool2_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] create BUFFER POOL#2, status=%d\n", status);
        return 1;
    }
    printf("[TEST|add] BUFFER POOL#2  " OID_FMT "  %s  size=%u  th_mode=%s\n",
           (uint64_t)pool2_oid, pool_type_name(pool2_attrs[0].value.s32),
           pool2_attrs[1].value.u32, th_mode_name(pool2_attrs[2].value.s32));

    sai_object_id_t pool_bad_oid;
    sai_attribute_t pool_bad_attrs[1];
    pool_bad_attrs[0].id = SAI_BUFFER_POOL_ATTR_TYPE;
    pool_bad_attrs[0].value.s32 = SAI_BUFFER_POOL_INGRESS;
    status = buffer_api->create_buffer_pool(&pool_bad_oid, 1, pool_bad_attrs);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] create without SIZE should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] create without mandatory SIZE  status=%d\n", status);

    sai_attribute_t pool_no_type_attrs[1];
    pool_no_type_attrs[0].id = SAI_BUFFER_POOL_ATTR_SIZE;
    pool_no_type_attrs[0].value.u32 = 4096;
    status = buffer_api->create_buffer_pool(&pool_bad_oid, 1, pool_no_type_attrs);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] create without mandatory TYPE should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] create without mandatory TYPE  status=%d\n", status);

    TEST_SECTION("CREATING PROFILES");

    sai_object_id_t profile1_oid;
    sai_attribute_t profile1_attrs[3];
    profile1_attrs[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    profile1_attrs[0].value.oid = pool1_oid;
    profile1_attrs[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    profile1_attrs[1].value.u32 = 4096;
    profile1_attrs[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    profile1_attrs[2].value.s8 = -3;
    status = buffer_api->create_buffer_profile(&profile1_oid, 3, profile1_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] create BUFFER PROFILE#1, status=%d\n", status);
        return 1;
    }
    printf("[TEST|add] BUFFER PROFILE#1  " OID_FMT "  pool=" OID_FMT
           "  size=%u  dynamic_th=%d\n",
           (uint64_t)profile1_oid, (uint64_t)pool1_oid,
           profile1_attrs[1].value.u32, profile1_attrs[2].value.s8);

    sai_object_id_t profile2_oid;
    sai_attribute_t profile2_attrs[3];
    profile2_attrs[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    profile2_attrs[0].value.oid = pool2_oid;
    profile2_attrs[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    profile2_attrs[1].value.u32 = 8192;
    profile2_attrs[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    profile2_attrs[2].value.u32 = 16384;
    status = buffer_api->create_buffer_profile(&profile2_oid, 3, profile2_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] create BUFFER PROFILE#2, status=%d\n", status);
        return 1;
    }
    printf("[TEST|add] BUFFER PROFILE#2  " OID_FMT "  pool=" OID_FMT
           "  size=%u  static_th=%u\n",
           (uint64_t)profile2_oid, (uint64_t)pool2_oid,
           profile2_attrs[1].value.u32, profile2_attrs[2].value.u32);

    sai_object_id_t profile_bad_oid;
    sai_attribute_t profile_bad_attrs[2];
    profile_bad_attrs[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    profile_bad_attrs[0].value.oid = pool1_oid;
    profile_bad_attrs[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    profile_bad_attrs[1].value.u32 = 1024;
    status = buffer_api->create_buffer_profile(&profile_bad_oid, 2, profile_bad_attrs);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] profile without SHARED_DYNAMIC_TH should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] profile without dynamic th on dynamic pool  status=%d\n", status);

    sai_attribute_t profile_invalid_pool_attrs[3];
    profile_invalid_pool_attrs[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    profile_invalid_pool_attrs[0].value.oid = 0;
    profile_invalid_pool_attrs[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    profile_invalid_pool_attrs[1].value.u32 = 1024;
    profile_invalid_pool_attrs[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    profile_invalid_pool_attrs[2].value.s8 = -1;
    status = buffer_api->create_buffer_profile(&profile_bad_oid, 3, profile_invalid_pool_attrs);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] profile with invalid pool OID should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] profile with invalid pool OID  status=%d\n", status);

    TEST_SECTION("GETTING");

    sai_attribute_t pool1_get_attrs[3];
    pool1_get_attrs[0].id = SAI_BUFFER_POOL_ATTR_TYPE;
    pool1_get_attrs[1].id = SAI_BUFFER_POOL_ATTR_SIZE;
    pool1_get_attrs[2].id = SAI_BUFFER_POOL_ATTR_TH_MODE;
    status = buffer_api->get_buffer_pool_attr(pool1_oid, 3, pool1_get_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] get BUFFER POOL#1, status=%d\n", status);
        return 1;
    }
    if (pool1_get_attrs[0].value.s32 != SAI_BUFFER_POOL_INGRESS ||
        pool1_get_attrs[1].value.u32 != pool1_attrs[1].value.u32 ||
        pool1_get_attrs[2].value.s32 != SAI_BUFFER_THRESHOLD_MODE_DYNAMIC) {
        printf("[TEST|fail] BUFFER POOL#1 mismatch: type=%s size=%u th_mode=%s\n",
               pool_type_name(pool1_get_attrs[0].value.s32),
               pool1_get_attrs[1].value.u32,
               th_mode_name(pool1_get_attrs[2].value.s32));
        return 1;
    }
    printf("[TEST|get] BUFFER POOL#1  type=%s  size=%u  th_mode=%s\n",
           pool_type_name(pool1_get_attrs[0].value.s32),
           pool1_get_attrs[1].value.u32,
           th_mode_name(pool1_get_attrs[2].value.s32));

    sai_attribute_t pool2_get_attrs[3];
    pool2_get_attrs[0].id = SAI_BUFFER_POOL_ATTR_TYPE;
    pool2_get_attrs[1].id = SAI_BUFFER_POOL_ATTR_SIZE;
    pool2_get_attrs[2].id = SAI_BUFFER_POOL_ATTR_TH_MODE;
    status = buffer_api->get_buffer_pool_attr(pool2_oid, 3, pool2_get_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] get BUFFER POOL#2, status=%d\n", status);
        return 1;
    }
    if (pool2_get_attrs[0].value.s32 != SAI_BUFFER_POOL_EGRESS ||
        pool2_get_attrs[1].value.u32 != pool2_attrs[1].value.u32 ||
        pool2_get_attrs[2].value.s32 != SAI_BUFFER_THRESHOLD_MODE_STATIC) {
        printf("[TEST|fail] BUFFER POOL#2 mismatch: type=%s size=%u th_mode=%s\n",
               pool_type_name(pool2_get_attrs[0].value.s32),
               pool2_get_attrs[1].value.u32,
               th_mode_name(pool2_get_attrs[2].value.s32));
        return 1;
    }
    printf("[TEST|get] BUFFER POOL#2  type=%s  size=%u  th_mode=%s\n",
           pool_type_name(pool2_get_attrs[0].value.s32),
           pool2_get_attrs[1].value.u32,
           th_mode_name(pool2_get_attrs[2].value.s32));

    TEST_SECTION("GETTING PROFILES");

    sai_attribute_t profile1_get_attrs[3];
    profile1_get_attrs[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    profile1_get_attrs[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    profile1_get_attrs[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
    status = buffer_api->get_buffer_profile_attr(profile1_oid, 3, profile1_get_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] get BUFFER PROFILE#1, status=%d\n", status);
        return 1;
    }
    if (profile1_get_attrs[0].value.oid != pool1_oid ||
        profile1_get_attrs[1].value.u32 != profile1_attrs[1].value.u32 ||
        profile1_get_attrs[2].value.s8 != profile1_attrs[2].value.s8) {
        printf("[TEST|fail] BUFFER PROFILE#1 mismatch: pool=" OID_FMT " size=%u dynamic_th=%d\n",
               (uint64_t)profile1_get_attrs[0].value.oid,
               profile1_get_attrs[1].value.u32, profile1_get_attrs[2].value.s8);
        return 1;
    }
    printf("[TEST|get] BUFFER PROFILE#1  pool=" OID_FMT "  size=%u  dynamic_th=%d\n",
           (uint64_t)profile1_get_attrs[0].value.oid,
           profile1_get_attrs[1].value.u32, profile1_get_attrs[2].value.s8);

    sai_attribute_t profile2_get_attrs[3];
    profile2_get_attrs[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
    profile2_get_attrs[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    profile2_get_attrs[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_STATIC_TH;
    status = buffer_api->get_buffer_profile_attr(profile2_oid, 3, profile2_get_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] get BUFFER PROFILE#2, status=%d\n", status);
        return 1;
    }
    if (profile2_get_attrs[0].value.oid != pool2_oid ||
        profile2_get_attrs[1].value.u32 != profile2_attrs[1].value.u32 ||
        profile2_get_attrs[2].value.u32 != profile2_attrs[2].value.u32) {
        printf("[TEST|fail] BUFFER PROFILE#2 mismatch: pool=" OID_FMT " size=%u static_th=%u\n",
               (uint64_t)profile2_get_attrs[0].value.oid,
               profile2_get_attrs[1].value.u32, profile2_get_attrs[2].value.u32);
        return 1;
    }
    printf("[TEST|get] BUFFER PROFILE#2  pool=" OID_FMT "  size=%u  static_th=%u\n",
           (uint64_t)profile2_get_attrs[0].value.oid,
           profile2_get_attrs[1].value.u32, profile2_get_attrs[2].value.u32);

    TEST_SECTION("REMOVE POOL WITH PROFILES");

    status = buffer_api->remove_buffer_pool(pool1_oid);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] remove POOL#1 with active profile should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] remove POOL#1 with active profile  status=%d\n", status);

    status = buffer_api->remove_buffer_pool(pool2_oid);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] remove POOL#2 with active profile should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] remove POOL#2 with active profile  status=%d\n", status);

    TEST_SECTION("REMOVING PROFILES");

    status = buffer_api->remove_buffer_profile(profile2_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] remove BUFFER PROFILE#2, status=%d\n", status);
        return 1;
    }
    printf("[TEST|del] BUFFER PROFILE#2  " OID_FMT "\n", (uint64_t)profile2_oid);

    status = buffer_api->remove_buffer_profile(profile1_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] remove BUFFER PROFILE#1, status=%d\n", status);
        return 1;
    }
    printf("[TEST|del] BUFFER PROFILE#1  " OID_FMT "\n", (uint64_t)profile1_oid);

    status = buffer_api->remove_buffer_profile(profile1_oid);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] double remove profile should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] double remove profile  status=%d\n", status);

    status = buffer_api->remove_buffer_profile(0);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] remove invalid profile OID should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] remove invalid profile OID  status=%d\n", status);

    TEST_SECTION("REMOVING POOLS");

    status = buffer_api->remove_buffer_pool(pool2_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] remove BUFFER POOL#2, status=%d\n", status);
        return 1;
    }
    printf("[TEST|del] BUFFER POOL#2  " OID_FMT "\n", (uint64_t)pool2_oid);

    status = buffer_api->remove_buffer_pool(pool1_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] remove BUFFER POOL#1, status=%d\n", status);
        return 1;
    }
    printf("[TEST|del] BUFFER POOL#1  " OID_FMT "\n", (uint64_t)pool1_oid);

    status = buffer_api->remove_buffer_pool(pool1_oid);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] double remove pool should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] double remove pool  status=%d\n", status);

    status = buffer_api->remove_buffer_pool(0);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] remove invalid pool OID should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] remove invalid pool OID  status=%d\n", status);

    TEST_SECTION("GET ON REMOVED");

    sai_attribute_t removed_pool_attr;
    removed_pool_attr.id = SAI_BUFFER_POOL_ATTR_SIZE;
    status = buffer_api->get_buffer_pool_attr(pool1_oid, 1, &removed_pool_attr);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] get removed BUFFER POOL#1 should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] get removed BUFFER POOL#1  status=%d\n", status);

    status = buffer_api->get_buffer_pool_attr(pool2_oid, 1, &removed_pool_attr);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] get removed BUFFER POOL#2 should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] get removed BUFFER POOL#2  status=%d\n", status);

    sai_attribute_t removed_profile_attr;
    removed_profile_attr.id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
    status = buffer_api->get_buffer_profile_attr(profile1_oid, 1, &removed_profile_attr);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] get removed BUFFER PROFILE#1 should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] get removed BUFFER PROFILE#1  status=%d\n", status);

    status = buffer_api->get_buffer_profile_attr(profile2_oid, 1, &removed_profile_attr);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] get removed BUFFER PROFILE#2 should have been rejected\n");
        return 1;
    }
    printf("[TEST|neg] get removed BUFFER PROFILE#2  status=%d\n", status);

    TEST_SECTION("POOL DB LIMIT");

    sai_object_id_t limit_pools[4];
    sai_attribute_t limit_pool_attrs[2];
    for (uint32_t i = 0; i < 4; i++) {
        limit_pool_attrs[0].id = SAI_BUFFER_POOL_ATTR_TYPE;
        limit_pool_attrs[0].value.s32 = (i % 2) ? SAI_BUFFER_POOL_EGRESS : SAI_BUFFER_POOL_INGRESS;
        limit_pool_attrs[1].id = SAI_BUFFER_POOL_ATTR_SIZE;
        limit_pool_attrs[1].value.u32 = 4096 * (i + 1);
        status = buffer_api->create_buffer_pool(&limit_pools[i], 2, limit_pool_attrs);
        if (status != SAI_STATUS_SUCCESS) {
            printf("[TEST|fail] create limit pool[%" PRIu32 "], status=%d\n", i, status);
            return 1;
        }
        printf("[TEST|add] limit pool[%" PRIu32 "]  " OID_FMT "  size=%u\n",
               i, (uint64_t)limit_pools[i], limit_pool_attrs[1].value.u32);
    }

    sai_object_id_t pool_overflow_oid;
    status = buffer_api->create_buffer_pool(&pool_overflow_oid, 2, limit_pool_attrs);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] 5th buffer pool should have been rejected (max 4)\n");
        return 1;
    }
    if (status != SAI_STATUS_INSUFFICIENT_RESOURCES) {
        printf("[TEST|fail] 5th buffer pool expected INSUFFICIENT_RESOURCES (%d), got %d\n",
               (int)SAI_STATUS_INSUFFICIENT_RESOURCES, status);
        return 1;
    }
    printf("[TEST|neg] 5th buffer pool (DB full)  status=%d\n", status);

    for (uint32_t i = 0; i < 4; i++) {
        status = buffer_api->remove_buffer_pool(limit_pools[i]);
        if (status != SAI_STATUS_SUCCESS) {
            printf("[TEST|fail] remove limit pool[%" PRIu32 "], status=%d\n", i, status);
            return 1;
        }
        printf("[TEST|del] limit pool[%" PRIu32 "]  " OID_FMT "\n",
               i, (uint64_t)limit_pools[i]);
    }

    TEST_SECTION("PROFILE DB LIMIT");

    sai_object_id_t profile_limit_pool_oid;
    sai_attribute_t profile_limit_pool_attrs[2];
    profile_limit_pool_attrs[0].id = SAI_BUFFER_POOL_ATTR_TYPE;
    profile_limit_pool_attrs[0].value.s32 = SAI_BUFFER_POOL_INGRESS;
    profile_limit_pool_attrs[1].id = SAI_BUFFER_POOL_ATTR_SIZE;
    profile_limit_pool_attrs[1].value.u32 = 1024 * 1024;
    status = buffer_api->create_buffer_pool(&profile_limit_pool_oid, 2, profile_limit_pool_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] create pool for profile limit test, status=%d\n", status);
        return 1;
    }
    printf("[TEST|add] profile-limit pool  " OID_FMT "\n", (uint64_t)profile_limit_pool_oid);

    sai_object_id_t limit_profiles[MAX_BUFFER_PROFILES];
    sai_attribute_t limit_profile_attrs[3];
    for (uint32_t i = 0; i < MAX_BUFFER_PROFILES; i++) {
        limit_profile_attrs[0].id = SAI_BUFFER_PROFILE_ATTR_POOL_ID;
        limit_profile_attrs[0].value.oid = profile_limit_pool_oid;
        limit_profile_attrs[1].id = SAI_BUFFER_PROFILE_ATTR_BUFFER_SIZE;
        limit_profile_attrs[1].value.u32 = 1024 + i * 512;
        limit_profile_attrs[2].id = SAI_BUFFER_PROFILE_ATTR_SHARED_DYNAMIC_TH;
        limit_profile_attrs[2].value.s8 = -8;
        status = buffer_api->create_buffer_profile(&limit_profiles[i], 3, limit_profile_attrs);
        if (status != SAI_STATUS_SUCCESS) {
            printf("[TEST|fail] create limit profile[%" PRIu32 "], status=%d\n", i, status);
            return 1;
        }
        printf("[TEST|add] limit profile[%" PRIu32 "]  " OID_FMT "  size=%u\n",
               i, (uint64_t)limit_profiles[i], limit_profile_attrs[1].value.u32);
    }

    sai_object_id_t profile_overflow_oid;
    status = buffer_api->create_buffer_profile(&profile_overflow_oid, 3, limit_profile_attrs);
    if (status == SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] 9th buffer profile should have been rejected (max %d)\n",
               MAX_BUFFER_PROFILES);
        return 1;
    }
    if (status != SAI_STATUS_INSUFFICIENT_RESOURCES) {
        printf("[TEST|fail] 9th buffer profile expected INSUFFICIENT_RESOURCES (%d), got %d\n",
               (int)SAI_STATUS_INSUFFICIENT_RESOURCES, status);
        return 1;
    }
    printf("[TEST|neg] 9th buffer profile (DB full)  status=%d\n", status);

    for (uint32_t i = 0; i < MAX_BUFFER_PROFILES; i++) {
        status = buffer_api->remove_buffer_profile(limit_profiles[i]);
        if (status != SAI_STATUS_SUCCESS) {
            printf("[TEST|fail] remove limit profile[%" PRIu32 "], status=%d\n", i, status);
            return 1;
        }
        printf("[TEST|del] limit profile[%" PRIu32 "]  " OID_FMT "\n",
               i, (uint64_t)limit_profiles[i]);
    }

    status = buffer_api->remove_buffer_pool(profile_limit_pool_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[TEST|fail] remove profile-limit pool, status=%d\n", status);
        return 1;
    }
    printf("[TEST|del] profile-limit pool  " OID_FMT "\n", (uint64_t)profile_limit_pool_oid);

    printf("\n========== HAPPY END ==========\n");
    return 0;
}
