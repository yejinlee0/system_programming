#include "20181668.h"

int find_command(char* input, int flag, int token){ // 입력된 값에서 명령어를 추출하여 번호로 반환하는 함수 
	if(flag == 0 && token == 1){ // 명령어
		if((!strcmp(input,"h")) || (!strcmp(input,"help"))) return 1;
		else if((!strcmp(input,"d")) || (!strcmp(input,"dir"))) return 2;
		else if((!strcmp(input,"q")) || (!strcmp(input,"quit"))) return 3;
		else if((!strcmp(input,"hi")) || (!strcmp(input,"history"))) return 4;
		else if((!strcmp(input,"du")) || (!strcmp(input,"dump"))) return 5; // dump
		else if(!strcmp(input,"reset")) return 10;
		else if(!strcmp(input,"opcodelist")) return 12;
		else if(!strcmp(input,"symbol")) return 15;
		else if(!strcmp(input,"bp")) return 18;
		else if(!strcmp(input,"run")) return 20;
		else return 0; // 수행할 수 없는 잘못된 명령어인 경우
	}
	
	else if(token > 1){ // 명령어 + 인자
		if(flag == 0 && token == 2){ // 명령어 인자 1개
			if((!strcmp(input,"du")) || (!strcmp(input,"dump"))) return 6; // dump start
			else if(!strcmp(input,"opcode")) return 11;
			else if(!strcmp(input,"assemble")) return 13;
			else if(!strcmp(input,"type")) return 14;
			else if(!strcmp(input,"progaddr")) return 16;
			else if(!strcmp(input,"loader")) return 17;
			else if(!strcmp(input,"bp")) return 19;
			else return 0;
			
		}
		
		else{ // 명령어 인자, 인자 ... 형태
			if((!strcmp(input,"du")) || (!strcmp(input,"dump"))) return 7; // dump start, end
			else if((!strcmp(input,"e")) || (!strcmp(input,"edit"))) return 8; // edit address, value
			else if((!strcmp(input,"f")) || (!strcmp(input,"fill"))) return 9; // fill start, end, value
			else if(!strcmp(input,"loader")) return 17;
			else return 0;
		}
		
	}
	
	return 0; // 입력 에러는 모두 0을 반환한다.
}

void print_help(){ // 명령어 리스트 출력하는 함수
	printf("h[elp]\n");
	printf("d[ir]\n");
	printf("q[uit]\n");
	printf("hi[story]\n");
	printf("du[mp] [start, end]\n");
	printf("e[dit] address, value\n");
	printf("f[ill] start, end, value\n");
	printf("reset\n");
	printf("opcode mnemonic\n");
	printf("opcodelist\n");
	printf("assemble filename\n");
	printf("type filename\n");
	printf("symbol\n");
	printf("progaddr [address]\n");
	printf("loader [object filename1] [object filename2] [object filename3]\n");
	printf("bp [address]\n");
	printf("run\n");
	return;
}

int print_dir(){ // 디렉터리 내용 출력하는 함수
	DIR* dirp = opendir(".");
	struct dirent *entry = NULL;
	struct stat type;
	int i = 0;

	if(!dirp) return 1;

	while((entry = readdir(dirp))!= NULL){
		i++;
		lstat(entry->d_name, &type);
		if(S_ISDIR(type.st_mode)) printf("\t%10s/", entry->d_name); // 디렉터리인 경우
		else if(S_IEXEC & type.st_mode) printf("\t%10s*", entry->d_name); // 실행 파일인 경우
		else printf("\t%10s", entry->d_name); // 그 이외 파일인 경우
		if(i % 4 == 0) printf("\n");
	}
	printf("\n");
	closedir(dirp);

	return 0;	
}

void insert_history(char* command){ // 히스토리 링크드리스트에 명령어를 삽입하는 함수 
	HistNode* node = (HistNode*)malloc(sizeof(HistNode));
	strcpy(node->command, command);
	node -> ptr = NULL;
	
	if(!first){ // 링크드리스트에 처음 노드를 삽입하는 경우
		first = node;
		last = node;
	}
	
	else{
		last -> ptr = node;
		last = node;
	}

	return;
}

void print_history(){ // 히스토리 링크드리스트에 있는 노드들을 출력한다.
		int index = 1;
		HistNode* temp = first;
		
		while(1){
			if(!temp) break;
			printf("%5d %s\n", index, temp->command);
			temp = temp -> ptr;
			index += 1;
		}
	return;
}

void reset_array(char* arr){ // 문자 배열의 값을 초기화해주는 함수
	int i, len = 0;
	len = strlen(arr);
	
	for(i = 0 ; i < len ; i++) arr[i] = '\0';
	
	return;
}

void reset_memory(){ // 메모리 전체를 0으로 초기화해주는 함수
	int i = 0;
	
	for(i = 0 ; i < max_size ; i++) memory[i] = 0;
	return;
}

int check_boundary(int dump_case, int* start, int* end){
	if(dump_case == 1){ // dump
		if((memory_address + 160) >= max_size){ // 메모리에 저장된 주소에서 160개 출력 시 최대 주소를 넘어가는 경우
			*start = memory_address;
			*end = max_size; // 마지막을 최대 주소로 설정한다.
			memory_address = 0; // 메모리 주소를 초기화한다.
		}
	
		else{
			*start = memory_address;
			*end = memory_address + 159;
			memory_address = *end + 1;
		}
	}
	
	else if(dump_case == 2){ // dump + start
		if((*start + 160) >= max_size){
			*end = max_size;
			memory_address = 0;
		}
	
		else{
			*end = *start + 159;
			memory_address = *end + 1;
		}
	}
	
	else{ // dump + start + end
		if(*start > *end) return 1; // 예외 발생 시 1을 반환하여 에러 처리를 한다.
		else if((*start >= 0 && *start <= max_size) && (*end >= 0 && *end <= max_size)) return 0;
		else return 1;
	}
	
	return 0;
}

int check_hexa(char* arr){ // 16진수인지 확인해주는 함수
	int i, len = strlen(arr);
	
	for(i = 0; i < len; i++){
		if(arr[i] == ',' || arr[i] == ' ' || arr[i] == '\t' || arr[i] == '\r' || arr[i] == '\n') continue;
		else if(arr[i] >= 'A' && arr[i] <= 'F') continue;
		else if(arr[i] >= 'a' && arr[i] <= 'f') continue;
		else if(arr[i] >= '0' && arr[i] <= '9') continue;

		return 0;
	}
	
	return 1;
}

void print_memory(int start, int end){ // 지정된 범위의 메모리를 출력해주는 함수
	int i, j;
	int start_line = start / 16;
	int end_line = end / 16;
	int current_addr, current_cont; // 현재 주소 위치를 저장하는 변수들이다.

	// 메모리 주소 + 내용 + 아스키 코드 로 나누어 내용을 출력한다.
	for(i = 0 ; i <= end_line - start_line ; i++){
		// 메모리 주소를 16진수로 출력한다.
		current_addr = start_line + i;
		printf("%05X ", current_addr * 16);
		
		// 메모리 내용을 16진수로 출력한다.
		for(j = 0 ; j < 16 ; j++){
			current_cont = current_addr * 16 + j;
			if((current_cont >= start) && (current_cont <= end)) printf("%02X ", memory[current_cont]);	
			else if(current_cont < start || current_cont > end) printf("   ");
		}
		
		printf("; ");
		
		// 메모리 내용을 아스키코드로 출력한다.
		for(j = 0 ; j < 16 ; j++){
			current_cont = current_addr * 16 + j;
			if(current_cont < start || current_cont > end) printf(".");
			else if(memory[current_cont] >= 32 && memory[current_cont] <= 126) printf("%c", memory[current_cont]);
			else printf(".");
		}

		printf("\n");
	}	
}

int refined_command(int token, char tokenarr[7][100]){ // 받은 입력을 정제하여 저장하는 함수
	int i, j, char_flag = 0, space = 0, comma = 0;
	int len1 = 0, len2 = 0;

	// tokenarr[1]에 입력 받은 토큰들을 이어 붙인다.
	for(i = 1; i < token - 1; i++){
		strcat(tokenarr[1], " ");
		strcat(tokenarr[1], tokenarr[i + 1]);
	}

	len1 = strlen(tokenarr[1]);

	// 콤마 없이 공백에 의해 문자가 분리된 예외는 -1을 반환해준다.
	for(i = 0 ; i < len1 ; i++){
		if(tokenarr[1][i] == ','){
			char_flag = 0;
			comma += 1;
		}
		else if(char_flag == 1){
			if(tokenarr[1][i] >= '0' && tokenarr[1][i] <= '9') return -1;
			if(tokenarr[1][i] >= 'a' && tokenarr[1][i] <= 'z') return -1;
			if(tokenarr[1][i] >= 'A' && tokenarr[1][i] <= 'Z') return -1;
			
		}
		else if(i != len1 - 1 && char_flag == 0 && (tokenarr[1][i+1] == ' ' || tokenarr[1][i+1] == '\t')){
			if(tokenarr[1][i] >= '0' && tokenarr[1][i] <= '9') char_flag = 1;
			if(tokenarr[1][i] >= 'a' && tokenarr[1][i] <= 'z') char_flag = 1;
			if(tokenarr[1][i] >= 'A' && tokenarr[1][i] <= 'Z') char_flag = 1;
		}
	}
	
	// 공백이 있는 경우 없애준다.
	for(i = 0 ; i < len1 ; i++){
		if(tokenarr[1][i] == ' ' || tokenarr[1][i] == '\t'){
			for(j = i ; j < len1 ; j++) tokenarr[1][j] = tokenarr[1][j + 1];
			space += 1;
		}
	}
	
	// 컴마가 있는 경우 컴마 뒤에 공백을 삽입한다.
	for(i = 0 ; i < 100 ; i++){	
		if(tokenarr[1][i] == ','){
			for(j = 100 ; j > i ; j--) tokenarr[1][j] = tokenarr[1][j - 1];
			tokenarr[1][i + 1] = ' ';
		}
	}
	
	len2 = len1 - space + comma;

	tokenarr[1][len2] = '\0';
		
	return comma; // 컴마의 개수를 반환한다.
}

int hash_function(char* str){ // 입력받은 문자열에 알맞은 인덱스를 반환해주는 함수
	int result = 0, i, len = strlen(str);
	for(i = 0 ; i < len ; i++) result += str[i];
	result %= 20;
	
	return result;
}

int make_opcodetable(){ // opcode 해시테이블을 생성해주는 함수
	int i, index = 0;
	FILE* opfile = fopen("opcode.txt", "rt");
	if(!opfile) return 0; // 파일관련 에러는 0을 반환한다.
	
	OpNode* node;
	node = (OpNode*)malloc(sizeof(OpNode));

	// 노드들이 삽입될 테이블을 초기화한다.
	for(i = 0 ; i < 20 ; i++) OpTable[i] = NULL;
	
	// 파일에서 내용을 읽어서 저장한다.
	while(fscanf(opfile, "%x %s %s", &node -> code, node -> mnemonic, node -> type) != EOF){
		index = hash_function(node->mnemonic);

		if(OpTable[index] == NULL){
			OpTable[index] = (OpNode*)malloc(sizeof(OpNode));
			OpTable[index] -> code = node -> code;
			strcpy(OpTable[index] -> mnemonic, node -> mnemonic);
			strcpy(OpTable[index] -> type, node -> type);
			OpTable[index] -> ptr = NULL;
		}

		else{
			OpNode* temp;
			temp = (OpNode*)malloc(sizeof(OpNode));
			
			temp -> code = node -> code;
			strcpy(temp -> mnemonic, node -> mnemonic);
			strcpy(temp -> type, node -> type);
			temp -> ptr = OpTable[index] -> ptr;
			OpTable[index] -> ptr = temp;
		}
	}
	
	return 1;	
}

