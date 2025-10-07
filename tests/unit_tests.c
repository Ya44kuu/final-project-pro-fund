#include "booking.h"
#include <assert.h>

static int g_failed = 0, g_passed = 0;

#define CHECK(MSG, COND) \
  do { \
    if (COND) { printf(GREEN "  PASS " RESET "%s\n", MSG); ++g_passed; } \
    else { printf(RED   "  FAIL " RESET "%s\n", MSG); ++g_failed; } \
  } while (0)

 static void date_plus_days(int days, char out[mx_date]){
    time_t now = time(NULL) + (time_t)days*24*60*60;
    struct tm t;
 #ifdef _WIN32
       localtime_s(&t, &now);
 #else
       localtime_r(&now, &t);
 #endif
     
     if (strftime(out, mx_date, "%d/%m/%Y", &t) == 0) {
         snprintf(out, mx_date, "01/01/1970");
     }
}

static void seed_booking(DB *db, int i,
                         const char *id, const char *name, const char *room,
                         const char *date, const char *time) {
    snprintf(db->BookingID[i],   mx_id,  "%s", id);
    snprintf(db->BookerName[i],  mx_name,"%s", name);
    snprintf(db->MeetingRoom[i], mx_room,"%s", room);
    snprintf(db->BookingDate[i], mx_date,"%s", date);
    snprintf(db->BookingTime[i], mx_time,"%s", time);
}


static void write_text_file(const char *path, const char *content){
    FILE *f = fopen(path, "wb");
    assert(f && "open temp input file");
    fputs(content, f);
    fclose(f);
}
#ifdef _WIN32
  #define CONIN "CON"
#else
  #define CONIN "/dev/tty"
#endif
static void redirect_stdin_from(const char *path){
    FILE *fp = freopen(path, "r", stdin);
    assert(fp && "freopen stdin failed");
}
static void restore_stdin_console(void){
    FILE *fp = freopen(CONIN, "r", stdin);
    assert(fp && "restore stdin failed");
}


static void ensure_tests_dir(void){
#ifdef _WIN32
    system("if not exist tests mkdir tests >nul 2>nul");
#else
    system("mkdir -p tests >/dev/null 2>&1");
#endif
}


typedef int (*test_fn)(void);
typedef struct { const char *name; test_fn fn; } TestCase;

static int run_test_case(const TestCase *tc){
    int rc = tc->fn();
    if (rc==0) printf(BOLD GREEN "✓ " RESET "%s\n", tc->name);
    else       printf(BOLD RED   "✗ " RESET "%s\n", tc->name);
    return rc;
}


static int test_to_lower_str_(void){
    // Arrange
    const char *in = "AbC123";
    // Act
    char *out = to_lower_str(in);
    // Assert
    assert(out);
    CHECK("to_lower_str converts", strcmp(out,"abc123")==0);
    free(out);
    return 0;
}
static int test_trim_(void){
    // Arrange
    char s[mx_name] = "   hello world   ";
    // Act
    trim(s);
    // Assert
    CHECK("trim removes leading/trailing spaces", strcmp(s,"hello world")==0);
    return 0;
}
static int test_space_check_(void){
    // Arrange / Act / Assert
    CHECK("space_check detects space", space_check("has space")==1);
    CHECK("space_check passes nospace", space_check("nospace")==0);
    return 0;
}

/* -------- ch_time -------- */
static int test_ch_time_valid_(void){
    // Arrange
    int s=0,e=0;
    // Act
    int ok = ch_time("09:15-10:45",&s,&e);
    // Assert
    CHECK("ch_time parses", ok==1);
    CHECK("start minutes", s==9*60+15);
    CHECK("end minutes",   e==10*60+45);
    return 0;
}
static int test_ch_time_invalid_(void){
    int s=0,e=0;
    CHECK("ch_time rejects invalid hour", ch_time("25:00-26:00",&s,&e)==0);
    CHECK("ch_time rejects equal range",  ch_time("13:00-13:00",&s,&e)==0);
    CHECK("ch_time rejects bad format",   ch_time("1300-1400",&s,&e)==0);
    return 0;
}

