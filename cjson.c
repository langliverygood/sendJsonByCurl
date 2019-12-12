#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <malloc.h>
#include <stdarg.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

static void json_format(char *str, int len)
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

static void delete_json(char *json)
{
	free(json);
	json = NULL;

	return;
}

static char *get_json(int ip_num, ...)
{
	char * jsonout;
	va_list args;
	int i;
	cJSON * json_root, *json_ip, *json_condition, *json_ip_data, *json_condition_data;

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
	cJSON_AddNumberToObject(json_root, "bk_biz_id", 19);
	cJSON_AddItemToObjectCS(json_root, "ip", json_ip);
	json_condition = cJSON_AddArrayToObject(json_root, "condition");
	json_ip_data = cJSON_AddArrayToObject(json_ip, "data");
	for(i = 0; i < ip_num; i++)
	{
		cJSON_AddStringToObject(json_ip_data, "ip", va_arg(args, char *));
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
	json_format(jsonout, strlen(jsonout));

	return jsonout;
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
	char url[4096];

	json = get_json(2, "10.172.33.23", "10.172.33.24");
	sprintf(url, "%s", "http://paas.bk.com:80/api/c/compapi/v2/cc/search_host/");
	retdata = calloc(1, sizeof (char));
	_post_by_curl(url, json, &retdata);
	delete_json(json);
	
	printf("%s\n", retdata);

	return 0;
}
