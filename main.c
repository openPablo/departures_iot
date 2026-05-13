#include "libs/json.h"
#include "libs/requests.c"
#include <curl/curl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct traminfo {
  char *bestemmingKort[30];
  int lijnnummer;
  char *dienstregelingTijdstip[30];
  int delayMinuten;
};

int main() {
  int http_res, i;
  struct memory *payload = calloc(1, 17000);
  struct json_value_s *root;
  struct json_object_s *object;
  char header[255];
  char *url = "https://api.delijn.be/DLKernOpenData/api/v1/haltes/1/105785/"
              "real-time?maxAantalDoorkomsten=6";
  CURL *curl = curl_easy_init();
  if (getenv("DELIJN_API_KEY") == NULL) {
    printf("Missing DELIJN_API_KEY environment variable.\n");
    exit(EXIT_FAILURE);
  }
  snprintf(header, sizeof(header), "Ocp-Apim-Subscription-Key: %s",
           getenv("DELIJN_API_KEY"));
  if (!curl) {
    fprintf(stderr, "curl_easy_init() failed\n");
    exit(EXIT_FAILURE);
  }

  http_res = get(curl, url, payload, header);
  if (http_res != 200) {
    printf("%s\n", payload->response);
  } else {
    root = json_parse(payload->response, payload->size);
    object = (struct json_object_s *)root->payload;
    struct json_object_element_s *halteDoorkomsten = object->start;
    struct json_array_s *haltes =
        (struct json_array_s *)halteDoorkomsten->value;
    struct json_array_element_s *halteElement =
        (struct json_array_element_s *)haltes->start;
    while (halteElement) {
      struct json_object_s *halte = (struct json_object_s *)halteElement->value;
      struct json_object_element_s *doorkomsten = halte->start->next;
      struct json_array_s *doorkomstArray =
          (struct json_array_s *)doorkomsten->value;
      struct json_array_element_s *doorkomst =
          (struct json_array_element_s *)doorkomstArray->start;
      while (doorkomst) {

        doorkomst = doorkomst->next;
      }
      halteElement = halteElement->next;
    }
  }

  curl_easy_cleanup(curl);
  free(payload);
  free(root);
}