int find_opcode(char* str){ // opcode 테이블에서 opcode를 찾아주는 함수
	int i, len = strlen(str);
	
	// 입력이 대문자가 아닌 경우 0을 반환한다.
	for(i = 0 ; i < len ; i++){
		if(str[i] >= 'A' && str[i] <= 'Z') continue;
		printf("Error : The mnemonic can only be capitalized.\n");
		
		return 0;
	}
	
	// 해시 함수에서 인덱스를 가져온다.
	int index = hash_function(str), flag = 0;
	OpNode* temp;

	temp = OpTable[index];

	// 알맞은 opcode를 출력해준다.
	while(temp){
		if(!strcmp(temp -> mnemonic, str)){
			printf("opcode is %02X\n", temp -> code);
			flag = 1;
			
			break;
		}
		
		temp = temp -> ptr;
	}
	
	// 일치하는 것이 없는 경우에는 0을 반환한다.
	if(flag == 0){
		printf("Error : %s does not exist in opcodelist.\n", str);
		
		return 0;
	}
	
	return 1;
}

void print_opcodelist(){ // opcodelist를 출력해주는 함수
	int i;
	OpNode* node;
	
	// opcode 해시테이블을 출력한다.
	for(i = 0 ; i < 20 ; i++){
		node = OpTable[i];
		printf("   %2d : ", i);

		if(!node) printf(" ");
		
		while(node){
			printf("[%s,%02X]", node -> mnemonic, node -> code);
			if(node -> ptr) printf(" -> ");
			node = node -> ptr;
		}
		
		printf("\n");
	}
	
	return;	
}

int print_type(char* filename){ // 입력받은 이름을 가진 파일의 내용을 출력하는 함수
	FILE *fp = fopen(filename, "rt");
	char temp[500];
	// 디렉터리에 파일이 존재하지 않는 경우 에러 메세지를 출력한다.
	if(!fp){
		printf("There is no %s file\n", filename);
		return 0;
	}
	// 파일이 존재하면 파일의 내용을 출력한다.
	while(fgets(temp, sizeof(temp), fp)) printf("%s", temp);

	fclose(fp);
	return 1;
}

int print_symtab(){ // symbol table을 출력하기 위한 함수
	int i;
	SymNode* temp;
	SymNode* node = NULL;
	SymNode *ptr = NULL, *pos = NULL, *new_head = NULL;
    
	// symbol table이 존재하지 않고 저장된 table도 없는 경우 에러 메세지 출력한다.
	if(sym_flag == 0 && save_flag == 0){
		printf("Error : Symbol table is empty.\n");
		return 0;
	}
	// 어셈블에 실패한 경우 최근 성공한 symbol table을 출력한다.
	else if(sym_flag == 0 && save_flag == 1){
		for(i = 0 ; i < s_idx ; i++){
			printf("\t%s\t%04X", save_symtab[i].name, save_symtab[i].location);
			printf("\n");
		}
		return 1;
	}
	// symbol table이 존재하는 경우 출력한다.
	else{
		for(i = 0 ; i < 100 ; i++){
			node = SymTable[i];
			while(node){ // 노드가 존재하는 경우 반복문을 수행한다.
				ptr = pos = new_head;
				// 문자열의 ASCII 값을 비교하여 내림차순 정렬할 때 삽입할 위치를 찾는다.
				while(ptr && strcmp(ptr -> name, node -> name) < 0){
					pos = ptr;
					ptr = ptr -> ptr;
				}
				
				// node의 정보를 data에 복사한 다음 temp에 저장한다.
				SymNode* data = (SymNode*)malloc(sizeof(SymNode));
				data -> location = node -> location;
				strcpy(data -> name, node -> name);
				data -> ptr = node -> ptr;			
				temp = data;
				temp -> ptr = ptr;
				// node를 삽입할 위치가 내림차순으로 정렬되었을 때 head에 해당하는 위치인 경우 
				if(ptr == new_head){
					SymNode* data2 = (SymNode*)malloc(sizeof(SymNode));
					data2 -> location = node -> location;
					strcpy(data2 -> name, node -> name);
					data2 -> ptr = node -> ptr;
					
					temp = data2;
					temp -> ptr = ptr;
					new_head = temp;
				}
				// head가 아닌 경우에는 삽입할 위치 다음에 temp를 연결한다.
				else pos -> ptr = temp;
				// 다음 node를 탐색한다.
				node = node -> ptr;
			}
			ptr = new_head;
		}
	
		// 오름차순으로 정렬된 symbol table을 출력한다.
		while(ptr){
			printf("\t%s\t%04X", ptr -> name, ptr -> location);
			ptr = ptr -> ptr;
			printf("\n");
		}
		return 1;
	}
	return 0;
}

int make_symtable(SymNode* node, int line_num){ // 심볼을 입력 받아 테이블을 생성하는 함수
	int index;
	SymNode *temp, *ptr;
	// 해시 함수를 이용하여 인덱스를 찾는다.
	index = hash_function(node -> name);
	// 심볼은 숫자로 시작하면 안되므로 확인하고 에러 처리한다.
	if(node -> name[0] >= '0' && node -> name[0] <= '9'){
		printf("Error : Line %d Symbol name is invalid.\n", line_num);
		return 1;
	}
	// 비어 있다면 테이블의 head에 연결한다.
	if(!SymTable[index]){
		SymTable[index] = (SymNode*)malloc(sizeof(SymNode));
		strcpy(SymTable[index] -> name, node -> name);
		SymTable[index] -> location = node -> location;
		SymTable[index] -> ptr = NULL;
	}
	// 비어 있지 않은 경우 알맞은 위치의 링크드 리스트에 연결한다.
	else{
		ptr = SymTable[index];
		while(ptr){
			// 중복되는 심볼이 있는 경우 에러 처리한다.
			if(!strcmp(ptr -> name, node -> name)){
				printf("Error : Line %d has duplicate symbol.\n", line_num);
				return 1;
			}
			ptr = ptr->ptr;
		}
		temp = (SymNode*)malloc(sizeof(SymNode));
		strcpy(temp -> name, node -> name);
		temp -> location = node -> location;
		temp -> ptr = SymTable[index] -> ptr;
		SymTable[index] -> ptr = temp;
	}
	// 연결된 심볼의 개수를 센다.
	s_num += 1;
	
	return 0;
}

int find_symloc(char* symbol){ // 심볼 테이블에서 입력된 심볼의 주소를 반환해주는 함수
	int i, len, flag = 1, index;
	SymNode* node;
	len = strlen(symbol);
	
	// 입력받은 symbol이 상수이면 -2를 반환한다.
	for(i = 0 ; i < len ; i++){
		if(symbol[i] >= '0' && symbol[i] <= '9') continue;
		else flag = 0;
	}
	if(flag == 1) return -2;
	
	// 해시 함수로 심볼의 인덱스를 찾아 심볼 테이블의 값을 node에 저장한다.
	index = hash_function(symbol);
	node = SymTable[index];

	while(node){
		// 심볼 테이블에 존재하는 경우 심볼의 주소를 반환한다.
		if(!strcmp(node -> name, symbol)) return node->location;
		node = node -> ptr;
	}
	// 심볼 테이블에 존재하지 않는 경우 -1을 반환한다.
	return -1;
}

int register_idx(char* reg){ // 입력한 레지스터의 번호를 반환한다.
	if(!strcmp(reg, "A")) return 0;
	else if(!strcmp(reg, "X")) return 1;
	else if(!strcmp(reg, "L")) return 2;
	else if(!strcmp(reg, "B")) return 3;
	else if(!strcmp(reg, "S")) return 4;
	else if(!strcmp(reg, "T")) return 5;
	else if(!strcmp(reg, "F")) return 6;
	else if(!strcmp(reg, "PC")) return 8;
	else if(!strcmp(reg, "SW")) return 9;
	else return -1;
}

// format과 mnemonic에 맞게 주소를 계산해주는 함수
void location_counter(char* format, int* location, char mnemonic){
	if(!strcmp(format, "3/4")){
		if(mnemonic == '+'){
			*location += 4;
			strcpy(format, "4");
		}
		else{
			*location += 3;
			strcpy(format, "3");
		}
	}
	else if(!strcmp(format, "1")) *location += 1;
	else if(!strcmp(format, "2")) *location += 2;
	else if(!strcmp(format, "3")) *location += 3;
	else if(!strcmp(format, "4")) *location += 4;

}

// 입력받은 mnemonic에 알맞은 opcode를 반환하는 함수
int mnemonic_to_opcode(char* mnemonic, char* format, int* opcode){
	int index;
	char data[10];
	OpNode *temp;
	// 4형식인 경우 + 기호를 제외한다.
	if(mnemonic[0] == '+')  strcpy(data, mnemonic + 1);
	else strcpy(data, mnemonic);
	// 해시 함수에서 data에 알맞은 인덱스를 가져온다.
	index = hash_function(data);
	temp = OpTable[index];

	while(temp){
		// opcode table에서 찾은 경우 opcode 정보를 저장하고 1을 반환한다.
		if(!strcmp(data, temp -> mnemonic)){
			strcpy(format, temp -> type);
			*opcode = temp -> code;
			return 1;
		}
		temp = temp -> ptr;
	}
	
	return 0;
}

// object code를 링크드 리스트의 알맞은 위치에 삽입해주는 함수
void insert_obcode(ObNode** head, ObNode** tail, int format, int first, int mid, int end, int location, char* var, int enter_flag){
	ObNode* node = (ObNode*)malloc(sizeof(ObNode));
	// node에 메모리를 할당받고 입력 받은 정보로 초기화한다.
	node -> format = format;
	node -> first_data = first;
	node -> mid_data = mid;
	node -> end_data = end;
	node -> location = location;
	node -> ptr = NULL;
	node -> enter_flag = enter_flag;
	strcpy(node -> var, var);
	// 테이블이 비어있는 경우
	if(!(*head)){
		*head = node;
		*tail = node;
	}
	// 비어있지 않은 경우
	else{
		(*tail) -> ptr = node;
		*tail = node;
	}
}

