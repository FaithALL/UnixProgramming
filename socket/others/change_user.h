/**
 * 以root身份启动的进程切换为一个普通用户身份运行
 */
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>

static bool switch_to_user(uid_t user_id, gid_t gp_id) {
    //当前用户必须是root用户或者目标用户
    gid_t gid = getgid();
    uid_t uid = getuid();
    if (gid == gp_id && uid == user_id) return true;
    if (gid == 0 && uid == 0) {
        if (setuid(uid) == 0 && setgid(gid) == 0) return true;
        else return false;
    }
    return false;
}
