#define _XOPEN_SOURCE

#include "libs/frozen.c"
#include "libs/requests.c"
#include <curl/curl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Get haltes from delijn here:
// https://data.delijn.be/api-details#api=KernOpenDataServicesV1&operation=get-haltes-indebuurt-latlng

struct traminfo {
  char bestemmingKort[30];
  short lijnnummer;
  short minuten_arrival;
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

void parse_tram_json(struct memory *payload, struct traminfo trams[10],
                     int *tram_index) {
  struct json_token t;

  for (int i = 0;
       json_scanf_array_elem(payload->response, payload->size,
                             ".halteDoorkomsten[0].doorkomsten", i, &t) > 0;
       i++) {
    char *bestemming = NULL, *tijd = NULL;
    int lijn = 0;
    json_scanf(t.ptr, t.len,
               "{lijnnummer: %d, bestemmingKort: %Q, real-timeTijdstip: %Q}",
               &lijn, &bestemming, &tijd);

    if (bestemming && tijd) {
      trams[*tram_index].lijnnummer = lijn;
      snprintf(trams[*tram_index].bestemmingKort,
               sizeof(trams[*tram_index].bestemmingKort), "%s", bestemming);
      trams[*tram_index].minuten_arrival = minutes_till_arrival(tijd);
      (*tram_index)++;
    }
    free(bestemming);
    free(tijd);
  }
}
// ToDo: implement quicksort
void quicksort() {}
int main() {
  char *haltes[] = {
      "101680", // Clara snellings
      "105785"  // venneborglaan
  };
  int haltes_length = 2;
  int requested_trams = 5;
  int http_res, found_trams = 0;
  struct memory *payload;
  char header[255], url[255];
  struct traminfo *trams =
      malloc(haltes_length * requested_trams * sizeof(struct traminfo));
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

  for (int k = 0; k < haltes_length; k++) {
    payload = calloc(1, 17000);
    snprintf(url, sizeof(url),
             "https://api.delijn.be/DLKernOpenData/api/v1/haltes/1/%s/"
             "real-time?maxAantalDoorkomsten=%d",
             haltes[k], requested_trams);
    http_res = get(curl, url, payload, header);
    if (http_res != 200) {
      printf("%s\n", payload->response);
    } else {
      parse_tram_json(payload, trams, &found_trams);
    }
  }
  for (int i = 0; i < found_trams; i++) {
    printf("%d %s: %d minuten \n", trams[i].lijnnummer, trams[i].bestemmingKort,
           trams[i].minuten_arrival);
  }
  curl_easy_cleanup(curl);
  free(payload);
}