int assemble_file(char* filename){ // 입력받은 파일을 어셈블해주는 함수
	FILE *fp, *itm_file;
	FILE *lst, *obj;
	char* lstname;
	char* objname;
	char temp[100];
	char operator[10], operand1[255], operand2[10], operand3[13], format[5];
	char opcodetmp[10], name[10];
	char space = ' ', pu = '.';
	int i, cnt = 0, base_flag = 0, remove_flag, num, len;
	int opcode, reg1, reg2, disp;
	int pc = 0, base = 0, var;
	int first_data = 0, mid_data = 0, symbol_flag = 0, end_flag = 0, start_flag = 0, error_flag = 0;
	int line_num = 5, location = 0, length;	
	int start_addr;
	OpNode* check_m;
	SymNode* node;
	ObNode *head = NULL, *tail = NULL;
	ObNode *ptr1, *ptr2, *ptr3;
	
	SymTable = (SymNode**)malloc(sizeof(SymNode*) * 100);
	// 파일이 .asm 파일이 아닌 경우 0을 반환한다.
	if(strcmp(filename + strlen(filename) - 4, ".asm")){
		printf("Error : File %s is not .asm file.\n", filename);
		sym_flag = 0;
		return 0;
	}

	// 파일이 존재하지 않는 경우 0을 반환한다.
	fp = fopen(filename, "rt");
	if(!fp){
		printf("Error : File %s does not exist.\n", filename);
		sym_flag = 0;
		return 0;
	}

	// Pass 1
	node = (SymNode*)malloc(sizeof(SymNode));
	
	s_num = 0;

	for(i = 0 ; i < 100 ; i++) SymTable[i] = NULL;
	// intermediate 파일을 생성한다.
	itm_file = fopen("intermediate.txt", "wt");
	
	sym_flag = 0;
	remove_flag = 0;

	while(fgets(temp, sizeof(temp), fp)){
		if(error_flag == 1) return 0;
		cnt += 1;
		
		// 변수들을 초기화한다.
		strcpy(operator, "");
		strcpy(operand1, "");
		strcpy(operand2, "");
		strcpy(format, "");
		strcpy(opcodetmp, "");
		strcpy(node->name, "");
		
		
		// 엔터는 무시하고, 주석은 형식에 맞게 intermediate 파일에 적는다.
		if(temp[0] == '\n') continue;
		
		else if(temp[0] == '.'){
			num = sscanf(temp, "%s %s", operator, operand1);
			if(num == 1) fprintf(itm_file, "=%-4d\t%4c\t%s",line_num, space, temp);
			else{
				for(i = 1 ; i < strlen(temp) ; i++){
					if(temp[i] != ' ' && temp[i] != '\t') break;
				}
				fprintf(itm_file, "=%-4d\t%4c\t%-8c\t%-8s", line_num, space, pu, temp + i);
			}
			line_num += 5;
			continue;
		}
		

		// 심볼이 존재하지 않는 경우
		else if(temp[0] == ' ' || temp[0] == '\t'){
			num = sscanf(temp, "%s %s %s", operator, operand1, operand2);
			// 토큰은 2개이지만 operand에 ,가 있는 경우 토큰을 조정한다.
			if(num == 2 && strchr(operand1, ',')){
				strcpy(operand2, strchr(operand1, ',') + 1);
				for(i = 0; i < strlen(operand1); i++)
					if(operand1[i] == ','){
						operand1[i + 1] = '\0';
						break;
					}
				num = 3;
			}
			// , 주변 공백 관련해서 입력을 정제해준다.
			else if(num == 3 && operand2[0] == ','){
				strcpy(operand3, "");
				num = sscanf(temp, "%s %s %s %s", operator, operand1, operand2, operand3);
				// operator operand1 , operand2 인 경우
				if(num == 4){
					strcpy(operand2, "");
					strcpy(operand2, operand3);
					num = strlen(operand1);
					operand1[num] = ',';
					operand1[num + 1] = '\0';
					num = 3;
				}
				// operator operand1 ,operand2 인 경우
				else if(num == 3){
					strcpy(operand3, "");
					strcpy(operand3, operand2);
					strcpy(operand2, "");
					strcpy(operand2, strchr(operand3, ',') + 1);
					num = strlen(operand1);
					operand1[num] = ',';
					operand1[num + 1] = '\0';
					num = 3;
				}
			}
		}

		// 심볼이 존재하는 경우
		else{
			num = sscanf(temp, "%s %s %s %s", node->name, operator, operand1, operand2);
			// 심볼 이름이 opcode의 mnemonic과 같은 경우에는 에러 메세지를 출력한다.
			check_m = OpTable[hash_function(node -> name)];
			while(check_m){
				if(!strcmp(check_m -> mnemonic, node -> name)){
					line_num += 5;
					printf("Error : Line %d Symbol name can not be opcode mnemonic.\n", line_num);
					return 0;
				}
				check_m = check_m -> ptr;
			}
			// BYTE의 경우 선언에 공백이 삽입되었으면 공백까지 포함하여 입력값을 조정한다.
			if(!strcmp(operator, "BYTE") && operand1[0] == 'C' && num == 4){
				sscanf(temp, "%s %s %[^\n]s", node->name, operator, operand1);
				num = 3;
			}
			// 토큰은 3개이지만 operand에 ,가 있는 경우 토큰을 조정한다.
			if(num == 3 && strchr(operand1, ',')){
				strcpy(operand2, strchr(operand1, ',') + 1);
				for(i = 0; i < strlen(operand1); i++)
					if(operand1[i] == ','){
						operand1[i + 1] = '\0';
						break;
					}
				num = 4;
			}
			// , 주변 공백 관련해서 입력을 정제해준다.
			else if(num == 4 && operand2[0] == ','){
				strcpy(operand3, "");
				num = sscanf(temp, "%s %s %s %s %s", node->name, operator, operand1, operand2, operand3);
				// operator operand1 , operand2 인 경우
				if(num == 5){
					strcpy(operand2, "");
					strcpy(operand2, operand3);
					num = strlen(operand1);
					operand1[num] = ',';
					operand1[num + 1] = '\0';
					num = 4;
				}
				// operator operand1 ,operand2 인 경우
				else if(num == 4){
					strcpy(operand3, "");
					strcpy(operand3, operand2);
					strcpy(operand2, "");
					strcpy(operand2, strchr(operand3, ',') + 1);
					num = strlen(operand1);
					operand1[num] = ',';
					operand1[num + 1] = '\0';
					num = 4;
				}
			}
		}
		
		// operator가 START
		if(strcmp(operator, "START") == 0){
			// 시작 주소를 저장한다.
			if(operand1[0] != '\0') sscanf(operand1, "%X", &location);
			else location = 0;
			start_addr = location;
			fprintf(itm_file, "=%-4d\t%04X\t%-8s\t%-8s\t%-15X\nS%s\n", line_num, location, node->name, operator, location, node->name);
			line_num += 5;
			// 프로그램 이름이 숫자로 시작하면 에러 메세지를 출력한다.
			if(num == 3 && node -> name[0] >= '0' && node -> name[0] <= '9'){
				printf("Error : Line %d symbol name is invalid.\n", line_num);
				return 0;
			}            
			continue;
		}
		// END
		else if(strcmp(operator, "END") == 0){
			fprintf(itm_file, "=%-4d\t%4c\t%-8s\t%-8s\t%-15s\nE\n", line_num, space, node->name, operator, operand1);
			line_num += 5;
			break;
		}
		// BYTE
		else if(strcmp(operator, "BYTE") == 0){
			node->location = location;
			len = strlen(operand1);
			switch(operand1[0]){
				// char인 경우
				case 'C':
					fprintf(itm_file, "%-4d\t%04X\t%-8s\t%-8s\t%-15s\n!", line_num, location, node->name, operator, operand1);
					for(i = 2; i < len - 1; i++) fprintf(itm_file, "%X", operand1[i]);
					fprintf(itm_file, "\n");
					error_flag = make_symtable(node, line_num);
					location = location + len - 3;
					node -> location = location;
					break;
				// hexa인 경우
				case 'X':
					// BYTE 단위이므로 짝수개가 아닌 경우 에러 메세지를 출력한다.
					if((len - 3) % 2){
						printf("Error : Line %d Hexa data is invalid.\n", line_num);
						return 0;
					}			
					fprintf(itm_file, "%-4d\t%04X\t%-8s\t%-8s\t%-15s\n!", line_num, location, node->name, operator, operand1);
					// 올바른 16진수 형태가 아닌 경우 에러 메세지를 출력한다.
					for(i = 2; i < len - 1; i++){	
						if(operand1[i] >= 'A' && operand1[i] <= 'F');
						else if(operand1[i] >= 'a' && operand1[i] <= 'f');
						else if(operand1[i] >= '0' && operand1[i] <= '9');
						else {
							printf("Error : LINE %d Hexa data is invalid.\n", line_num);
							return 0;
						}
						fprintf(itm_file, "%c", operand1[i]);
					}
					fprintf(itm_file, "\n");
					error_flag = make_symtable(node, line_num);
					location = location + (strlen(operand1) - 3) / 2;
					node->location = location;
					break;
				default:
					break;
			}
			line_num += 5;
			continue;
		}
		// WORD
		else if(strcmp(operator, "WORD") == 0){
			fprintf(itm_file, "%-4d\t%04X\t%-8s\t%-8s\t%-15s\n!", line_num, location, node->name, operator, operand1);
			sscanf(operand1, "%d", &var);
			// 입력받은 값의 범위를 확인한다.
			if(var >= min_val && var <= max_val){
				if(var <= -1 && var >= min_val){
					var += 0x1000000;
				}
			}
			else {
				printf("Error : Line %d WORD value overflow.\n", line_num);
				return 0;
			}
			fprintf(itm_file, "%06X\n", var);
			error_flag = make_symtable(node, line_num);
			location = location + 3;
			node -> location = location;
			line_num += 5;
			continue;
		}
		// RESB
		else if(strcmp(operator, "RESB") == 0){
			sscanf(operand1, "%d", &var);
			fprintf(itm_file, "=%-4d\t%04X\t%-8s\t%-8s\t%-15s\n~\n", line_num, location, node->name, operator, operand1);
			error_flag = make_symtable(node, line_num);
			location = location + var;
			node->location = location;
			line_num += 5;
			continue;
		}
		// RESW
		else if(strcmp(operator, "RESW") == 0){
			sscanf(operand1, "%d", &var);
			fprintf(itm_file, "=%-4d\t%04X\t%-8s\t%-8s\t%-15s\n~\n", line_num, location, node->name, operator, operand1);
			error_flag = make_symtable(node, line_num);
			location = location + 3 * var;
			node->location = location;
			line_num += 5;
			continue;
		}
		// BASE
		else if(strcmp(operator, "BASE") == 0){
			fprintf(itm_file, "=%-4d\t%4c\t%-8s\t%-8s\t%-15s\nB%s\n", line_num, space, node->name, operator, operand1, operand1);
			line_num += 5;
			continue;
		}
		// NOBASE
		else if(strcmp(operator, "NOBASE") == 0){
			fprintf(itm_file, "=%-4d\t%4c\t%-8s\t%-8s\t\nN\n", line_num, space, node->name, operator);
			line_num += 5;
			continue;
		}
		// directive와 테이블에 존재하는 opcode가 아닌 경우 에러 메세지를 출력한다.
		else{
			remove_flag = mnemonic_to_opcode(operator, format, &opcode);
			if(remove_flag == 0){
				printf("Error : Line %d %s is not directive or valid opcode.\n", line_num, operator);
				return 0;
			}
		}
		// directive 아니지만 valid opcode인 경우
		node -> location = location;
		if(strcmp(node -> name, "")) error_flag = make_symtable(node, line_num);

		location_counter(format, &location, operator[0]);
		strcpy(opcodetmp, operand1);

		if(strcmp(operand2, "")){
			opcodetmp[strlen(opcodetmp) - 1] = '\0';
			strcat(opcodetmp, "\t");
			strcat(opcodetmp, operand2);
			strcat(operand1, " ");
			strcat(operand1, operand2);
		}
		// 필요한 데이터를 모두 입력한다.
		fprintf(itm_file, "%-4d\t%04X\t%-8s\t%-8s\t%-15s\n\t%02X\t%2s\t%8s\t%8s\n", line_num, node->location, node->name, operator, operand1, opcode, format, operator, opcodetmp);
		line_num += 5;
	}
	// 프로그램 길이를 저장한다.
	length = location - start_addr;
	fclose(itm_file);

	// Pass 2
	// lst 파일
	lstname = (char*)malloc(sizeof(char) * strlen(filename));
	strcpy(lstname, filename);
	lstname[strlen(filename) - 4] = '\0';
	lst = fopen(strcat(lstname, ".lst"), "wt");

	itm_file = fopen("intermediate.txt", "rt");
	// intermediate 파일에서 한줄씩 읽어서 list 파일과 object 코드를 작성한다.
	while(fgets(temp, sizeof(temp), itm_file)){
		if(temp[strlen(temp) - 1] == '\n') temp[strlen(temp) - 1 ] = '\0';      
		if(temp[0] == '\t'){
			num = sscanf(temp, " %x %s %s %s %s ", &opcode, format, operator, operand1, operand2);
			// operand가 없는 경우
			if(num == 3){
				// RSUB
				if(strcmp(operator, "RSUB") == 0){
					fprintf(lst, "%-8s\n", "4F0000");
					insert_obcode(&head, &tail, 3, 0x4f, 0, 0, location, "", 0);
				}
				// 1형식 - opcode
				else if(strcmp(format, "1") == 0){
					pc = location + 1;
					fprintf(lst, "%02X\n", opcode);
					insert_obcode(&head, &tail, 1, opcode, 0, 0, location, "", 0);
				}
				// 에러인 경우 리스트 파일을 삭제하고 0을 반환한다.
				else{
					printf("Error : Line %d Operand is invalid.\n", line_num);
					fclose(lst);
					remove_flag = remove(lstname);
					free(lstname);
					return 0;
				}
			}
			// operand가 있는 경우
			else if(num >= 4){
				// 2형식 - opcode r1 r2
				if(strcmp(format, "2") == 0){
					pc = location + 2;    
					// register 1개 필요한 2형식
					if(num == 4){
						// 2형식 operator 이면서 레지스터가 1개 필요한 니모닉이 아닌 경우 에러 처리를 한다.
						if(strcmp(operator, "CLEAR") && strcmp(operator, "SVC") && strcmp(operator, "TIXR")){
							printf("Error : Line %d Format2 operand is invalid.\n", line_num);
							fclose(lst);
							remove_flag = remove(lstname);
							free(lstname);
							return 0;
						}
						if(!strcmp(operator, "SVC")) sscanf(operand1, "%d", &reg1);
						else reg1 = register_idx(operand1);
						// 알맞은 레지스터가 없는 경우 에러 처리를 한다.
						if(reg1 == -1){
							printf("Error : Line %d Register is invalid.\n", line_num);
							fclose(lst);
							remove_flag = remove(lstname);
							free(lstname);
							return 0;
						}
						fprintf(lst, "%02X%01X0\n", opcode, reg1);
						insert_obcode(&head, &tail, 2, opcode, reg1*16, 0, location, "", 0);
					}
					// register가 2개 필요한 2형식
					else if(num == 5){
						// 레지스터가 2개 필요한 니모닉이 아닌 경우 에러 처리를 한다.
						if(strcmp(operator, "CLEAR") == 0 || strcmp(operator, "SVC") == 0 || strcmp(operator, "TIXR") == 0 ){
							printf("Error : Line %d Format2 operand is invalid.\n", line_num);
							fclose(lst);
							remove_flag = remove(lstname);
							free(lstname);
							return 0;
						}

						if(strcmp(operator, "SHIFTL") == 0 || strcmp(operator, "SHIFTR") == 0){
							reg1 = register_idx(operand1);
							sscanf(operand2, "%d", &reg2);
						}
						else{
							reg1 = register_idx(operand1);
							reg2 = register_idx(operand2);
						}
						// 알맞은 레지스터가 아닌 경우 에러 처리를 한다.
						if(reg1 == -1 || reg2 == -1){
							printf("Error : Line %d Register is invalid.\n", line_num);
							fclose(lst);
							remove_flag = remove(lstname);
							free(lstname);
							return 0;
						}
						fprintf(lst, "%02X%01X%01X\n", opcode, reg1, reg2);
						insert_obcode(&head, &tail, 2, opcode, reg1*16 + reg2, 0, location, "", 0);
					}
				}
				// 3형식 4형식 opcode nixbpe disp
				else if(strcmp(format, "3") == 0 || strcmp(format, "4") == 0){
					if(strcmp(format, "3") == 0) pc = location + 3;
					else if(strcmp(format, "4") == 0) pc = location + 4;
					// immediate addressing
					// n = 0 i = 1 이므로 opcode에 1을 더하고 operand 앞 기호 제거한다.
					if(operand1[0] == '#'){
						first_data = opcode + 1;
						strcpy(operand1, operand1 + 1);
					}
					// indirect addressing
					// n = 1 i = 0 이므로 opcode에 2를 더하고 operand 앞 기호 제거한다.
					else if(operand1[0] == '@'){
						first_data = opcode + 2;
						strcpy(operand1, operand1 + 1);
					}
					// simple addressing
					// n = 1 i = 1 이므로 opcode에 3을 더한다.
					else first_data = opcode + 3;	
					// operand1의 symbol location값을 찾아온다.
					symbol_flag = find_symloc(operand1);
					// 알맞은 주소값을 찾지 못한 경우 에러 메세지를 출력한다.
					if(symbol_flag == -1){
						printf("Error : Line %d Symbol is invalid.\n", line_num);
						fclose(lst);
						remove_flag = remove(lstname);
						free(lstname);
						return 0;
					}
					// operand의 symbol이 상수인 경우
					else if(symbol_flag == -2){
						sscanf(operand1, "%d", &disp);
						// 3형식인 경우
						if(strcmp(format, "3") == 0){
							// overflow인 경우 에러 메세지를 출력한다.
							if(disp < 0 || disp >= 0x1000){
								printf("Error : Line %d disp value is overflow.\n", line_num);
								fclose(lst);
								remove_flag = remove(lstname);
								free(lstname);
								return 0;
							}
						}
						// 4형식인 경우
						else if(strcmp(format, "4") == 0){
							// overflow인 경우 에러 메세지를 출력한다.
							if(disp < 0 || disp >= 0x100000){
								printf("Error : Line %d disp value is overflow.\n", line_num);
								fclose(lst);
								remove_flag = remove(lstname);
								free(lstname);
								return 0;
							}
							mid_data = 1;
						}
					}				
					// 알맞은 주소값이 반환된 경우
					else{
						// 3형식인 경우
						if(strcmp(format, "3") == 0){
							disp = symbol_flag - pc;
							// 주소가 알맞은 16진수 범위 내에 있는 경우
							if(disp >= -0x800 && disp <= 0x7FF){
								// 음수의 값을 가지는 경우
								if(disp >= -0x800 && disp <= -1){
									disp += 0x1000;
								}
								mid_data = 2;
							}
							else{
								// BASE가 없었던 경우 에러 메세지를 출력한다.
								if(!base_flag){
									printf("Error : Line %d Base error or disp is invalid.\n",line_num);
									fclose(lst);
									remove_flag = remove(lstname);
									free(lstname);
									return 0;
								}
								// operand 주소값을 가져와서 base relative 식으로 주소 계산한다.
								symbol_flag = find_symloc(operand1);
								disp = symbol_flag - base;
								// 주소가 알맞은 16진수 범위 내에 있는 경우
								if(disp >= 0 && disp <= 0xFFF) mid_data = 4;
								else{
									printf("Error : Line %d disp value is overflow.\n", line_num);
									fclose(lst);
									remove_flag = remove(lstname);
									free(lstname);
									return 0;
								}
							}
						}
						// 4형식인 경우
						else if(strcmp(format, "4") == 0){
							disp = symbol_flag;
							mid_data = 1;
						}
					}			
					// indexed mode
					// X 레지스터가 사용되는 경우
					if(num == 5){
						if(strcmp(operand2, "X") == 0 || strcmp(operand2, "x") == 0) mid_data = mid_data + 8;
						else{
							printf("Error : Line %d -> %s is not X register\n", line_num, operand2);
							fclose(lst);
							remove_flag = remove(lstname);
							free(lstname);
							return 0;
						}
					}
					// 3형식 lst, obj 파일 작성
					if(strcmp(format, "3") == 0){
						fprintf(lst, "%02X%01X%03X\n", first_data, mid_data, disp);
						if(symbol_flag == -2)
							insert_obcode(&head, &tail, 3, first_data, mid_data, disp, location, "#", 0);
						else
							insert_obcode(&head, &tail, 3, first_data, mid_data, disp, location, "", 0);
					}
					// 4형식 lst, obj 파일 작성
					else if(strcmp(format, "4") == 0){
						fprintf(lst, "%02X%01X%05X\n", first_data, mid_data, disp);
						if(symbol_flag == -2)
							insert_obcode(&head, &tail, 4, first_data, mid_data, disp, location, "", 0);
						else
							insert_obcode(&head, &tail, 4, first_data, mid_data, disp, location, "#", 0);
					}
					first_data = 0;
					mid_data = 0;
				}
				else{
					printf("Error : Line %d Format is invalid.\n", line_num);
					fclose(lst);
					remove_flag = remove(lstname);
					free(lstname);
					return 0;
				}
				// 니모닉이 LDB인 경우 B 레지스터의 값을 저장한다.
				if(strstr(operator, "LDB")) base = symbol_flag;
			}
		}		
		else{
			switch(temp[0]){
				// BYTE, WORD
				case '!':
					fprintf(lst, "%s\n", temp + 1);
					insert_obcode(&head, &tail, 5, 0, 0, 0, location, temp + 1, 0);
					break;
				// START
				case 'S':
					strcpy(name, temp + 1);
					start_flag = 1;				
					break;
				// objectcode 계산 불필요
				case '=':
					fprintf(lst, "%s\n", temp + 1);
					break;
				// NOBASE
				case 'N':
					base_flag = 0;
					break;
				// BASE
				case 'B':
					base_flag = 1;
					base = find_symloc(temp + 1);
					break;
				// END
				case 'E':
					end_flag = 1;
					break;
				// RESB, RESW
				case '~':
					tail -> enter_flag = 1;
					break;
				// 나머지 object code 계산
				default:
					sscanf(temp, "%d %x", &line_num, &location);
					fprintf(lst, "%s", temp);
					break;
			}
		}
	}
	// START가 없는 경우
	if(!start_flag){
		printf("Error : START does not exist.\n");
		fclose(lst);
		remove_flag = remove(lstname);
		free(lstname);
		return 0;
	}

	// END가 없는 경우
	if(!end_flag){
		printf("Error : END does not exist.\n");
		fclose(lst);
		remove_flag = remove(lstname);
		free(lstname);
		return 0;
	}

	// Obj 파일 
	ptr1 = ptr2 = head;
	ptr3 = head;
	
	objname = (char*)malloc(sizeof(char) * strlen(filename));
	strcpy(objname, filename);
	objname[strlen(filename) - 4] = '\0';
	obj = fopen(strcat(objname, ".obj"), "wt");
	
	// Header Record 작성
	fprintf(obj, "H%-6s%06X%06X\n", name, start_addr,length);
	length = 0;
	
	// Text Record 작성
	while(ptr1){
		fprintf(obj, "T%06X", ptr1 -> location);
		// 한 라인의 길이를 계산한다.
		while(ptr2){
			if(ptr2->format == 5){
				// 길이가 30을 초과하는지 확인한다.
				if(length + strlen(ptr2 -> var) / 2 > 30)
					break;
				else{
					length = length + strlen(ptr2 -> var) / 2;
					// 엔터를 해야 하는 경우
					if(ptr2 -> enter_flag){
						ptr2 = ptr2 -> ptr;
						break;
					}
				}
			}
			else{
				// 길이가 30을 초과하는지 확인한다.
				if(length + ptr2 -> format > 30)
					break;
				else{
					length = length + ptr2 -> format;
					// 엔터를 해야 하는 경우
					if(ptr2 -> enter_flag){
						ptr2 = ptr2 -> ptr;
						break;
					}
				}
			}
			ptr2 = ptr2 -> ptr;
		}
		// 계산된 길이를 적는다.
		fprintf(obj, "%02X", length);
		// 적을 내용이 있는 경우 적는다.
		while(ptr1 != ptr2){
			switch(ptr1 -> format){
				case 1:
					fprintf(obj, "%02X", ptr1->first_data);
					break;
				case 2:
					fprintf(obj, "%02X%02X", ptr1->first_data, ptr1->mid_data);
					break;
				case 3:
					fprintf(obj, "%02X%01X%03X", ptr1->first_data, ptr1->mid_data, ptr1->end_data);
					break;
				case 4:
					fprintf(obj, "%02X%01X%05X", ptr1->first_data, ptr1->mid_data, ptr1->end_data);
					break;
				case 5:
					fprintf(obj, "%s", ptr1->var);
					break;
				default:
					break;
			}
			ptr1 = ptr1->ptr;
		}
		fprintf(obj, "\n");
		length = 0;
	}

	ptr1 = head;

	// Modification Record 작성
	while(ptr1){
		if(ptr1 -> format == 4 && strcmp(ptr1 -> var, "")) fprintf(obj, "M%06X05\n", ptr1 -> location + 1);
		ptr1 = ptr1 -> ptr;
	}
	
	// End Record 작성
	while(ptr3){
		if(ptr3 -> format > 0){
			fprintf(obj, "E%06X\n", ptr3->location);
			break;
		}
		ptr3 = ptr3 -> ptr;
	}
    
	// 어셈블이 성공한 경우 다음 내용을 출력한다.
	printf("[%s], [%s]\n", lstname, objname);

	free(lstname);
	free(objname);
	lstname = NULL;
	objname = NULL;
	fclose(itm_file);
	fclose(fp);
	fclose(lst);
	fclose(obj);

	// 이미 저장된 심볼 테이블이 존재하면 메모리를 반환한다.
	if(save_flag == 1){
		free(save_symtab);
	}

	sym_flag = 1; // 심볼 테이블 생성 성공
	save_flag = 1; // 최근 성공 테이블 저장된 상태
	save_symtab = (SymNode*)malloc(sizeof(SymNode) * (s_num + 1));
	
	// 최근 성공한 심볼 테이블을 저장한다.
	SymNode* node2;
	s_idx = 0;
	for(i = 0 ; i < 100 ; i++){
		node2 = SymTable[i];
		if(!node2) continue;
		while(node2){
			strcpy(save_symtab[s_idx].name, node2 -> name);
			save_symtab[s_idx].location = node2 -> location;
			s_idx += 1;
			node2 = node2 -> ptr;
		}
	}
	// 저장된 심볼 테이블을 오름차순으로 정렬한다.
	int j;
	SymNode key;
	for(i = 1 ; i < s_idx ; i++){
		strcpy(key.name, save_symtab[i].name);
		key.location = save_symtab[i].location;
		for(j = i - 1 ; j >= 0 ; j--){
			if(strcmp(save_symtab[j].name, key.name) > 0){
				strcpy(save_symtab[j+1].name, save_symtab[j].name);
				save_symtab[j+1].location = save_symtab[j].location;
			}
			else break;
		}
		strcpy(save_symtab[j+1].name, key.name);
		save_symtab[j+1].location = key.location;
	}
	
	remove_flag = remove("intermediate.txt");
	return 1;
}

