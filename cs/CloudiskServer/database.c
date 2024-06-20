#include "databases.h"

// 输入用户名获取
int select_user_table(MYSQL* mysql, user_table* ptable) {
    // 初始化MYSQL_STMT
    MYSQL_STMT* stmt = mysql_stmt_init(mysql);
    if (stmt == NULL) {
        printf("(%d, %s)\n", mysql_errno(mysql), mysql_error(mysql));
        return -1;
    } 

    // 执行PREPARE操作
    const char* sql = "SELECT * FROM user WHERE name = ?";
    int ret = mysql_stmt_prepare(stmt, sql, strlen(sql));
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    // 获取占位符的个数
    int count = mysql_stmt_param_count(stmt);
    if (count != 1) {
        printf("(%d, %s)\n", mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
        return -1;
    }

    // 设置输入参数
    char name[128] = {0};
    unsigned long  name_len;

    strcpy(name, ptable->user_name);
    name_len = strlen(name);

    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));

    // 绑定参数
    bind.buffer_type = MYSQL_TYPE_VAR_STRING;
    bind.buffer = name;
    bind.is_null = 0;
    bind.length = &name_len;

    // 执行绑定操作
    ret = mysql_stmt_bind_param(stmt, &bind);
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    // 执行EXECUTE操作
    ret = mysql_stmt_execute(stmt);
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    int res_user_id;
    char res_user_name[128];
    char res_cryptpasswd[128];
    char res_pwd[128];
    int res_pwd_id;

    MYSQL_BIND res_bind[5];
    memset(res_bind, 0, sizeof(res_bind));

    // 绑定输出参数
    res_bind[0].buffer_type = MYSQL_TYPE_LONG;
    res_bind[0].buffer = &res_user_id;
    res_bind[0].buffer_length = sizeof(int);

    res_bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
    res_bind[1].buffer = res_user_name;
    res_bind[1].buffer_length = sizeof(res_user_name); // 确定值，空间要大

    res_bind[2].buffer_type = MYSQL_TYPE_VAR_STRING;
    res_bind[2].buffer = res_cryptpasswd;
    res_bind[2].buffer_length = sizeof(res_cryptpasswd); // 确定值

    res_bind[3].buffer_type = MYSQL_TYPE_VAR_STRING;
    res_bind[3].buffer = res_pwd;
    res_bind[3].buffer_length = sizeof(res_pwd); // 确定值    
    
    res_bind[4].buffer_type = MYSQL_TYPE_LONG;
    res_bind[4].buffer = &res_pwd_id;
    res_bind[4].buffer_length = sizeof(int);

    // 执行绑定操作
    ret = mysql_stmt_bind_result(stmt, res_bind);
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    // 再获取数据信息
    ret = mysql_stmt_store_result(stmt);
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    while (1) {
        int status = mysql_stmt_fetch(stmt);
        if (status == 1 || status == MYSQL_NO_DATA) {
            break;
        }
        ptable->user_id = res_user_id;
        strcpy(ptable->user_name, res_user_name);
        strcpy(ptable->cryptpasswd, res_cryptpasswd);
        strcpy(ptable->pwd, res_pwd);
        ptable->pwd_id = res_pwd_id;
    }

    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    return 0;
}

