#include <kernel/types.h>
#include <kernel/stat.h>
#include <user/user.h>

int main(int argc, char** argv){
    int maskbits;
    if (argc < 3){
        fprintf(2, "usage: strace mask command [args]\n");
        exit(0);
    }
    maskbits= atoi(argv[1]);
    trace(maskbits);
    char* argarr[argc - 1];
    for (int i = 0; i < argc - 2; i++){
        argarr[i] = argv[i + 2];
    } //to start after strace x
    argarr[argc - 2] = 0;
    exec(argarr[0], argarr);
    // else
    // {
        fprintf(2, "exec %s failed\n", argarr[0]);
    // }
    exit(0);
    

    
}