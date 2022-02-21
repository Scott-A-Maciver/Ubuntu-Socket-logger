//        |----------------------------------| 
//        |    Scott Maciver - 102205184     |
//        |    Skye Bragg    - 107842171     |
//        |                                  |
//        |     A S S I G N M E N T  2       |
//        |----------------------------------|

#include <arpa/inet.h>
#include <iostream>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <string> 
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h> 
#include "Logger.h"

using namespace std;

const int PORT = XX; //REPLACE WITH DESIRED PORT OF SERVER
const char IP_ADDR[] = "XX"; //REPLACE WITH SERVER IP ADDRESS
const char myipaddr[] = "XX"; //REPLACE WITH LOCAL IP ADDRESS
const int BUF_LEN = 4096;
bool is_running = true;
pthread_mutex_t lock_x;    
struct sockaddr_in servaddr;
struct sockaddr_in myaddr; 
const int myPort = XX; //REPLACE WITH DESIRED PORT OF CLIENT
socklen_t addrlen = sizeof(servaddr);
LOG_LEVEL filterLevel;
int fd; 

void *recv_func(void *arg);

int InitializeLog() 
{
	//UDP socket creation 
	int ret, len;
    char buf[BUF_LEN];
    pthread_t tid; 
	
    fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if(fd < 0) 
    {
        cout << "ERROR: can't create socket" << endl;
        cout << strerror(errno) << endl;
		return -1;
    }
	
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    ret = inet_pton(AF_INET, IP_ADDR, &servaddr.sin_addr);
    if(ret == 0) 
    {
        cout << "ERROR: Address doesn't exist" << endl;
        cout << strerror(errno) << endl;
        close(fd);
        return -1;
    }
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    myaddr.sin_port = htons(myPort);
    
    ret = bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr));
     if(ret < 0) 
     {
        cout << "ERROR: Failed to bind to local address" << endl;
        cout << strerror(errno) << endl;
        return -1;
     }
     ret = inet_pton(AF_INET, IP_ADDR, &servaddr.sin_addr);
     if(ret == 0) 
     {
    	cout << "ERROR: Address doesn't exist" << endl;
    	cout << strerror(errno) << endl;
        close(fd);
        return -1;
    }
        servaddr.sin_port = htons(PORT);
	//create mutex
	pthread_mutex_init(&lock_x, NULL);
	//start receive thread, pass file descriptor
	ret = pthread_create(&tid, NULL, recv_func, &fd);
	if (ret != 0)
	{
		cout << "ERROR: can't create receive thread" << endl;
		cout << strerror(errno) << endl;
		close(fd);
		return -1;
	}
	return 0;
}

void SetLogLevel(LOG_LEVEL level)
{
	//filter log level store in global var	
	filterLevel = level; 
}

void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message)
{
	 char buf[BUF_LEN];
	//compare severity of log to filter log severity
	if(level >= filterLevel)
	{	//create time stamp
		time_t now = time(0);
		char *dt = ctime(&now);
		memset(buf, 0, BUF_LEN);
		char levelStr[][16]={"DEBUG", "WARNING", "ERROR", "CRITICAL"};
		int len = sprintf(buf, "%s %s %s:%s:%d %s\n", dt, levelStr[level], prog, func, line, message)+1;
		buf[strlen(buf)-1]='\0';
		//apply mutexing to shared resources within log function
		pthread_mutex_lock(&lock_x);
		sendto(fd, buf, strlen(buf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
		pthread_mutex_unlock(&lock_x);
	}
}

void ExitLog()
{
	//stop receive thread via is_running and close file descriptor
	close(fd);
	is_running = false;
} 

//receive thread
void *recv_func(void *arg)
{
	u_long imode = 1; 
	int fd1 = *(int *)arg;
	int ret; 
	char buf[BUF_LEN];
	
	while(is_running)
	{
		//changes socket to non-blocking
		ioctl(fd1, FIONBIO, &imode); 
		if(ret != 0)
		{
			cout << "ERROR: ioctl socket failed" << endl;
		}
		else
		{
			pthread_mutex_lock(&lock_x);
			memset(buf, 0, BUF_LEN);
			int len = recvfrom(fd1, buf, BUF_LEN, 0, (struct sockaddr *)&servaddr, &addrlen);
			pthread_mutex_unlock(&lock_x);
			//if we receive nothing, sleep for one second
			if(len < 0)
			{
				sleep(1);
			}
			else
			{
				string tmpbuf(buf);
				LOG_LEVEL tmp; 
				if(tmpbuf.compare(0, 15, "Set Log Level =") == 0)
				{
					tmpbuf = tmpbuf.substr(16);
					if(tmpbuf == "DEBUG")
					{
						tmp = DEBUG;
					}
					else if (tmpbuf == "WARNING")
					{
						tmp = WARNING;
					}
					else if (tmpbuf == "ERROR")
					{
						tmp = ERROR;
					}
					else if(tmpbuf == "CRITICAL")
					{
						tmp = CRITICAL;
					} 
					SetLogLevel(tmp);
				}
			}
		}
	}
	pthread_exit(NULL);
}
