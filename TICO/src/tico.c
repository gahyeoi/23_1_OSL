#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  READ,
  WRITE,
  ASSIGN,
  MOVE,
  LOAD,
  STORE,
  ADD,
  MINUS,
  MULT,
  MOD,
  EQ,
  LESS,
  JUMP,
  JUMPIF,
  TERM,
  NWRITE
} oper ;

typedef struct _instruction{
  oper operator ;
  unsigned char r[3] ;
} instruction ;

typedef union _memory{
  char num ;
  instruction inst ;
} value ;

oper oper_sort(char *s) {

    oper result ; 

    if(strcmp(s, "READ") == 0){
        result = 0 ; 
    } else if(strcmp(s, "WRITE") == 0){
        result = 1 ; 
    } else if(strcmp(s, "ASSIGN") == 0){
        result = 2 ; 
    } else if(strcmp(s, "MOVE") == 0){
        result = 3 ; 
    } else if(strcmp(s, "LOAD") == 0){
        result = 4 ; 
    } else if(strcmp(s, "STORE") == 0){
        result = 5 ; 
    } else if(strcmp(s, "ADD") == 0){
        result = 6 ;
    } else if(strcmp(s, "MINUS") == 0){
        result = 7 ;
    } else if(strcmp(s, "MULT") == 0){
        result = 8 ;
    } else if(strcmp(s, "MOD") == 0){
        result = 9 ;
    } else if(strcmp(s, "EQ") == 0){
        result = 10 ; 
    } else if(strcmp(s, "LESS") == 0){
        result = 11 ; 
    } else if(strcmp(s, "JUMP") == 0){
        result = 12 ; 
    } else if(strcmp(s, "JUMPIF") == 0){
        result = 13 ; 
    } else if(strcmp(s, "TERM") == 0){
        result = 14 ; 
    } else if(strcmp(s, "NWRITE") == 0){
        result = 15 ;
    } else {
        printf("Invalid Instruction\n");
        exit(0);
    }

    return result ; 
}

int check_num_valid(int num){

    if(num < -128 || num > 127)
        return 0 ; 

    return 1 ; 
}

int check_memory_valid(int address){

    if(address < 0 || address > 255)
        return 0 ; 

    return 1 ;
}

char * read_a_line (FILE * fp)
{
	static char buf[BUFSIZ] ;
	static int buf_n = 0 ;
	static int curr = 0 ;

	if (feof(fp) && curr == buf_n - 1)
		return 0x0 ;

	char * s = 0x0 ;
	size_t s_len = 0 ;
	do {
		int end = curr ;
		while (!(end >= buf_n || !iscntrl(buf[end]))) {
			end++ ;
		}
		if (curr < end && s != 0x0) {
			curr = end ;
			break ;
		}
		curr = end ;
		while (!(end >= buf_n || iscntrl(buf[end]))) {
			end++ ;
		}
		if (curr < end) {
			if (s == 0x0) {
				s = strndup(buf + curr, end - curr) ;
				s_len = end - curr ;
			}
			else {
				s = realloc(s, s_len + end - curr + 1) ;
				s = strncat(s, buf + curr, end - curr) ;
				s_len = s_len + end - curr ;
			}
		}
		if (end < buf_n) {
			curr = end + 1 ;
			break ;
		}

		buf_n = fread(buf, 1, sizeof(buf), fp) ;
		curr = 0 ;
	} while (buf_n > 0) ;
	return s ;
}

void set_memory(FILE * fp, value memory[]){

    char * s = 0x0 ; 

    while((s = read_a_line(fp))){
        
        s = strtok(s, " ") ; 

        int address = atoi(s) ;

        s = strtok(NULL, "\n") ;

        if(s == 0x0){
            memory[address].num = 0 ; 
        } else {
            if(isalpha(*s)){

                s = strtok(s, " ") ;
                
                memory[address].inst.operator = oper_sort(s) ;

                s = strtok(NULL, " ");

                int count = 0 ; 

                while(s != 0x0){
                    
                    while(1){
                        if(isdigit(*s))
                            break ; 
                        
                        s++ ;
                    }
                    memory[address].inst.r[count] = atoi(s) ;

                    count++ ; 
                    s = strtok(NULL, " ") ;
                   
                }

            } else {

                 s = strtok(s, " ") ;
                
                while(1){
                    if(isdigit(*s))
                        break ; 
                        
                    s++ ;
                }

                memory[address].num = atoi(s) ;
            }

        }
    }
}

