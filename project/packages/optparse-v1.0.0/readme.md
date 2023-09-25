# optparse #

optparse是一个开源, 可移植的, 可重入的和可嵌入的类getopt命令行参数解析器. 它支持POSIX getopt选项字符串, GNU风格的长参数解析, 参数置换和子命令处理.  

## 1. 为什么不在RTT下直接使用getopt函数?

POSIX getopt解析器有三个致命的缺陷. 这些缺陷都是可以通过optparse来解决的: 

1. getopt的解析状态存储在全局变量中, 其中一些变量是静态不可访问的. 这意味着在RTT(RT-Thread)下只能有一个msh命令可以使用getopt, 其他msh命令再次使用getopt会出现bug. 而optparse将每次解析的状态存储在一个属于msh的私有结构体中, 每一个optparse命令解析器不会相互影响. 

2. 在RTT中BSP一般支持Keil, IAR和GCC, 对于keil环境下是无法支持原生的POSIX getopt解析器, 这样就造成了在RTT下实现命令解析是无法移植到Keil中使用的, 代码的兼容性降低!

3. 在getopt中, 错误消息被打印到stderr. 使用opterr可以禁用此功能, 但消息本身仍然不可访问. optparse通过将错误消息写入它的errmsg字段来解决这个问题, 该字段可以被打印到任何地方. optparse的缺点是, 这个错误消息总是用英语而不是当前语言环境. 

## 2. 命令顺序

默认情况下, argv在被解析时会将非选项的参数移动到数组的末尾. 可以通过在初始化后将`permute`字段设置为0来禁用它. 

~~~c
struct optparse options;
optparse_init(&options, argv);
options.permute = 0;
~~~

## 3. 对比

optparse的接口应该是任何熟悉getopt的人都熟悉的. 选项字符串具有相同的格式, 解析器结构体成员变量与getopt全局变量(optarg、optind、optopt)具有相同的名称. 

long选项解析器`optparse_long()`API与GNU的`getopt_long()`非常相似，可以作为替代api. 

optparse没有分配内存. 此外optparse没有依赖项, 包括libc本身. 因此可以在标准C库无法使用的情况下使用. 

查看`optparse.h`文件可以了解完整的API. 

## 4. 示例使用

这是一个通用的getopt命令解析代码: 

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

int main(int argc, char **argv)
{
    bool amend = false;
    bool brief = false;
    const char *color = "white";
    int delay = 0;

    int option;
    while ((option = getopt(argc, argv, "abc:d::")) != -1) {
        switch (option) {
        case 'a':
            amend = true;
            break;
        case 'b':
            brief = true;
            break;
        case 'c':
            color = optarg;
            break;
        case 'd':
            delay = optarg ? atoi(optarg) : 1;
            break;
        case '?':
            exit(EXIT_FAILURE);
        }
    }

    /* Print remaining arguments. */
    for (; optind < argc; optind++)
        printf("%s\n", argv[optind]);
    return 0;
}
~~~

如果使用optparse, 同样的功能将使用如下的代码的实现: 

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "optparse.h"

int main(int argc, char **argv)
{
    bool amend = false;
    bool brief = false;
    const char *color = "white";
    int delay = 0;

    char *arg;
    int option;
    struct optparse options;

    optparse_init(&options, argv);
    while ((option = optparse(&options, "abc:d::")) != -1) {
        switch (option) {
        case 'a':
            amend = true;
            break;
        case 'b':
            brief = true;
            break;
        case 'c':
            color = options.optarg;
            break;
        case 'd':
            delay = options.optarg ? atoi(options.optarg) : 1;
            break;
        case '?':
            fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
            exit(EXIT_FAILURE);
        }
    }

    /* Print remaining arguments. */
    while ((arg = optparse_arg(&options)))
        printf("%s\n", arg);
    return 0;
}
~~~

这是optparse对长选项参数的支持: 

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "optparse.h"

int main(int argc, char **argv)
{
    struct optparse_long longopts[] = {
        {"amend", 'a', OPTPARSE_NONE},
        {"brief", 'b', OPTPARSE_NONE},
        {"color", 'c', OPTPARSE_REQUIRED},
        {"delay", 'd', OPTPARSE_OPTIONAL},
        {0}
    };

    bool amend = false;
    bool brief = false;
    const char *color = "white";
    int delay = 0;

    char *arg;
    int option;
    struct optparse options;

    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
        switch (option) {
        case 'a':
            amend = true;
            break;
        case 'b':
            brief = true;
            break;
        case 'c':
            color = options.optarg;
            break;
        case 'd':
            delay = options.optarg ? atoi(options.optarg) : 1;
            break;
        case '?':
            fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
            exit(EXIT_FAILURE);
        }
    }

    /* Print remaining arguments. */
    while ((arg = optparse_arg(&options)))
        printf("%s\n", arg);

    return 0;
}
~~~

## 5. 感谢

1. 该库移植于https://github.com/skeeto/optparse. 
2. 感谢skeeto. 本移植是修改了部分原作者的代码针对RTT在线包实现的版本, 该仓库保留原作者的许可声明! 具体原作者许可请查看https://github.com/skeeto/optparse/blob/master/UNLICENSE. 
