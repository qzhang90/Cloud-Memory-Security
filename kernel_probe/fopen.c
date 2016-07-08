#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
int main(void)
{
	
    char buff[512];
    int i;
  

    for(i = 0; i < 512; i++)
            buff[i] = 'a';
    FILE *f1 = fopen("text.txt", "wb");
    assert(f1);
    int r1 = fwrite(buff, 1, 512, f1);
    //printf("wrote %d elements\n", r1);
    //sleep(1000);
    fclose(f1);
    sleep(1000);
 
    /*
    double b[SIZE];
    FILE *f2 = fopen("file.bin", "rb");
    int r2 = fread(b, sizeof b[0], SIZE, f2);
    fclose(f2);
    printf("read back: ");
    for(int i = 0; i < r2; i++)
        printf("%f ", b[i]);
    */
}
