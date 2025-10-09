#include "booking.h"
#include <assert.h>



/* ---- silence stdout (Windows/Linux) ---- */
#ifdef _WIN32
  #include <io.h>
  #define dup    _dup
  #define dup2   _dup2
  #define fileno _fileno
  #define close  _close
  #define DEVNULL "NUL"
#else
  #include <unistd.h>
  #define DEVNULL "/dev/null"
#endif

typedef struct { int saved_fd; int active; } MuteGuard;

static MuteGuard mute_begin(void){
    MuteGuard g = { -1, 0 };
    fflush(stdout);
    int fd = fileno(stdout);
    int dupfd = dup(fd);
    if (dupfd == -1) return g;
    if (!freopen(DEVNULL, "w", stdout)) { close(dupfd); return g; }
    g.saved_fd = dupfd; g.active = 1; return g;
}
static void mute_end(MuteGuard *g){
    if (!g || !g->active) return;
    fflush(stdout);
    dup2(g->saved_fd, fileno(stdout));
    close(g->saved_fd);
    g->saved_fd = -1; g->active = 0;
}
#define SILENCE(EXPR) do { MuteGuard __g = mute_begin(); EXPR; mute_end(&__g); } while(0)
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
typedef int (*test_fn)(void);
typedef struct { const char *name; test_fn fn; } TestCase;