int is_hexa(char *arr){ // 입력 받은 문자열을 16진수인지 확인하는 함수
	int i, len;
	len = strlen(arr);
	// 문자열이 16진수로 이루어져 있는지 확인한다.
	for(i = 0; i < len ; i++){
		if(arr[i] >= '0' && arr[i] <= '9') continue;
		else if(arr[i] >= 'a' && arr[i] <= 'f') continue;
		else if(arr[i] >= 'A' && arr[i] <= 'F') continue;
		else return 1;
	}
	// 16진수 문자열이면 0을 반환하고 아니면 1을 반환한다.
	return 0;
}

int find_esymbol(char *name){ // External symbol table에서 심볼을 찾는 함수
	// 해시 함수로 심볼의 인덱스를 찾아 심볼 테이블의 값을 node에 저장한다.
	int index = hash_function(name);
	EsNode* node = EsymTable[index];

	while(node){
		// 심볼 테이블에 존재하는 경우 심볼의 주소를 반환한다.
		if(!strcmp(node -> name, name)) return node->address;
		node = node -> ptr;
	}
	// 심볼을 찾지 못하면 -1을 반환한다.
	return -1;
}

void make_estab(EsNode** last, char* symbol, char* length, char* address, int csaddress){ // 입력 받은 값들로 external symbol table에 노드를 삽입한다.	
	int index;
	EsNode *temp;
	// 해시 함수를 이용하여 인덱스를 찾는다.
	index = hash_function(symbol);

	// 비어 있다면 테이블의 head에 연결한다.
	if(!EsymTable[index]){
		EsymTable[index] = (EsNode*)malloc(sizeof(EsNode));
		strcpy(EsymTable[index] -> name, symbol);
		sscanf(address, "%x", &EsymTable[index]->address);
		sscanf(length, "%x", &EsymTable[index]->length);
		EsymTable[index]->address += csaddress;
		EsymTable[index] -> ptr = NULL;
		EsymTable[index]->eidx = eindex;
		*last = EsymTable[index];
	}
	// 비어 있지 않은 경우 알맞은 위치의 링크드 리스트에 연결한다.
	else{	
		temp = (EsNode*)malloc(sizeof(EsNode));
		strcpy(temp -> name, symbol);
		sscanf(address, "%x", &temp->address);
		sscanf(length, "%x", &temp->length);
		temp->address += csaddress;
		temp->eidx = eindex;
		temp -> ptr = EsymTable[index] -> ptr;
		EsymTable[index] -> ptr = temp;
		*last = EsymTable[index]->ptr;
	}
	eindex += 1;
	return;
}