// 输入user_table中的file_id获取 
int select_file_table(MYSQL* mysql, file_table* ptable) {
    // 初始化MYSQL_STMT
    MYSQL_STMT* stmt = mysql_stmt_init(mysql);
    if (stmt == NULL) {
        printf("(%d, %s)\n", mysql_errno(mysql), mysql_error(mysql));
        return -1;
    } 

    // 执行PREPARE操作
    const char* sql = "SELECT * FROM user WHERE parent_id = ? AND filename = ? AND owner_id = ?";
    int ret = mysql_stmt_prepare(stmt, sql, strlen(sql));
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    // 获取占位符的个数
    int count = mysql_stmt_param_count(stmt);
    if (count != 3) {
        printf("(%d, %s)\n", mysql_stmt_errno(stmt), mysql_stmt_errno(stmt));
        return -1;
    }

    // 设置输入参数
    int parent_id = ptable->parent_id;

    char file_name[128] = {0};
    strcpy(file_name, ptable->file_name);
    unsigned long name_len = strlen(file_name);

    int owner_id = ptable->owner_id;


    MYSQL_BIND bind[3];
    memset(&bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &parent_id;
    bind[0].is_null = 0;//该参数不为空,设置为0；参数为空，设置为1
    bind[0].length = NULL;//整型数据不需要指定长度缓冲区
    // 绑定参数
    bind[1].buffer_type = MYSQL_TYPE_VAR_STRING;
    bind[1].buffer = file_name;
    bind[1].is_null = 0;
    bind[1].length = &name_len;

    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = &owner_id;
    bind[2].is_null = 0;
    bind[2].length = NULL;

    // 执行绑定操作
    ret = mysql_stmt_bind_param(stmt, bind);
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    // 执行EXECUTE操作
    ret = mysql_stmt_execute(stmt);
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    int res_file_id;
    int res_parent_id;
    char res_file_name[128];
    int res_owner_id;
    char res_sha1[64];
    int res_filesize;
    char res_type;
    char res_tomb;

    MYSQL_BIND res_bind[8];
    memset(res_bind, 0, sizeof(res_bind));

    // 绑定输出参数
    res_bind[0].buffer_type = MYSQL_TYPE_LONG;
    res_bind[0].buffer = &res_file_id;
    res_bind[0].buffer_length = sizeof(int);

    res_bind[1].buffer_type = MYSQL_TYPE_LONG;
    res_bind[1].buffer = &res_parent_id;
    res_bind[1].buffer_length = sizeof(int);

    res_bind[2].buffer_type = MYSQL_TYPE_VAR_STRING;
    res_bind[2].buffer = res_file_name;
    res_bind[2].buffer_length = sizeof(res_file_name); // 确定值，空间要大
    
    res_bind[3].buffer_type = MYSQL_TYPE_LONG;
    res_bind[3].buffer = &res_owner_id;
    res_bind[3].buffer_length = sizeof(int);

    res_bind[4].buffer_type = MYSQL_TYPE_VAR_STRING;
    res_bind[4].buffer = res_sha1;
    res_bind[4].buffer_length = sizeof(res_sha1); // 确定值

    res_bind[5].buffer_type = MYSQL_TYPE_LONG;
    res_bind[5].buffer = &res_filesize;
    res_bind[5].buffer_length = sizeof(int);   

    res_bind[6].buffer_type = MYSQL_TYPE_STRING;
    res_bind[6].buffer = &res_type;
    res_bind[6].buffer_length = sizeof(char);

    res_bind[7].buffer_type = MYSQL_TYPE_STRING;
    res_bind[7].buffer = &res_tomb;
    res_bind[7].buffer_length = sizeof(char);

    // 执行绑定操作
    ret = mysql_stmt_bind_result(stmt, res_bind);
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    // 再获取数据信息
    ret = mysql_stmt_store_result(stmt);
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    while (1) {
        int status = mysql_stmt_fetch(stmt);
        if (status == 1 || status == MYSQL_NO_DATA) {
            break;
        }
        ptable->file_id = res_file_id;
        ptable->parent_id = res_parent_id;
        strcpy(ptable->file_name, res_file_name);
        ptable->owner_id = res_owner_id;
        strcpy(ptable->sha1, res_sha1);
        ptable->filesize = res_filesize;
        ptable->type = res_type;
        ptable->tomb = res_tomb;
    }

    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    return 0;
}

int search_file(MYSQL* mysql, const char* sha1_hash) {
    // 初始化MYSQL_STMT
    MYSQL_STMT* stmt = mysql_stmt_init(mysql);
    if (stmt == NULL) {
        printf("(%d, %s)\n", mysql_errno(mysql), mysql_error(mysql));
        return -1;
    } 

    // 执行PREPARE操作
    const char* sql = "SELECT * FROM user WHERE shal = ?";
    int ret = mysql_stmt_prepare(stmt, sql, strlen(sql));
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    // 获取占位符的个数
    int count = mysql_stmt_param_count(stmt);
    if (count != 1) {
        printf("(%d, %s)\n", mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
        return -1;
    }

    // 设置输入参数
    char sha1[64] = {0};
    unsigned long  sha1_len;

    strcpy(sha1, sha1_hash);
    sha1_len = strlen(sha1);

    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));

    // 绑定参数
    bind.buffer_type = MYSQL_TYPE_VAR_STRING;
    bind.buffer = sha1;
    bind.is_null = 0;
    bind.length = &sha1_len;

    // 执行绑定操作
    ret = mysql_stmt_bind_param(stmt, &bind);
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    // 执行EXECUTE操作
    ret = mysql_stmt_execute(stmt);
    MYSQL_STMT_ERROR_CHECK(ret, stmt);

    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    return 0;
}
