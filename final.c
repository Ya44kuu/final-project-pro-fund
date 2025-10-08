#include "booking.h"

#ifdef _WIN32
#include <windows.h>
void enable_vt_mode(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);
}
#endif

int ensure_field_fits(const char *value, size_t cap, const char *label, const char *context) {
    if (!value) {
        printf(BOLD RED "Error(%s): %s is missing\n" RESET, context, label);
        return 0;
    }
    size_t len = strlen(value);
    if (len >= cap) {
        printf(BOLD RED "Error(%s): %s must be at most %zu characters\n" RESET, context, label, cap - 1);
        return 0;
    }
    return 1;
}

void store_field(char *dest, size_t cap, const char *value) {
    if (!dest || !value) return;
    snprintf(dest, cap, "%.*s", (int)(cap - 1), value);
}



void save_all(DB *db,char *file_name){
    FILE *f = fopen(file_name,"w");
    if (!f) { perror("open write"); return; }
    fprintf(f,"BookingID,BookerName,MeetingRoom,BookingDate,BookingTime\n");
    for(int i = 0;i<Row;i++){
        if(db->BookingID[i][0] == '\0') continue;
        fprintf(f,"%s,%s,%s,%s,%s\n",
                db->BookingID[i],
                db->BookerName[i],
                db->MeetingRoom[i],
                db->BookingDate[i],
                db->BookingTime[i]);
    }
    printf(BOLD GREEN"Save Complete!!\n"RESET);
    fclose(f);
}

void show_log(const DB *db){ int W_ID=mx_id, W_NAME=mx_name, W_ROOM=mx_room, W_DATE=mx_date, W_TIME=mx_time; 
    printf("\n-----------------------------------------------------------------------------------------------------------------------------------\n");
     printf("%-*s | %-*s | %-*s | %-*s | %-*s\n", W_ID, "BookingID", W_NAME,"BookerName", W_ROOM,"MeetingRoom", W_DATE,"BookingDate", W_TIME,"BookingTime");
      printf("------------------------------------------------------------------------------------------------------------------------------------\n");
       for (int i=0;i<Row;i++){ 
            if (db->BookingID[i][0]=='\0') continue; 
                printf("%-*.*s | %-*.*s | %-*.*s | %-*.*s | %-*.*s\n", W_ID, W_ID, db->BookingID[i], W_NAME,W_NAME,db->BookerName[i],
                W_ROOM,W_ROOM,db->MeetingRoom[i], W_DATE,W_DATE,db->BookingDate[i], W_TIME,W_TIME,db->BookingTime[i]);
            } 
            printf("------------------------------------------------------------------------------------------------------------------------------------\n"); }
char* to_lower_str(const char *s){
    if(!s) return NULL;
    size_t n = strlen(s);
    char *lower = (char*)malloc(n + 1);
    if (!lower) return NULL; 
    for(size_t i = 0; i < n;i++){
        lower[i] = tolower((unsigned char)s[i]);
    }
    lower[n] = '\0';
    return lower;

}
typedef struct { int idx, s, e; } Slot;

int cmp_slot(const void *a, const void *b){
    const Slot *A = (const Slot*)a, *B = (const Slot*)b;
    return (A->s - B->s);
}


