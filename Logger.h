//        |----------------------------------| 
//        |           Scott Maciver          |
//        |                                  |
//        |                                  |
//        |     A S S I G N M E N T  2       |
//        |----------------------------------|


enum LOG_LEVEL {DEBUG, WARNING, ERROR, CRITICAL}; 

int InitializeLog();
void SetLogLevel(LOG_LEVEL level);
void Log(LOG_LEVEL level, const char *prog, const char *func, int line, const char *message);
void ExitLog(); 

