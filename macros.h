
#define JBR_FILE_LOG_APPEND(code)\
    if (debugMessage) {\
    debugMessage->append(code);\
    }\

#if NO_JBR_FILE_LOG
#define JBR_FILE_LOG_APPEND(code) Q_UNUSED(debugMessage);
#endif