/* -------- parse_date_ddmmyyyy -------- */
static int test_parse_date_(void){
    struct tm tmv; time_t tt;
    CHECK("parse_date real date",   parse_date_ddmmyyyy("31/01/2025",&tmv,&tt)==1);
    CHECK("parse_date leap day",    parse_date_ddmmyyyy("29/02/2024",&tmv,&tt)==1);
    CHECK("parse_date rejects 29/02/2025", parse_date_ddmmyyyy("29/02/2025",&tmv,&tt)==0);
    CHECK("parse_date rejects 31/04/2025", parse_date_ddmmyyyy("31/04/2025",&tmv,&tt)==0);
    return 0;
}

/* -------- validate_booking_datetime -------- */
static int test_validate_booking_datetime_(void){
    char tomorrow[mx_date]; date_plus_days(1, tomorrow);
    char fourdays[mx_date]; date_plus_days(4, fourdays);
    char yesterday[mx_date]; date_plus_days(-1, yesterday);
    char err[128];

    CHECK("validate ok (tomorrow, 09:00-10:00)", validate_booking_datetime(tomorrow,"09:00-10:00",err,sizeof(err))==0);
    CHECK("validate rejects past date",          validate_booking_datetime(yesterday,"09:00-10:00",err,sizeof(err))==4);
    CHECK("validate rejects >3 days",            validate_booking_datetime(fourdays,"09:00-10:00",err,sizeof(err))==5);
    CHECK("validate rejects bad time fmt",       validate_booking_datetime(tomorrow,"0900-1000",err,sizeof(err))==1);
    return 0;
}

/* -------- overlaps -------- */
static int test_overlaps_(void){
    DB db = (DB){0};
    seed_booking(&db,0,"X1","Alice","A","01/10/2099","09:00-10:00");

    CHECK("overlaps same room conflict",
          overlaps(&db,"Bob","A","09:30-09:45","01/10/2099",-1)==1);
    CHECK("overlaps same person diff room conflict",
          overlaps(&db,"Alice","B","09:30-09:45","01/10/2099",-1)==1);
    CHECK("overlaps ignores diff room/person",
          overlaps(&db,"Bob","B","09:30-09:45","01/10/2099",-1)==0);
    CHECK("overlaps edge end==start OK",
          overlaps(&db,"Bob","A","10:00-10:30","01/10/2099",-1)==0);
    CHECK("overlaps bad time → -1",
          overlaps(&db,"Bob","A","1000-1030","01/10/2099",-1)==-1);
    CHECK("overlaps excludes self (editing)",
          overlaps(&db,"Alice","A","09:00-10:00","01/10/2099",0)==0);
    return 0;
}

/* -------- save_all + load_file + show_bookings_for -------- */
static int test_save_load_show_(void){
    ensure_tests_dir();
#ifdef _WIN32
    const char *csv = "tests\\tmp_unit_roundtrip.csv";
#else
    const char *csv = "tests/tmp_unit_roundtrip.csv";
#endif
    // Arrange
    DB db = (DB){0};
    seed_booking(&db,0,"S1","Sam","A","01/01/2099","09:00-10:00");
    seed_booking(&db,1,"S2","Sara","A","01/01/2099","10:00-11:00");

    // Act
    save_all(&db, (char*)csv);
    DB db2 = (DB){0};
    load_file(&db2,(char*)csv);

    // Assert
    CHECK("round-trip row0 id/name", strcmp(db2.BookingID[0],"S1")==0 && strcmp(db2.BookerName[0],"Sam")==0);
    CHECK("round-trip row1 time",    strcmp(db2.BookingTime[1],"10:00-11:00")==0);

    show_bookings_for(&db2,"01/01/2099","A");
    return 0;
}


static int test_search_user_ui_(void){
    ensure_tests_dir();

    // Arrange
    DB db = (DB){0};
    seed_booking(&db,0,"ID1","Alice","A","01/01/2099","09:00-10:00");
    seed_booking(&db,1,"ID2","Alicia","B","01/01/2099","10:00-11:00");
    const char *in_path = "tests/tmp_in_search.txt";
    write_text_file(in_path, "Ali\nID1\n");

    // Act
    redirect_stdin_from(in_path);
    int idx = search_user(&db);
    restore_stdin_console();

    // Assert
    CHECK("search_user returns found index 0", idx==0);
    return 0;
}

