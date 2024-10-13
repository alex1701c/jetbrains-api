#include "jetbrains_api_debug.h" // IWYU pragma: keep

#define JBR_FILE_LOG_APPEND(code)                                                                                                                              \
    if (debugMessage) {                                                                                                                                        \
        debugMessage->append(code);                                                                                                                            \
        debugMessage->append("\n");                                                                                                                            \
    }                                                                                                                                                          \
    qDebug(JETBRAINS).noquote() << (code);
