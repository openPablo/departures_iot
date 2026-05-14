#define _XOPEN_SOURCE

#include "libs/frozen.c"
#include "libs/requests.c"
#include <curl/curl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
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
void swap(struct traminfo *trams, int i, int k) {
  struct traminfo tmp = trams[i];
  trams[i] = trams[k];
  trams[k] = tmp;
}

int partition(struct traminfo *trams, int low, int high) {
  int pivot = trams[high].minuten_arrival;
  int i = low - 1;
  for (int j = low; j < high; j++) {
    if (trams[j].minuten_arrival < pivot) {
      i++;
      swap(trams, i, j);
    }
  }
  swap(trams, i + 1, high);
  return i + 1;
}
void quick_sort(struct traminfo *trams, int low, int high) {
  if (low < high) {
    int partition_indx = partition(trams, low, high);

    quick_sort(trams, low, partition_indx - 1);
    quick_sort(trams, partition_indx + 1, high);
  }
}
void display_trams(struct traminfo *trams, int found_trams) {
  for (int i = 0; i < found_trams; i++) {
    printf("%d %s: %d minuten \n", trams[i].lijnnummer, trams[i].bestemmingKort,
           trams[i].minuten_arrival);
  }
}
int main() {
  char *haltes[] = {
      "101680", // Clara snellings
      "105785"  // venneborglaan
  };
  int haltes_length = 2;
  int found_trams = 0;
  int requested_trams = 5;
  char (*urls)[255] = malloc(haltes_length * 255 * sizeof(char));
  char header[255];
  struct memory *payload;
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
    snprintf(urls[k], 255,
             "https://api.delijn.be/DLKernOpenData/api/v1/haltes/1/%s/"
             "real-time?maxAantalDoorkomsten=%d",
             haltes[k], requested_trams);
  }
  for (;;) {
    for (int k = 0; k < haltes_length; k++) {
      payload = calloc(1, 17000);
      if (get(curl, urls[k], payload, header) != 200) {
        printf("%s\n", payload->response);
      } else {
        parse_tram_json(payload, trams, &found_trams);
      }
    }
    quick_sort(trams, 0, found_trams - 1);
    display_trams(trams, found_trams);
    found_trams = 0;
    sleep(5);
  }
  curl_easy_cleanup(curl);
  free(payload);
}
