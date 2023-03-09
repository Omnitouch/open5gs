
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "spatial_validity_rm.h"

OpenAPI_spatial_validity_rm_t *OpenAPI_spatial_validity_rm_create(
    OpenAPI_list_t* presence_info_list
)
{
    OpenAPI_spatial_validity_rm_t *spatial_validity_rm_local_var = ogs_malloc(sizeof(OpenAPI_spatial_validity_rm_t));
    ogs_assert(spatial_validity_rm_local_var);

    spatial_validity_rm_local_var->presence_info_list = presence_info_list;

    return spatial_validity_rm_local_var;
}

void OpenAPI_spatial_validity_rm_free(OpenAPI_spatial_validity_rm_t *spatial_validity_rm)
{
    OpenAPI_lnode_t *node = NULL;

    if (NULL == spatial_validity_rm) {
        return;
    }
    if (spatial_validity_rm->presence_info_list) {
        OpenAPI_list_for_each(spatial_validity_rm->presence_info_list, node) {
            OpenAPI_map_t *localKeyValue = (OpenAPI_map_t*)node->data;
            ogs_free(localKeyValue->key);
            OpenAPI_presence_info_free(localKeyValue->value);
            OpenAPI_map_free(localKeyValue);
        }
        OpenAPI_list_free(spatial_validity_rm->presence_info_list);
        spatial_validity_rm->presence_info_list = NULL;
    }
    ogs_free(spatial_validity_rm);
}

cJSON *OpenAPI_spatial_validity_rm_convertToJSON(OpenAPI_spatial_validity_rm_t *spatial_validity_rm)
{
    cJSON *item = NULL;
    OpenAPI_lnode_t *node = NULL;

    if (spatial_validity_rm == NULL) {
        ogs_error("OpenAPI_spatial_validity_rm_convertToJSON() failed [SpatialValidityRm]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (!spatial_validity_rm->presence_info_list) {
        ogs_error("OpenAPI_spatial_validity_rm_convertToJSON() failed [presence_info_list]");
        return NULL;
    }
    cJSON *presence_info_list = cJSON_AddObjectToObject(item, "presenceInfoList");
    if (presence_info_list == NULL) {
        ogs_error("OpenAPI_spatial_validity_rm_convertToJSON() failed [presence_info_list]");
        goto end;
    }
    cJSON *localMapObject = presence_info_list;
    if (spatial_validity_rm->presence_info_list) {
        OpenAPI_list_for_each(spatial_validity_rm->presence_info_list, node) {
            OpenAPI_map_t *localKeyValue = (OpenAPI_map_t*)node->data;
            cJSON *itemLocal = localKeyValue->value ?
                OpenAPI_presence_info_convertToJSON(localKeyValue->value) :
                cJSON_CreateNull();
            if (itemLocal == NULL) {
                ogs_error("OpenAPI_spatial_validity_rm_convertToJSON() failed [inner]");
                goto end;
            }
            cJSON_AddItemToObject(localMapObject, localKeyValue->key, itemLocal);
        }
    }

end:
    return item;
}

OpenAPI_spatial_validity_rm_t *OpenAPI_spatial_validity_rm_parseFromJSON(cJSON *spatial_validity_rmJSON)
{
    OpenAPI_spatial_validity_rm_t *spatial_validity_rm_local_var = NULL;
    OpenAPI_lnode_t *node = NULL;
    cJSON *presence_info_list = NULL;
    OpenAPI_list_t *presence_info_listList = NULL;
    presence_info_list = cJSON_GetObjectItemCaseSensitive(spatial_validity_rmJSON, "presenceInfoList");
    if (!presence_info_list) {
        ogs_error("OpenAPI_spatial_validity_rm_parseFromJSON() failed [presence_info_list]");
        goto end;
    }
        cJSON *presence_info_list_local_map = NULL;
        if (!cJSON_IsObject(presence_info_list) && !cJSON_IsNull(presence_info_list)) {
            ogs_error("OpenAPI_spatial_validity_rm_parseFromJSON() failed [presence_info_list]");
            goto end;
        }
        if (cJSON_IsObject(presence_info_list)) {
            presence_info_listList = OpenAPI_list_create();
            OpenAPI_map_t *localMapKeyPair = NULL;
            cJSON_ArrayForEach(presence_info_list_local_map, presence_info_list) {
                cJSON *localMapObject = presence_info_list_local_map;
                if (cJSON_IsObject(localMapObject)) {
                    localMapKeyPair = OpenAPI_map_create(
                        ogs_strdup(localMapObject->string), OpenAPI_presence_info_parseFromJSON(localMapObject));
                } else if (cJSON_IsNull(localMapObject)) {
                    localMapKeyPair = OpenAPI_map_create(ogs_strdup(localMapObject->string), NULL);
                } else {
                    ogs_error("OpenAPI_spatial_validity_rm_parseFromJSON() failed [inner]");
                    goto end;
                }
                OpenAPI_list_add(presence_info_listList, localMapKeyPair);
            }
        }

    spatial_validity_rm_local_var = OpenAPI_spatial_validity_rm_create (
        presence_info_listList
    );

    return spatial_validity_rm_local_var;
end:
    if (presence_info_listList) {
        OpenAPI_list_for_each(presence_info_listList, node) {
            OpenAPI_map_t *localKeyValue = (OpenAPI_map_t*) node->data;
            ogs_free(localKeyValue->key);
            OpenAPI_presence_info_free(localKeyValue->value);
            OpenAPI_map_free(localKeyValue);
        }
        OpenAPI_list_free(presence_info_listList);
        presence_info_listList = NULL;
    }
    return NULL;
}

OpenAPI_spatial_validity_rm_t *OpenAPI_spatial_validity_rm_copy(OpenAPI_spatial_validity_rm_t *dst, OpenAPI_spatial_validity_rm_t *src)
{
    cJSON *item = NULL;
    char *content = NULL;

    ogs_assert(src);
    item = OpenAPI_spatial_validity_rm_convertToJSON(src);
    if (!item) {
        ogs_error("OpenAPI_spatial_validity_rm_convertToJSON() failed");
        return NULL;
    }

    content = cJSON_Print(item);
    cJSON_Delete(item);

    if (!content) {
        ogs_error("cJSON_Print() failed");
        return NULL;
    }

    item = cJSON_Parse(content);
    ogs_free(content);
    if (!item) {
        ogs_error("cJSON_Parse() failed");
        return NULL;
    }

    OpenAPI_spatial_validity_rm_free(dst);
    dst = OpenAPI_spatial_validity_rm_parseFromJSON(item);
    cJSON_Delete(item);

    return dst;
}

