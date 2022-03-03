#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/stat.h>

#define max_size 0xfffff // 메모리 최대 크기 1048576
#define max_val 0x7FFFFF // value의 최대 크기
#define min_val -0x800000 // value의 최소 크기

typedef struct _HistNode{ // history 링크드리스트에 이용할 구조체
	char command[100];
	struct _HistNode *ptr;
}HistNode;

typedef struct _OpNode{ // opcode 해시 테이블에 이용할 구조체
	int code;
	char mnemonic[10];
	char type[5];
	struct _OpNode *ptr;
}OpNode;

typedef struct _ObNode{ // objectcode 저장할 linked list
    int format, first_data, mid_data, end_data, location;
    struct _ObNode* ptr;
    int enter_flag;
    char var[255];
}ObNode;

typedef struct _SymNode{ // symbol table에 이용할 linked list
    int location;
    char name[10];
    struct _SymNode* ptr;
}SymNode;

unsigned char memory[max_size]; // 메모리 정보를 저장할 배열
int memory_address; // 메모리 주소

int sym_flag; // symbol table 생성되었는지 flag
int save_flag; // symbol table 저장본 있는지 flag
int s_idx, s_num; // saved symbol table 인덱스, symbol 개수

HistNode* first; // 히스토리 링크드리스트의 시작
HistNode* last; // 히스토리 링크드리스트의 마지막

OpNode* OpTable[20]; // opcode 해시 테이블

SymNode** SymTable; // symbol 해시 테이블
SymNode* save_symtab; // 최근 성공한 파일에 대한 symbol 테이블

/*
설명 : 입력된 명령어의 종류를 구분해주는 함수
Input : 사용자의 입력에서 첫번째 토큰, comma 존재 여부에 대한 flag, 토큰의 개수 
Output : 각 기능에 해당하는 번호, 잘못된 명령어는 0을 반환
*/
int find_command(char* input, int flag, int token);

/*
설명 : 사용이 가능한 명령어들을 출력하는 함수
Input : 없음 
Output : 없음
*/
void print_help();

/*
설명 : 디렉터리 내용을 출력하는 함수
Input : 없음 
Output : 디렉터리 오류이면 1을 반환, 정상이면 0을 반환
*/
int print_dir();

/*
설명 : 명령어를 히스토리에 삽입해주는 함수
Input : 히스토리에 삽입할 명령어
Output : 없음
*/
void insert_history(char* command);

/*
설명 : 히스토리 linked list를 출력하는 함수
Input : 없음
Output : 없음
*/
void print_history();

/*
설명 : 배열 초기화 함수
Input : 초기화할 배열
Output : 없음
*/
void reset_array(char* arr);

/*
설명 : 메모리 초기화 함수
Input : 초기화할 메모리
Output : 없음
*/
void reset_memory();

/*
설명 : dump, fill 명령어 관련하여 주소의 범위를 조정해주는 함수
Input : dump, fill 명령어의 종류, 시작 주소, 끝 주소
Output : 오류인 경우 1을 반환하고 정상인 경우 0을 반환한다.
*/
int check_boundary(int dump_case, int* start, int* end);

/*
설명 : 입력이 알맞은 16진수 형태인지 확인해주는 함수
Input : 문자 배열
Output : 오류인 경우 0을 반환하고 정상인 경우 1을 반환한다.
*/
int check_hexa(char* arr);

/*
설명 : 메모리를 출력하는 함수
Input : 시작 주소, 끝 주소
Output : 없음
*/
void print_memory(int start, int end);

/*
설명 : 입력을 히스토리에 저장하기 위해 정제하는 함수
Input : 토큰 개수, 토큰 저장한 배열
Output : 컴마의 개수
*/
int refined_command(int token, char tokenarr[7][100]);

/*
설명 : 해시 테이블을 위해 적절한 인덱스를 반환하는 함수
Input : 문자 배열
Output : 인덱스
*/
int hash_function(char *str);

/*
설명 : 해시 테이블 생성 함수
Input : 없음
Output : 파일 관련 에러는 0을 반환, 정상인 경우는 1을 반환
*/
int make_opcodetable();

/*
설명 : 해시 테이블에서 명령어에 알맞은 opcode를 찾아주는 함수
Input : 문자 배열
Output : 입력 오류는 0을 반환, 정상인 경우는 1을 반환
*/
int find_opcode(char *str);

/*
설명 : 해시 테이블을 출력하는 함수
Input : 없음
Output : 없음
*/
void print_opcodelist();

/*
설명 : 입력 받은 파일을 출력하는 함수
Input : 파일 이름
Output : 정상적으로 출력되었으면 1, 파일이 존재하지 않으면 0을 반환
*/
int print_type(char* filename);

/*
설명 : 심볼 테이블을 출력하는 함수
Input : 없음
Output : 정상적으로 출력되었으면 1, 에러가 발생하면 0을 반환
*/
int print_symtab();

/*
설명 : 심볼을 입력 받아 심볼 테이블을 생성하는 함수
Input : 심볼 정보가 담긴 노드, 에러 발생 시 라인 넘버
Output : 정상적으로 생성되었으면 0, 에러가 발생하면 1을 반환
*/
int make_symtable(SymNode* node, int line_num);

/*
설명 : 심볼 테이블에서 입력된 심볼의 주소를 반환해주는 함수
Input : 심볼 정보가 담긴 노드
Output : 정상적으로 값을 찾았으면 주소, 에러가 발생하면 상수일때 -2을, 탐색에 실패한 경우 -1을 반환
*/
int find_symloc(char* symbol);

/*
설명 : 입력한 레지스터의 번호를 반환하는 함수
Input : 레지스터 이름
Output : 레지스터 번호 (해당하는 레지스터가 없는 경우 -1을 반환)
*/
int register_idx(char* reg);

/*
설명 : format과 mnemonic에 맞게 주소를 계산해주는 함수
Input : 포맷 번호, 주소, operand의 첫 글자
Output : 없음
*/
void location_counter(char* format, int* location, char mnemonic);

/*
설명 : 입력받은 mnemonic에 알맞은 opcode를 반환하는 함수
Input : mnemonic, 포맷 번호, opcode
Output : 정상적으로 반환한다면 1, 아니면 0을 반환
*/
int mnemonic_to_opcode(char *mnemonic, char *format, int *opcode);

/*
설명 : object code를 링크드 리스트의 알맞은 위치에 삽입해주는 함수
Input : 테이블의 시작 위치, 테이블의 마지막 위치, format 번호, 첫번째 데이터, 중간 데이터, 마지막 데이터, 주소, var 문자열, 엔터플래그
Output : 없음
*/
void insert_obcode(ObNode** head, ObNode** tail, int format, int first, int mid, int end, int location, char* var, int enter_flag);

/*
설명 : 입력받은 파일을 어셈블해주는 함수
Input : 입력받은 파일 이름
Output : 정상적으로 어셈블했다면 1, 아니면 0을 반환
*/
int assemble_file(char* filename);