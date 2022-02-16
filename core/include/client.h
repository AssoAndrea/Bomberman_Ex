#pragma once
#include <time.h>

int client_init();

void send_data(char *data, int len);
void send_position(float x, float y);
void keep_alive(clock_t *start);
void send_auth_req();
char client_is_authorized();

char *receive_data();

int _set_timeout();