//เช็คว่า id เป็นเลขไหม
int id_num_ch(char* id){
    if (!*id) return 0;     
    if(!(strspn(id, "0123456789") == strlen(id))) return 0; 
    return 1;  
}
int ch_time(const char *time ,int *start_m,int *end_m){
    int s_hr,s_min,ed_hr,ed_min;
    if(sscanf(time," %d : %d - %d : %d ",&s_hr,&s_min,&ed_hr,&ed_min) != 4) return 0;
    
    if(s_hr < 0 || s_hr > 23 || s_min < 0 || s_min > 59 || ed_hr < 0 || ed_hr > 23 || ed_min < 0 || ed_min > 59) return 0;
    *start_m = s_hr*60 + s_min;
    *end_m = ed_hr*60 + ed_min;
    if(*start_m >=  *end_m) return 0;
    return 1;
}
int overlaps(DB *db,const char *name,const char *room,const char *time,const char *date,int exclude_idx){
    int start1,end1;
    if (!ch_time(time,&start1,&end1)) return -1;
    char *lower_search = to_lower_str(name);
    char *lower_search_room = to_lower_str(room);

    if (!lower_search || !lower_search_room){
        free(lower_search); free(lower_search_room);
        return -1;
    }

    for(int i = 0; i< Row;i++){
        if (i == exclude_idx) continue;
        if(db->BookingID[i][0] == '\0') continue;
        if(strcmp(db->BookingDate[i],date) != 0) continue;

        char *lower_name = to_lower_str(db->BookerName[i]);
        char *lower_room = to_lower_str(db->MeetingRoom[i]);

        if (!lower_name || !lower_room) {
            free(lower_name);
            free(lower_room);
            continue; 
        }
        if(!(strcmp(lower_room,lower_search_room) == 0 || strcmp(lower_name, lower_search) == 0)) {
            free(lower_name);
            free(lower_room);
            continue;
        }
        int start2,end2;
        if (!ch_time(db->BookingTime[i],&start2,&end2)) {
            free(lower_name); 
            free(lower_room);

            continue;
        }
        if((start2 < end1) && (start1 < end2)) {
            free(lower_name);
            free(lower_room);
            free(lower_search_room);
            free(lower_search);
            return 1;
        }
        free(lower_name);
        free(lower_room);
    }
    free(lower_search_room);
    free(lower_search);
    return 0;

}

int parse_date_ddmmyyyy(const char *s, struct tm *out_tm, time_t *out_midnight) {
    int d, m, y;
    if (!s || sscanf(s, " %d / %d / %d ", &d, &m, &y) != 3) return 0;
    if (y < 1900 || m < 1 || m > 12 || d < 1) return 0;

    struct tm t = {0};
    t.tm_year = y - 1900;
    t.tm_mon  = m - 1;
    t.tm_mday = d;
    t.tm_hour = 0; t.tm_min = 0; t.tm_sec = 0;
    t.tm_isdst = -1;

    time_t tt = mktime(&t);
    if (tt == (time_t)-1) return 0;

    
    struct tm *chk = localtime(&tt);
    if (!chk) return 0;
    if (chk->tm_year != t.tm_year || chk->tm_mon != t.tm_mon || chk->tm_mday != t.tm_mday) return 0;

    if (out_tm) *out_tm = *chk;
    if (out_midnight) *out_midnight = tt;
    return 1;
}