void interpret(value memory[]){

    int cursor = 0 ;

    int operator = memory[cursor].inst.operator ;

    while(operator != 14){ // while the operator is not "TERM"
        
        operator = memory[cursor].inst.operator ;
    
        switch (operator)
        {
        case 0: // READ
        {
            int address = memory[cursor].inst.r[0] ;

            int num = 0 ; 

            scanf("%d", &num) ;

            if(!check_num_valid(num)){
                printf("Invalid Number\n") ;
                exit(1) ;
            }

            memory[address].num = num ; 
            cursor++ ;
             
        } break ;
        case 1: // WRITE
        {
            int address = memory[cursor].inst.r[0] ;

            if(!check_memory_valid(address)){
                printf("Invalid Memory Address\n") ;
                exit(1) ;
            }

            printf("%d\n", memory[address].num) ;
            cursor++ ;

        } break;
        case 2: // ASSIGN
        {
            int address = memory[cursor].inst.r[0] ;

            if(!check_memory_valid(address)){
                printf("Invalid Memory Address\n") ;
                exit(1) ;
            }

            int num = memory[cursor].inst.r[1] ;

            if(!check_num_valid(num)){
                printf("Invalid Number\n") ;
                exit(1) ;
            }

            memory[address].num = num ; 
            cursor++ ;

        } break;
        case 3: // MOVE 
        {
            int md = memory[cursor].inst.r[0] ;
            int ms = memory[cursor].inst.r[1] ;

            int ms_value = memory[ms].num ; 

            memory[md].num = ms_value ;
            cursor ++;

        } break ; 
        case 4: // LOAD 
        {
            int md = memory[cursor].inst.r[0] ;
            int ms = memory[cursor].inst.r[1] ;

            int ms_num = memory[ms].num ; 

            if(!check_num_valid(ms_num)){
                printf("Invalid Number\n") ; 
                exit(1) ;
            }
            
            memory[md].num = memory[ms_num].num ; 
            cursor++ ;

        } break ; 
        case 5: // STORE
        {
            int md = memory[cursor].inst.r[0] ;
            int ms = memory[cursor].inst.r[1] ;

            int md_num = memory[md].num ; 

            if(!check_memory_valid(md_num)){
                printf("Invalid Memory Access\n") ;
                exit(1) ;
            }

            memory[md_num].num = memory[ms].num ; 
            cursor++ ;

        } break ; 
        case 6 : // ADD
        {
            int md = memory[cursor].inst.r[0] ;
            int mx = memory[cursor].inst.r[1] ;
            int my = memory[cursor].inst.r[2] ;

            int result = memory[mx].num + memory[my].num ; 
            
            if(!check_num_valid(result)){
                exit(1) ;
            }

            memory[md].num = result ; 
            cursor++ ;

        } break ; 
        case 7: // MINUS
        {
            int md = memory[cursor].inst.r[0] ;
            int mx = memory[cursor].inst.r[1] ;
            int my = memory[cursor].inst.r[2] ;

            int result = memory[mx].num - memory[my].num ; 
            
            if(!check_num_valid(result)){
                exit(1) ;
            }

            memory[md].num = result ; 
            cursor++ ;

        } break ; 
        case 8: // MULT
        {
            int md = memory[cursor].inst.r[0] ;
            int mx = memory[cursor].inst.r[1] ;
            int my = memory[cursor].inst.r[2] ;

            int result = memory[mx].num * memory[my].num ; 
            
            if(!check_num_valid(result)){
                exit(1) ;
            }

            memory[md].num = result ; 
            cursor++ ;

        } break ;
        case 9: // MOD
        {
            int md = memory[cursor].inst.r[0] ;
            int mx = memory[cursor].inst.r[1] ;
            int my = memory[cursor].inst.r[2] ;

            if(memory[my].num == 0){
                exit(1) ;
            }

            int result = memory[mx].num % memory[my].num ; 

            if(!check_num_valid(result)){
                exit(1) ;
            }

            memory[md].num = result ; 
            cursor++ ;

        } break ;
        case 10: // EQ
        {
            int address = memory[cursor].inst.r[0] ;
            int mx = memory[cursor].inst.r[1] ;
            int my = memory[cursor].inst.r[2] ;

            if(memory[mx].num == memory[my].num){
                memory[address].num = 1 ; 
            } else{
                memory[address].num = 0 ; 
            }

            cursor++ ;

        } break ;
        case 11 : // LESS
        {
            int address = memory[cursor].inst.r[0] ;
            int mx = memory[cursor].inst.r[1] ;
            int my = memory[cursor].inst.r[2] ;

            if(memory[mx].num < memory[my].num){
                memory[address].num = 1 ; 
            } else{
                memory[address].num = 0 ; 
            }

            cursor++ ;

        } break ;
        case 12: // JUMP
        {
            int address = memory[cursor].inst.r[0] ;
            
            cursor = address ; 

        } break ; 
        case 13: // JUMPIF
        {
            int address = memory[cursor].inst.r[0] ;
            int check = memory[cursor].inst.r[1] ;

            if(memory[check].num != 0){
                cursor = address ; 
            } else{
                cursor++ ; 
            }

        } break ; 
        case 14: // TERM
            break ;
        case 15: // NWRITE 
        {
            int address = memory[cursor].inst.r[0] ; 
    
            printf("%d", memory[address].num) ; 
            cursor++ ;

        } break ; 

        default:
            break;
        }
    }
}

int main(int argc, char ** argv){

    if(argc != 2){
        exit(1) ;
    }

    FILE * program = fopen(argv[1], "r") ;

    if(program == 0x0){
        printf("Failed to open the program %s\n", argv[1]) ;
        exit(1) ;
    }

    // make 256 memories and initialize them 
    value memory[256] ;
    for(int i = 0 ; i < 256 ; i++){
        memory[i].inst.operator =  0 ; 
        memory[i].num = 0 ; 
    }

    set_memory(program, memory) ;
    
    interpret(memory) ;

    fclose(program) ;
    return 0 ; 
}
