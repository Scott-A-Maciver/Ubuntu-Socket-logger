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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>  

using namespace std;
 
const int PORT = XX; // REPLACE WITH DESIRED PORT NUMBER
const char IP_ADDR[] = "XX"; // REPLACE WITH SERVER IP
const char CLIENT_IP[] = "XX"; //REPLACE WITH LOGGER IP
const int BUF_LEN = 4096;
bool is_running = true;
pthread_mutex_t lock_x;
struct sockaddr_in servaddr;
socklen_t addrlen = sizeof(servaddr);
struct sockaddr_in destaddr;
    
void *recv_func(void *arg);

//graceful shut down using signal handlers
static void sigHandler(int sig)
{
	if(sig == SIGINT)
	{
		is_running = false;
	}
}

int main()
{
	sighandler_t err1 = signal(SIGINT, sigHandler);
	//UDP socket creation
	int fd, ret, len;    
    char buf[BUF_LEN];
    pthread_t tid;
	//Non-blocking socket
    fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
    if(fd<0) 
    {
        cout << "ERROR: can't create socket" << endl;
        cout << strerror(errno) << endl;
		return -1;    }
	
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    //binding IP address
    ret = inet_pton(AF_INET, IP_ADDR, &servaddr.sin_addr);
    if(ret == 0) 
    {
        cout << "ERROR: Address doesn't exist" << endl;
		cout << strerror(errno) << endl;
        close(fd);
        return -1;
    }
    cout << "socket fd: " << fd << " Bound to address " << inet_ntoa(servaddr.sin_addr) << endl;
    //binding the port
    servaddr.sin_port = htons(PORT);
    ret = bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(ret < 0) 
    {
	    cout << "ERROR: Cannot bind the socket to the local address" << endl;
	    cout << strerror(errno) << endl;
	    return -1;
    }


    //creating mutex

    pthread_mutex_init(&lock_x, NULL);
    //receive thread and pass file descriptor to it
    ret = pthread_create(&tid, NULL, recv_func, &fd);
	if (ret != 0)
	{
		cout << "ERROR: can't create receive thread" << endl;
		cout << strerror(errno) << endl;
		close(fd);
		return -1;
	}

	memset((char *)&destaddr, 0, sizeof(destaddr));
	destaddr.sin_family = AF_INET;
	ret = inet_pton(AF_INET, CLIENT_IP, &destaddr.sin_addr);
	if(ret==0){
		cout << "ERROR: no such address" << endl;
		cout << strerror(errno) << endl;
	}
	destaddr.sin_port = htons(1154);
 	char dest[32];
       sprintf(dest, "%s:%d", inet_ntoa(destaddr.sin_addr), ntohs(destaddr.sin_port));       
    int opt = -1; 
    int lvl = 0;	
    int fdin;
  	char log[] = "log"; 
    while(is_running)
    {
    	cout << "-------------MENU--------------" << endl; 
    	cout << "OPTION 1 : Set Log Filter Level" << endl;
    	cout << "OPTION 2 : Dump Log File" << endl;
    	cout << "OPTION 0 : Shut Down" << endl; 
    	cout << "-------------------------------" << endl; 
    	cout << "Please Select from the above: "; 
    	while (!(cin >> opt) && is_running) 
  				{
    				cout << "Invalid, please try again...";
    				cin.clear();
    				cin.ignore(2056, '\n');
				}
		switch(opt) 
		{
  		case 1:
  		//option 1: set log level: enter log severity / send to logger
  			//clean input
  			lvl = 0;
  			while(lvl < 1 || lvl > 4 && is_running)
  			{
  				cout << "Enter severity level filter (1 - 4): ";
  				while (!(cin >> lvl) && is_running) 
  				{
    				cout << "Invalid, please try again...";
    				cin.clear();
    				cin.ignore(2056, '\n');
				}
  			}
    		memset(buf, 0, BUF_LEN);
    		if(lvl == 1)
    		{
    			len = sprintf(buf, "Set Log Level = DEBUG");
    		}
    		else if(lvl == 2)
    		{
    			len = sprintf(buf, "Set Log Level = WARNING");
    		}
    		else if(lvl == 3)
    		{
    			len = sprintf(buf, "Set Log Level = ERROR");
    		}
    		else if(lvl == 4)
    		{
    			len = sprintf(buf, "Set Log Level = CRITICAL");
    		}
			ret = sendto(fd, buf, len, 0, (struct sockaddr *)&destaddr, addrlen);
    		break;
  		case 2:
  		//option 2: dump log file: open log file for read only / read log file + display / press any to continue
  		fdin = open(log, O_RDONLY);
    	ret = read(fdin, buf, BUF_LEN);
    	if(ret > 0)
    	{
    		cout << buf << endl; 
    	}
    	else
    	{
    		cout << "ERROR: File is empty" << endl;
    	}
    	cout << "Press Any Key to Continue..." << endl;
    	system("bash -c \"read -s -N 1 REPLY\"");
    		break;
    	case 0:
    	//option 0: shut down: receive thread via is_running / exit user menu/ join receiveive thread to itself so it dont shut down 
    		cout << "Shutting Down..." << endl;
    		is_running = false; 
    		pthread_join(tid, NULL);
    		break;
  		default:
    		cout << "INVALID OPTION" << endl;
    		break; 
		}
	}
}

void *recv_func(void *arg)
{
	u_long imode = 1; 
	int fd = *(int *)arg;
	int ret; 
	char buf[BUF_LEN];
	mode_t filePerms;
    struct sockaddr_in recvaddr;
	socklen_t addrlen = sizeof(recvaddr);       
	int fdout;
	char log[] = "log"; 
	 //thread open server log file for write only
	filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; 
	fdout = open(log, O_WRONLY | O_CREAT, filePerms);
	//run endlessly while is_running
	while(is_running)
	{
		//changes socket to non-blocking
		ioctl(fd, FIONBIO, &imode); 
			//apply mutexing to shared resources w/ revcfrom()
			pthread_mutex_lock(&lock_x);
			int len = recvfrom(fd, buf, BUF_LEN, 0, (struct sockaddr *)&recvaddr, &addrlen);
			pthread_mutex_unlock(&lock_x); 
			if(len < 0)
			{
				//if we receive nothing, sleep for one second
				sleep(1);
			}
			else
			{
				//write recvfrom() content to server log file
				ret = write(fdout, buf, strlen(buf));
			}
	}
	pthread_exit(NULL);
}
