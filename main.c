#include "gtfs_standard.pb-c.h"
#include "requests.c"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  char *url = "https://api.delijn.be/gtfs/v3/realtime?position=true";
  char header[255];
  snprintf(header, sizeof(header), "Ocp-Apim-Subscription-Key: %s",
           getenv("DELIJN_API_KEY"));
  struct memory *payload = calloc(1, 17000);
  CURL *curl = curl_easy_init();
  if (!curl) {
    fprintf(stderr, "curl_easy_init() failed\n");
    return EXIT_FAILURE;
  }

  get(curl, url, payload, header);

  FILE *f = fopen("result", "w");
  if (!f) {
    printf("Failed to open file\n");
  }
  fwrite(payload->response, payload->size, 1, f);
  printf("Saved result\n");
  curl_easy_cleanup(curl);
  free(payload);
}