int load_pass1(char TokenArr[7][100], int token, int *execaddr){ // loader pass1 수행하는 함수
	int i, j, e, cslength, csaddress, sym_cnt, dup_flag;
	char temp[100], obname[10], oblength[10], obaddress[10];
	FILE *fp;
	EsNode *fptr = NULL;

	EsymTable = (EsNode**)malloc(sizeof(EsNode*) * 20);
	for(i = 0 ; i < 20 ; i++) EsymTable[i] = NULL;
	//control section의 시작 주소는 progaddr로 초기화한다.
	csaddress = progaddr;
	// 사용할 문자열들을 초기화한다.
	strcpy(temp, "");
	strcpy(obname, "");
	strcpy(oblength, "");
	strcpy(obaddress, "");
	
	eindex = 0;

	// 입력 받은 파일의 개수만큼 반복한다.
	for(i = 1 ; i < token ; i ++){
		fp = fopen(TokenArr[i], "rt");
		// 파일 열기를 실패하면 에러 메세지를 출력한다.
		if(!fp){
			printf("Error : File %s does not exist.\n", TokenArr[i]);
			return 1;
		}
		// 파일에서 한 줄씩 읽어와서 처리를 해준다.
		while(fgets(temp, sizeof(temp), fp)){
			// H Record 인 경우
			if(temp[0] == 'H'){
				// 이름과 주소를 복사한다.
				strncpy(obname, temp + 1, 6);
				obname[6] = '\0';				
				strncpy(obaddress, temp + 7, 6);
				obaddress[6] = '\0';
				// 주소가 16진수 형태인지 확인해서 아니면 에러 메세지를 반환한다.
				e = is_hexa(obaddress);
				if(e == 1){
					printf("Error : File %s header address is invalid.\n", TokenArr[i]);
					return 1;
				}
				// 길이를 복사한다.
				strncpy(oblength, temp + 13, 6);
				oblength[6] = '\0';
				// 길이가 16진수 형태인지 확인하여 아니면 에러 메세지를 반환한다.
				e = is_hexa(oblength);
				if(e == 1){
					printf("Error : File %s length is invalid.", TokenArr[i]);
					return 1;
				}
				// control section 이름이 ESTAB에 존재하는지 확인하여 중복이면 에러 처리를 한다.
				dup_flag = find_esymbol(obname);
				if(dup_flag != -1){
					printf("Error : Control section name %s is duplicated.\n", obname);
					return 1;
				}
				// 중복이 아니면 ESTAB에 삽입한다.
				make_estab(&fptr, obname, oblength, obaddress, csaddress);
				// control section 길이에 더한다.
				cslength = fptr -> length;			
			}
			// D Record 인 경우
			else if(temp[0] == 'D'){
				// 외부 심볼의 개수를 센다.
				sym_cnt = strlen(temp + 1) / 12;
				for(j = 0 ; j < sym_cnt ; j++){
					// 이름과 주소를 복사한다.
					strncpy(obname, temp + 1 + (j * 12), 6);
					for(int q = 0 ; q < 7 ; q++){
						if(obname[q] >= 'a' && obname[q] <= 'z')continue;
						else if(obname[q] >= 'A' && obname[q] <= 'Z')continue;
						else if(obname[q] >= '0' && obname[q] <= '9')continue;
						else obname[q] = '\0';
					}
					strncpy(obaddress, temp + 7 + (j * 12), 6);
					obaddress[6] = '\0';
					// 주소가 16진수 형태인지 확인하여 아니면 에러 메세지를 반환한다.
					e = is_hexa(obaddress);
					if(e == 1){
						printf("Error : File %s address is invalid.\n", TokenArr[i]);
						return 1;
					}
					// 길이는 일단 0으로 저장한다.
					strcpy(oblength, "000000");					
					oblength[6] = '\0';
					// symbol 이름이 ESTAB에 존재하는지 확인하여 중복이면 에러 처리를 한다. 
					dup_flag = find_esymbol(obname);
					if(dup_flag != -1){
						printf("Error : External Symbol %s is duplicated.\n", obname);
						return 1;
					}
					// 중복이 아니면 ESTAB에 삽입한다.
					make_estab(&fptr, obname, oblength, obaddress, csaddress);
				}			
			}
			// 그 이외의 정상적인 레코드는 무시한다.
			else if(temp[0] == 'R' || temp[0] == 'T' || temp[0] == 'M' || temp[0] == 'E' || temp[0] == '.');
			// 레코드 이름이 유효하지 않으면 에러 메세지를 출력한다.
			else{
				printf("Error : Record is invalid.\n");
				return 1;
			}
			// 사용하는 문자열들을 초기화한다.
			strcpy(temp, "");			
			strcpy(obname, "");
			strcpy(oblength, "");
			strcpy(obaddress, "");
		}
		// control section 주소에 control section 길이를 더해준다.
		csaddress += cslength;
		fclose(fp);
	}
	// 프로그램의 전체 길이를 저장한다.
	proglength = csaddress;
	// 로드되는 주소가 메모리 최대 크기를 벗어나면 에러 메세지를 출력한다.
	if(csaddress > 0xfffff){
		printf("Error : Memory boundary is invalid.\n");
		return 1;
	}
	// pass 2를 수행한다.
	int pass_flag = load_pass2(TokenArr, token, &(*execaddr));
	// 반환값이 1이면 에러가 발생했다는 것이므로 1을 반환한다.
	if(pass_flag == 1) return 1;

	return 0;
}

