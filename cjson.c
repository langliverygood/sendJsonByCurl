#ifdef __cplusplus 
extern "C" { 
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <stdarg.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "link.h"
#include "cjson.h"

static void _json_format(char *str, int len)
{
	int i, j;
	char *buf;

	buf = (char *)malloc(len);
	strncpy(buf, str, len);
	for(i = 0, j = 0; i < len; i++)
	{
		if(isgraph(buf[i]))
		{
			str[j++] = buf[i];
		}
	}
	str[j] = 0;
	free(buf);
	
	return;
}

static void _delete_json(char *json)
{
	free(json);
	json = NULL;

	return;
}

static char *_get_json(int ip_num, ...)
{
	int i;
	char *jsonout;
	host_info_s hi;
	va_list args;
	cJSON *json_root, *json_ip, *json_condition, *json_ip_data, *json_condition_data;

	/* 获取可变参数相关信息*/
	va_start(args, ip_num);
	/* 根据需求构建json串 */
	jsonout = NULL;
	json_root = cJSON_CreateObject();
	json_ip = cJSON_CreateObject();
	json_condition_data = cJSON_CreateObject();
	cJSON_AddStringToObject(json_root, "bk_app_code", "test");
	cJSON_AddStringToObject(json_root, "bk_app_secret", "0d4c361b-510b-46ff-a2ed-adf6a718b500");
	cJSON_AddStringToObject(json_root, "bk_username", "admin");
	cJSON_AddItemToObjectCS(json_root, "ip", json_ip);
	json_condition = cJSON_AddArrayToObject(json_root, "condition");
	json_ip_data = cJSON_AddArrayToObject(json_ip, "data");
	for(i = 0; i < ip_num; i++)
	{
		hi = va_arg(args, host_info_s);
		//cJSON_AddStringToObject(json_ip_data, "ip", hi.ip);
	}
	va_end(args);
	cJSON_AddNumberToObject(json_ip, "exact", 1);
	cJSON_AddStringToObject(json_ip, "flag", "bk_host_innerip|bk_host_outerip");
	cJSON_AddStringToObject(json_condition_data, "bk_obj_id", "set");
	cJSON_AddArrayToObject(json_condition_data, "fields");
	cJSON_AddArrayToObject(json_condition_data, "condition");
	cJSON_AddItemToArray(json_condition, json_condition_data);
	/* 解析成字符串 */
	jsonout = cJSON_Print(json_root);
	/* 销毁json对象，释放空间 */
	cJSON_Delete(json_root);
	/* 去掉json中的tab和换行 */
	_json_format(jsonout, strlen(jsonout));

	return jsonout;
}

static char *_parse_json(char *raw_json)
{
	int i;
	cJSON *jsonout, *retcode, *data, *count, *arrays, *array, *set, *bk_set_name;
	
	jsonout = cJSON_Parse(raw_json);
	retcode = cJSON_GetObjectItem(jsonout, "code");
	if(retcode->valueint != 0)
	{
		fprintf(stderr, "%s\n", "Returned code error!");
		return NULL;
	}
	data = cJSON_GetObjectItem(jsonout, "data");
	count = cJSON_GetObjectItem(data, "count");
	arrays = cJSON_GetObjectItem(data, "info");

	printf("%d\n", count->valueint);

	link_init();
	for(i = 0; i < count->valueint; i++)
	{
		array = cJSON_GetArrayItem(arrays, i);
		set = cJSON_GetObjectItem(array, "set")->child;
		bk_set_name = cJSON_GetObjectItemCaseSensitive(set, "bk_set_name");
		link_insert(bk_set_name->valuestring, strlen(bk_set_name->valuestring));
		//printf("%s\n", bk_set_name->valuestring);
	}
	cJSON_Delete(jsonout);

	return NULL;
}

static int _writedata_curl(char *ptr, int size, int nmemb, void *userdata)
{
    char **buffer;
    int realsize;

    realsize = size * nmemb;
    if (userdata != NULL)
    {
        buffer = userdata;
        *buffer = realloc(*buffer, strlen(*buffer) + realsize + 1);
        strncat(*buffer, ptr, realsize);
    }
    
    return realsize;
}

static int _post_by_curl(char *url, char *json, char **response)
{
    long status_code;
	char *payload;
    CURLcode c;
    CURL *handle;

    handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(handle, CURLOPT_URL, url); /* 设置URL */
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, _writedata_curl); /* 设置写数据的函数 */
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, response); /* 设置写数据的变量 */
    if (json != NULL)
    {
		payload = curl_easy_escape(NULL, json, 0);
        curl_easy_setopt(handle, CURLOPT_POSTFIELDS, json); /* 传递一个作为HTTP “POST”操作的所有数据的字符串 */
    }

	c = curl_easy_perform(handle);
    if (c == CURLE_OK)
    {
        status_code = 0;
        if (curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status_code) == CURLE_OK)
        {
            c = status_code;
        }
    }
    curl_easy_cleanup(handle);
	free(payload);

    return c;
}

int main(int argc, char *argv[])
{
	char *json, *retdata;
	char *bk_set_name;

	host_info_s s1, s2;
	memset(&s1, 0, sizeof(s1));
	memset(&s2, 0, sizeof(s2));
	strcpy(s1.ip, "10.172.33.23");
	strcpy(s2.ip, "10.172.49.1");
	//json = _get_json(2, s1, s2);
	json = _get_json(0);
	retdata = calloc(1, sizeof (char));
	_post_by_curl(URL, json, &retdata);
	_delete_json(json);
	_parse_json(retdata);

	bk_set_name = link_search(0);
	if(bk_set_name == NULL)
	{
		strcpy(s1.set_name, "Unknown");
	}
	else
	{
		strcpy(s1.set_name, bk_set_name);
	}

	bk_set_name = link_search(1);
	if(bk_set_name == NULL)
	{
		strcpy(s2.set_name, "Unknown");
	}
	else
	{
		strcpy(s2.set_name, bk_set_name);
	}
	printf("%s:%s\n", s1.ip, s1.set_name);
	printf("%s:%s\n", s2.ip, s2.set_name);
	link_delete();
	
	return 0;
}

#ifdef __cplusplus 
} 
#endif 
