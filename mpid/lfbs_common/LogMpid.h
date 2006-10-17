#ifndef __LOG_INCLUDE__
#define __LOG_INCLUDE__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOG_MPID
void LogMsgAvail(int end);
void LogAnyCtrl(int end);
void LogWaitSend(int end);
void LogTestSend(int end);
void LogStartRequest(int end);
void LogTestRequest(int end);
void LogReadCtrl(int end);
void LogSend(int end);
void LogRecv(int end);
void LogSHWaitSend(int end);
void LogSHWaitRecv(int end);
void LogSHMBlock(int end);

#define LOG_MSG_AVAIL(m) LogMsgAvail(m)
#define LOG_ANY_CTRL(m) LogAnyCtrl(m)
#define LOG_WAIT_SEND(m) LogWaitSend(m)
#define LOG_TEST_SEND(m) LogTestSend(m)
#define LOG_START_REQUEST(m) LogStartRequest(m)
#define LOG_TEST_REQUEST(m) LogTestRequest(m)
#define LOG_READ_CTRL(m) LogReadCtrl(m)
#define LOG_SEND(m) LogSend(m)
#define LOG_RECV(m) LogRecv(m)
#define LOG_SHM_WAIT_SEND(m) LogSHWaitSend(m)
#define LOG_SHM_WAIT_RECV(m) LogSHWaitRecv(m)
#define LOG_BLOCK(m) LogSHMBlock(m)

#else
#define LOG_MSG_AVAIL(m)
#define LOG_ANY_CTRL(m)
#define LOG_WAIT_SEND(m)
#define LOG_TEST_SEND(m)
#define LOG_START_REQUEST(m)
#define LOG_TEST_REQUEST(m)
#define LOG_READ_CTRL(m)
#define LOG_SEND(m)
#define LOG_RECV(m)
#define LOG_SHM_WAIT_SEND(m)
#define LOG_SHM_WAIT_RECV(m)
#define LOG_BLOCK(m)
#endif

#ifdef __cplusplus
}
#endif

#endif