int load_pass2(char TokenArr[7][100], int token, int *execaddr){
	int i, j, x, cslength, sym_cnt, len, addr, value, half, exec, spcf, refnum;
	int ref[11] = {0};
	char temp[100], obname[10], oblength[10], obaddress[10], data[2], modi[10];
	char op;
	FILE *fp;
	
	// 사용할 문자열을 초기화한다.
	strcpy(obname, "");
	strcpy(oblength, "");
	strcpy(obaddress, "");
	strcpy(temp, "");	
	// 입력 받은 파일의 개수만큼 반복한다.
	for(i = 1 ; i < token ; i++){
		// 파일을 열어 한 줄씩 처리한다.
		fp = fopen(TokenArr[i], "rt");
		while(fgets(temp, sizeof(temp), fp)){
			// H Record 인 경우
			if(temp[0] == 'H'){
				// 이름을 복사한다.
				strncpy(obname, temp + 1, 6);
				obname[6] = '\0';
				// ESTAB에서 해당 이름의 주소를 가져온다.
				addr = find_esymbol(obname);
				// ESTAB에 해당 이름이 없으면 에러 메세지를 출력한다.
				if(addr == -1){
					printf("Error : Control section name %s is not in ESTAB.\n", obname);
					return 1;
				}
				// 주소를 저장한다.
				ref[1] = addr;
			}
			// R Record 인 경우
			else if(temp[0] == 'R'){
				// external 심볼의 개수를 센다.
				len = strlen(temp + 1);
				sym_cnt = len / 8;
				if((len % 8) > 2) sym_cnt++;
				// 심볼 개수만큼 반복한다.
				for(j = 0 ; j < sym_cnt ; j++){
					// 심볼 이름을 복사한다.
					strncpy(obname, temp + 3 + (j * 8), 6);
					for(int q = 0 ; q < 7 ; q++){
						if(obname[q] >= 'a' && obname[q] <= 'z')continue;
						else if(obname[q] >= 'A' && obname[q] <= 'Z')continue;
						else if(obname[q] >= '0' && obname[q] <= '9')continue;
						else obname[q] = '\0';
					}
					len = strlen(obname);
					if(obname[len-1]=='\n'){
						for(x = len - 1 ; x < 6 ; x++) obname[x] = ' ';
						obname[x] = '\0';
					}
					// 인덱스 2부터 심볼 개수만큼 심볼의 주소를 저장한다.
					ref[j + 2] = find_esymbol(obname);
					if(ref[j + 2] == -1){
						printf("Error : External symbol %s is undefined.\n", obname);
						return 1;
					}
				}
			}
			// T Record 인 경우
			else if(temp[0] == 'T'){
				// 주소를 복사하고 16진수 형태로 저장한다.
				strncpy(obaddress, temp + 1, 6);
				obaddress[6] = '\0';
				sscanf(obaddress, "%x", &addr);
				// 길이를 복사하고 16진수 형태로 저장한다.
				strncpy(oblength, temp + 7, 2);
				oblength[2] = '\0';
				sscanf(oblength, "%x", &cslength);
				for(j = 0 ; j < cslength ; j++){
					strncpy(data, temp + 9 + (2 * j), 2);
					data[2] = '\0';
					sscanf(data, "%x", &value);
					// specified address를 구해서 csaddr에 더한 곳에 값을 저장한다.
					spcf = j + ref[1];
					memory[addr + spcf] = (unsigned char)value;
				}
			}
			// M Record 인 경우
			else if(temp[0] == 'M'){
				// 주소와 길이를 복사한다.
				strncpy(obaddress, temp + 1, 6);
				obaddress[6] = '\0';
				strncpy(oblength, temp + 7, 2);
				oblength[2] = '\0';
				// operator를 저장한다.
				op = temp[9];
				// referenced number을 저장한다.
				sscanf(temp + 10, "%d", &refnum);
				// referenced number의 boundary를 확인한다.
				if(refnum > sym_cnt + 1){
					printf("Error : Referenced number %d is invalid.\n", refnum);
					return 1;
				}
				// 주소와 길이를 16진수 형태로 저장한다.
				sscanf(obaddress, "%x", &addr);
				sscanf(oblength, "%x", &cslength);
				// 메모리에 저장되어 있던 값을 가져온다.
				sprintf(modi, "%02X%02X%02X", memory[addr + ref[1]], memory[addr + ref[1] + 1], memory[addr + ref[1] + 2]);
				sscanf(modi, "%x", &value);
				// 길이가 05인 경우 고쳐야 할 부분을 저장한다.
				if(cslength == 5)
					half = value & 0x00F00000;
				// operator에 따라서 심볼 value를 location에 더하거나 뺀다.
				if(op == '+')
					value += ref[refnum];
				else if(op == '-')
					value -= ref[refnum];
				else{
					printf("Error : Operator is invalid.\n");
					return 1;
				}
				// 길이가 05인 경우 값을 수정한다.
				if(cslength == 5){
					value = value & 0xFFFFF;
					value = half + value;
				}
				// 길이가 06인 경우 값을 수정한다.
				else if(cslength == 6)
					value = value & 0xFFFFFF;
				// 수정된 값을 저장한다.
				sprintf(modi, "%06X", value);
				// 메모리에 수정된 값을 저장한다.
				for(j = 0 ; j < 3 ; j++){
					strncpy(data, modi + (2 * j), 2);
					data[2] = '\0';
					sscanf(data, "%x", &value);
					spcf = ref[1] + j;
					memory[addr + spcf] = (unsigned char)value;
				}
			}
			// E Record 인 경우
			else if(temp[0] == 'E'){
				len = strlen(temp);
				// 첫 번째 control section인 경우
				if(len > 3){
					strncpy(data, temp + 1, 6);
					sscanf(data,"%06X", &exec);
					// 실행 가능한 주소를 저장한다.
					*execaddr = ref[1] + exec;
				}
			}
			// 이외의 정상 레코드는 무시한다.
			else if(temp[0] == 'D' || temp[0] == '.');
			else{
				printf("Error : Record is invalid.\n");
				return 1;
			}
			strcpy(obname, "");
			strcpy(oblength, "");
			strcpy(obaddress, "");
			strcpy(temp, "");				
		}
		fclose(fp);
	}
	
	// 로드가 완료되면 estab을 출력한다.
	EsNode *tempptr;
	int t = 0;
	int esflag = 0;
	printf("control\tsymbol\taddress\tlength\n");
	printf("section\tname\n");
	printf("-----------------------------------\n");
	for(j = 0 ; j < eindex ; j++){
		esflag = 0;
		for(int z = 0 ; z < 20 ; z++){
			tempptr = EsymTable[z];
			while(tempptr){
				if(tempptr -> eidx == j){
					if(tempptr -> length)
						printf("%s\t\t%04X\t%04X\n", tempptr -> name, tempptr -> address, tempptr -> length);
					else
						printf("\t%6s\t%04X\n", tempptr -> name, tempptr -> address);
					
					t += tempptr -> length;
					esflag = 1;	
					break;
				}
				tempptr = tempptr->ptr;
			}
			if(esflag == 1)break;
		}
	}

	printf("-----------------------------------\n");
	printf("\ttotal length\t%04X\n", t);

	for(i = 0 ; i < 20 ; i++){
		EsNode* follow = EsymTable[i];
		EsNode* del;
		while(follow){
			del = follow;
			follow = follow -> ptr;
			free(del);
		}
	}
	
	return 0;
}

void print_bp(){ // break point 링크드리스트를 출력하는 함수
	Bp *temp = bphead;
	printf("\t\tbreakpoint\n");
	printf("\t\t----------\n");
	while(temp){
		printf("\t\t%X\n", temp->loc);
		temp = temp -> ptr;
	}
}

void clear_bp(){ // break point 링크드리스트를 삭제하는 함수
	Bp* del;
	while(bphead){
		del = bphead;
		bphead = bphead -> ptr;
		free(del);
	}	
}

int create_bp(char* loc){ // break point 링크드리스트를 생성하는 함수
	Bp *node, *cur, *temp; 
	// 새로운 bp 노드를 만든다.
	node = (Bp*)malloc(sizeof(Bp));
	sscanf(loc, "%x", &node -> loc);
	node->ptr = NULL;
	// bp 링크드 리스트가 빈 경우와 아닌 경우를 나누어서 노드를 삽입한다.
	if(!bphead)
		bphead = node;
	else{
		// bp는 오름차순으로 저장한다.
		cur = bphead;
		while(cur && cur -> loc <= node -> loc){
			// bp 주소가 중복된 경우 에러 메세지를 반환한다.
			if(cur -> loc == node -> loc){
				printf("Error : Break point is duplicate.\n");
				return 1;
			}
			temp = cur;
			cur = cur -> ptr;
		}
		node->ptr = cur;
		if(cur == bphead)
			bphead = node;
		else temp -> ptr = node;
	}
	printf("\t   [\x1b[32mok\x1b[0m] create breakpoint %s\n", loc);
	return 0;
}

void reg_to_mem(int reg, int taddr){ // 레지스터의 값을 메모리에 저장하는 함수
	memory[taddr] = (unsigned char)((reg & 0xFF0000) / 0x010000);
	memory[taddr + 1] = (unsigned char)((reg & 0x00FF00) / 0x000100);
	memory[taddr + 2] = (unsigned char)((reg & 0x0000FF) / 0x000001);
}

void reg_operation(int op, int* cc, int taddr, int* data){ // opcode에 따라 register를 사용하여 작업하는 함수
	// COMP
	if(op == 0x28){
		if(reg[0] == (*data)) (*cc) = 0;
		else if(reg[0] > (*data)) (*cc) = 1;
		else if(reg[0] < (*data)) (*cc) = 2;
	}
	// J
	else if(op == 0x3C) reg[8] = taddr;
	// JEQ
	else if(op == 0x30 && (*cc) == 0) reg[8] = taddr;
	// JGT
	else if(op == 0x34 && (*cc) == 1) reg[8] = taddr;
	// JLT
	else if(op == 0x38 && (*cc) == 2) reg[8] = taddr;
	// JSUB
	else if(op == 0x48){
		reg[2] = reg[8];
		reg[8] = taddr;
	}
	// LDA
	else if(op == 0x00) reg[0] = (*data);
	// LDB
	else if(op == 0x68) reg[3] = (*data);
	// LDCH
	else if(op == 0x50){
		reg[0] = reg[0] & 0xFFFF00;
		(*data) = ((*data) & 0xFF0000) / 0x010000;
		reg[0] = reg[0] | (*data);
	}
	// LDL
	else if(op == 0x08) reg[2] = (*data);
	// LDS
	else if(op == 0x6C) reg[4] = (*data);
	// LDT
	else if(op == 0x74) reg[5] = (*data);
	// LDX
	else if(op == 0x04) reg[1] = (*data);
	// RD
	else if(op == 0xD8) (*cc) = 0;
	// RSUB
	else if(op == 0x4C) reg[8] = reg[2];
	// STA
	else if(op == 0x0C) reg_to_mem(reg[0], taddr);
	// STB
	else if(op == 0x78) reg_to_mem(reg[3], taddr);
	// STCH
	else if(op == 0x54) memory[taddr] = (unsigned char)(reg[0] & 0x0000FF);
	// STL
	else if(op == 0x14) reg_to_mem(reg[2], taddr);
	// STS
	else if(op == 0x7C) reg_to_mem(reg[4], taddr);
	// STSW
	else if(op == 0xE8) reg_to_mem(reg[9], taddr);
	// STT
	else if(op == 0x84) reg_to_mem(reg[5], taddr);
	// STX
	else if(op == 0x10) reg_to_mem(reg[1], taddr);
	// TD
	else if(op == 0xE0) (*cc) = 2;
	// TIX
	else if(op == 0x2C){
		reg[1] += 1;
		if(reg[1] == (*data)) (*cc) = 0;
		else if(reg[1] > (*data)) (*cc) = 1;
		else if(reg[1] < (*data)) (*cc) = 2;
	}
	// WD
	else if(op == 0xDC) (*cc) = -1;
	return;
}