int validate_booking_datetime(const char *date_str, const char *time_str, char *err, size_t errsz) {

    int start_m, end_m;
    if (!ch_time(time_str, &start_m, &end_m)) {
        snprintf(err, errsz, "Time format must be HH:MM-HH:MM and Time start < Time end");
        return 1;
    }

    time_t now = time(NULL);
    if (now == (time_t)-1) {
        snprintf(err, errsz, "Unable to read current time");
        return 2;
    }
    struct tm now_tm;
    #ifdef _WIN32
        localtime_s(&now_tm, &now);
    #else
        localtime_r(&now, &now_tm);
    #endif

   
    struct tm today0 = now_tm;
    today0.tm_hour = today0.tm_min = today0.tm_sec = 0;
    today0.tm_isdst = -1;
    time_t today_midnight = mktime(&today0);

    
    struct tm book_tm;
    time_t book_midnight;
    if (!parse_date_ddmmyyyy(date_str, &book_tm, &book_midnight)) {
        snprintf(err, errsz, "The date format must be dd/mm/yyyy and must be an actual day.");
        return 3;
    }

    //ห้ามย้อนหลัง (ก่อนเที่ยงคืนวันนี้)
    if (book_midnight < today_midnight) {
        snprintf(err, errsz, "Cannot book in the past");
        return 4;
    }

    
    const time_t three_days = 3 * 24 * 60 * 60; 
    if (book_midnight - today_midnight > three_days) {
        snprintf(err, errsz, "You Can Reserve A Meeting Room No More Than 3 Days In Advance");
        return 5;
    }

    
    int today_ymd = (now_tm.tm_year==book_tm.tm_year) && (now_tm.tm_yday==book_tm.tm_yday);
    if (today_ymd) {
        int now_min = now_tm.tm_hour * 60 + now_tm.tm_min;
        if (start_m < now_min) {
            snprintf(err, errsz, "If booking today, the start time must be >= current time");
            return 6;
        }
    }

    
    if (errsz) err[0] = '\0';
    return 0;
}
int has_forbidden_chars(const char *s) {
    if (!s) return 0;
    for (const char *p = s; *p; ++p) {
        if (*p == ',' || *p == '\n') return 1;
    }
    return 0;
}
void show_bookings_for(const DB *db, const char *date, const char *room){
    Slot slots[Row]; int n = 0;
    char *room_l = to_lower_str(room);
    if (!room_l) return;

    for (int i = 0; i < Row; i++){
        if (db->BookingID[i][0] == '\0') continue;
        if (strcmp(db->BookingDate[i], date) != 0) continue;

        char *r = to_lower_str(db->MeetingRoom[i]);
        if (!r) continue;

        int s, e;
        if (strcmp(r, room_l) == 0 && ch_time(db->BookingTime[i], &s, &e)){
            slots[n++] = (Slot){ i, s, e };
        }
        free(r);
    }
    free(room_l);

    printf(CYAN "---- Booked for room %s on %s ----\n" RESET, room, date);
    if (n == 0){
        printf(GREEN "(no bookings yet)\n" RESET);
        return;
    }
    qsort(slots, n, sizeof(Slot), cmp_slot);
    for (int k = 0; k < n; k++){
        int i = slots[k].idx;
        printf("%-13s | %-20s | ID:%s\n",
               db->BookingTime[i],
               db->BookerName[i],
               db->BookingID[i]);
    }
    printf("-----------------------------------\n");
}
void trim(char *s){
    if (!s) return;
    size_t n = strlen(s);
    while (n && isspace((unsigned char)s[n-1])) s[--n] = '\0'; 
    size_t i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;          
    if (i) memmove(s, s+i, n - i + 1);
}
int space_check(char *s){
    if(!s) return 0;
    return strchr(s, ' ') != NULL;

}
int stdin_edit_add(DB *db){
    char id[mx_id] ,name[mx_name], room[mx_room],date[mx_date],time[mx_time];
    printf("------------------------------------------\n");
    printf(YELLOW"Enter your ID: "RESET);
    scanf("%19s",id);
    getchar();

    printf(YELLOW"Enter your Name: "RESET);
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    printf("-There Is 4 Room You Can Reserve \"A B C D\" \n");
    printf("-Inputs Can be Upper And Lower letter\n");
    printf("Ex. A or a\n"RESET);
    printf(YELLOW"Enter your Room: "RESET);
    scanf("%9s",room);
    
    printf("-You Can Reserve A Meeting Room No More Than 3 Days In Advance\n");
    printf("Ex."RESET);
    printf("3/12/2025\n");
    printf(YELLOW"Enter your Date: "RESET);
    scanf("%11s",date);

    trim(room); trim(date);
    show_bookings_for(db, date, room);

    printf("-Meeting Room Open 24 Hour\n");
    printf("Ex."RESET);
    printf("13:00-14:00\n");
    printf(YELLOW"Enter your Period: "RESET);
    scanf("%19s",time);

    trim(id); trim(name); trim(time);   

    snprintf(db->tmpo[0],mx_id,"%s",id);           
    snprintf(db->tmpo[1],mx_name,"%s",name);           
    snprintf(db->tmpo[2],mx_room,"%s",room);    
    snprintf(db->tmpo[3],mx_date,"%s",date);           
    snprintf(db->tmpo[4],mx_time,"%s",time);

    for(int i = 0 ;i<5;i++){
        if(db->tmpo[i][0] == '\0'){
            return 0;
        }
    }

    return 1;
}
int stdin_sch(DB *db){
    char name[mx_name]; 
    printf(YELLOW"Search by name:"RESET);
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';
    trim(name);
    snprintf(db->tmpo[0],mx_name,"%s",name);

    if(db->tmpo[0][0] == '\0'){
        return 0;
    }
    return 1;
    
}
int stdin_id_match(DB *db){
    char buf[mx_id];
    printf("---------------------------------------------------------------------\n");
    printf("Press Enter (or 'q') to cancel.\n");
    printf(YELLOW "type ID of data you want to use: " RESET);
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = '\0';
    trim(buf);
    if (buf[0] == '\0' || buf[0]=='q' || buf[0]=='Q') return 0;
    snprintf(db->tmpo[0], mx_id, "%s", buf);
    return 1;
}
char comfirm_pms(){
    char c;
    printf("Are You Sure? You Want To Delete This Data(y/n) ");
    if (scanf(" %c", &c) != 1) return 'n';
    return c;

}
int search_user(DB *db){
    int found_idx[Row];
    found_idx[0] = -1;
    int idx = 0;
    int name_sch = stdin_sch(db);
    if(name_sch == 0) return -1;
    char *lower_search = to_lower_str(db->tmpo[0]);
    for(int i = 0; i < Row;i++){
        if(db->BookerName[i][0] == '\0') continue;
        char *lower_name = to_lower_str(db->BookerName[i]);
        if(strcmp(lower_name,lower_search) == 0 || strstr(lower_name,lower_search) != NULL){
            found_idx[idx++] = i;
            
        }
        free(lower_name);
    }
    free(lower_search);
    if(idx == 0){
        printf(BOLD RED"Error: Name Not Found Pls Check Again\n"RESET);
        return -1;
    }else if(idx == 1){
        int k = found_idx[0];
        printf(BOLD GREEN"found one result\n"RESET);
        printf("(%d)%s,%s,%s,%s,%s (found at row %d)\n",1,db->BookingID[k],db->BookerName[k],db->MeetingRoom[k],db->BookingDate[k],db->BookingTime[k],k);      
        return k;

    }
    printf(BOLD GREEN"here is your result\n"RESET);
    for(int i = 0;i < idx;i++){
        int k = found_idx[i];
        printf("(%d)%s,%s,%s,%s,%s (found at row %d)\n",i+1,db->BookingID[k],db->BookerName[k],db->MeetingRoom[k],db->BookingDate[k],db->BookingTime[k],k);      
    }
    
    int sch_id = stdin_id_match(db);
    if(sch_id == 0) return -1;
    char *id_pick = db->tmpo[0];
    if (id_pick[0] == '\0' || id_pick[0] == 'q' || id_pick[0] == 'Q') {
        printf(BOLD RED"Canceled.\n"RESET);
        return -1;
    }
    for(int i = 0;i < idx;i++){
        if(strcmp(db->BookingID[found_idx[i]],id_pick) == 0){
            return found_idx[i];
        }
    }
    printf(BOLD RED"Error: Selection not matched\n"RESET);
    return -1;
        
    
}
void add_user(DB *db,char *file_name){
    int inputs = stdin_edit_add(db);
    if(inputs == 0) return;

    char *id = db->tmpo[0];
    char *name = db->tmpo[1];
    char *room = db->tmpo[2];
    char *date = db->tmpo[3];
    char *time = db->tmpo[4];
    //เช็ค id เป็นเลขไหม
    if(!id_num_ch(id)){
        printf(BOLD RED"Error: ID must be number\n");
        return;
    }

    //เช็คความถูกต้องของห้อง
    int valid_room = 0;
    char *input= to_lower_str(room);
    if (input) {
    if (strcmp(input, "a") == 0 || strcmp(input, "b") == 0 ||
        strcmp(input, "c") == 0 || strcmp(input, "d") == 0) {
        valid_room = 1;
    }
    free(input);
    }
    if (!valid_room) {
    printf(BOLD RED "Error: Not Match Room\n" RESET);
    return;
    }
    //เช็คช่องว่าง
    char *nullcheck[] = {id,name,room,date,time};
    for(int i = 0;i < 5;i++){
        if((nullcheck[i] == NULL)){
            printf(RED BOLD"Error: Invalid Pattern Try Pls Again\n" RESET);
            return;
        }
        if(nullcheck[i][0] == '\0'){
            printf(RED BOLD"Error: Invalid Pattern Try Pls Again\n" RESET);
            return;
        }
    }
    if (has_forbidden_chars(id) || has_forbidden_chars(name) ||
    has_forbidden_chars(room) || has_forbidden_chars(date) ||
    has_forbidden_chars(time)) {
    printf(RED BOLD "Error: Fields must not contain ',' or newline\n" RESET);
    return;
} 
    if (space_check(id) || space_check(room) || space_check(date) || space_check(time)) {
        printf(BOLD RED"Error: ID/Room/Date/Time must not contain spaces\n"RESET);
        return;
    }
    //เช็คซ้ำ

    
    for(int i=0;i<Row;i++){
        if(db->BookingID[i][0]=='\0') continue;

        if(strcmp(db->BookingID[i], id)==0){ 
            printf(RED BOLD"Error: Duplicate ID\n" RESET); 
            return; 
        }
        
        
    }
    if (!ensure_field_fits(id, mx_id, "ID", "add_user") ||
        !ensure_field_fits(name, mx_name, "Name", "add_user") ||
        !ensure_field_fits(room, mx_room, "Room", "add_user") ||
        !ensure_field_fits(date, mx_date, "Date", "add_user") ||
        !ensure_field_fits(time, mx_time, "Time", "add_user")) {
        return;
    }
    char err[128];
    int v = validate_booking_datetime(date, time, err, sizeof(err) );
    if (v != 0) {
        printf(BOLD RED "Error: %s\n" RESET, err);
        return;
    }
    int ovl = overlaps(db,name,room,time,date,-1);
    if(ovl < 0){
        printf(BOLD RED"Error: Time format must be HH:MM-HH:MM\n");
        return;
    }
    if (ovl == 1) {
    printf(RED BOLD"Error: Time overlaps an existing booking in the same room and date\n"RESET);
    return;
    }
    int idx = -1;
    for(int i = 0;i<Row;i++){
        if(db->BookingID[i][0] == '\0'){
            idx  = i;
            break;
        }
    }
    if (idx == -1){
        printf(BOLD RED"Error: Storage Full\n"RESET);
        return;
    }
    
    store_field(db->BookingID[idx],   mx_id,  id);
    store_field(db->BookerName[idx],  mx_name, name);
    store_field(db->MeetingRoom[idx], mx_room, room);
    store_field(db->BookingDate[idx], mx_date, date);
    store_field(db->BookingTime[idx], mx_time, time);
    save_all(db,file_name);
}
void edit_user(DB *db,char *file_name){
    printf(CYAN"Welcome To Edit User\n");
    int found_idx = search_user(db);
    if (found_idx < 0) return;

   
    int inputs = stdin_edit_add(db);
    if(inputs == 0) return;
    char *id = db->tmpo[0];
    char *name = db->tmpo[1];
    char *room = db->tmpo[2];
    char *date = db->tmpo[3];
    char *time = db->tmpo[4];

    if(!id_num_ch(id)){
        printf(BOLD RED"Error: ID must be number\n");
        return;
    } 
    //เช็คความถูกต้องของห้อง
    int valid_room = 0;
    char *input= to_lower_str(room);
    if (input) {
    if (strcmp(input, "a") == 0 || strcmp(input, "b") == 0 ||
        strcmp(input, "c") == 0 || strcmp(input, "d") == 0) {
        valid_room = 1;
    }
    free(input);
    }
    if (!valid_room) {
    printf(BOLD RED "Error: Not Match Room\n" RESET);
    return;
    }
     //เช็คช่องว่าง
    char *nullcheck[] = {id,name,room,date,time};
    for(int i = 0;i < 5;i++){
        if((nullcheck[i] == NULL)){
            printf(RED BOLD"Error: Invalid Pattern Pls Try Again\n" RESET);
            return;
        }
        if(nullcheck[i][0] == '\0'){
            printf(RED BOLD"Error: Invalid Pattern Pls Try Again\n" RESET);
            return;
        }
    } 
    if (has_forbidden_chars(id) || has_forbidden_chars(name) ||
    has_forbidden_chars(room) || has_forbidden_chars(date) ||
    has_forbidden_chars(time)) {
    printf(RED BOLD "Error: Fields must not contain ',' or newline\n" RESET);
    return;  /* ฟังก์ชันนี้เป็น void ให้ return เปล่า ๆ */
}
    if (space_check(id) || space_check(room) || space_check(date) || space_check(time)) {
        printf(BOLD RED"Error: ID/Room/Date/Time must not contain spaces\n"RESET);
        return;
    }
    //เช็คซ้ำ
    for(int i=0;i<Row;i++){
        if(db->BookingID[i][0]=='\0') continue;
        if(i == found_idx) continue;

        if(strcmp(db->BookingID[i], id)==0){ 
            printf(RED BOLD"Error: Duplicate ID\n" RESET); 
            return; 
        }
    }
    if (!ensure_field_fits(id, mx_id, "ID", "add_user") ||
        !ensure_field_fits(name, mx_name, "Name", "add_user") ||
        !ensure_field_fits(room, mx_room, "Room", "add_user") ||
        !ensure_field_fits(date, mx_date, "Date", "add_user") ||
        !ensure_field_fits(time, mx_time, "Time", "add_user")) {
        return;
    }
    char err[128];
    int v = validate_booking_datetime(date, time, err, sizeof(err));
    if (v != 0) {
        printf(BOLD RED "Error: %s\n" RESET, err);
        return;
    }
    int ovl = overlaps(db,name,room,time,date,found_idx);
    if(ovl < 0){
        printf(BOLD RED"Error:Error: Time format must be HH:MM-HH:MM\n");
        return;
    }
    if (ovl == 1) {
    printf(RED BOLD"Error: Time overlaps an existing booking in the same room and date\n"RESET);
    return;
    }

    store_field(db->BookingID[found_idx], mx_id, id);
    store_field(db->BookerName[found_idx], mx_name, name);
    store_field(db->MeetingRoom[found_idx], mx_room, room);
    store_field(db->BookingDate[found_idx], mx_date, date);
    store_field(db->BookingTime[found_idx], mx_time, time);

    save_all(db,file_name);
}
void delete_user(DB *db,char *file_name){
    int found_idx = search_user(db);
    if(found_idx == -1) return;
    char confirm = comfirm_pms();
    if(isspace((unsigned char)confirm)) return;
    if(tolower(confirm) == 'y'){
        for (int k = 0; k < mx_id;k++) db->BookingID[found_idx][k]   = '\0';
        for (int k = 0; k < mx_name;k++) db->BookerName[found_idx][k]  = '\0';
        for (int k = 0; k < mx_room;k++) db->MeetingRoom[found_idx][k] = '\0';
        for (int k = 0; k < mx_date;k++) db->BookingDate[found_idx][k] = '\0';  
        for (int k = 0; k < mx_time;k++) db->BookingTime[found_idx][k] = '\0';
        save_all(db,file_name);
    }else if (tolower(confirm) == 'n'){
        return;
    }else{
        printf(BOLD RED"Error: Selection not matched\n"RESET);
        return;
    }

}

