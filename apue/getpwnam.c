#include <pwd.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

struct passwd* getpwname(const char* name) {
    struct passwd* ptr;
    setpwent();
    while ((ptr = getpwent()) != NULL)
        if (strcmp(name, ptr->pw_name) == 0) break;
    endpwent();
    return(ptr);
}

int main() {
    struct passwd* p = getpwnam("faithall");
    if (p != NULL) {
        printf("%s:%s:%s;%s\n", p->pw_name, p->pw_passwd, p->pw_dir, p->pw_shell);
    }
    return 0;
}
