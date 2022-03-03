[system programming lecture]

-project 4 baseline

csapp.{c,h}
        CS:APP3e functions

shellex.c
        Simple shell example

README
시스템 프로그래밍
프로젝트 #4
Phase 3 : run processes in background in your shell
사용자에게 입력 받은 명령을 child process를 사용하여 수행하는 쉘이다.
파이프 기호를 사용하여 pipeline 명령을 처리할 수 있는 쉘이다.
명령을 백그라운드에서 수행할 수 있고 job control 관련 명령을 처리할 수 있는 쉘이다.

20181668 이예진

1. 컴파일 방법
make 명령어를 입력한다.
아래의 파일이 모두 같은 폴더에 있어야 한다.
myshell.c
csapp.h
csapp.c
Makefile

2. 실행 방법
컴파일을 완료한 다음 ./myshell 를 입력하면 프로그램이 실행된다.
프로그램이 실행되면 > 이 화면에 출력된다.
원하는 명령어를 입력하면 사용이 가능하다.
아래의 명령어가 사용이 가능하다.

cd : 쉘에 존재하는 디렉터리 탐색
ls : 디렉터리 내용물 출력
mkdir : 디렉터리 생성
rmdir : 디렉터리 삭제
touch : 파일 생성
cat : 파일 내용 읽기
echo : 파일 내용 출력
exit : 쉘 종료
quit : 쉘 종료

Following shell commands with piping can be evaluated, e.g.,
ls -al | grep filename
cat filename | less
cat filename | grep -v “abc” | sort – r

Any command from project phase II can be given with ‘&’ at the end of commandline
ls -al | grep filename &
cat sample | grep -v a &
jobs: List the running and stopped background jobs.
bg <job>: Change a stopped background job to a running background job.
fg <job>: Change a stopped or running background job to a running in the foreground.
kill <job>: job에게 신호를 보낸다. 주로 종료 신호이다.

echo는 실제 쉘과 최대한 비슷하게 구현하려 했으며
phase 3에서는 파이프 사용이 있으므로 echo `pwd` 와 같은 명령어는 파이프의 방식을 사용하여 구현했다.

'' 같은 경우에는 ''로 둘러싸인 모든 특수문자 의미를 제거하도록 했다.
"" 같은 경우에는 ',",!,/,$ 를 제외한 특수문자 의미를 제거하도록 했다.

시그널이 발생하면 그룹에 있는 모든 프로세스에 시그널을 전달하기 위해서 setpgid를 사용했다.
less의 경우에는 권한이 있어야 수행할 수 있는 명령이므로 lflag로 less가 있는지 확인하여
less가 있는 경우에는 setpgid를 사용하지 않도록 했다.

kill 의 경우에는 기존 리눅스 쉘에서와 같이 kill -9 pid 형태로도 사용할 수 있다.
프로세스를 종료시키기 위해 job id를 사용할 수 있는 기능을 추가했다.
kill %jobid 를 입력하면 job id에 맞는 프로세스를 종료해준다.