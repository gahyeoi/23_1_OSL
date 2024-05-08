#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PATH_MAX 4096

char cwd[PATH_MAX] ; 
int zero = 0 ; 

int retrack(char * path, FILE * fp){

    DIR * dir = opendir(path) ; 
    if(dir == 0x0){
        printf("retrack failed %s!", path) ;
        return 0 ; 
    }

    for(struct dirent * i = readdir(dir) ; i != 0x0; i=readdir(dir)){
        
        if((strcmp(i->d_name, ".") == 0) || (strcmp(i->d_name, "..") == 0))
            continue ; 

        char filePath[PATH_MAX] ;
        strcpy(filePath, path) ;
        strcpy(filePath + strlen(path), "/") ;
        strcat(filePath, i->d_name) ;

        if(i->d_type == DT_DIR){
            retrack(filePath, fp) ;
        } else {
            fprintf(fp, "%s, %d\n", filePath, 0) ;
        }
    }

    if(closedir(dir) == -1){
        printf("Failed to close directory %s\n", path) ;
        return 0 ; 
    }

    return 1 ; 
}

void copyFile(const char *sourcePath, const char *destinationPath)
{
    
    FILE * sourceFile = fopen(sourcePath, "rb");
    if (sourceFile == 0x0){
        return;
    }

    struct stat st ; 
    stat(destinationPath, &st) ;

    if(stat(destinationPath, &st) == -1) { // 파일이 존재하지 않는 경우 
        
        char *mutablePath = strdup(destinationPath);  // 입력된 path 문자열을 수정 가능한 문자열로 복사
        char *token = strtok(mutablePath, "/");
        char currentPath[256] = "";

        while (token != NULL) {
            
            strcat(currentPath, token);
            if((token = strtok(NULL, "/")) == 0x0)
                break ; 
            strcat(currentPath, "/");

            if (access(currentPath, F_OK) != 0) {
                int result = mkdir(currentPath, 0755);
                if (result == -1) {
                    free(mutablePath);
                    return;
                } 
            }
        }
        free(mutablePath) ;
    }

    FILE * destinationFile = fopen(destinationPath, "wb");
    if (destinationFile == 0x0)
    {
        return;
    }

    char buffer[4096];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0)
    {
        fwrite(buffer, 1, bytesRead, destinationFile);
    }

    fclose(sourceFile);
    fclose(destinationFile);
}

int init()
{   
    if(access(".keep", F_OK) == 0){
        printf(".keep already exists.\n") ;
        exit(1) ;
    } else {
        mkdir(".keep", 0777) ;

        FILE * tracking_files = fopen("./.keep/tracking-files", "w") ;
        if(tracking_files == 0x0){
            printf("Failed to create tracking-files.\n") ;
            exit(1) ;
        }

        FILE * latest_version = fopen("./.keep/latest-version", "w") ; 
        if(latest_version == 0x0){
            printf("Failed to create latest-version.\n") ;
            exit(1) ;
        }

        fputs("0", latest_version) ;
        fclose(tracking_files) ;
        fclose(latest_version) ;
    }    

    return 1 ; 
}

