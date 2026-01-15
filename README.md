# ICOP MMO Game Server

 

# ServerCore
- Windows IOCP 기반 네트워크/스레드/메모리/잡 시스템을 묶은 서버 코어 라이브러리
- 게임서버 구현부나 더미 클라이언트(봇)는 `ServerCore` 모듈 추가 및 [CoreService](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/CoreService.h)를 상속받아 네트워크 이벤트만 구현하면 되도록 설게하였습니다.

## 멀티스레드 설계
### 1. IOCP 워커 스레드
- `IocpCore::StartWorkerThreads`가 Network 스레드를 CPU 코어의 1.5배 수준으로 생성
- `GetQueuedCompletionStatus` 기반으로 완료통지를 받아 Accept, Connect, Recv, Send를 처리
- 네트워크 I/O는 지정한 워커 스레드에서만 처리되도록 할당하여 경합 지점을 줄임
- 관련 코드: [IocpCore.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Network/IocpCore.cpp)

### 2. Job Group 스레드
- `JobPriorityQueue`는 그룹당 1개의 스레드를 생성하여 해당 그룹 작업을 처리
- 그룹 분리를 통해 락 없이도 안전한 직렬 처리 가능
- 타이머성 작업, 지연 실행은 `DoAsyncAfter`로 예약하여 실행
- `ServerCore`를 추가한 프로젝트 구현부에서 동적으로 그룹을 생성하여, JobGroup에 추가할 수 있도록 개발
  - 동적으로 그룹을 생성하여, 그룹별로 락 경합없이 직렬적으로 빠르게 처리할 수 있도록 설계
- 관련 코드: [JobPriorityQueue.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Job/JobPriorityQueue.cpp)

### 3. 비동기 로깅
- 서버에서 발생하는 모든 로그를 Job의 System 그룹으로 보내 별도 스레드에서 처리
- 관련 코드: [LogManager.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Log/LogManager.cpp)

## 소켓, 세션 관리
### 1. 세션
- `SessionManager`가 서버에 접속한 모든 유저의 세션을 관리
- IocpCore::StartHeartbeatTask`가 주기적으로 하트비트 패킷을 처리, 유령 세션, 소켓 삭제
- 관련 코드: [SessionManager.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Network/SessionManager.cpp)

### 2. 소켓
- 설정해둔 소켓 설정들을 통해 클라이언트 접속 허용
- 관련코드: [SocketUtil.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Network/SocketUtil.cpp)

소켓 옵션
SO_LINGER 활성화
- TCP 4-way handshake를 거치지 않고 RST 패킷을 보내 즉시 종료 함으로써 TIME_WAIT 상태 생략
  - 불필요한 종료 대기로 인해 타 클라이언트의 접속 지연을 방지하고자 함
  - 불필요한 Non-paged-pool 커널 메모리 점유 방지

TCP_NODELAY 활성화
- Nagle 알고리즘을 OFF 함으로써 패킷을 즉시 전송
  - 클라이언트 패킷 즉시 수신, 캐릭터 반응성 향상

## 메모리 관리
MemoryPool & Object Pool
- 서버 시작시 청크별로 일괄 할당 후, placement new로 메모리 재사용
- 동적할당시에 malloc, virtualAlloc 호출 최소화, 메모리 단편화 방지
  - 서버에서 일어나는 모든 메모리 할당은 메모리 풀을 거치기 때문에 디버깅 용이
- 관련코드: [MemoryManger.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Memory/MemoryManger.cpp), [ObjectPool.h](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Memory/ObjectPool.h)

## 디버깅 편의
### 1. 뮤텍스 락, 데드락 탐지
- 디버그 모드에서 `LockGuard`가 락 요청 순서를 추적해 순환을 탐지하면 즉시 크래시, 스레드별 락 상태와 로그 컨텍스트를 유지
- mutex 락을 래핑하여 데드락 탐지 기능을 삽입, 외부에서 매크로로 손쉽게 사용하도록 개발
- 개발단계에서 데드락을 사전에 탐지하여, 안정적인 운영 가능
- 관련코드: [Lock.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Thread/Lock.cpp)

### 2. 크래시 파일
- 크래시 덤프 파일 생성으로 장애 분석
- 관련코드: [CrashDump.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Utils/CrashDump.cpp)

### 3. 로깅
- 디버그 모드에서 콘솔 로그, 디버그/운영 모드에서는 파일 I/O 전담 스레드를 통해 시간별 로그 파일 처리
- 관련코드: [LogManager.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Log/LogManager.cpp)

### 4. 메모리
- 서버에서 할당되는 모든 메모리는 MemoryPool을 거치게 설계
- 디버그 모드에서 메모리 동적 할당시 메모리 오염 감지를 위한 경계 패턴을 삽입하여 over & underflow를 사전에 감지하도록 설계
  - 메모리 청크 초기화시 메모리 주소 위/아래로 8바이트씩 추가로 붙여서 특정 패턴을 삽입해 둔다.
  - 16바이트를 할당요청을 받게되면 32바이트를 반환하게되며 8바이트(오염방지) + 16바이트 + 8바이트(오염방지)형태로 메모리를 할당해준다.
  - 메모리 사용 후 반납시에 위/아래로 메모리를 확인하여 over & underflow 여부를 확인한다.
- 관련코드: [MemoryHeader.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Memory/MemoryHeader.cpp), [MemoryManger.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Memory/MemoryManger.cpp)

# 시연영상
### 서버
- C++17, IOCP
- [서버 시연영상](https://metadium.jeondoh.synology.me/swarm-server.mp4)

### 클라이언트
- C++17, UE 5.3
- [클라이언트 시연영상](https://metadium.jeondoh.synology.me/swarm-client.mp4)
