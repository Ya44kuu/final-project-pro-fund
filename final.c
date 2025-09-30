#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>
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
#define YELLOW "\033[38;5;228m"
#define GREEN "\033[32m"
#define WHITE "\033[37m"


typedef struct {
    char BookingID[Row][mx_id];
    char BookerName[Row][mx_name];
    char MeetingRoom[Row][mx_room];
    char BookingDate[Row][mx_date];
    char BookingTime[Row][mx_time];
} DB;
void save_all(DB *db,char *file_name){
    FILE *f = fopen(file_name,"w");
    if (!f) { perror("open write"); return; }
    fprintf(f,"BookingID,BookerName,MeetingRoom,BookingDate,BookingTime\n");
    for(int i = 0;i<Row;i++){
        if(db->BookingID[i][0] == '\0') continue;
        fprintf(f,"%s,%s,%s,%s,%s\n",db->BookingID[i],db->BookerName[i],db->MeetingRoom[i],db->BookingDate[i],db->BookingTime[i]);
    }
    printf(BOLD GREEN"Save Complete!!\n"RESET);
    fclose(f);
}
void cmp_time(DB *db){
    int i=0;
    char *day[Row]; 
    char *month[Row];
    char *year[Row]; 

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
int search_user(DB *db){
    char search_name[mx_name]; 
    int found_idx[Row];
    found_idx[0] = -1;
    int idx = 0;
    printf(YELLOW"Search By Name: "RESET);
    fgets(search_name,sizeof(search_name),stdin);
    search_name[strcspn(search_name,"\n")] = '\0';
    trim(search_name);
    char *lower_search = to_lower_str(search_name);
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
    char id_pick[20];
    printf(BOLD GREEN"here is your result\n"RESET);
    for(int i = 0;i < idx;i++){
        int k = found_idx[i];
        printf("(%d)%s,%s,%s,%s,%s (found at row %d)\n",i+1,db->BookingID[k],db->BookerName[k],db->MeetingRoom[k],db->BookingDate[k],db->BookingTime[k],k);      
    }
    printf("---------------------------------------------------------------------\n");
    printf("Press Enter (or 'q') to cancel.\n");
    printf(YELLOW"type ID of data you want to use "RESET);
    fgets(id_pick,sizeof(id_pick),stdin);
    id_pick[strcspn(id_pick,"\n")] = '\0';
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
    char tmp[1024];
    printf("=================================================================\n"RESET);
    printf(" **NOTE Spaces allowed only in Name And No Comma In Field**\n"RESET);
    printf( " **IMPORTANT Use Comma(,) To Cut Collumns**\n"RESET);
    printf("-----------------------------------------------------------------\n"RESET);
    printf("Example Format\n");
    printf(CYAN" BookingID,BookerName,MeetingRoom,BookingDate,BookingTime\n");
    printf(GREEN" [✓] 123,Cristianno Ronaldo,B,11/11/2025,12:00\n"RESET);
    printf(RED " [✗] 123,Leo,nel,Jessi,B,11,/11/2025,12,00\n"RESET);
    printf(RED " [✗] 12 3, Lamean Jaamal, B,11 /11/ 2025,12: 00\n"RESET);
    printf("=================================================================\n"RESET);
    printf(YELLOW"Type Here : "RESET);

    fgets(tmp,sizeof(tmp),stdin);
    tmp[strcspn(tmp,"\n")] = '\0';
    int comma_count = 0;
    for(int i = 0;tmp[i] != '\0';i++){
        if(tmp[i] == ','){
            comma_count += 1;
        }
    }
    if(comma_count != 4){
        printf(BOLD RED"Error: Need exactly 5 fields (4 commas)\n"RESET);
        return;
    }
    char *id = strtok(tmp,",");
    char *name = strtok(NULL,",");
    char *room = strtok(NULL,",");
    char *date = strtok(NULL,",");
    char *time = strtok(NULL,",");
    trim(id); trim(name); trim(room); trim(date); trim(time);
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
    if (space_check(id) || space_check(room) || space_check(date) || space_check(time)) {
        printf(BOLD RED"Error: ID/Room/Date/Time must not contain spaces\n"RESET);
        return;
    }
    //เช็คซ้ำ

    
    char *lower_room_stdin = to_lower_str(room);
    for(int i=0;i<Row;i++){
        if(db->BookingID[i][0]=='\0') continue;
        char *lower_room = to_lower_str(db->MeetingRoom[i]);

        if(strcmp(db->BookingID[i], id)==0){ printf(RED BOLD"Error: Duplicate ID\n" RESET); return; }

        if(strcmp(lower_room,lower_room_stdin)==0 &&
            strcmp(db->BookingDate[i], date)==0 &&
            strcmp(db->BookingTime[i], time)==0){
            printf(RED BOLD"Error: Time Slot Already Taken\n" RESET);
            free(lower_room);
            free(lower_room_stdin);
            return;
        }
        
        if (strcmp(db->BookerName[i],  name) == 0 &&
            strcmp(db->BookingDate[i], date) == 0 &&
            strcmp(db->BookingTime[i], time) == 0) {
            printf(RED BOLD"Error: You Can't Reserve More Than One Room In Same Time\n" RESET);
            free(lower_room_stdin);
            free(lower_room);
            return;   
        }
        free(lower_room);
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
        free(lower_room_stdin);
        return;
    }
    snprintf(db->BookingID[idx],20,"%s",id);
    snprintf(db->BookerName[idx],20,"%s",name);
    snprintf(db->MeetingRoom[idx],20,"%s",room);
    snprintf(db->BookingDate[idx],20,"%s",date);
    snprintf(db->BookingTime[idx],20,"%s",time);
    free(lower_room_stdin);
    save_all(db,file_name);
}
void edit_user(DB *db,char *file_name){
    printf(CYAN"Welcome To Edit User\n");
    int found_idx = search_user(db);
    if (found_idx < 0) return;
    char tmp[1024];
    printf("=================================================================\n"RESET);
    printf(" **NOTE Spaces allowed only in Name And No Comma In Field**\n"RESET);
    printf( " **IMPORTANT Use Comma(,) To Cut Collumns**\n"RESET);
    printf("-----------------------------------------------------------------\n"RESET);
    printf( "Example Format\n");
    printf(CYAN" BookingID,BookerName,MeetingRoom,BookingDate,BookingTime\n");
    printf(GREEN" [✓] 123,Cristianno Ronaldo,B,11/11/2025,12:00\n"RESET);
    printf(RED " [✗] 123,Leo,nel,Jessi,B,11,/11/2025,12,00\n"RESET);
    printf(RED " [✗] 12 3, Lamean Jaamal, B,11 /11/ 2025,12: 00\n"RESET);
    printf("=================================================================\n"RESET);
    printf(YELLOW"Type Here : "RESET);

    fgets(tmp,sizeof(tmp),stdin);
    tmp[strcspn(tmp,"\n")] = '\0';
    int comma_count = 0;
    for(int i = 0;tmp[i] != '\0';i++){
        if(tmp[i] == ','){
            comma_count += 1;
        }
    }
    if(comma_count != 4){
        printf(BOLD RED"Error: Need exactly 5 fields (4 commas)\n"RESET);
        return;
    }
    char *id = strtok(tmp,",");
    char *name = strtok(NULL,",");
    char *room = strtok(NULL,",");
    char *date = strtok(NULL,",");
    char *time = strtok(NULL,",");
    trim(id); trim(name); trim(room); trim(date); trim(time);
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
    if (space_check(id) || space_check(room) || space_check(date) || space_check(time)) {
        printf(BOLD RED"Error: ID/Room/Date/Time must not contain spaces\n"RESET);
        return;
    }
    //เช็คซ้ำ
    char *lower_room_stdin = to_lower_str(room);
    for(int i=0;i<Row;i++){
        if(db->BookingID[i][0]=='\0') continue;
        if(i == found_idx) continue;
        char *lower_room = to_lower_str(db->MeetingRoom[i]);

        if(strcmp(db->BookingID[i], id)==0){ printf(RED BOLD"Error: Duplicate ID\n" RESET); return; }
        if(strcmp(lower_room,lower_room_stdin) == 0 &&
            strcmp(db->BookingDate[i], date)==0 &&
            strcmp(db->BookingTime[i], time)==0){
            printf(RED BOLD"Error: Time Slot Already Taken\n" RESET);
            free(lower_room);
            free(lower_room_stdin);
            return;
        }
        
        if (strcmp(db->BookerName[i],  name) == 0 &&
            strcmp(db->BookingDate[i], date) == 0 &&
            strcmp(db->BookingTime[i], time) == 0) {
            printf(RED BOLD"Error: You Can't Reserve More Than One Room In Same Time\n" RESET);
            free(lower_room_stdin);
            free(lower_room);
            return;   
        }
        free(lower_room);
    }
    
    
    snprintf(db->BookingID[found_idx],mx_id,"%s",id);
    snprintf(db->BookerName[found_idx],mx_name,"%s",name);
    snprintf(db->MeetingRoom[found_idx],mx_room,"%s",room);
    snprintf(db->BookingDate[found_idx],mx_date,"%s",date);
    snprintf(db->BookingTime[found_idx],mx_time,"%s",time);
    free(lower_room_stdin);

    save_all(db,file_name);
}
void delete_user(DB *db,char *file_name){
    int found_idx = search_user(db);
    if(found_idx == -1) return;
    char confirm;
    printf("Are You Sure? You Want To Delete This Data(y/n)");
    scanf("%c",&confirm);
    getchar();
    if( tolower(confirm) == 'y'){
        for(int i =0 ;i < 20;i++){
        db->BookingID[found_idx][i] = '\0';db->BookerName[found_idx][i] = '\0';db->MeetingRoom[found_idx][i] = '\0';db->BookingDate[found_idx][i] = '\0';db->BookingTime[found_idx][i] = '\0';
        }
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
        if(count_row < Row){
            snprintf(db->BookingID[count_row],mx_id,"%s",id);
            snprintf(db->BookerName[count_row],mx_name,"%s",name);
            snprintf(db->MeetingRoom[count_row],mx_room,"%s",room);
            snprintf(db->BookingDate[count_row],mx_date,"%s",date);
            snprintf(db->BookingTime[count_row],mx_time,"%s",time);
            count_row++;
        }
    }
    cmp_time(db);
    fclose(f);
}
static void clear_screen() {
    printf("\033[2J\033[H");
    fflush(stdout);
}

static int menu_once(void) {
    printf(BOLD CYAN"Wellcome To csv Manger\n"RESET);
    printf("---------------------------------------------\n");
    printf("(1) Add user\n");
    printf("(2) Edit user\n");
    printf("(3) Delete user\n");
    printf("(4) Search user\n");
    printf("(0) Quit\n");
    printf("---------------------------------------------\n");
    int choice;
    printf(YELLOW"type number(0-4) here: "RESET);
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
            case 0:running = 0; break;
            default:
            printf("Error: Not macth number\n");

        }
        
    }
}
int main(){
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