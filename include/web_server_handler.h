#ifndef WEB_SERVER_HANDLER_H
#define WEB_SERVER_HANDLER_H

#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 启动Web服务器
 * @param base_path 文件系统挂载点
 * @return esp_err_t 执行结果
 */
esp_err_t start_web_server(const char *base_path);

/**
 * @brief 停止Web服务器
 */
void stop_web_server(void);

#ifdef __cplusplus
}
#endif

#endif // WEB_SERVER_HANDLER_H
