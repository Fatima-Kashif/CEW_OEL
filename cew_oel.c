#include <stdio.h>

#include <string.h>

#include <curl/curl.h>

#include <jansson.h>

#include <time.h>



// Struct to hold email body information

struct EmailBodyInfo

{

    const char *data;

    size_t size;

};



size_t write_callback(void *contents, size_t size, size_t nmemb, json_t **json)

{

    size_t total_size = size * nmemb;

    json_error_t error;

    *json = json_loadb(contents, total_size, 0, &error);

    return total_size;

}



size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userdata)

{

    struct EmailBodyInfo *email_body = (struct EmailBodyInfo *)userdata;

    size_t total_size = size * nmemb;



    if (email_body->size > 0)

    {

        size_t copy_size = (total_size < email_body->size) ? total_size : email_body->size;

        memcpy(ptr, email_body->data, copy_size);

        email_body->data += copy_size;

        email_body->size -= copy_size;

        return copy_size;

    }

    else

    {

        return 0; // Signal end of data

    }

}



void save_raw_data(const char *raw_data)

{

    FILE *raw_file = fopen("raw_file.json", "a");

    if (raw_file != NULL)

    {

        fprintf(raw_file, "%s\n", raw_data); // Add a newline character

        fclose(raw_file);

    }

}



void process_and_save_data(int pressure_val, int humidity_val)

{

    FILE *processed_file = fopen("processed.txt", "a");

    if (processed_file != NULL)

    {

        fprintf(processed_file, "Pressure: %d hPa\n", pressure_val);

        fprintf(processed_file, "Humidity: %d%%\n\n", humidity_val); // Add a newline character

        fclose(processed_file);

    }

}



void calculate_and_save_trends()

{

    FILE *file = fopen("data.txt", "r");



    int pressure, humidity;

    int num_data_points = 0;

    int pressure_sum = 0, humidity_sum = 0;

    int pressure_min, pressure_max, humidity_min, humidity_max;



    // Loop through the data

    while (fscanf(file, "Pressure: %d hPa\nHumidity: %d%%", &pressure, &humidity) == 2)

    {

        pressure_sum += pressure;

        humidity_sum += humidity;



        if (num_data_points == 0 || pressure < pressure_min)

            pressure_min = pressure;

        if (num_data_points == 0 || pressure > pressure_max)

            pressure_max = pressure;



        if (num_data_points == 0 || humidity < humidity_min)

            humidity_min = humidity;

        if (num_data_points == 0 || humidity > humidity_max)

            humidity_max = humidity;



        num_data_points++;

    }



    fclose(file);



    if (num_data_points > 0)

    {

        double pressure_average = (double)pressure_sum / num_data_points;

        double humidity_average = (double)humidity_sum / num_data_points;

        int pressure_difference = pressure_max - pressure_min;

        int humidity_difference = humidity_max - humidity_min;



        FILE *trends_file = fopen("processed.txt", "a");

        if (trends_file != NULL)

        {

            fprintf(trends_file, "\nTrends:\n");

            fprintf(trends_file, "Pressure Average: %.2f hPa\n", pressure_average);

            fprintf(trends_file, "Pressure Minimum: %d hPa\n", pressure_min);

            fprintf(trends_file, "Pressure Maximum: %d hPa\n", pressure_max);

            fprintf(trends_file, "Pressure Difference: %d hPa\n", pressure_difference);

            fprintf(trends_file, "Humidity Average: %.2f%%\n", humidity_average);

            fprintf(trends_file, "Humidity Minimum: %d%%\n", humidity_min);

            fprintf(trends_file, "Humidity Maximum: %d%%\n", humidity_max);

            fprintf(trends_file, "Humidity Difference: %d%%\n\n", humidity_difference); // Add a newline character

            fclose(trends_file);

        }

    }

    else

    {

        fprintf(stderr, "No valid data points found in the file\n");

    }

}



void send_email(const char *subject, const char *body)