/* Decide per-test pass/fail by delta of g_failed */
static int run_test_case(const TestCase *tc) {
    int failed_before = g_failed;
    (void)tc->fn();                       /* ignore fn's return value */
    int failed_after  = g_failed;
    return (failed_after > failed_before) ? 1 : 0;  /* 1 = this test failed */
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

/*-------------- Storage Full ---------------*/
static int test_add_user_storage_full_(void){
    ensure_tests_dir();

    DB db = (DB){0};
    for (int i = 0; i < Row; ++i) {
        snprintf(db.BookingID[i],   mx_id,  "%d", i+1);
        snprintf(db.BookerName[i],  mx_name,"User%d", i+1);
        snprintf(db.MeetingRoom[i], mx_room,"A");
        snprintf(db.BookingDate[i], mx_date,"01/01/2099");
        snprintf(db.BookingTime[i], mx_time,"09:00-10:00");
    }

#ifdef _WIN32
    const char *csv = "tests\\tmp_full.csv";
    const char *in_path = "tests\\tmp_in_full.txt";
#else
    const char *csv = "tests/tmp_full.csv";
    const char *in_path = "tests/tmp_in_full.txt";
#endif

    /* กันไฟล์ค้างจากรอบก่อน */
    remove(csv);

    char d[mx_date]; date_plus_days(1, d);
    char script[256];
    snprintf(script, sizeof(script),
        "999999\n"
        "New Guy\n"
        "A\n"
        "%s\n"
        "10:00-11:00\n", d);
    write_text_file(in_path, script);

    redirect_stdin_from(in_path);
    SILENCE( add_user(&db, (char*)csv) );
    restore_stdin_console();

    int last = Row - 1;
    CHECK("no overwrite when full",
          strcmp(db.BookingID[last], "1000") == 0 &&
          strcmp(db.BookerName[last], "User1000") == 0);

    FILE *f = fopen(csv,"r");
    CHECK("no file written on full", f == NULL);
    if (f) fclose(f);
    return 0;
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
/* -------- id_num_ch -------- */
static int test_id_num_ch_valid(void){
    char id[mx_id] = "213";

    int return_val  = id_num_ch(id);

    CHECK("If ID is number",return_val == 1);
    return 0;
}
static int test_id_num_ch_invalid(void){
    char id[mx_id] = "asdaSD";

    int return_val  = id_num_ch(id);

    CHECK("If ID is not number",return_val == 0);
    return 0;
}
static int test_space_check_(void){
    
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
    char d[mx_date];date_plus_days(3, d);
    int rc = validate_booking_datetime(d, "09:00-10:00", err, sizeof(err));
    CHECK("boundary 3 days ahead should pass", rc==0);
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
/* -------- comma in field: name -------- */
static int test_add_user_rejects_comma_in_name(void){
    ensure_tests_dir();
#ifdef _WIN32
    const char *in  = "tests\\tmp_in_bad_name.txt";
    const char *csv = "tests\\tmp_bad_name.csv";
#else
    const char *in  = "tests/tmp_in_bad_name.txt";
    const char *csv = "tests/tmp_bad_name.csv";
#endif

    DB db = (DB){0};
    remove(csv);  /* กันไฟล์ค้าง */

    char d[mx_date]; date_plus_days(1, d);
    char script[256];
    snprintf(script, sizeof(script),
        "100\n"
        "Ali,ce\n"   /* error: มี comma */
        "A\n"
        "%s\n"
        "09:00-10:00\n", d);
    write_text_file(in, script);

    redirect_stdin_from(in);
    SILENCE( add_user(&db, (char*)csv) );
    restore_stdin_console();

    CHECK("rejects comma in name (no DB write)", db.BookingID[0][0]=='\0');
    FILE *f = fopen(csv,"r");
    CHECK("rejects comma -> no file created", f==NULL);
    if (f) fclose(f);
    return 0;
}



static int test_add_user_rejects_comma_in_id(void){
    ensure_tests_dir();
#ifdef _WIN32
    const char *in  = "tests\\tmp_in_bad_id.txt";
    const char *csv = "tests\\tmp_bad_id.csv";
#else
    const char *in  = "tests/tmp_in_bad_id.txt";
    const char *csv = "tests/tmp_bad_id.csv";
#endif

    DB db = (DB){0};
    remove(csv);  /* กันไฟล์ค้าง */

    char d[mx_date]; date_plus_days(1, d);
    char script[256];
    snprintf(script, sizeof(script),
        "1,00\n"     /* error: มี comma ใน ID */
        "Alice\n"
        "A\n"
        "%s\n"
        "09:00-10:00\n", d);
    write_text_file(in, script);

    redirect_stdin_from(in);
    SILENCE( add_user(&db, (char*)csv) );
    restore_stdin_console();

    CHECK("rejects comma in id (no DB write)", db.BookingID[0][0]=='\0');
    FILE *f = fopen(csv,"r");
    CHECK("rejects comma id -> no file created", f==NULL);
    if (f) fclose(f);
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
    CHECK("Loaded row 0 matches: ID=S1 and Name=Sam", strcmp(db2.BookingID[0],"S1")==0 && strcmp(db2.BookerName[0],"Sam")==0);
    CHECK("Loaded row 1 time is 10:00-11:00",        strcmp(db2.BookingTime[1],"10:00-11:00")==0);

    SILENCE( show_bookings_for(&db2,"01/01/2099","A") );
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
    CHECK("Searching by 'Ali' selects booking with ID ID1 (idx must be index 0)", idx==0);
    return 0;
}

static int test_add_user_ui_(void){
    ensure_tests_dir();

    DB db = (DB){0};

#ifdef _WIN32
    const char *in_path = "tests\\tmp_in_add.txt";
    const char *csv     = "tests\\tmp_add.csv";
#else
    const char *in_path = "tests/tmp_in_add.txt";
    const char *csv     = "tests/tmp_add.csv";
#endif

    remove(csv);  /* กันไฟล์ค้าง */

    char d[mx_date]; date_plus_days(1, d);  /* พรุ่งนี้: ผ่านกฎ 3 วัน */
    char script[256];
    snprintf(script,sizeof(script),
        "1001\n"
        "Alice\n"
        "A\n"
        "%s\n"
        "09:00-10:00\n", d);
    write_text_file(in_path, script);

    redirect_stdin_from(in_path);
    SILENCE( add_user(&db,(char*)csv) );
    restore_stdin_console();

    CHECK("Added booking successfully: ID=1001, Name=Alice",
          strcmp(db.BookingID[0],"1001")==0 && strcmp(db.BookerName[0],"Alice")==0);
    return 0;
}

static int test_update_user_ui_(void){
    ensure_tests_dir();

    // Arrange
    DB db = (DB){0};
    char d[mx_date]; date_plus_days(1, d);
    seed_booking(&db,0,"1001","Alice","A",d,"09:00-10:00");
    const char *in_path = "tests/tmp_in_edit.txt";
    char script[512];

    snprintf(script,sizeof(script),
        "Alice\n"  
              
        "1001\n"         
        "Alice\n"        
        "A\n"            
        "%s\n"           
        "10:00-11:00\n", 
        d);
    write_text_file(in_path, script);

#ifdef _WIN32
    const char *csv = "tests\\tmp_edit.csv";
#else
    const char *csv = "tests/tmp_edit.csv";
#endif

    // Act
    redirect_stdin_from(in_path);
    SILENCE( update_user(&db,(char*)csv) );
    restore_stdin_console();

    // Assert
    CHECK("Update booking time updated to 10:00-11:00",strcmp(db.BookingTime[0],"10:00-11:00")==0);
    return 0;
}

static int test_delete_user_ui_(void){
    ensure_tests_dir();

    // Arrange
    DB db = (DB){0};
    seed_booking(&db,0,"D001","Dave","A","01/01/2099","09:00-10:00");
    const char *in_path = "tests/tmp_in_delete.txt";
    write_text_file(in_path, "Dave\ny\n");

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

    CHECK("Deleted booking removed all fields in row 0",
        db.BookingID[0][0]=='\0' && db.BookerName[0][0]=='\0' &&
        db.MeetingRoom[0][0]=='\0' && db.BookingTime[0][0]=='\0');
    return 0;
}


int run_unit_tests(void){
    printf(BOLD CYAN "Running unit tests...\n" RESET);

    const TestCase cases[] = {
        {"add_user when storage full", test_add_user_storage_full_},
        {"to_lower_str",               test_to_lower_str_},
        {"trim",                       test_trim_},
        {"id_num_ch(valid case)",      test_id_num_ch_valid},
        {"id_num_ch(invalid case)",    test_id_num_ch_invalid},
        {"space_check",                test_space_check_},
        {"ch_time (valid/invalid)",    test_ch_time_valid_},
        {"ch_time (invalid cases)",    test_ch_time_invalid_},
        {"parse_date_ddmmyyyy",        test_parse_date_},
        {"validate_booking_datetime",  test_validate_booking_datetime_},
        {"overlaps",                   test_overlaps_},
        {"comma in name",              test_add_user_rejects_comma_in_name},
        {"comma in id",                test_add_user_rejects_comma_in_id},

        {"save/load/show",             test_save_load_show_},
        {"search_user",                test_search_user_ui_},
        {"add_user",                   test_add_user_ui_},
        {"update_user",                  test_update_user_ui_},
        {"delete_user",                test_delete_user_ui_},
    };

    int suite_failed = 0;

    for (size_t i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        const TestCase *tc = &cases[i];

        printf(BOLD WHITE "[%zu/%zu] %s\n" RESET,
            i+1, sizeof(cases)/sizeof(cases[0]), tc->name);

        int this_failed = run_test_case(tc);  /* ← ใช้ฟังก์ชันที่ตัดสินจาก CHECK counters */

        if (this_failed) {
            ++suite_failed;
            printf(RED   "  -> TEST FAILED\n" RESET);
        } else {
            printf(GREEN "  -> TEST PASSED\n" RESET);
        }
        printf("\n");
    }

    if (suite_failed == 0) {
        printf(BOLD GREEN "All unit tests passed! " RESET
               "(%d checks passed, %d failed)\n", g_passed, g_failed);
    } else {
        printf(BOLD RED "%d unit test(s) failed. " RESET
               "(%d checks passed, %d failed)\n", suite_failed, g_passed, g_failed);
    }

    g_passed = 0;
    g_failed = 0;

    return suite_failed;
}

