#include "sai.h"
#include "assert.h"
#include "stub_sai.h"

sai_status_t stub_create_lag(
    _Out_ sai_object_id_t* lag_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    static int32_t next_lag_id = 1;
    sai_status_t status;

    status = stub_create_object(SAI_OBJECT_TYPE_LAG, next_lag_id++, lag_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create a LAG OID\n");
        return status;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_remove_lag(
    _In_ sai_object_id_t  lag_id)
{
    sai_object_id_t id = lag_id;

    return stub_destroy_object(SAI_OBJECT_TYPE_LAG, &id);
}

sai_status_t stub_set_lag_attribute(
    _In_ sai_object_id_t  lag_id,
    _In_ const sai_attribute_t *attr)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_attribute(
    _In_ sai_object_id_t lag_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_create_lag_member(
    _Out_ sai_object_id_t* lag_member_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    static int32_t next_lag_member_id = 1;
    sai_status_t status;

    status = stub_create_object(SAI_OBJECT_TYPE_LAG_MEMBER, next_lag_member_id++, lag_member_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create a LAG OID\n");
        return status;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_remove_lag_member(
    _In_ sai_object_id_t  lag_member_id)
{
    sai_object_id_t id = lag_member_id;

    return  stub_destroy_object(SAI_OBJECT_TYPE_LAG_MEMBER, &id);
}

sai_status_t stub_set_lag_member_attribute(
    _In_ sai_object_id_t  lag_member_id,
    _In_ const sai_attribute_t *attr)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t stub_get_lag_member_attribute(
    _In_ sai_object_id_t lag_member_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    return SAI_STATUS_SUCCESS;
}

const sai_lag_api_t lag_api = {
    stub_create_lag,
    stub_remove_lag,
    stub_set_lag_attribute,
    stub_get_lag_attribute,
    stub_create_lag_member,
    stub_remove_lag_member,
    stub_set_lag_member_attribute,
    stub_get_lag_member_attribute
};