void load_file(DB *db,char *file_name) {

    FILE *f = fopen(file_name,"r");
    if(!f){
        save_all(db,file_name);
        f = fopen(file_name,"r");
    }
    char line[1024];
    int count_row = 0;
    if (!fgets(line, sizeof(line), f)) {   
    fclose(f);
    save_all(db,file_name);                        
    f = fopen(file_name,"r");
    if (!f) return ;
    fgets(line, sizeof(line), f);     
    }
    while(fgets(line,sizeof(line),f)){
        trim(line);
        if(line[0] == '\0') continue;
        char *id = strtok(line,",");
        char *name = strtok(NULL,",");
        char *room = strtok(NULL,",");
        char *date = strtok(NULL,",");
        char *time = strtok(NULL,",");

        if (!id || !name || !room || !date || !time) continue; 

        trim(id); trim(name); trim(room); trim(date); trim(time);

        if(count_row < Row){
            snprintf(db->BookingID[count_row],mx_id,"%s",id);
            snprintf(db->BookerName[count_row],mx_name,"%s",name);
            snprintf(db->MeetingRoom[count_row],mx_room,"%s",room);
            snprintf(db->BookingDate[count_row],mx_date,"%s",date);
            snprintf(db->BookingTime[count_row],mx_time,"%s",time);
            count_row++;
        }
    }
    fclose(f);
}
static void clear_screen() {
    printf("\033[2J\033[H");
    fflush(stdout);
}



