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
		else return 0; // 수행할 수 없는 잘못된 명령어인 경우
	}
	
	else if(token > 1){ // 명령어 + 인자
		if(flag == 0 && token == 2){ // 명령어 인자 1개
			if((!strcmp(input,"du")) || (!strcmp(input,"dump"))) return 6; // dump start
			else if(!strcmp(input,"opcode")) return 11;
			else if(!strcmp(input,"assemble")) return 13;
			else if(!strcmp(input,"type")) return 14;
			else return 0;
			
		}
		
		else{ // 명령어 인자, 인자 ... 형태
			if((!strcmp(input,"du")) || (!strcmp(input,"dump"))) return 7; // dump start, end
			else if((!strcmp(input,"e")) || (!strcmp(input,"edit"))) return 8; // edit address, value
			else if((!strcmp(input,"f")) || (!strcmp(input,"fill"))) return 9; // fill start, end, value
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

int main(){
	int file = 0, i, command_case = 0,  comma_n = 0, len = 0, token = 0, gb = 0;
	int dir_flag = 0, up_flag = 0, comma_flag = 1, token_flag = 0, boundary_flag = 0, hexa_flag = 0;
	int result_flag, remove_mid;
	char input[100] = {"\0"}, temp[100] = {"\0"}, com[100] = {"\0"};
	char TokenArr[7][100];
	char *ptr;
	char *space = " ";
	int start, end, edit_address, value;
	HistNode* hist_tmp;
	OpNode* op_tmp;
	
	SymTable = NULL;

	reset_memory(); // 메모리와 메모리 주소를 초기화하고 시작한다.
	memory_address = 0;
	
	sym_flag = 0;
	save_flag = 0;
	
	file = make_opcodetable(); // opcode 해시테이블을 생성한다.

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
	}

	return 0;
}
