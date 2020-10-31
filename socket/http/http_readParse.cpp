/**
 * 通过状态机完成HTTP请求的读取和分析(书上代码有待讨论，暂时放弃，待后续重写)
 */
#include <string.h>
#include <stdio.h>
//读缓冲区的大小
#define BUFFER_SIZE 4096
//主状态机的两种状态:正在分析请求行，正在分析头部字段
enum CHECK_STATE {
    CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER
};
//从状态机的三种状态，即行的读取状态：读取到一个完整的行、行出错和行数据尚且不完整
enum LINE_STATUS {
    LINE_OK = 0, LINE_BAD, LINE_OPEN
};
//服务器处理HTTP请求的结果：
//NO_REQUEST表示请求不完整，需要继续读取客户数据；
//GET_REQUEST表示获取一个完整的客户请求
//BAD_REQUEST表示客户请求有语法错误
//FORBIDDEN_REQUEST表示客户对资源没有足够的访问权限
//INTERNAL_ERROR表示服务器内部错误
//CLOSED_CONNECTION表示客户端已经关闭连接
enum HTTP_CODE {
    NO_REQUEST, GET_REQUEST, BAD_REQUEST, FORBIDDEN_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION
};

//简化问题，根据服务器处理结果发送成功或失败信息
static const char *szret[] = {"I get a correct result\n", "Something wrong\n"};

//从状态机，用于解析出一行内容
enum LINE_STATUS parse_line(char *buffer, int checked_index, int read_index) {
    //check_index指向buffer中当前正在分析的字节,read_index指向buffer中客户数据的尾部下一字节。
    //buffer中0~checked_index字节已经分析完毕,第checked_index~(read_index - 1)字节由下面的循环挨个分析
    for (; checked_index < read_index; ++checked_index) {
        //当前获取字节
        char temp = buffer[checked_index];
        //一行末尾是\r\n
        /**
         * 注：破坏性的请求报文会产生问题,例如字段故意加入'\0'
         */
        if (temp == '\r') {
            if (checked_index + 1 == read_index) return LINE_OPEN;
            else if (buffer[checked_index + 1] == '\n') {
                buffer[checked_index++] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            } else return LINE_BAD;
        } else if (temp == '\n') {
            if ((checked_index > 1) && buffer[checked_index - 1] == '\r') {
                buffer[checked_index - 1] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

//分析请求行
HTTP_CODE parse_requestline(char *temp, CHECK_STATE &checkState) {
    //HTTP 空格分隔方法和url
    char *url = strpbrk(temp, " ");
    if (!url) return BAD_REQUEST;
    *url++ = '\0';
    char *method = temp;
    //HTTP method不忽略大小写
    if (strcmp(method, "GET") == 0) printf("The request methon is GET\n");
    else return BAD_REQUEST;

    char *version = strpbrk(url, " ");
    if (!version) return BAD_REQUEST;
    *version++ = '\0';

    if (strcmp(version, "HTTP/1.1") != 0) return BAD_REQUEST;
    printf("The request URL is: %s\n", url);
    checkState = CHECK_STATE_HEADER;
    return NO_REQUEST;
}