static void run_tests_menu(void) {
    int running = 1;
    while (running) {
        clear_screen();
        printf(BOLD CYAN "Test Runner\n" RESET);
        printf("---------------------------------------------\n");
        printf("(1) Run unit tests\n");
        printf("(2) Run end-to-end tests\n");
        printf("(3) Run all tests\n");
        printf("(0) Back\n");
        printf("---------------------------------------------\n");
        printf(YELLOW "type number(0-3) here: " RESET);
        int choice;
        if (scanf("%d", &choice) != 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            printf(BOLD RED "Error: Invalid input. Please enter a number.\n" RESET);
            printf(YELLOW "Press Enter to continue..." RESET);
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue;
        }
        getchar();
        clear_screen();
        switch (choice) {
            case 1: {
                int failed = run_unit_tests();
                if (failed == 0) {
                    printf(BOLD GREEN "Unit tests completed successfully.\n" RESET);
                } else {
                    printf(BOLD RED "Unit tests reported %d failure(s).\n" RESET, failed);
                }
                break;
            }
            case 2: {
                int failed = run_e2e_tests();
                if (failed == 0) {
                    printf(BOLD GREEN "End-to-end tests completed successfully.\n" RESET);
                } else {
                    printf(BOLD RED "End-to-end tests reported %d failure(s).\n" RESET, failed);
                }
                break;
            }
            case 3: {
                int failed_unit = run_unit_tests();
                int failed_e2e = run_e2e_tests();
                if (failed_unit + failed_e2e == 0) {
                    printf(BOLD GREEN "All tests completed successfully.\n" RESET);
                } else {
                    printf(BOLD RED "See above for failing tests.\n" RESET);
                }
                break;
            }
            case 0:
                running = 0;
                continue;
            default:
                printf(BOLD RED "Error: Not match number\n" RESET);
                break;
        }
        if (!running) {
            break;
        }
        printf(YELLOW "Press Enter to continue..." RESET);
        while (1) {
            int c = getchar();
            if (c == '\n' || c == EOF) {
                break;
            }
        }
    }
    clear_screen();
}

