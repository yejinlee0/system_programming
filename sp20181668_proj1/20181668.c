#include "20181668.h"

unsigned char memory[max_size]; // 메모리 정보를 저장할 배열
int memory_address; // 메모리 주소

HistNode* first; // 히스토리 링크드리스트의 시작
HistNode* last; // 히스토리 링크드리스트의 마지막

OpNode* OpTable[20]; // opcode 해시 테이블

int find_command(char* input, int flag, int token){ // 입력된 값에서 명령어를 추출하여 번호로 반환하는 함수 
	if(flag == 0 && token == 1){ // 명령어
		if((!strcmp(input,"h")) || (!strcmp(input,"help"))) return 1;
		else if((!strcmp(input,"d")) || (!strcmp(input,"dir"))) return 2;
		else if((!strcmp(input,"q")) || (!strcmp(input,"quit"))) return 3;
		else if((!strcmp(input,"hi")) || (!strcmp(input,"history"))) return 4;
		else if((!strcmp(input,"du")) || (!strcmp(input,"dump"))) return 5; // dump
		else if(!strcmp(input,"reset")) return 10;
		else if(!strcmp(input,"opcodelist")) return 12;
		else return 0; // 수행할 수 없는 잘못된 명령어인 경우
	}
	
	else if(token > 1){ // 명령어 + 인자
		if(flag == 0 && token == 2){ // 명령어 인자 1개
			if((!strcmp(input,"du")) || (!strcmp(input,"dump"))) return 6; // dump start
			else if(!strcmp(input,"opcode")) return 11;
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

int main(){
	int file = 0, i, command_case = 0,  comma_n = 0, len = 0, token = 0, gb = 0;
	int dir_flag = 0, up_flag = 0, comma_flag = 1, token_flag = 0, boundary_flag = 0, hexa_flag = 0;
	char input[100] = {"\0"}, temp[100] = {"\0"}, com[100] = {"\0"};
	char TokenArr[7][100];
	char *ptr;
	char *space = " ";
	int start, end, edit_address, value;
	HistNode* hist_tmp;
	OpNode* op_tmp;

	reset_memory(); // 메모리와 메모리 주소를 초기화하고 시작한다.
	memory_address = 0;
	
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
		}
	}


	return 0;
}
