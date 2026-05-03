#include "sai.h"
#include "assert.h"
#include "stub_sai.h"
#include <stdbool.h>
#include <stdio.h>

#define MAX_NUMBER_OF_LAG_MEMBERS 16
#define MAX_NUMBER_OF_LAGS 5
#define MAX_NUMBER_OF_PORTS 32

typedef struct _lag_member_db_entry_t {
    bool            is_ised;
    sai_object_id_t port_oid;
    sai_object_id_t lag_oid;
} lag_member_db_entry_t;

typedef struct _lag_db_entry_t {
    bool            is_used;
    sai_object_id_t members_ids[MAX_NUMBER_OF_LAG_MEMBERS];
} lag_db_entry_t;

struct lag_db_t {
    lag_db_entry_t        lags[MAX_NUMBER_OF_LAGS];
    lag_member_db_entry_t members[MAX_NUMBER_OF_LAG_MEMBERS];
} lag_db;

static const sai_attribute_entry_t lag_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST, true, true, false, true,
      "List of ports in LAG", SAI_ATTR_VAL_TYPE_OIDLIST },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t lag_vendor_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST,
      { true, false, false, true },
      { true, false, false, true },
      NULL, NULL,
      NULL, NULL }
};

sai_status_t get_lag_member_attribute(_In_ const sai_object_key_t   *key,
    _Inout_ sai_attribute_value_t *value,
    _In_ uint32_t                  attr_index,
    _Inout_ vendor_cache_t        *cache,
    void                          *arg)
{
    sai_status_t status;
    uint32_t     db_index;

    assert((SAI_LAG_MEMBER_ATTR_LAG_ID == (int64_t)arg || SAI_LAG_MEMBER_ATTR_PORT_ID == (int64_t)arg));

    status = stub_object_to_type(key->object_id, SAI_OBJECT_TYPE_LAG_MEMBER, &db_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot get LAG DB index.\n");
        return status;
    }

    switch ((int64_t)arg) {
        case SAI_LAG_MEMBER_ATTR_LAG_ID:
            value->oid = lag_db.members[db_index].lag_oid;
        break;
        case SAI_LAG_MEMBER_ATTR_PORT_ID:
         value->oid = lag_db.members[db_index].port_oid;
        break;
        default:
            printf("Got unexpected attribute ID\n");
            return SAI_STATUS_FAILURE;
        }

    return SAI_STATUS_SUCCESS;
}

static const sai_attribute_entry_t lag_member_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID, true, true, false, true,
      "LAG ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_LAG_MEMBER_ATTR_PORT_ID, true, true, false, true,
      "PORT ID", SAI_ATTR_VAL_TYPE_OID },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_vendor_attribute_entry_t lag_member_vendor_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID,
      { true, false, false, true },
      { true, false, false, true },
      get_lag_member_attribute, (void*) SAI_LAG_MEMBER_ATTR_LAG_ID,
      NULL, NULL },
    { SAI_LAG_MEMBER_ATTR_PORT_ID,
      { true, false, false, true },
      { true, false, false, true },
      get_lag_member_attribute, (void*) SAI_LAG_MEMBER_ATTR_PORT_ID,
      NULL, NULL }
};



