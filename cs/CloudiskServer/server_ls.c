#include "thread_pool.h"
#include "databases.h"
#include "log.h"

extern  log_LockFn my_lock_func;
void lsCommand(task_t *task, MYSQL *conn) {
    // 打印执行 `ls` 命令的消息
    write_log("Executing ls command.", "info", NULL);

    // 从全局变量获取当前目录 ID
    int currentDirId = client_users[task->peerfd].pwd_id;

    // 初始化预处理语句
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt) {
        write_log("Failed to initialize statement", "error", NULL);
        return;
    }

    // 准备 SQL 查询语句
    const char *sql = "SELECT name FROM VirtualFileSystem WHERE parent_id =? AND tomb = 1";
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        const char *error_msg = mysql_stmt_error(stmt);
        write_log(error_msg, "error", NULL);
        mysql_stmt_close(stmt);
        return;
    }

    // 绑定参数
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));

    // 第一个参数，parent_id
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char *)&currentDirId;
    bind[0].is_null = 0;
    bind[0].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        write_log("Failed to bind parameters", "error", NULL);
        mysql_stmt_close(stmt);
        return;
    }

    // 执行预处理语句
    if (mysql_stmt_execute(stmt)) {
        const char *error_msg = mysql_stmt_error(stmt);
        write_log(error_msg, "error", NULL);
        mysql_stmt_close(stmt);
        return;
    }

    // 获取查询结果
    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        const char *error_msg = mysql_error(conn);
        write_log(error_msg, "error", NULL);
        mysql_stmt_close(stmt);
        return;
    }

    // 获取查询结果的行数
    unsigned long num_rows = mysql_num_rows(result);

    // 为文件名数组分配内存
    char **filenames = (char **)malloc(num_rows * sizeof(char *));
    if (!filenames) {
        write_log("Memory allocation failed for filenames array", "error", NULL);
        mysql_free_result(result);
        mysql_stmt_close(stmt);
        return;
    }

    // 初始化文件名数组
    memset(filenames, 0, num_rows * sizeof(char *));

    // 读取查询结果并填充文件名数组
    for (unsigned long i = 0; i < num_rows; ++i) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row) {
            filenames[i] = strdup(row[0]);
            if (!filenames[i]) {
                write_log("Memory allocation failed for filename", "error", NULL);
                // 清理已分配的内存
                for (unsigned long j = 0; j < i; ++j) {
                    free(filenames[j]);
                }
                free(filenames);
                mysql_free_result(result);
                mysql_stmt_close(stmt);
                return;
            }
        }
    }

    // 释放结果集资源
    mysql_free_result(result);

    // 将文件名数组发送到客户端
    int sockfd = task->peerfd;
    send(sockfd, &num_rows, sizeof(unsigned long), 0);  // 发送文件数量
    for (unsigned long i = 0; i < num_rows; ++i) {
        if (filenames[i]) {
            int filename_len = strlen(filenames[i]) + 1;  // +1 为了发送字符串的结束符 '\0'
            send(sockfd, &filename_len, sizeof(int), 0);  // 发送文件名长度
            send(sockfd, filenames[i], filename_len, 0);   // 发送文件名
        }
    }

    // 清理内存
    for (unsigned long i = 0; i < num_rows; ++i) {
        free(filenames[i]);
    }
    free(filenames);

    // 关闭预处理语句
    mysql_stmt_close(stmt);
}