int track(char *path)
{
    // .keep directory가 존재하는지 확인 
    if (access(".keep", F_OK) != 0)
    {
        printf(".keep doesn't exist.\n");
        exit(1);
    }

    // track하고자 하는 파일 혹은 디렉토리가 존재하는지 확인 
    if (access(path, F_OK) != 0)
    {
        printf("There's no '%s'.\n", path);
        exit(1);
    }

    FILE * tracking_file = fopen(".keep/tracking-files", "r");
    if (tracking_file == 0x0)
    {
        printf("tracking-files doesn't exist.\n");
        exit(1);
    }

    char line[4096] ;
    while(fgets(line, sizeof(line), tracking_file)){
        
        char * tracked_file = strtok(line, ", ") ;

        struct stat pathStat ; 
        stat(path, &pathStat) ;

        if(S_ISREG(pathStat.st_mode)){
            if(strcmp(path, tracked_file) == 0){
                printf("%s is already tracked.", path) ;
                return 0 ; 
            }
        } else if (S_ISDIR(pathStat.st_mode)){
            if(strstr(tracked_file, path) != 0x0){
                printf("%s is already tracked.", path) ;
                return 0 ; 
            }
        }
    }
    fclose(tracking_file) ;

    // regular file인지 혹은 directory인지 확인 
    struct stat st ; 
    stat(path, &st) ; 

    FILE * t = fopen(".keep/tmp_tracking", "w") ; 
    if(t == 0x0){
        printf("Failed") ;
        exit(1) ;
    }
   
   FILE * t2 = fopen(".keep/tmp", "w") ; 
    if(t2 == 0x0){
        printf("Failed t2") ;
        exit(1) ;
    }

    if(S_ISREG(st.st_mode)){
        fprintf(t, "%s, %d\n", path, 0) ;  
    } else if (S_ISDIR(st.st_mode)){

        DIR * dir = opendir(path) ; 

        for(struct dirent * i = readdir(dir); i != 0x0; i = readdir(dir)){

            if(strcmp(i->d_name, ".") == 0 || strcmp(i->d_name, "..") == 0)
                continue;
            
            char file_path[PATH_MAX];
            snprintf(file_path, sizeof(file_path), "%s/%s",path, i->d_name);

            struct stat st ; 
            stat(file_path, &st) ;

            if(S_ISREG(st.st_mode)){
                fprintf(t, "%s, %d\n", file_path, 0) ;
            } else if (S_ISDIR(st.st_mode)){
                retrack(file_path, t) ;
            }

        }
        if(closedir(dir) == -1){
            printf("Failed to close directory %s", path) ;
            return 0 ; 
        }
    }

    fclose(t) ;

    tracking_file = fopen(".keep/tracking-files", "r") ;

    while(fgets(line, sizeof(line), tracking_file)){
        char * filePath = strtok(line, ",") ; 

        struct stat filePathStat ; 
        stat(filePath, &filePathStat) ; 
        
        fprintf(t2, "%s, %ld\n", filePath, filePathStat.st_mtime) ; 
    }
    fclose(t2) ;
    fclose(tracking_file) ;
    
    // Open tracking file for writing
    tracking_file = fopen(".keep/tracking-files", "w");
    if (tracking_file == 0x0)
    {
        printf("Failed to open tracking-files for writing.\n");
        exit(1);
    }
    
    t = fopen(".keep/tmp_tracking", "r") ;
    if(t == 0x0){
        printf("Failed to open") ;
        exit(1) ;
    }   

    t2 = fopen(".keep/tmp", "r") ;
    if(t2 == 0x0){
        printf("Failed to open") ;
        exit(1) ;
    }  

    while(fgets(line, sizeof(line), t2)){
        fprintf(tracking_file, "%s", line) ;
    }

    while(fgets(line, sizeof(line), t)){
        fprintf(tracking_file, "%s", line) ;
    }

    fclose(t) ; 
    fclose(t2) ;
    
    remove(".keep/tmp") ;
    remove(".keep/tmp_tracking") ; 

    fclose(tracking_file);

    return 1;
}

int untrack(char * path) 
{
    if(access(".keep", F_OK) != 0){
        printf(".keep doesn't exist.") ;
        return 0 ;
    }

    if(access(path, F_OK) != 0){
        printf("There's no '%s'.\n", path) ;
        return 0;
    }

    FILE * tracking_files = fopen(".keep/tracking-files", "r") ; 
    if(tracking_files == 0x0){
        printf("tracking-files doesn't exists.\n") ;
        return 0 ; 
    } 

    char line[4096] ;

    int check = 0 ; // check가 1으로 바껴야 tracking-file 내부에 해당 파일 혹은 디렉토리가 있는 거야  
    while(fgets(line, sizeof(line), tracking_files)){
        char * path_in_track = strtok(line, ", ") ;
        
        struct stat st ; 
        stat(path, &st) ;

        if(S_ISREG(st.st_mode)){
            if(strcmp(path, path_in_track) == 0)
                check = 1 ; 
        } else if(S_ISDIR(st.st_mode)){
            if(strstr(path, path_in_track) != 0x0)
                check = 1 ; 
        }

    }   

    rewind(tracking_files) ;

    if(check == 0){
        printf("%s not in tracking-files, couldn't untrack.", path);
        return 0 ; 
    }

    struct stat st ; 
    stat(path, &st) ; 

    FILE * tmp = fopen(".keep/tracking-files.new", "w") ;
    if(tmp == 0x0){
        return 0 ; 
    }

    if(S_ISREG(st.st_mode)){
         
        while(fgets(line, sizeof(line), tracking_files)){
            
            char * copied_line ; 
            copied_line =  (char *)malloc(sizeof(char) * (strlen(line) + 1)) ;
            strcpy(copied_line, line) ;

            char * ptr = strtok(line, ",") ;

            if(strcmp(path, ptr) != 0){
                fprintf(tmp, "%s", copied_line) ;
            }
            
            free(copied_line) ;
        }
    } else if(S_ISDIR(st.st_mode)){
        while(fgets(line, sizeof(line), tracking_files)){
            char * copied_line ; 
            copied_line =  (char *)malloc(sizeof(char) * (strlen(line) + 1)) ;
            strcpy(copied_line, line) ;

            char * ptr = strtok(line, ",") ;
            if(strncmp(ptr, path, strlen(path)) != 0 ){
                fprintf(tmp, "%s", copied_line) ;
            }

            free(copied_line) ;
        }
    }
    
    fclose(tracking_files) ;
    fclose(tmp) ;

    remove(".keep/tracking-files") ;
    rename(".keep/tracking-files.new", ".keep/tracking-files") ;

    return 1 ;
}