static int menu_once(void) {
    printf(BOLD CYAN"Welcome To Meeting Room Booking Management\n"RESET);
    printf("---------------------------------------------\n");
    printf(YELLOW"Rule:\n"RESET);
    printf("1.You Can Reserve A Meeting Room No More Than 3 Days In Advance\n");
    printf("2.There Is 4 Room You Can Reserve \"A B C D\" \n");
    printf("3.Meeting Room Open 24 Hour\n");
    printf("4.You Cannot Book Two Rooms At The Same Time\n");

    printf("---------------------------------------------\n");
    printf("(1) Add user\n");
    printf("(2) Edit user\n");
    printf("(3) Delete user\n");
    printf("(4) Search user\n");
    printf("(5) Show Data\n");
    printf("(6) Run Tests\n");
    printf("(0) Quit\n");
    printf("---------------------------------------------\n");
    int choice;
    printf(YELLOW"type number(0-6) here: "RESET);
    scanf("%d",&choice);
    getchar();
    return choice;
}
void display(DB *db,char *csv){
    int running = 1;
    while(running){
        int choice = menu_once();
        clear_screen();
        switch (choice){
            case 1:add_user(db,csv);  break;
            case 2:edit_user(db,csv);  break;
            case 3:delete_user(db,csv); break;
            case 4:search_user(db); break;
            case 5:show_log(db); break;
            case 6:run_tests_menu(); break;
            case 0:running = 0; break;
            default:
            printf(BOLD RED"Error: Not match number\n"RESET);

        }
        
    }
}
int main(){
#ifdef _WIN32
    enable_vt_mode();
#endif
    DB db = {0};
    char CSV[100];
    printf(CYAN"Name of your csv file\n");
    printf(YELLOW"type here: "RESET);
    fgets(CSV,sizeof(CSV),stdin);
    CSV[strcspn(CSV,"\n")] = '\0';
    load_file(&db,CSV);
    display(&db,CSV);
    clear_screen();
    printf(GREEN "Bye..."RESET);
    return 0;
}