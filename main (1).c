#include "weather_functions.h"

#include <stdio.h>



int main(void) {

    // Weather-related code

    const char *api_link = "https://api.openweathermap.org/data/2.5/weather";

    const char *api_key = "9c9aef63f7c293a8286dfcb1baa5d2e6";

    const char *location = "Karachi";

    const char *data_file = "data.txt";

    const int pressure_threshold = 1000; // Set your pressure threshold here



    // Construct URL

    char url[256];

    snprintf(url, sizeof(url), "%s?q=%s&appid=%s", api_link, location, api_key);



    CURL *curl;

    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();



    if (curl) {

        curl_easy_setopt(curl, CURLOPT_URL, url);

        json_t *json = NULL;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);

        res = curl_easy_perform(curl);



        if (res == CURLE_OK) {

            json_t *main_data = json_object_get(json, "main");



            if (main_data != NULL) {

                json_t *pressure_json = json_object_get(main_data, "pressure");



                if (pressure_json != NULL && json_is_integer(pressure_json)) {

                    int pressure_val = json_integer_value(pressure_json);



                    // Append pressure data to text file

                    append_pressure_to_data_file(pressure_val);

                    

                    // Check if pressure exceeds the threshold

                    if (pressure_val > pressure_threshold) {

                        // Send email with the alert message

                        const char *subject = "WEATHER ALERT!!!";

                        const char *body = "Atmospheric pressure has exceeded the threshold. Take necessary precautions.";

                        send_email(subject, body);

                    }



                    // Save raw data to a file

                    save_raw_data(json_dumps(json, JSON_COMPACT | JSON_ENSURE_ASCII));



                    // Retrieve humidity information

                    json_t *humidity_json = json_object_get(main_data, "humidity");

                    if (humidity_json != NULL && json_is_integer(humidity_json)) {

                        int humidity_val = json_integer_value(humidity_json);



                        // Append humidity data to text file

                        append_humidity_to_data_file(humidity_val);



                        // Process and save data

                        process_and_save_data(pressure_val, humidity_val);

                    }

                }

            }



            // Calculate and save trends

            calculate_and_save_trends();

        } else {

            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        }



        json_decref(json);

        curl_easy_cleanup(curl);

    }



    curl_global_cleanup();

    return 0;

}