int store(char * note)
{
    // track하고 있는 파일에 수정사항이 있는지 체크 
    FILE * k_track = fopen(".keep/tracking-files", "r") ;
    if(k_track == 0x0){
        printf("Failed to open .keep/tracking-files") ;
        return 0 ; 
    }

    char line[4096] ;
    int check = 0 ; // check가 0이면 수정시간 변동 x
    while(fgets(line, sizeof(line), k_track)){
        
        char * file_path ;
        time_t modified_time ; 

        file_path = strtok(line, ", ");
        char * tmp = strtok(NULL, ",") ;

        struct stat st ;    
        stat(file_path, &st) ; 
        
        modified_time = atoi(tmp) ;

        if(modified_time != st.st_mtime){
            check = 1 ;
        }
    }
    fclose(k_track) ;

    if(check == 0){  // 업데이트 된 부분이 없다면 
        printf("nothing to store\n") ; 
        return 0 ; 
    }

    // .keep 아래에 version directory 생성하기 위해 latest-version 확인하기 
    FILE * k_version = fopen(".keep/latest-version", "r") ; 
    if(k_version == 0x0){
        printf("Failed to open .keep/latest-version") ;
        return 0 ; 
    }

    int tmp ;
    fscanf(k_version, "%d", &tmp) ; 
    char version[100] ; 
    tmp = tmp + 1;
    sprintf(version, "%d", tmp) ;
    fclose(k_version) ;

    char version_path[PATH_MAX] ;
    strcpy(version_path, ".keep/") ;
    strcpy(version_path + 6, version) ;
  
    mkdir(version_path, 0755) ;

    char v_tracking_path[PATH_MAX] ;
    strcpy(v_tracking_path, version_path) ;
    strcpy(v_tracking_path + strlen(version_path), "/tracking-files") ;

    FILE * v_track = fopen(v_tracking_path, "w") ;
    if(v_track == 0x0){
        printf("Failed to open %s\n", v_tracking_path) ;
        exit(1) ;
    }

    k_track = fopen(".keep/tracking-files", "r") ;
    if(k_track == 0x0){
        printf("Failed to open .keep/tracking-files") ;
        exit(1) ;
    }

    while(fgets(line, sizeof(line), k_track)){
        char * file = strtok(line, ", ") ;

        struct stat fileStat ; 
        stat(file, &fileStat) ;

        fprintf(v_track, "%s, %ld\n", file, fileStat.st_mtime) ;
    }

    fclose(v_track) ;
    fclose(k_track) ;

    copyFile(v_tracking_path, ".keep/tracking-files") ;
    

    // .keep/note 생성 후 내용 넣기 
    char v_note_path[PATH_MAX] ;
    strcpy(v_note_path, version_path) ;
    strcpy(v_note_path + strlen(version_path), "/note") ;

    FILE * v_note = fopen(v_note_path, "w") ; 
    if(v_note == 0x0){
        printf("Failed to load %s", v_note_path) ;
        return 0 ; 
    }
    fprintf(v_note, "%s", note) ;
    fclose(v_note) ;

    char v_target_path[PATH_MAX] ;
    strcpy(v_target_path, version_path) ;
    strcpy(v_target_path + strlen(version_path), "/target") ; 
    mkdir(v_target_path, 0777) ; 

    FILE * ver_tracking = fopen(v_tracking_path, "r") ;
    if(ver_tracking == 0x0){
        exit(1) ;
    }

    while(fgets(line, sizeof(line), ver_tracking)){
        char * file_path = strtok(line, ", ") ;

        char version_file_path[PATH_MAX] ; 
        strcpy(version_file_path, v_target_path) ;
        strcpy(version_file_path + strlen(v_target_path), "/") ;
        strcpy(version_file_path + strlen(v_target_path) + 1, file_path) ;

        copyFile(file_path, version_file_path) ;
    }   

    fclose(ver_tracking) ;

    k_version = fopen(".keep/latest-version", "w") ; 
    if(k_version == 0x0){
        exit(1) ;
    }
    fprintf(k_version, "%s", version) ;
    
    fclose(k_version) ; 

    return 1 ;
}

