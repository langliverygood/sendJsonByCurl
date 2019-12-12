#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <malloc.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

void json_format(char *str, int len)
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

char *http_url_escape(char *url, int len)
{
	int i, j;
	char *buf;

	buf = (char *)malloc(len * 3);
	for(i = 0, j = 0; i < len; i++)
	{
		if(url[i] == '\"')
		{
		   strncpy(&buf[j], "%22", 3);
		   j += 3;
		}
		else if(url[i] == '{')
		{
			strncpy(&buf[j], "%7b", 3);
		    j += 3;
		}
		else if(url[i] == '}')
		{
		    strncpy(&buf[j], "%7d", 3);
		    j += 3;
		}
		else if(url[i] == ',')
		{
		    strncpy(&buf[j], "%2c", 3);
		    j += 3;
		}
		else if(url[i] == ':')
		{
		    strncpy(&buf[j], "%3a", 3);
		    j += 3;
		}
		else
		{
		    buf[j++] = url[i];
		}
	}
	buf[j] = 0;

	return buf;
}

void delete_json(char *json)
{
	free(json);
	json = NULL;

	return;
}

char *get_json()
{
	char * jsonout;
	cJSON * json_root, *json_ip, *json_condition, *json_ip_data, *json_condition_data;

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
	cJSON_AddStringToObject(json_ip_data, "ip", "10.172.33.23");
	cJSON_AddNumberToObject(json_ip, "exact", 1);
	cJSON_AddStringToObject(json_ip, "flag", "bk_host_innerip|bk_host_outerip");
	cJSON_AddStringToObject(json_condition_data, "bk_obj_id", "module");
	cJSON_AddArrayToObject(json_condition_data, "fields");
	cJSON_AddItemToArray(json_condition, json_condition_data);
	
	/* 解析成字符串 */
	jsonout = cJSON_Print(json_root);
	/* 销毁json对象，释放空间 */
	cJSON_Delete(json_root);
	/*cJSON_Delete(json_ip);
	cJSON_Delete(json_condition);
	cJSON_Delete(json_ip_data);
	cJSON_Delete(json_condition_data);*/
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
		//printf("%s\n", payload);
        curl_easy_setopt(handle, CURLOPT_POSTFIELDS, payload); /* 传递一个作为HTTP “POST”操作的所有数据的字符串 */
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

	json = get_json();
	sprintf(url, "%s", "http://paas.bk.com:80/api/c/compapi/v2/cc/search_host/");
	retdata = calloc(1, sizeof (char));
	_post_by_curl(url, json, &retdata);
	
	printf("%s\n", retdata);

	return 0;
}
#if 0
uint32_t get_ipaddr_by_host(char *host)
{
	struct hostent *h;
	struct sockaddr_in addr_in;
	
	h = gethostbyname(host);
	if(h == NULL)
	{
		fprintf(stderr, "Can't get %s's ip address!\n", host);
		return 0;
	}
	else
	{
		memcpy(&addr_in.sin_addr.s_addr, h->h_addr, sizeof(addr_in.sin_addr.s_addr));
	}

	return addr_in.sin_addr.s_addr;
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
    char payload[2048], receive[2048], post[256], host[256], acpt[128], con_len[128], con_type[128];
	char *json, *request;
    struct sockaddr_in addr;

	json = get_json();
	if(json == NULL)
	{
		fprintf(stderr, "Can't get json!\n");
		return -1;
	}
	
    while((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1);

	memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = get_ipaddr_by_host("paas.bk.com");
	if(addr.sin_addr.s_addr == 0)
	{
		return -1;
	}
	memset(acpt, 0, sizeof(acpt));

	sprintf(post, "post %s Http/1.1\n", "/api/c/compapi/v2/cc/search_host/");
	sprintf(host, "Host:%s\n", "paas.bk.com");
	sprintf(acpt, "Accept:*/*\n");
	sprintf(con_len, "Content-Length:%d\n", (int)strlen(json));
	sprintf(con_type, "Content-Type:%s\n", "application/x-www-form-urlencoded");
	sprintf(payload, "%s%s%s%s%s\n%s\n", post, host, acpt, con_len, con_type, json);
	request = http_url_escape(payload, strlen(payload));
    
    while(connect (sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1);

    numbytes = send(sockfd, request, strlen(request), 0);
	printf("send:\n%s\n", request);
    numbytes = recv(sockfd, receive, sizeof(receive), 0);  
    receive[numbytes] = '\0';
	
    printf("received:\n%s\n", receive);  
    close(sockfd);
	delete_json(json);
	
    return 0;
}

#endif

