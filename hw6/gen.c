#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <math.h>

/*
 * xoroshiro random generator
 * source : https://prng.di.unimi.it/xoroshiro64star.c
 */

static inline uint32_t rotl(const uint32_t x, int k) {
	return (x << k) | (x >> (32 - k));
}

static uint32_t s[2];

uint32_t next(void) {
	const uint32_t s0 = s[0];
	uint32_t s1 = s[1];
	const uint32_t result = s0 * 0x9E3779BB;

	s1 ^= s0;
	s[0] = rotl(s0, 26) ^ s1 ^ (s1 << 9); // a, b
	s[1] = rotl(s1, 13); // c

	return result;
}

/*
 * f : function choice, arg : arg for function
 */
int f[16], arg[16];
/*
 * fd for open file
 * count for restricting the number of fib and fac
 * n for the bread number
 */
int fd, count, n;
/*
 * estimate total run time
 */
float total;
/*
 * 1 : without bank2, 2: with bank2
 */
int mod;

int main(int argc, char *argv[]) {
    fd = open("/dev/urandom", O_RDONLY);
    read(fd,&s[0],sizeof(s[0]));
    read(fd,&s[1],sizeof(s[1]));
    close(fd);
    total = 0.0;
    n = atoi(argv[1]);
    mod = atoi(argv[2]);

    for(int i = 0; i < n; ++i){
        if(mod == 1){
            if(count < n / 3) f[i] = next() % 3;
            else              f[i] = 2;
        }else if(mod == 2){
            if(count < n / 4) f[i] = next() % 4;
            else              f[i] = next() % 2 + 2;
        }
        /*
         * I don't think more than 8 could get WA
         */
        if(f[i] < 2) arg[i] = next() % 8 + 1, count++, total += arg[i];
        /*
         * divide 256 to prevent overflow
         */
        else         arg[i] = (int) next() / 256, total += 3;
    }
    FILE *fin = fopen("cmd.sh", "wb");
    fprintf(fin, "./$1 %d ", next() % 8 + 1);
    for(int i = 0; i < n; ++i) fprintf(fin, "%d %d ", f[i], arg[i]);
    fprintf(fin, "%d &\nchild=$!\n", next() / 512);
    float tmp = 0.5;
    while(total > 0.0){
        /*
         * use tmp make random tstp signal
         */
        int bios = (int) (next() >> 29) + 1;
        /*
         * %.2f or more could cause race condition
         * upd : floor to 0.5
         * upd2 : make sure tstp is not on integer
         */
        tmp += bios;
        if(total > tmp) fprintf(fin, "sleep %.1f\nkill -TSTP $child\n", tmp);
        else break;
        total -= tmp;
        tmp = 0.0;
    }
    fprintf(fin, "wait $child");
    fclose(fin);
    system("chmod +x cmd.sh");
    return 0;
}