static int test_add_user_ui_(void){
    ensure_tests_dir();

    // Arrange
    DB db = (DB){0};
    char d[mx_date]; date_plus_days(1, d); // พรุ่งนี้ 
    const char *in_path = "tests/tmp_in_add.txt";
    char script[256];
    snprintf(script,sizeof(script),
        "U001\n"      
        "Alice\n"     
        "A\n"         
        "%s\n"        
        "09:00-10:00\n", d); 
    write_text_file(in_path, script);

#ifdef _WIN32
    const char *csv = "tests\\tmp_add.csv";
#else
    const char *csv = "tests/tmp_add.csv";
#endif

    // Act
    redirect_stdin_from(in_path);
    add_user(&db,(char*)csv);
    restore_stdin_console();

    // Assert
    CHECK("add_user wrote id/name", strcmp(db.BookingID[0],"U001")==0 && strcmp(db.BookerName[0],"Alice")==0);
    return 0;
}

static int test_edit_user_ui_(void){
    ensure_tests_dir();

    // Arrange
    DB db = (DB){0};
    char d[mx_date]; date_plus_days(1, d);
    seed_booking(&db,0,"U001","Alice","A",d,"09:00-10:00");
    const char *in_path = "tests/tmp_in_edit.txt";
    char script[512];
    snprintf(script,sizeof(script),
        "Alice\n"       
        "U001\n"        
        "U001\n"        
        "Alice\n"       
        "A\n"           
        "%s\n"           
        "10:00-11:00\n", d);
    write_text_file(in_path, script);

#ifdef _WIN32
    const char *csv = "tests\\tmp_edit.csv";
#else
    const char *csv = "tests/tmp_edit.csv";
#endif

    // Act
    redirect_stdin_from(in_path);
    edit_user(&db,(char*)csv);
    restore_stdin_console();

    // Assert
    CHECK("edit_user changed time", strcmp(db.BookingTime[0],"10:00-11:00")==0);
    return 0;
}

static int test_delete_user_ui_(void){
    ensure_tests_dir();

    // Arrange
    DB db = (DB){0};
    seed_booking(&db,0,"D001","Dave","A","01/01/2099","09:00-10:00");
    const char *in_path = "tests/tmp_in_delete.txt";
    write_text_file(in_path, "Dave\nD001\ny\n");

#ifdef _WIN32
    const char *csv = "tests\\tmp_delete.csv";
#else
    const char *csv = "tests/tmp_delete.csv";
#endif

    // Act
    redirect_stdin_from(in_path);
    delete_user(&db,(char*)csv);
    restore_stdin_console();

    // Assert
    CHECK("delete_user clears row0",
          db.BookingID[0][0]=='\0' && db.BookerName[0][0]=='\0' &&
          db.MeetingRoom[0][0]=='\0' && db.BookingTime[0][0]=='\0');
    return 0;
}


int run_unit_tests(void){
    printf(BOLD CYAN "Running unit tests...\n" RESET);

    const TestCase cases[] = {

        {"to_lower_str",               test_to_lower_str_},
        {"trim",                       test_trim_},
        {"space_check",                test_space_check_},
        {"ch_time (valid/invalid)",    test_ch_time_valid_},
        {"ch_time (invalid cases)",    test_ch_time_invalid_},
        {"parse_date_ddmmyyyy",        test_parse_date_},
        {"validate_booking_datetime",  test_validate_booking_datetime_},
        {"overlaps",                   test_overlaps_},
        {"save/load/show",             test_save_load_show_},

        {"search_user",      test_search_user_ui_},
        {"add_user",         test_add_user_ui_},
        {"edit_user",        test_edit_user_ui_},
        {"delete_user",      test_delete_user_ui_},
    };

    int failed = 0;
    for (size_t i=0;i<sizeof(cases)/sizeof(cases[0]);++i){
        failed += run_test_case(&cases[i]);
    }

    printf("\n");
    if (failed==0){
        printf(BOLD GREEN "All unit tests passed!  " RESET "(%d checks passed)\n", g_passed);
    } else {
        printf(BOLD RED "%d unit test(s) failed. " RESET "(%d checks passed, %d failed)\n", failed, g_passed, g_failed);
    }
    return failed;
}
