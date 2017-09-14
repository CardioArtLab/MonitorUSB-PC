#ifndef VERSION_H
#define VERSION_H

#define STRNX(x) #x
#define STRX(x) STRNX(x)

#define MY_VERSION_MAJOR 0
#define MY_VERSION_MINOR 1
#define MY_VERSION_BUILD 0

#define APP_VERSION STRX(MY_VERSION_MAJOR) "." STRX(MY_VERSION_MINOR) "." STRX(MY_VERSION_BUILD)
#define APP_TITLE ("USB Monitor PC (version " APP_VERSION ")")

#endif // VERSION_H
