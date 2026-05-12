#include <stdio.h>
#include <inttypes.h>
#include "sai.h"

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
    sai_lag_api_t *lag_api;

    status = sai_api_initialize(0, &test_services);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to initialize SAI API, status=%d\n", status);
        return 1;
    }

    status = sai_api_query(SAI_API_LAG, (void**)&lag_api);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to query LAG API, status=%d\n", status);
        return 1;
    }

    // ========================== CREATING ==========================
    printf("\t ========================== CREATING ===================\n");
    // --- Create LAG#1
    sai_object_id_t lag1_oid;
    sai_object_id_t lg1_port_list[32];
    sai_attribute_t lg1_attrs[1];
    lg1_attrs[0].id = SAI_LAG_ATTR_PORT_LIST;
    lg1_attrs[0].value.objlist.list = lg1_port_list;
    lg1_attrs[0].value.objlist.count = 32;
    status = lag_api->create_lag(&lag1_oid, 1, lg1_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to create a LAG#1, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|add] LAG#1: 0x%lX\n", lag1_oid);
    }

    // --- Create LAG_MEMBER#1 {LAG_ID:LAG#1, PORT_ID:PORT#1}
    sai_object_id_t lg1_mem1_oid;
    sai_attribute_t lg1_mem1_attrs[2];
    lg1_mem1_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lg1_mem1_attrs[0].value.oid = lag1_oid;
    lg1_mem1_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    lg1_mem1_attrs[1].value.u64 = 1;
    status = lag_api->create_lag_member(&lg1_mem1_oid, 2, lg1_mem1_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to create a LAG#1 MEMBER#1, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|add] CREATE LAG#1 MEMBER#1: 0x%lX\n", lg1_mem1_oid);
    }

    // --- Create LAG_MEMBER#2 {LAG_ID:LAG#1, PORT_ID:PORT#2}
    sai_object_id_t lg1_mem2_oid;
    sai_attribute_t lg1_mem2_attrs[2];
    lg1_mem2_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lg1_mem2_attrs[0].value.oid = lag1_oid;
    lg1_mem2_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    lg1_mem2_attrs[1].value.u64 = 2;
    status = lag_api->create_lag_member(&lg1_mem2_oid, 2, lg1_mem2_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to create a LAG#1 MEMBER#2, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|add] LAG#1 MEMBER#2: 0x%lX\n", lg1_mem2_oid);
    }

    // --- Create LAG#2
    sai_object_id_t lag2_oid;
    sai_object_id_t lg2_port_list[32];
    sai_attribute_t lg2_attrs[1];
    lg2_attrs[0].id = SAI_LAG_ATTR_PORT_LIST;
    lg2_attrs[0].value.objlist.list = lg2_port_list;
    lg2_attrs[0].value.objlist.count = 32;
    status = lag_api->create_lag(&lag2_oid, 1, lg2_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to create a LAG#2, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|add] CREATE LAG#2: 0x%lX\n", lag2_oid);
    }

    // --- Create LAG_MEMBER#3 {LAG_ID:LAG#2, PORT_ID:PORT#3}
    sai_object_id_t lg2_mem3_oid;
    sai_attribute_t lg2_mem3_attrs[2];
    lg2_mem3_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lg2_mem3_attrs[0].value.oid = lag2_oid;
    lg2_mem3_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    lg2_mem3_attrs[1].value.u64 = 3;
    status = lag_api->create_lag_member(&lg2_mem3_oid, 2, lg2_mem3_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to create a LAG#2 MEMBER#3, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|add] LAG#2 MEMBER#3: 0x%lX\n", lg2_mem3_oid);
    }

    // --- Create LAG_MEMBER#4 {LAG_ID:LAG#2, PORT_ID:PORT#4}
    sai_object_id_t lg2_mem4_oid;
    sai_attribute_t lg2_mem4_attrs[2];
    lg2_mem4_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lg2_mem4_attrs[0].value.oid = lag2_oid;
    lg2_mem4_attrs[1].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    lg2_mem4_attrs[1].value.u64 = 4;
    status = lag_api->create_lag_member(&lg2_mem4_oid, 2, lg2_mem4_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to create a LAG#2 MEMBER#4, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|add] CREATE LAG#2 MEMBER#4: 0x%lX\n", lg2_mem4_oid);
    }

    // ========================== GETTING  ==========================
    printf("\t ========================== GETTING ===================\n");
    // --- Get LAG#1 PORT_LIST [Expected: (PORT#1, PORT#2)]
    sai_attribute_t lg1_port_list_attrs[1];
    lg1_port_list_attrs[0].id = SAI_LAG_ATTR_PORT_LIST;
    lg1_port_list_attrs[0].value.objlist.list = lg1_port_list;
    lg1_port_list_attrs[0].value.objlist.count = 32;
    status = lag_api->get_lag_attribute(lag1_oid, 1, lg1_port_list_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to get a LAG#1 PORT_LIST, status=%d\n", status);
        return 1;
    } else {
        sai_object_list_t *ol = &lg1_port_list_attrs[0].value.objlist;
        printf("\t[TEST|get] LAG#1 PORT_LIST list=%p count=%" PRIu32 " [",
               (void *)ol->list, ol->count);
        for (uint32_t i = 0; i < ol->count; i++) {
            printf("%s0x%" PRIx64, i ? ", " : "", (uint64_t)ol->list[i]);
        }
        printf("]\n");
    }
    // --- Get LAG#2 PORT_LIST [Expected: (PORT#3, PORT#4)]
    sai_attribute_t lg2_port_list_attrs[1];
    lg2_port_list_attrs[0].id = SAI_LAG_ATTR_PORT_LIST;
    lg2_port_list_attrs[0].value.objlist.list = lg2_port_list;
    lg2_port_list_attrs[0].value.objlist.count = 32;
    status = lag_api->get_lag_attribute(lag2_oid, 1, lg2_port_list_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to get a LAG#2 PORT_LIST, status=%d\n", status);
        return 1;
    } else {
        sai_object_list_t *ol = &lg2_port_list_attrs[0].value.objlist;
        printf("\t[TEST|get] LAG#2 PORT_LIST list=%p count=%" PRIu32 " [",
               (void *)ol->list, ol->count);
        for (uint32_t i = 0; i < ol->count; i++) {
            printf("%s0x%" PRIx64, i ? ", " : "", (uint64_t)ol->list[i]);
        }
        printf("]\n");
    }
    // --- Get LAG_MEMBER#1 LAG_ID [Expected: LAG#1]
    sai_attribute_t lg1_mem1_lag_id_attrs[1];
    lg1_mem1_lag_id_attrs[0].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    lg1_mem1_lag_id_attrs[0].value.oid = lg1_mem1_oid;
    status = lag_api->get_lag_member_attribute(lg1_mem1_oid, 1, lg1_mem1_lag_id_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to get a LAG#1 MEMBER#1 LAG_ID, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|get] LAG#1 MEMBER#1 LAG_ID value.oid=0x%" PRIx64 " [Expected: LAG#1 -> 0x%" PRIx64 "]\n",
               (uint64_t)lg1_mem1_lag_id_attrs[0].value.oid, (uint64_t)lag1_oid);
    }
    // --- Get LAG_MEMBER#3 PORT_ID [Expected: PORT#3]
    sai_attribute_t lg2_mem3_port_id_attrs[1];
    lg2_mem3_port_id_attrs[0].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    lg2_mem3_port_id_attrs[0].value.oid = lg2_mem3_oid;
    status = lag_api->get_lag_member_attribute(lg2_mem3_oid, 1, lg2_mem3_port_id_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to get a LAG#2 MEMBER#3 PORT_ID, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|get] LAG#2 MEMBER#3 PORT_ID value.oid=0x%" PRIx64 " [Expected: PORT#3 -> 0x%" PRIx64 "]\n",
               (uint64_t)lg2_mem3_port_id_attrs[0].value.oid,
               (uint64_t)lg2_mem3_attrs[1].value.oid);
    }

    // ========================== REMOVE and GETTING =================
    printf("\t ========================== REMOVE and GETTING ==========\n");
    // --- Remove LAG#1 MEMBER#2
    status = lag_api->remove_lag_member(lg1_mem2_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to remove a LAG#1 MEMBER#2, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|del] LAG#1 MEMBER#2: 0x%lX\n", lg1_mem2_oid);
    }
    // --- Get LAG#1 PORT_LIST [Expected: (PORT#1)]
    lg1_port_list_attrs[0].id = SAI_LAG_ATTR_PORT_LIST;
    lg1_port_list_attrs[0].value.objlist.list = lg1_port_list;
    lg1_port_list_attrs[0].value.objlist.count = 32;
    status = lag_api->get_lag_attribute(lag1_oid, 1, lg1_port_list_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to get a LAG#1 PORT_LIST, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|get] LAG#1 PORT_LIST\n");
    }
    // --- Remove LAG#2 MEMBER#3
    status = lag_api->remove_lag_member(lg2_mem3_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to remove a LAG#2 MEMBER#3, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|del] LAG#2 MEMBER#3: 0x%lX\n", lg2_mem3_oid);
    }
    // --- Get LAG#2 PORT_LIST [Expected: (PORT#4)]
    lg2_port_list_attrs[0].id = SAI_LAG_ATTR_PORT_LIST;
    lg2_port_list_attrs[0].value.objlist.list = lg2_port_list;
    lg2_port_list_attrs[0].value.objlist.count = 32;
    status = lag_api->get_lag_attribute(lag2_oid, 1, lg2_port_list_attrs);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to get a LAG#2 PORT_LIST, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|get] LAG#2 PORT_LIST\n");
    }

    // ========================== REMOVING ==========================
    printf("\t ========================== REMOVING ===================\n");
    // --- Remove LAG#1 MEMBER#1
    status = lag_api->remove_lag_member(lg1_mem1_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to remove a LAG#1 MEMBER#1, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|del] LAG#1 MEMBER#1: 0x%lX\n", lg1_mem1_oid);
    }

    // --- Remove LAG#2 MEMBER#4
    status = lag_api->remove_lag_member(lg2_mem4_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to remove a LAG#2 MEMBER#4, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|del] LAG#2 MEMBER#4: 0x%lX\n", lg2_mem4_oid);
    }

    // --- Remove LAG#2
    status = lag_api->remove_lag(lag2_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to remove a LAG#2, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|del] LAG#2: 0x%lX\n", lag2_oid);
    }

    // --- Remove LAG#1
    status = lag_api->remove_lag(lag1_oid);
    if (status != SAI_STATUS_SUCCESS) {
        printf("\tFailed to remove a LAG#1, status=%d\n", status);
        return 1;
    } else {
        printf("\t[TEST|del] LAG#1: 0x%lX\n", lag1_oid);
    }

    printf("\n\tHappy End\n\n");
    return 0;
}