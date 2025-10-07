#include "booking.h"


static void seed_booking(DB *db,
                         int idx,
                         const char *id,
                         const char *name,
                         const char *room,
                         const char *date,
                         const char *time) {
    snprintf(db->BookingID[idx], mx_id, "%s", id);
    snprintf(db->BookerName[idx], mx_name, "%s", name);
    snprintf(db->MeetingRoom[idx], mx_room, "%s", room);
    snprintf(db->BookingDate[idx], mx_date, "%s", date);
    snprintf(db->BookingTime[idx], mx_time, "%s", time);
}

static int files_match_booking(const DB *expected, const DB *actual, int count) {
    for (int i = 0; i < count; ++i) {
        if (strcmp(expected->BookingID[i], actual->BookingID[i]) != 0) return 0;
        if (strcmp(expected->BookerName[i], actual->BookerName[i]) != 0) return 0;
        if (strcmp(expected->MeetingRoom[i], actual->MeetingRoom[i]) != 0) return 0;
        if (strcmp(expected->BookingDate[i], actual->BookingDate[i]) != 0) return 0;
        if (strcmp(expected->BookingTime[i], actual->BookingTime[i]) != 0) return 0;
    }
    return 1;
}

static int e2e_save_and_reload_cycle(void) {
    DB db = {0};
    seed_booking(&db, 0, "B001", "Bruce", "B", "02/10/2025", "11:00-12:00");
    seed_booking(&db, 1, "B002", "Clark", "C", "03/10/2025", "15:00-16:00");

    char path[] = "tests/tmp_e2e.csv";
    save_all(&db, path);

    DB loaded = {0};
    load_file(&loaded, path);

    int ok = files_match_booking(&db, &loaded, 2);
    remove(path);
    return ok ? 0 : 1;
}

static int e2e_overlap_after_load(void) {
    DB db = {0};
    seed_booking(&db, 0, "C001", "Carol", "A", "04/10/2025", "09:00-10:00");

    char path[] = "tests/tmp_e2e_overlap.csv";
    save_all(&db, path);

    DB loaded = (DB){0};
    load_file(&loaded, path);

    int ovl = overlaps(&loaded, "Dan", "A", "09:30-10:30", "04/10/2025", -1);
    remove(path);
    return ovl == 1 ? 0 : 1;
}

static int e2e_non_overlap_after_load(void) {
    DB db = {0};
    seed_booking(&db, 0, "D001", "Eve", "D", "05/10/2025", "13:00-14:00");

    char path[] = "tests/tmp_e2e_non_overlap.csv";
    save_all(&db, path);

    DB loaded = (DB){0};
    load_file(&loaded, path);

    int ovl = overlaps(&loaded, "Frank", "D", "14:00-15:00", "05/10/2025", -1);
    remove(path);
    return ovl == 0 ? 0 : 1;
}

typedef int (*test_fn)(void);

typedef struct {
    const char *name;
    test_fn fn;
} TestCase;

static int run_test_case(const TestCase *tc) {
    int rc = tc->fn();
    if (rc == 0) {
        printf(GREEN "  PASS" RESET " %s\n", tc->name);
    } else {
        printf(RED "  FAIL" RESET " %s\n", tc->name);
    }
    return rc;
}

int run_e2e_tests(void) {
    const TestCase cases[] = {
        {"save_all -> load_file roundtrip", e2e_save_and_reload_cycle},
        {"overlaps detects conflicts after load", e2e_overlap_after_load},
        {"overlaps allows free slots after load", e2e_non_overlap_after_load},
    };

    int failed = 0;
    printf(BOLD CYAN "Running end-to-end tests...\n" RESET);
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        failed += run_test_case(&cases[i]);
    }
    if (failed == 0) {
        printf(BOLD GREEN "All end-to-end tests passed!\n" RESET);
    } else {
        printf(BOLD RED "%d end-to-end test(s) failed.\n" RESET, failed);
    }
    return failed;
}

