#ifndef BOOKING_H
#define BOOKING_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define Row 1000
#define mx_id 20
#define mx_room 10
#define mx_name 64
#define mx_date 12
#define mx_time 20

#define BOLD "\033[1m"
#define BLUE "\033[38;2;82;59;255m"
#define CYAN "\033[36m"
#define RESET "\033[0m"
#define MAGENTA "\033[35m"
#define RED "\033[31m"
#define YELLOW "\033[38;2;255;255;153m"
#define GREEN "\033[32m"
#define WHITE "\033[37m"

typedef struct {
    char BookingID[Row][mx_id];
    char BookerName[Row][mx_name];
    char MeetingRoom[Row][mx_room];
    char BookingDate[Row][mx_date];
    char BookingTime[Row][mx_time];
    char tmpo[10][mx_name];
} DB;

void save_all(DB *db, char *file_name);
void show_log(const DB *db);
char *to_lower_str(const char *s);
int id_num_ch(char* id);
int ch_time(const char *time, int *start_m, int *end_m);
int overlaps(DB *db, const char *name, const char *room, const char *time, const char *date, int exclude_idx);
void show_bookings_for(const DB *db, const char *date, const char *room);
void trim(char *s);
int has_forbidden_chars(const char *s);
int space_check(char *s);
int stdin_edit_add(DB *db);
int stdin_sch(DB *db);
int stdin_id_match(DB *db);
char comfirm_pms(void);
int search_user(DB *db);
void add_user(DB *db, char *file_name);
void update_user(DB *db, char *file_name);
void delete_user(DB *db, char *file_name);
void load_file(DB *db, char *file_name);
void display(DB *db, char *csv);
int parse_date_ddmmyyyy(const char *s, struct tm *out_tm, time_t *out_midnight);
int validate_booking_datetime(const char *date_str, const char *time_str, char *err, size_t errsz);

int run_unit_tests(void);
int run_e2e_tests(void);

#endif
