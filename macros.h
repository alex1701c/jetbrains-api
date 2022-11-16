#include "jetbrains_api_debug.h"

#define JBR_FILE_LOG_APPEND(code)                                                                                                                              \
    if (debugMessage) {                                                                                                                                        \
        debugMessage->append(code);                                                                                                                            \
        debugMessage->append("\n");                                                                                                                            \
    }                                                                                                                                                          \
    qDebug(JETBRAINS).noquote() << (code);
