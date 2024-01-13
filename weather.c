// weather_functions.c

#include "weather_functions.h"



size_t write_callback(void *contents, size_t size, size_t nmemb, json_t **json) {

    size_t total_size = size * nmemb;

    json_error_t error;

    *json = json_loadb(contents, total_size, 0, &error);

    return total_size;

}



size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {

    struct EmailBodyInfo *email_body = (struct EmailBodyInfo *)userdata;

    size_t total_size = size * nmemb;



    if (email_body->size > 0) {

        size_t copy_size = (total_size < email_body->size) ? total_size : email_body->size;

        memcpy(ptr, email_body->data, copy_size);

        email_body->data += copy_size;

        email_body->size -= copy_size;

        return copy_size;

    } else {

        return 0; // Signal end of data

    }

}



void save_raw_data(const char *raw_data) {

    FILE *raw_file = fopen("raw_file.json", "a");

    if (raw_file != NULL) {

        fprintf(raw_file, "%s\n", raw_data); // Add a newline character

        fclose(raw_file);

    }

}



void process_and_save_data(int pressure_val, int humidity_val) {

    FILE *processed_file = fopen("processed.txt", "a");

    if (processed_file != NULL) {

        fprintf(processed_file, "Pressure: %d hPa\n", pressure_val);

        fprintf(processed_file, "Humidity: %d%%\n\n", humidity_val); // Add a newline character

        fclose(processed_file);

    }

}



void calculate_and_save_trends() {

     FILE *file = fopen("data.txt", "r");

    int pressure, humidity;

    int num_data_points = 0;

    int pressure_sum = 0, humidity_sum = 0;

    int pressure_min, pressure_max, humidity_min, humidity_max;



    // Loop through the data

    while (fscanf(file, "Pressure: %d hPa\nHumidity: %d%%", &pressure, &humidity) == 2) {

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



    if (num_data_points > 0) {

        double pressure_average = (double)pressure_sum / num_data_points;

        double humidity_average = (double)humidity_sum / num_data_points;

        int pressure_difference = pressure_max - pressure_min;

        int humidity_difference = humidity_max - humidity_min;



        append_trends_to_processed_file(pressure_average, pressure_min, pressure_max,

                                        pressure_difference, humidity_average, humidity_min,

                                        humidity_max, humidity_difference);

    } else {

        fprintf(stderr, "No valid data points found in the file\n");

    }

}



}



void send_email(const char *subject, const char *body) {

    // Gmail SMTP server details and email details

    #define SMTP_SERVER "smtps://smtp.gmail.com:465"

    #define USERNAME "your_email@gmail.com"  // Replace with your Gmail email

    #define PASSWORD "your_email_password"   // Replace with your Gmail app password

    #define TO_ADDRESS "recipient_email@example.com"  // Replace with the recipient's email



    struct EmailBodyInfo email_body_info = {body, strlen(body)};

    CURL *curl;

    CURLcode res;

    struct curl_slist *recipients = NULL;



    curl = curl_easy_init();

    if (curl) {

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

        if (res != CURLE_OK) {

            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        } else {

            printf("Email sent successfully!\n");

        }



        curl_slist_free_all(recipients);

        curl_slist_free_all(headers);

        curl_easy_cleanup(curl);

    }

}

}



void append_to_data_file(const char *filename, const char *format, ...) {

    va_list args;

    va_start(args, format);

    FILE *file = fopen(filename, "a");



    if (file != NULL) {

        vfprintf(file, format, args);

        fclose(file);

    }



    va_end(args);

}



void append_pressure_to_data_file(int pressure_val) {

    append_to_data_file("data.txt", "Pressure: %d hPa\n", pressure_val);

}



void append_humidity_to_data_file(int humidity_val) {

    append_to_data_file("data.txt", "Humidity: %d%%\n", humidity_val);

}



void append_trends_to_processed_file(double pressure_average, int pressure_min, int pressure_max,

                                     int pressure_difference, double humidity_average, int humidity_min,

                                     int humidity_max, int humidity_difference) {

    FILE *trends_file = fopen("processed.txt", "a");

    if (trends_file != NULL) {

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