{

    // Gmail SMTP server details and email details

    #define SMTP_SERVER "smtps://smtp.gmail.com:465"

    #define USERNAME "mehdihassan7887@gmail.com"

    #define PASSWORD "rjds zxll drfy fakx"

    #define TO_ADDRESS "alayrazmi9090@gmail.com"



    struct EmailBodyInfo email_body_info = {body, strlen(body)};



    CURL *curl;

    CURLcode res;

    struct curl_slist *recipients = NULL;



    curl = curl_easy_init();



    if (curl)

    {

        curl_easy_setopt(curl, CURLOPT_URL, SMTP_SERVER);

        curl_easy_setopt(curl, CURLOPT_USERNAME, USERNAME);

        curl_easy_setopt(curl, CURLOPT_PASSWORD, PASSWORD);

        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, USERNAME);

        recipients = curl_slist_append(recipients, TO_ADDRESS);

        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

        curl_easy_setopt(curl, CURLOPT_READDATA, &email_body_info);

        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);



        // Set email subject

        char subject_header[256];

        snprintf(subject_header, sizeof(subject_header), "Subject: %s", subject);

        struct curl_slist *headers = curl_slist_append(NULL, subject_header);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);



        res = curl_easy_perform(curl);



        if (res != CURLE_OK)

        {

            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        }

        else

        {

            printf("Email sent successfully!\n");



            // Append information to log file

            FILE *log_file = fopen("email_log.txt", "a");

            if (log_file != NULL)

            {

                time_t t = time(NULL);

                struct tm *tm_info = localtime(&t);



                fprintf(log_file, "Recipient: %s\n", TO_ADDRESS);

                fprintf(log_file, "Date: %04d-%02d-%02d\n", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday);

                fprintf(log_file, "Time: %02d:%02d:%02d\n", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);

                fprintf(log_file, "Subject: %s\n", subject);

                fprintf(log_file, "Body: %s\n\n", body);



                fclose(log_file);

            }

        }



        curl_slist_free_all(recipients);

        curl_slist_free_all(headers);

        curl_easy_cleanup(curl);

    }

}



int main(void)

{

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



    if (curl)

    {

        curl_easy_setopt(curl, CURLOPT_URL, url);



        json_t *json = NULL;



        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);



        res = curl_easy_perform(curl);



        if (res == CURLE_OK)

        {

            json_t *main_data = json_object_get(json, "main");



            if (main_data != NULL)

            {

                json_t *pressure_json = json_object_get(main_data, "pressure");



                if (pressure_json != NULL && json_is_integer(pressure_json))

                {

                    int pressure_val = json_integer_value(pressure_json);



                    // Append pressure data to text file

                    FILE *data_file_ptr = fopen(data_file, "a");

                    if (data_file_ptr != NULL)

                    {

                        fprintf(data_file_ptr, "Pressure: %d hPa\n", pressure_val);

                        fclose(data_file_ptr);

                        printf("Pressure: %d hPa\n", pressure_val);



                        // Check if pressure exceeds the threshold

                        if (pressure_val > pressure_threshold)

                        {

                            // Send email with the alert message

                            const char *subject = "WEATHER ALERT!!!";

                            const char *body = "Atmospheric pressure has exceeded the threshold. Take necessary precautions.";

                            send_email(subject, body);

                        }



                        // Save raw data to a file

                        save_raw_data(json_dumps(json, JSON_COMPACT | JSON_ENSURE_ASCII));



                        // Retrieve humidity information

                        json_t *humidity_json = json_object_get(main_data, "humidity");

                        if (humidity_json != NULL && json_is_integer(humidity_json))

                        {

                            int humidity_val = json_integer_value(humidity_json);



                            // Append humidity data to text file

                            FILE *data_file_ptr = fopen(data_file, "a");

                            if (data_file_ptr != NULL)

                            {

                                fprintf(data_file_ptr, "Humidity: %d%%\n", humidity_val);

                                fclose(data_file_ptr);

                                printf("Humidity: %d%%\n", humidity_val);



                                // Process and save data

                                process_and_save_data(pressure_val, humidity_val);

                            }

                        }

                    }

                }

            }



            // Calculate and save trends

            calculate_and_save_trends();

        }

        else

        {

            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        }



        json_decref(json);

        curl_easy_cleanup(curl);

    }



    curl_global_cleanup();

    return 0;

}

