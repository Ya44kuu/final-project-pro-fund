#include<stdio.h>
#include<ctype.h>
#include<string.h>

#define BOLD "\033[1m"
#define CYAN "\033[36m"
#define RESET "\033[0m"
#define MAGENTA "\033[35m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define Row 1000

char BookingID[Row][20];
char BookerName[Row][20];
char MeetingRoom[Row][20];
char BookingDate[Row][20];
char BookingTime[Row][20];

void save_all(){
    FILE *f = fopen("Book1.csv","w");
    if (!f) { perror("open write"); return; }
    fprintf(f,"BookingID,BookerName,MeetingRoom,BookingDate,BookingTime\n");
    for(int i = 0;i<Row;i++){
        if(BookingID[i][0] == '\0') continue;
        fprintf(f,"%s,%s,%s,%s,%s\n",BookingID[i],BookerName[i],MeetingRoom[i],BookingDate[i],BookingTime[i]);
    }
    printf(BOLD GREEN"Save Complete!!\n"RESET);
    fclose(f);
}
void trim(char *s){
    if(!s) return;
    int n = strlen(s);
    while(n && (s[n-1] == '\n' || s[n-1] == '\r' || isspace((unsigned char)s[n-1]))){
        s[--n] = '\0';
    }
}
int space_check(char *s){
    if(!s) return 0;
    return strchr(s, ' ') != NULL;

}
void add_user(){
    char tmp[1024];
    printf("----------------------------------------------------------------------\n");
    printf(BOLD CYAN " **NOTE Not Allow Space Bar And Comma In Field**\n"RESET);
    printf(BOLD RED" **IMPORTANT Use Comma(,) To Cut Collumns**\n"RESET);
    printf("----------------------------------------------------------------------\n");
    printf("Example Format\n");
    printf(GREEN" **CORECT 123,Cristianno Ronaldo,B,11/11/2025,12:00\n"RESET);
    printf(RED" **INCORECT 123,Leo,nel,Jessi,B,11,/11/2025,12,00\n"RESET);
    printf(RED" **INCORECT 12 3, Lamean Jaamal, B,11 /11/ 2025,12: 00\n"RESET);
    printf("----------------------------------------------------------------------\n");
    printf(YELLOW"type here : "RESET);

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
    //เช็คช่องว่าง
    char *nullcheck[] = {id,name,room,date,time};
    for(int i = 0;i < 5;i++){
        if((nullcheck[i] == NULL)){
            printf(RED BOLD"**Invalid Pattern Try Pls Again**\n" RESET);
            return;
        }
        if(nullcheck[i][0] == '\0'){
            printf(RED BOLD"**Invalid Pattern Try Pls Again**\n" RESET);
            return;
        }
    } 
    if (space_check(id) || space_check(room) || space_check(date) || space_check(time)) {
        printf(BOLD RED"Error: ID/Room/Date/Time must not contain spaces\n"RESET);
        return;
}
    //เช็คซ้ำ
    for(int i=0;i<Row;i++){
        if(BookingID[i][0]=='\0') continue;

        if(strcmp(BookingID[i], id)==0){ printf(RED BOLD"**Duplicate ID**\n" RESET); return; }

        if(strcmp(MeetingRoom[i],room)==0 &&
            strcmp(BookingDate[i], date)==0 &&
            strcmp(BookingTime[i], time)==0){
            printf(RED BOLD"**Time Slot Already Taken**\n" RESET);
            return;
        }
        
        if (strcmp(BookerName[i],  name) == 0 &&
            strcmp(BookingDate[i], date) == 0 &&
            strcmp(BookingTime[i], time) == 0) {
            printf(RED BOLD"**You Can't Reserve More Than One Room In Same Time**\n" RESET);
            return;   
        }
    }
    
    int idx = -1;
    for(int i = 0;i<Row;i++){
        if(BookingID[i][0] == '\0'){
            idx  = i;
            break;
        }
    }
    if (idx == -1){
        printf(BOLD RED"**Storage Full**\n"RESET);
        return;
    }
    snprintf(BookingID[idx],20,"%s",id);
    snprintf(BookerName[idx],20,"%s",name);
    snprintf(MeetingRoom[idx],20,"%s",room);
    snprintf(BookingDate[idx],20,"%s",date);
    snprintf(BookingTime[idx],20,"%s",time);

    save_all();
}
void edit_user(){
    char search_id[20]; 
    int found_count = 0;
    printf(YELLOW "Search By ID : "RESET);
    scanf("%19s",search_id);
    getchar();
    for(int i = 0; i < Row;i++){
        if(strcmp(BookingID[i],search_id) == 0){
            printf(BOLD GREEN"ID Found!!\n"RESET);
            for(int j = 0;j<20;j++){
                BookingID[i][j] ='\0';BookerName[i][j] ='\0';MeetingRoom[i][j] ='\0';BookingDate[i][j] ='\0';BookingTime[i][j] ='\0';
            }
            add_user();
            found_count += 1;
            break;
        }
    }
    if(!found_count){
        printf(RED"ID Not Found Pls Check Again\n"RESET);
        return;
    }
    

}
int main() {
    for (int i = 0; i < Row; i++) BookingID[i][0] = '\0';

    FILE *f = fopen("Book1.csv","r");
    if(!f){
        save_all();
        f = fopen("Book1.csv","r");
    }
    char line[1024];
    int count_row = 0;
    if (!fgets(line, sizeof(line), f)) {   
    fclose(f);
    save_all();                        
    f = fopen("Book1.csv","r");
    if (!f) return 1;
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
            snprintf(BookingID[count_row],20,"%s",id);
            snprintf(BookerName[count_row],20,"%s",name);
            snprintf(MeetingRoom[count_row],20,"%s",room);
            snprintf(BookingDate[count_row],20,"%s",date);
            snprintf(BookingTime[count_row],20,"%s",time);
            count_row++;
        }
    }
    fclose(f);
    edit_user();

    return 0;
}