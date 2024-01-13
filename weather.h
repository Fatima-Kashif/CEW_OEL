// weather_functions.h

#ifndef WEATHER_FUNCTIONS_H

#define WEATHER_FUNCTIONS_H



#include <stdio.h>

#include <string.h>

#include <curl/curl.h>

#include <jansson.h>



struct EmailBodyInfo {

    const char *data;

    size_t size;

};



size_t write_callback(void *contents, size_t size, size_t nmemb, json_t **json);

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userdata);

void save_raw_data(const char *raw_data);

void process_and_save_data(int pressure_val, int humidity_val);

void calculate_and_save_trends();

void send_email(const char *subject, const char *body);

void append_to_data_file(const char *filename, const char *format, ...);

void append_pressure_to_data_file(int pressure_val);

void append_humidity_to_data_file(int humidity_val);

void append_trends_to_processed_file(double pressure_average, int pressure_min, int pressure_max,

                                     int pressure_difference, double humidity_average, int humidity_min,

                                     int humidity_max, int humidity_difference);



#endif // WEATHER_FUNCTIONS_H

