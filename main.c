#define _XOPEN_SOURCE

#include "libs/json.h"
#include "libs/requests.c"
#include <curl/curl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct traminfo {
  char bestemmingKort[30];
  char lijnnummer[4];
  int minuten_arrival;
};

int minutes_till_arrival(const char *timestr) {
  struct tm target_tm = {0};
  time_t now = time(NULL);

  if (strptime(timestr, "%Y-%m-%dT%H:%M:%S", &target_tm) == NULL) {
    return 0;
  }

  target_tm.tm_isdst = -1;
  time_t target_time = mktime(&target_tm);
  double seconds = difftime(target_time, now);
  return (int)(seconds / 60);
}

void parse_tram_json(struct memory *payload, struct traminfo trams[10]) {
  int tram_index;
  struct json_value_s *root;
  struct json_object_s *object;
  root = json_parse(payload->response, payload->size);
  object = (struct json_object_s *)root->payload;

  struct json_object_element_s *halteDoorkomsten = object->start;
  struct json_array_s *haltes = (struct json_array_s *)halteDoorkomsten->value;
  struct json_array_element_s *halteElement =
      (struct json_array_element_s *)haltes->start;
  while (halteElement) {
    struct json_object_s *halte = (struct json_object_s *)halteElement->value;
    struct json_object_element_s *doorkomsten = halte->start->next;
    struct json_array_s *doorkomstArray =
        (struct json_array_s *)doorkomsten->value;
    struct json_array_element_s *doorkomst =
        (struct json_array_element_s *)doorkomstArray->start;
    tram_index = 0;
    while (doorkomst) {
      struct json_object_s *doorkomstObject =
          (struct json_object_s *)doorkomst->value;
      struct json_object_element_s *doorkomstElement =
          (struct json_object_element_s *)doorkomstObject->start;

      struct traminfo tram;
      while (doorkomstElement) {
        struct json_string_s *jsonStr = doorkomstElement->name;
        struct json_value_s *doorkomstElementValue = doorkomstElement->value;

        if (strcmp("lijnnummer", jsonStr->string)) {
          struct json_number_s *number =
              (struct json_number_s *)doorkomstElementValue->payload;
          snprintf(tram.lijnnummer, sizeof(tram.lijnnummer), "%s",
                   number->number);
        } else if (strcmp("bestemmingKort", jsonStr->string)) {
          struct json_string_s *str =
              (struct json_string_s *)doorkomstElementValue->payload;
          snprintf(tram.bestemmingKort, sizeof(tram.bestemmingKort), "%s",
                   str->string);

        } else if (strcmp("real-timeTijdstip", jsonStr->string)) {
          struct json_string_s *str =
              (struct json_string_s *)doorkomstElementValue->payload;
          tram.minuten_arrival = minutes_till_arrival(str->string);
        }
        doorkomstElement = doorkomstElement->next;
      }
      trams[tram_index] = tram;
      tram_index++;
      doorkomst = doorkomst->next;
    }
    halteElement = halteElement->next;
  }
  free(root);
}

int main() {
  int http_res, i;
  struct memory *payload = calloc(1, 17000);
  char header[255];
  char *url = "https://api.delijn.be/DLKernOpenData/api/v1/haltes/1/105785/"
              "real-time?maxAantalDoorkomsten=10";
  struct traminfo trams[10] = {0};
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
    parse_tram_json(payload, trams);
  }

  for (i = 0; i < 10; i++) {
    printf("%s\n", trams->bestemmingKort);
  }
  curl_easy_cleanup(curl);
  free(payload);
}
