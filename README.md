# IOCP MMO Game Server

## ServerCore 개요
- Windows IOCP 기반 네트워크, 스레드, 메모리, 잡 시스템을 묶은 서버 코어 라이브러리입니다.
- 게임 서버 구현부나 더미 클라이언트(봇)는 `ServerCore` 모듈을 추가하고, [CoreService](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/CoreService.h)를 상속해 네트워크 이벤트만 구현하도록 설계했습니다.

## 멀티스레드 설계
### 1) IOCP 워커 스레드
- `IocpCore::StartWorkerThreads`에서 Network 스레드를 CPU 코어 * 1.5 수준으로 생성합니다.
- `GetQueuedCompletionStatus` 기반으로 완료 통지를 받아 Accept, Connect, Recv, Send를 처리합니다.
- 네트워크 I/O는 지정한 워커 스레드에서만 처리되도록 제한해 경합 지점을 줄였습니다.
- 관련 코드: [IocpCore.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Network/IocpCore.cpp)

### 2) Job Group 스레드
- `JobPriorityQueue`는 그룹당 1개의 스레드를 생성해 해당 그룹 작업을 처리합니다.
- 그룹 분리를 통해 락 없이도 안전한 직렬 처리를 하도록 설계하였습니다.
- 타이머성 작업, 지연 실행은 `DoAsyncAfter`로 예약 실행합니다.
- `ServerCore`를 포함한 상위 모듈에서 동적으로 그룹을 생성하고 JobGroup에 추가하도록 구성했습니다.
- 관련 코드: [JobPriorityQueue.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Job/JobPriorityQueue.cpp)

### 3) 비동기 로깅
- 서버 로그는 Job의 System 그룹으로 전달해 별도 스레드에서 처리합니다.
- 네트워크/게임 로직과 파일 I/O를 분리해 지연을 최소화했습니다.
- 관련 코드: [LogManager.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Log/LogManager.cpp)

## 소켓/세션 관리
### 1) 세션
- `SessionManager`가 접속한 모든 유저 세션을 관리합니다.
- `IocpCore::StartHeartbeatTask`가 주기적으로 하트비트 패킷을 처리해 유령 세션을 정리합니다.
- 관련 코드: [SessionManager.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Network/SessionManager.cpp)

### 2) 소켓
- 사전에 정의된 소켓 옵션으로 클라이언트 접속 정책을 설정하였습니다.
- 관련 코드: [SocketUtil.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Network/SocketUtil.cpp)

#### 소켓 옵션 요약
- `SO_LINGER` 활성화
  - TCP 4-way handshake를 생략하고 RST로 즉시 종료해 TIME_WAIT 상태를 줄임
  - 불필요한 종료 대기와 커널 Non-paged-pool 점유를 최소화
- `TCP_NODELAY` 활성화
  - Nagle 알고리즘을 비활성화해 패킷을 즉시 전송
  - 입력 반응성을 높이고 지연을 줄임

## 메모리 관리
### MemoryPool & ObjectPool
- 서버 시작 시 청크 단위로 일괄 할당 후 placement new로 메모리를 재사용합니다.
- 동적 할당에서 `malloc`, `VirtualAlloc` 호출을 줄이고 메모리 단편화를 완화했습니다.
- 모든 할당 경로가 풀을 거치므로 디버깅에 용이하도록 설계하였습니다.
- 관련 코드: [MemoryManger.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Memory/MemoryManger.cpp), [ObjectPool.h](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Memory/ObjectPool.h)

## 디버깅 편의 기능
### 1) 뮤텍스 락/데드락 탐지
- 디버그 모드에서 `LockGuard`가 락 요청 순서를 추적하고 순환을 탐지하면 즉시 크래시가 일어나도록 하였습니다.
- mutex 락을 래핑해 데드락 탐지를 삽입했고, 매크로로 손쉽게 락을 사용하도록 구성했습니다.
- 개발 단계에서 데드락을 사전에 방지해 안정성을 높였습니다.
- 관련 코드: [Lock.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Thread/Lock.cpp)

### 2) 크래시 덤프
- 에러시 크래시 덤프 파일을 생성해 장애 분석
- 관련 코드: [CrashDump.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Utils/CrashDump.cpp)

### 3) 로깅
- 디버그 모드에서는 콘솔 로그를, 운영 모드에서는 파일 I/O 전담 스레드로 시간별 로그를 기록합니다.
- 관련 코드: [LogManager.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Log/LogManager.cpp)

### 4) 메모리 오염 감지
- 모든 메모리는 MemoryPool을 거치게 설계했습니다.
- 디버그 모드에서는 경계 패턴을 삽입해 over/underflow를 사전에 감지합니다.
  - 메모리 청크 초기화 시 상하 8바이트에 패턴을 삽입
  - 16바이트 요청 시 32바이트 영역을 할당하고 패턴을 보호 영역으로 사용
  - 해제 시 경계 영역을 검사해 오염 여부 확인
- 관련 코드: [MemoryHeader.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Memory/MemoryHeader.cpp), [MemoryManger.cpp](https://github.com/VIA-VOID/Swarm-Server/blob/main/ServerCore/Memory/MemoryManger.cpp)

## 시연 영상
### 서버
- C++17, IOCP
- [서버 시연영상](https://metadium.jeondoh.synology.me/swarm-server.mp4)

### 클라이언트
- C++17, UE 5.3
- [클라이언트 시연영상](https://metadium.jeondoh.synology.me/swarm-client.mp4)