int find_type(int opcode){ // opcode의 형식을 찾아주는 함수
	int i;
	OpNode* opptr;
	for(i = 0 ; i < 20 ; i++){
		opptr = OpTable[i];
		while(opptr){
			if(opptr->code == opcode){
				// opcode table에서 알맞은 형식을 찾으면 반환한다.
				if(opptr->type[0] == '1')return 1;
				else if(opptr->type[0] == '2')return 2;
				else if(opptr->type[0] == '3')return 3;
				break;
			}
			opptr = opptr -> ptr;
		}
	}
	// 알맞은 형식이 없으면 0을 반환한다.
	return 0;
}

void run_file(int* execpos, int* run_flag, int* is_bp, char* objectcode){ // run 명령어를 수행하는 함수
	int j, form, val, op;
	int startp, endp, end_flag = 0;
	int reg1, reg2, cc = -1;
	int n, i, x, b, p, e, ta, data, indirect;
	Bp* bpptr = bphead;

	// 실행할 위치를 지정해준다.
	if(*execpos == -1)*execpos = progaddr;
	// run이 처음 수행되는 것이라면 변수들을 초기화한다.
	if((*run_flag) == 0){
		for(j = 0 ; j < 10 ; j++)reg[j] = 0;
		reg[2] = proglength;
		reg[8] = *execpos;
		*run_flag = 1;
	}
	// 프로그램 길이만큼 반복한다.
	while(reg[8] < proglength){
		// bp 플래그가 0이라면 object코드를 메모리에서 가져온다.
		if(!(*is_bp)){
			sprintf(objectcode, "%02X%02X%02X", memory[reg[8]], memory[reg[8] + 1], memory[reg[8] + 2]);
			objectcode[6] = '\0';
			sscanf(objectcode, "%06X", &save_val);
		}
		// 저장되어 있던 값을 넣어준다.
		val = save_val;
		// opcode랑 시작 지점, 끝 지점을 저장한다.
		op = (val & 0xFC0000) / 0x010000;		
		startp = reg[8];
		endp = reg[8] + 2;
		// 4형식인 경우 endpoint 범위를 늘려준다.
		if((val & 0x001000) / 0x001000 == 1) endp += 1;
		// opcode의 형식을 저장한다.
		form = 0;
		form = find_type(op);
		// 0이 반환된 경우 에러 메세지를 출력한다.
		if(form == 0){
			printf("Error : Opcode is not found.\n");
			return;
		}
		// bp 플래그가 0이라면 bp가 해당 범위내에 있으면 출력해주고 run을 종료한다.
		if(!(*is_bp)){
			while(bpptr){
				if(bpptr->loc >= startp && bpptr->loc <= endp){
					printf("A : %06X  X : %06X\n", reg[0], reg[1]);
					printf("L : %06X PC : %06X\n", reg[2], reg[8]);
					printf("B : %06X  S : %06X\n", reg[3], reg[4]);
					printf("T : %06X\n", reg[5]);
					if(end_flag == 0)printf("\t    Stop at checkpoint[%X]\n", bpptr->loc);
					else printf("\t    End Program\n");
					*is_bp = 1;
					return;
				}
				bpptr = bpptr->ptr;
			}
			bpptr = bphead;
		}
		else *is_bp = 0;
		
		// 1형식인 경우
		if(form == 1) reg[8] += 1;
		// 2형식인 경우
		else if(form == 2){
			// 레지스터 정보를 가져온다.
			reg1 = (val & 0x00F000) / 0x001000;
			reg2 = (val & 0x000F00) / 0x000100;
			// ADDR
			if(op == 0x90) reg[reg2] = reg[reg1] + reg[reg2];
			// CLEAR
			else if(op == 0xB4) reg[reg1] = 0;
			// COMPR
			else if(op == 0xA0){
				if(reg[reg1] == reg[reg2]) cc = 0; // equal
				else if(reg[reg1] > reg[reg2]) cc = 1; // greater than
				else if(reg[reg1] < reg[reg2]) cc = 2; // less than
			}
			// DIVR
			else if(op == 0x9C) reg[reg2] = reg[reg2] / reg[reg1];
			// MULTR
			else if(op == 0x98) reg[reg2] = reg[reg2] * reg[reg1];
			// SUBR
			else if(op == 0x94) reg[reg2] = reg[reg2] - reg[reg1];
			// TIXR
			else if(op == 0xB8){
				reg[1]+=1;
				if(reg[1] == reg[reg1]) cc = 0; // equal
				else if(reg[1] > reg[reg1]) cc = 1; // greater than
				else if(reg[1] < reg[reg1]) cc = 2; // less than
			}			
			// 기타 명령
			else;
			// pc 레지스터 업데이트
			reg[8] += 2;
		}
		// 3, 4 형식인 경우
		else if(form == 3){
			// pc 레지스터 업데이트
			reg[8] += 3;
			// target address
			ta = val & 0x000FFF;
			// nixbpe bit 업데이트
			n = (val & 0x020000) / 0x020000;
			i = (val & 0x010000) / 0x010000;
			x = (val & 0x008000) / 0x008000;
			b = (val & 0x004000) / 0x004000;
			p = (val & 0x002000) / 0x002000;
			e = (val & 0x001000) / 0x001000;
			// 4형식인 경우		
			if(e == 1){
				form = 4;
				sprintf(objectcode, "%s%02X", objectcode, memory[reg[8]]);
				objectcode[8] = '\0';
				sscanf(objectcode, "%08X", &val);
				ta = val & 0x000FFFFF;
				reg[8] += 1;
			}
			// base relative 인 경우
			if(b == 1){
				ta += reg[3];
				ta = ta & 0xFFFFF;
			}
			// pc relative 인 경우
			else if(p == 1){
				if(form == 3){
					if((ta & 0x800) == 0x800) ta += 0xFF000;
				}
				ta += reg[8];
				ta = ta & 0xFFFFF;
			}
			// indexed 모드인 경우
			if(x == 1){
				ta += reg[1];
				ta = ta & 0xFFFFF;
			}
			// immediate addressing인 경우
			if(n == 0 && i == 1) data = ta;
			// indirect addressing인 경우
			else if(n == 1 && i == 0){
				// address가 프로그램 길이와 같은 경우 end 플래그에 1을 저장한다.
				if(ta == proglength) end_flag = 1;
				else if(ta > 0xFFFFD){
					printf("Error : Boundary is invalid.\n");
					return;				
				}
				if(end_flag == 0){
					sprintf(objectcode, "%02X%02X%02X", memory[ta], memory[ta + 1], memory[ta + 2]);
					sscanf(objectcode, "%06X", &indirect);
				}
				// address가 프로그램 길이와 같은 경우 end 플래그에 1을 저장한다.
				if(indirect == proglength) end_flag = 1;
				else if(indirect > 0xFFFFD){
					printf("Error : Boundary is invalid.\n");
					return;					
				}
				if(end_flag == 0){
					sprintf(objectcode, "%02X%02X%02X", memory[indirect], memory[indirect + 1], memory[indirect + 2]);
					sscanf(objectcode, "%06X", &data);
				}
				ta = indirect;
			}
			// simple addressing인 경우
			else if(n == 1 && i == 1){
				if(ta <= 0xFFFFD){
					sprintf(objectcode, "%02X%02X%02X", memory[ta], memory[ta + 1], memory[ta + 2]);
					sscanf(objectcode, "%06X", &data);
				}
				else{
					printf("Error : Boundary is invalid.\n");
					return;
				}
			}			
			// 연산을 수행하여 register 값을 업데이트한다.
			reg_operation(op, &cc, ta, &data);	
		}
	}
	// 모든 메모리 주소를 읽으면 end 플래그에 1을 저장한다.
	end_flag = 1;
	// pc 레지스터 값을 변경한다.
	reg[8] = progaddr + proglength;
	// 레지스터 상태를 출력한다.
	printf("A : %06X  X : %06X\n", reg[0], reg[1]);
	printf("L : %06X PC : %06X\n", reg[2], reg[8]);
	printf("B : %06X  S : %06X\n", reg[3], reg[4]);
	printf("T : %06X\n", reg[5]);
	printf("\t    End Program\n");
	*run_flag = 0;
	
	return;
}