int restore(char * version)
{   
    struct dirent *entry;

    char version_path[PATH_MAX];
    strcpy(version_path, ".keep/");
    strcpy(version_path + 6, version);
    strcpy(version_path + 7, "/tracking-files");

    FILE *k_version = fopen(".keep/latest-version", "r");
    if (k_version == 0x0)
    {
        exit(1);
    }

    int tmp ;
    fscanf(k_version, "%d", &tmp) ; 
    char latest_ver[100] ; 
    sprintf(latest_ver, "%d", tmp) ;

    fclose(k_version);

    if (strcmp(latest_ver, version) == 0)
    {
        printf("restored as version %s", version);
        return 1 ;
    }

    // void copyFile(const char *sourcePath, const char *destinationPath)
    char *main_path = ".keep/tracking-files";
    copyFile(version_path, main_path);

    char version_file_path[PATH_MAX];
    strcpy(version_file_path, ".keep/");
    strcpy(version_file_path + 6, version);
    strcpy(version_file_path + 7, "/target/");

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    DIR * dir = opendir(cwd) ;
    if(dir == 0x0){
        printf("Failed to open %s\n", cwd) ;
        return 0 ; 
    }

    // what? 
    for(struct dirent * i = readdir(dir); i != 0x0; i = readdir(dir)){

        if(strcmp(i->d_name, ".") == 0 || strcmp(i->d_name, "..") == 0 || strcmp(i->d_name, ".keep") == 0)
            continue ; 

        char path[PATH_MAX] ;
        strcpy(path, cwd) ;
        strcpy(path + strlen(cwd), "/") ;
        strcpy(path + strlen(cwd) + 1, i->d_name) ;

        remove(path) ;
        
    }

    if((closedir(dir) == -1)){
        printf("Failed to close %s", cwd) ;
        return 0 ; 
    }

    DIR * ver_target = opendir(version_file_path) ;
    if(ver_target == 0x0) {
        printf("Failed to open %s", version_file_path) ;
        return 0 ; 
    }

    for(struct dirent * i = readdir(ver_target); i != 0x0 ; i = readdir(ver_target)){

        if(strcmp(i->d_name, ".") == 0 || strcmp(i->d_name, "..") == 0)
            continue ; 
        
        char src_path[PATH_MAX] ;
        strcpy(src_path, version_file_path) ;
        strcpy(src_path + strlen(version_file_path), i->d_name) ;

        char dest_path[PATH_MAX] ; 
        strcpy(dest_path,i->d_name) ;

        copyFile(src_path, dest_path) ;
    }

    if((closedir(ver_target) == -1)){
        printf("Failed to close .keep/%s/target", version) ;
        return 0 ; 
    }

    printf("restored as version %s", version) ;
    return 1 ; 
}

int versions()
{      
    DIR * keep = opendir(".keep") ;
    if(keep == 0x0){
        printf("Failed to open .keep") ;
        return 0 ; 
    }

    for(struct dirent * i = readdir(keep); i != 0x0; i = readdir(keep)){
        if(i->d_type == DT_DIR){
            
            if((strcmp(i->d_name, ".") == 0) || (strcmp(i->d_name, "..") == 0))
                continue ; 

            char * version = i->d_name ;
    
            char v_note_path[PATH_MAX] ;
            strcpy(v_note_path, ".keep/") ; 
            strcpy(v_note_path + 6, i->d_name) ;
            strcat(v_note_path, "/note") ;

            FILE * v_note = fopen(v_note_path, "r") ;
            if(v_note == 0x0){
                printf("Failed to open %s\n", v_note_path) ;
                exit(1) ;
            }

            char note[4096] ;
            fgets(note, sizeof(note), v_note) ;

            printf("%s %s\n", version, note) ; 
            fclose(v_note) ;
        }
    }

    if(closedir(keep) == -1){
        printf("Failed to close .keep") ;
        return 0 ; 
    }
    
    return 1 ; 
}

int main(int argc, char ** argv)
{
    getcwd(cwd, PATH_MAX) ;

    if(argc == 2){
        if(strcmp(argv[1], "init") == 0){
            init() ;
        } else if(strcmp(argv[1], "versions") == 0){
            versions() ;
        } else {
            printf("Invalid command") ;
            exit(1) ;
        }
    } else if (argc == 3){
        if(strcmp(argv[1], "track") == 0){
            track(argv[2]) ;
        } else if(strcmp(argv[1], "untrack") == 0){
            untrack(argv[2]) ;
        } else if(strcmp(argv[1], "store") == 0){
            store(argv[2]) ;
        } else if(strcmp(argv[1], "restore") == 0){
            restore(argv[2]) ;
        } else {
            printf("Invalid command") ;
            exit(1) ;
        }
    } else {
        printf("Invalid command") ;
        exit(1) ;
    }
    return 0 ; 
}
