#include <curl/curl.h>
#include <curl/header.h>
#include <stdlib.h>
#include <string.h>

struct memory {
  char *response;
  size_t size;
};
// https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
static size_t response_callback(char *data, size_t size, size_t realsize,
                                void *clientp) {
  struct memory *mem = (struct memory *)clientp;

  char *ptr = realloc(mem->response, mem->size + realsize + 1);
  if (!ptr)
    return 0;

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}
long get(CURL *curl, char *url, struct memory *payload, char *header) {
  long http_code = 0;
  if (header != NULL) {
    struct curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, header);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
  }
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, response_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)payload);
  curl_easy_perform(curl);
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  return http_code;
}
