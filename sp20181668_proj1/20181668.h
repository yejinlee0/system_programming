#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/stat.h>

#define max_size 0xfffff // 메모리 최대 크기 1048576

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