int main(){
	int file = 0, i, command_case = 0,  comma_n = 0, len = 0, token = 0, gb = 0;
	int dir_flag = 0, up_flag = 0, comma_flag = 1, token_flag = 0, boundary_flag = 0, hexa_flag = 0;
	int result_flag, remove_mid, load_complete = 0;
	char input[100] = {"\0"}, temp[100] = {"\0"}, com[100] = {"\0"};
	char TokenArr[7][100];
	char *ptr;
	char *space = " ";
	int start, end, edit_address, value;
	int execaddr = -1, run_flag = 0, create_flag, bp_flag = 0;
	char objectcode[15];
	HistNode* hist_tmp;
	OpNode* op_tmp;
	
	SymTable = NULL;
	EsymTable = NULL;
	
	reset_memory(); // 메모리와 메모리 주소를 초기화하고 시작한다.
	memory_address = 0;
	
	sym_flag = 0;
	save_flag = 0;
	
	file = make_opcodetable(); // opcode 해시테이블을 생성한다.

	progaddr = 0;
	proglength = 0;
	bphead = NULL;
	save_val = 0;
	eindex = 0;

	for(i = 0 ; i < 10 ; i++)reg[i] = 0;
	while(1){
		// 배열과 변수들을 초기화한다.
		reset_array(input);
		reset_array(temp);
		reset_array(com);
		reset_array(TokenArr[0]);
		reset_array(TokenArr[1]);
		reset_array(TokenArr[2]);
		reset_array(TokenArr[3]);
		
		dir_flag = 0;
		up_flag = 0;
		comma_flag = 1;
		token_flag = 0;
		token = 0;
		boundary_flag = 0;
		hexa_flag = 1;
		result_flag = 0;
		comma_n = 0;
		
		// 입력을 받는다.
		printf("sicsim> ");
		fgets(input,sizeof(input),stdin);
		if(!strcmp(input,"\n")) continue; 
		
		len = strlen(input);
		input[len - 1] = '\0';

		strcpy(temp,input);
		
		char *is_comma = strchr(temp,','); // ,가 command에 있는지 확인한다.
		if(is_comma == NULL) comma_flag = 0; // comma가 없으면 comma에 0을 저장한다.
	
		ptr = strtok(temp, " \t\r\n");
		if(!ptr) continue; // 입력이 없는 경우 다시 처음으로 돌아가서 입력을 받는다.
		
		token += 1;
		strcpy(TokenArr[0],ptr); // 명령어 앞부분만 따로 저장한다.
		
		for(i = 1 ; i < 7 ; i++){
			ptr = strtok(NULL, " \t\r\n");
			if(!ptr) break;
			if(i == 6){ // 최대 토큰 개수를 벗어나는 경우
				token_flag = 1;
				break;
			}
			token += 1;
			strcpy(TokenArr[i], ptr);
		}
		
		if(token_flag == 1){
			printf("Error : Invalid input.\n");
			continue;
		}
		
		// 함수를 통해 사용자의 입력에 알맞은 번호를 반환 값으로 가져온다.
		command_case = find_command(TokenArr[0], comma_flag, token);
		// 해당하는 번호에 맞게 기능을 수행한다.
		switch(command_case){
			case 0: // input error
				printf("Error : Invalid input.\n");
				break;
			case 1: // help
				print_help();
				insert_history(TokenArr[0]);
				break;
			case 2: // dir
				dir_flag = print_dir();
				if(dir_flag == 1){
					printf("Error : Cannot open the directory.\n");
					break;
				}
				insert_history(TokenArr[0]);
				break;
			case 3: // quit
				// history 링크드리스트 메모리를 반환한다.
				hist_tmp = first;
				while(first){
					hist_tmp = first;
					first = first -> ptr;
					free(hist_tmp);
				}
				// opcode 테이블 메모리를 반환한다.
				for(i = 0 ; i < 20 ; i++){
					op_tmp = OpTable[i];
					while(OpTable[i]){
						op_tmp = OpTable[i];
						OpTable[i] = OpTable[i] -> ptr;
						free(op_tmp);
					}
				}
				// 심볼 테이블 메모리를 반환한다.
				if(sym_flag == 1){
					for(i=0;i<100;i++){
						free(SymTable[i]);
					}
					free(SymTable);
				}
				// 저장된 심볼 테이블 메모리를 반환한다.
				if(save_flag == 1) free(save_symtab);
				
				return 0;
			case 4: // history
				insert_history(TokenArr[0]);
				print_history();
				break;
			case 5: // dump
				boundary_flag = check_boundary(1,&start,&end);
				print_memory(start, end);
				insert_history(TokenArr[0]);
				break;
			case 6: // dump start
				hexa_flag = check_hexa(TokenArr[1]);
				// 16진수 형태인지 확인한다.
				if(hexa_flag == 0){
					printf("Error : Parameter is invalid hexadecimal form.\n");
					break;
				}
				sscanf(TokenArr[1], "%x", &start);
				// 알맞은 범위인지 확인한다.
				if(start > max_size){
					printf("Error : Invalid boundary.\n");
					break;
				}				
				boundary_flag = check_boundary(2,&start,&end);
				print_memory(start, end);
				// 명령어와 인자를 붙인다.
				sprintf(com,"%s%s%s",TokenArr[0],space,TokenArr[1]);
				insert_history(com);
				break;
			case 7: // dump start end
				comma_n = refined_command(token, TokenArr);
				// 함수를 통해 발견한 입력 오류이면 예외 처리 해준다.
				if(comma_n == -1){
					printf("Error : Invalid input.\n");
					break;
				}		
				
				// 컴마가 2개 이상일 시 Parameter Error
				if(comma_n != 1){
					printf("Error : Invalid parameters.\n");
					break;
				}		
				//알맞은 16진수 형태인지 확인한다.
				hexa_flag = check_hexa(TokenArr[1]);
				if(hexa_flag == 0){
					printf("Error : Parameter is invalid hexadecimal form.\n");
					break;
				}
				
				i = sscanf(TokenArr[1], "%x, %x %x", &start, &end, &gb);
				if(i != 2){
					printf("Error : Invalid parameters.\n");
					break;
				}
				// 알맞은 범위인지 확인한다.
				boundary_flag = check_boundary(3, &start, &end);
				if(boundary_flag == 1){
					printf("Error : Invalid boundary.\n");
					break;
				}
				print_memory(start,end);
				if(end == max_size) memory_address = 0;
				else memory_address = end + 1;
				// 명령어와 인자를 붙인다.
				sprintf(com,"%s%s%s",TokenArr[0],space,TokenArr[1]);
				insert_history(com);				
				break;
			case 8: // edit
				comma_n = refined_command(token, TokenArr);
				// 함수를 통해 얻은 입력 오류는 예외 처리한다.
				if(comma_n == -1){
					printf("Error : Invalid input.\n");
					break;
				}		
				if(comma_n != 1){
					printf("Error : Invalid parameters.\n");
					break;
				}
				// 알맞은 16진수 형태인지 확인힌다.
				hexa_flag = check_hexa(TokenArr[1]);
				if(hexa_flag == 0){
					printf("Error : Parameter is invalid hexadecimal form.\n");
					break;
				}
				
				i = sscanf(TokenArr[1], "%x, %x %x", &edit_address, &value, &gb);
				if(i != 2){
					printf("Error : Invalid parameters.\n");
					break;
				}
				// 알맞은 범위인지 확인한다.
				if((edit_address < 0x00000) || (edit_address > max_size)){
					printf("Error : Invalid boundary of address.\n");
					break;					
				}

				if((value < 0x00) || (value > 0xFF)){
					printf("Error : Invalid boundary of value.\n");
					break;					
				}
				// 입력받은 위치에 입력받은 값을 삽입한다.
				memory[edit_address] = value;
				sprintf(com,"%s%s%s",TokenArr[0],space,TokenArr[1]);
				insert_history(com);
				break;
			case 9: // fill
				comma_n = refined_command(token, TokenArr);
				// 함수를 통해 얻은 입력 오류는 예외 처리한다.
				if(comma_n == -1){
					printf("Error : Invalid input.\n");
					break;
				}		
				if(comma_n != 2){
					printf("Error : Invalid parameters.\n");
					break;
				}				
				// 알맞은 16진수 형태인지 확인한다.
				hexa_flag = check_hexa(TokenArr[1]);
				if(hexa_flag == 0){
					printf("Error : Parameter is invalid hexadecimal form.\n");
					break;
				}
				
				i = sscanf(TokenArr[1], "%x, %x, %x %x", &start, &end, &value, &gb);
				if(i != 3){
					printf("Error : Invalid parameters.\n");
					break;
				}
				// 알맞은 범위인지 확인한다.
				boundary_flag = check_boundary(3,&start,&end);
				if(boundary_flag == 1){
					printf("Error : Invalid boundary.\n");
					break;
				}

				if((value < 0x00) || (value > 0xFF)){
					printf("Error : Invalid boundary of value.\n");
					break;					
				}
				// 주어진 범위에 메모리를 입력받은 값으로 채운다.
				for(i = start ; i < end + 1 ; i++) memory[i] = value;
				// 명령어와 인자를 붙인다.
				sprintf(com,"%s%s%s",TokenArr[0],space,TokenArr[1]);
				insert_history(com);
				break; 
			case 10: // reset
				reset_memory();
				insert_history(TokenArr[0]);
				break;
			case 11: // opcode mnemonic
				if(file == 0){
					printf("Error : Opcode table does not exist.\n");
					break;
				}
				up_flag = find_opcode(TokenArr[1]);
				// opcode가 존재하는 경우에만 명령어와 인자를 붙여 히스토리 linked list에 삽입한다.
				if(up_flag == 1){
					sprintf(com,"%s%s%s",TokenArr[0],space,TokenArr[1]);
					insert_history(com);
				}				
				break;
			case 12: //opcodelist
				if(file == 0){
					printf("Error : opcode.txt file open error.\n");
					break;
				}
				print_opcodelist();
				insert_history(TokenArr[0]);
				break;
			case 13: //assemble
				// 입력 받은 파일을 어셈블한다.
				result_flag = assemble_file(TokenArr[1]);
				// 어셈블 실패한 경우 중간 파일이 존재한다면 삭제한다.
				if(result_flag == 0){
					remove_mid = remove("intermediate.txt");
					if(remove_mid == -1);
				}
				// symbol table이 생성되지 않았다면 할당받은 메모리를 반환한다.
				if(sym_flag == 0){
					free(SymTable);
				}
				if(result_flag == 1){
					sprintf(com,"%s%s%s",TokenArr[0],space,TokenArr[1]);
					insert_history(com);
				}
				break;
			case 14: //type
				// 입력 받은 파일을 열어 출력하고 history에 명령어를 저장한다.
				result_flag = print_type(TokenArr[1]);
				if(result_flag == 1){
					sprintf(com,"%s%s%s",TokenArr[0],space,TokenArr[1]);
					insert_history(com);
				}
				break;
			case 15: //symbol
				// symbol table을 출력하고 history에 명령어를 저장한다.
				result_flag = print_symtab();
				insert_history(TokenArr[0]);				
				break;
			default:
				break;
		}
		if(command_case == 16){ // progaddr
			int pr_flag = is_hexa(TokenArr[1]);
			if(pr_flag == 1){ // 16진수 형태가 아닌 경우
				printf("Error : Address is invalid.\n");
				continue;
			}
			// 히스토리에 저장한다.
			sscanf(TokenArr[1], "%x", &progaddr);
			sprintf(com,"%s%s%s",TokenArr[0],space,TokenArr[1]);
			insert_history(com);
		}
		else if(command_case == 17){ // loader
			int load_flag = load_pass1(TokenArr, token, &execaddr);
			if(load_flag == 1){ // 에러가 발생한 경우
				continue;
			}
			// 파일이 하나일 떄
			if(token == 2){
				sprintf(com,"%s%s%s",TokenArr[0],space,TokenArr[1]);
				insert_history(com);				
			}
			// 파일 두 개일 때
			else if(token == 3){
				sprintf(com,"%s%s%s%s%s",TokenArr[0],space,TokenArr[1],space,TokenArr[2]);
				insert_history(com);				
			}
			// 파일 세 개일 때
			else if(token == 4){
				sprintf(com,"%s%s%s%s%s%s%s",TokenArr[0],space,TokenArr[1],space,TokenArr[2],space,TokenArr[3]);
				insert_history(com);
			}
			// 로드 성공하면 변수에 1을 저장한다.
			load_complete = 1;
		}
		else if(command_case == 18){ // bp만 입력으로 들어온 경우
			// bp들을 출력한다.
			print_bp();
			insert_history(TokenArr[0]);
		}
		else if(command_case == 19){
			if(!strcmp(TokenArr[1], "clear")){ // bp + clear
				// bp들을 전체 삭제한다.
				clear_bp();
				printf("\t   [\x1b[32mok\x1b[0m] clear all breakpoints\n");
			}
			else{ // bp + 주소
				create_flag = is_hexa(TokenArr[1]);
				if(create_flag == 1){ // 주소가 16진수 형태가 아닌 경우
					printf("Error : Break point location is invalid.\n");
					continue;
				}
				// bp를 추가한다.
				create_flag = create_bp(TokenArr[1]);
				if(create_flag == 1)continue;
			}
			sprintf(com,"%s%s%s",TokenArr[0],space,TokenArr[1]);
			insert_history(com);
		}
		else if(command_case == 20){ // run
			if(load_complete == 0){ // 로드한 적이 없는 경우
				printf("Error : Loaded object file does not exist.\n");
				continue;
			}
			// run을 수행한다.
			run_file(&execaddr, &run_flag, &bp_flag, objectcode);
			insert_history(TokenArr[0]);
		}
	}

	return 0;
}