sai_status_t stub_create_lag(
    _Out_ sai_object_id_t* lag_oid,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t status;
    uint32_t idx = 0;

    status = check_attribs_metadata(attr_count, attr_list, lag_attribs, lag_vendor_attribs, SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed attributes check\n");
        return status;
    }

    const sai_attribute_value_t *port_list;
    uint32_t port_list_index;
    assert(SAI_STATUS_SUCCESS ==
           find_attrib_in_list(attr_count, attr_list, SAI_LAG_ATTR_PORT_LIST, &port_list, &port_list_index));
    if (port_list->objlist.count > MAX_NUMBER_OF_PORTS) {
        printf("Cannot create LAG: port list count is too large\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (; idx < MAX_NUMBER_OF_LAGS; idx++) {
        if (!lag_db.lags[idx].is_used) {
            break;
        }
    }

    if (idx == MAX_NUMBER_OF_LAGS) {
        printf("Cannot create LAG: limit is reached\n");
        return SAI_STATUS_FAILURE;
    }
    uint32_t lag_db_id = idx;

    lag_db.lags[lag_db_id].is_used = true;
    status = stub_create_object(SAI_OBJECT_TYPE_LAG, lag_db_id, lag_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create a LAG OID\n");
        lag_db.lags[lag_db_id].is_used = false;
        return status;
    }

    char list_str[MAX_LIST_VALUE_STR_LEN];
    sai_attr_list_to_str(attr_count, attr_list, lag_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    printf("[NEW] CREATE LAG[%d], type:0x%x, attr list: (%s)\n", lag_db_id, SAI_OBJECT_TYPE_LAG, list_str);

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_remove_lag(
    _In_ sai_object_id_t  lag_id)
{
    sai_status_t status;
    uint32_t lag_db_id;
    sai_object_id_t id = lag_id;

    status = stub_object_to_type(lag_id, SAI_OBJECT_TYPE_LAG, &lag_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }
    if (lag_db_id >= MAX_NUMBER_OF_LAGS || !lag_db.lags[lag_db_id].is_used) {
        printf("Cannot remove LAG: invalid or not allocated LAG id\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    status = stub_destroy_object(SAI_OBJECT_TYPE_LAG, &id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }

    memset(&lag_db.lags[lag_db_id], 0, sizeof(lag_db.lags[lag_db_id]));
    printf("[DEL] REMOVE LAG[%d], type:0x%x\n", lag_db_id, SAI_OBJECT_TYPE_LAG);
    return SAI_STATUS_SUCCESS;
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
    sai_status_t status;
    uint32_t idx = 0;

    status = check_attribs_metadata(attr_count, attr_list, lag_member_attribs, lag_member_vendor_attribs, SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed attributes check\n");
        return status;
    }

    const sai_attribute_value_t *lag_id;
    uint32_t lag_id_index;
    assert(SAI_STATUS_SUCCESS ==
        find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_LAG_ID, &lag_id, &lag_id_index));

    const sai_attribute_value_t *port_id;
    uint32_t port_id_index;
    assert(SAI_STATUS_SUCCESS ==
            find_attrib_in_list(attr_count, attr_list, SAI_LAG_MEMBER_ATTR_PORT_ID, &port_id, &port_id_index));

    for (; idx < MAX_NUMBER_OF_LAG_MEMBERS; idx++) {
        if (!lag_db.members[idx].is_ised) {
            break;
        }
    }
    if (idx == MAX_NUMBER_OF_LAG_MEMBERS) {
        printf("Cannot create LAG MEMBER: limit is reached\n");
        return SAI_STATUS_FAILURE;
    }
    uint32_t lag_member_db_id = idx;
    lag_db.members[lag_member_db_id].is_ised = true;
    status = stub_create_object(SAI_OBJECT_TYPE_LAG_MEMBER, lag_member_db_id, lag_member_id);
    if (status != SAI_STATUS_SUCCESS) {
        lag_db.members[lag_member_db_id].is_ised = false;
        printf("Cannot create a LAG MEMBER OID\n");
        return status;
    }

    // retrieve LAG DB id from OID
    uint32_t lag_db_id;
    status = stub_object_to_type(lag_id->oid, SAI_OBJECT_TYPE_LAG, &lag_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }
    if (lag_db_id >= MAX_NUMBER_OF_LAGS || !lag_db.lags[lag_db_id].is_used) {
        printf("Cannot create LAG MEMBER: invalid or not allocated LAG id\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    //searching free slot in lag_db.lags[lag_db_id].members_ids
    uint32_t free_slot = MAX_NUMBER_OF_LAG_MEMBERS;
    for (uint32_t i = 0; i < MAX_NUMBER_OF_LAG_MEMBERS; i++) {
        if (lag_db.lags[lag_db_id].members_ids[i] == 0) {
            free_slot = i;
            break;
        }
    }
    if (free_slot == MAX_NUMBER_OF_LAG_MEMBERS) {
        printf("Cannot create LAG MEMBER: limit is reached\n");
        return SAI_STATUS_FAILURE;
    }
    /* Record this LAG's child LAG_MEMBER object id in the LAG's member list. */
    lag_db.lags[lag_db_id].members_ids[free_slot] = *lag_member_id;

    char list_str[MAX_LIST_VALUE_STR_LEN];
    sai_attr_list_to_str(attr_count, attr_list, lag_member_attribs, MAX_LIST_VALUE_STR_LEN, list_str);
    printf("[NEW] CREATE LAG MEMBER[%d], type:0x%x, attr list: (%s)\n", lag_member_db_id, SAI_OBJECT_TYPE_LAG_MEMBER, list_str);

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_remove_lag_member(
    _In_ sai_object_id_t  lag_member_id)
{
    sai_status_t status;
    uint32_t lag_member_db_id;
    sai_object_id_t id = lag_member_id;

    status = stub_object_to_type(lag_member_id, SAI_OBJECT_TYPE_LAG_MEMBER, &lag_member_db_id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }
    if (lag_member_db_id >= MAX_NUMBER_OF_LAG_MEMBERS || !lag_db.members[lag_member_db_id].is_ised) {
        printf("Cannot remove LAG MEMBER: invalid or not allocated LAG member id\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    bool cleared_from_lag = false;
    for (uint32_t lag_i = 0; lag_i < MAX_NUMBER_OF_LAGS; lag_i++) {
        if (!lag_db.lags[lag_i].is_used) {
            continue;
        }
        for (uint32_t j = 0; j < MAX_NUMBER_OF_LAG_MEMBERS; j++) {
            if (lag_db.lags[lag_i].members_ids[j] == lag_member_id) {
                lag_db.lags[lag_i].members_ids[j] = 0;
                cleared_from_lag = true;
                break;
            }
        }
        if (cleared_from_lag) {
            break;
        }
    }

    status = stub_destroy_object(SAI_OBJECT_TYPE_LAG_MEMBER, &id);
    if (status != SAI_STATUS_SUCCESS) {
        return status;
    }

    memset(&lag_db.members[lag_member_db_id], 0, sizeof(lag_db.members[lag_member_db_id]));
    printf("[DEL] REMOVE LAG MEMBER[%d], type:0x%x\n", lag_member_db_id, SAI_OBJECT_TYPE_LAG_MEMBER);
    return SAI_STATUS_SUCCESS;
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
    const sai_object_key_t key = { .object_id = lag_member_id };
    return sai_get_attributes(&key, NULL, lag_member_attribs, lag_member_vendor_attribs, attr_count, attr_list);
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