#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define READTIMEOUTSEC 5
//#define DEBUG

long double elapsedTime;
double elaTime[10];

static char BMP[] = {0x42, 0x4d};
static char PNG[] = {0x89, 0x50, 0x4e, 0x47};
char* Format = NULL;
int FormatSize = 0;

double meanVal()	//Gleitender Mittelwert
{
	int j;
	double res = 0;
	for(j = 0; j < 10; j++)
		res+=elaTime[j];
	return res/10;
}

/// Reads until it discovers the first matching char. Calls checkWholeIdentifier afterwards.
void getFormat()
{
    char input = 0;
    printf("Format identifier: ");

    do
    {
        fread(&input,1, 1,stdin);
        if(input == PNG[0])
        {
            // assume png
            Format = PNG;
            FormatSize = sizeof(PNG);
            checkWholeIdentifier(1);
        }
        else if(input == BMP[0])
        {
            // assume bmp
            Format = BMP;
            FormatSize = sizeof(BMP);
            checkWholeIdentifier(1);
        }

    }while(Format == NULL);
    printf("%.*s\n", FormatSize, Format);
    return;
}

/// Reads at max FormatSize chars to compare the identifier
/// \param start pos of identifier to compare
/// \return true if Format is found
int checkWholeIdentifier(unsigned int start)
{
    char input = 0;
    for(int i = start;Format != NULL && i < FormatSize; i++)
    {
        fread(&input,1, 1,stdin);
        if(input != Format[i])
        {
            Format = NULL;
            FormatSize = 0;
        }
    }
    return Format != NULL;
}

/// Reads until it discovers first char of file identifier.
/// \return Bytes read.
int findNextStart()
{
    struct timeval lastSuccessfulRead, now;
    gettimeofday(&lastSuccessfulRead, NULL);
    char ch;
    int aggReadInBit = 0, readInBits = 0;
    while(Format != NULL)
    {
        readInBits += fread(&ch, 1, 1, stdin);
        if(readInBits == 0)
        {
            gettimeofday(&now, NULL);
            if((now.tv_sec - lastSuccessfulRead.tv_sec) >= READTIMEOUTSEC)
            {
                printf("Reading interrupted. %d sec timout reached!\n",READTIMEOUTSEC);
                break;
            }

        } else
        {
            aggReadInBit += readInBits;
            readInBits = 0;
            gettimeofday(&lastSuccessfulRead, NULL);
        }
        if(ch == Format[0])
            break;
    }
    return aggReadInBit;
}

int main()
{
    unsigned long bitsRead = 0, frames = 0;
    int i = 0;
    struct timeval t1, t2;

    FILE *fp = fopen("stats.csv", "w");
    fprintf(fp,"Frame, diff. Time, FPS\n");


#ifdef DEBUG
    sleep(10);
#endif
    getFormat();
    bitsRead += FormatSize;
    bitsRead += findNextStart();
    frames++;

    gettimeofday(&t1, NULL);
    while(Format != NULL)
	{

        bitsRead += FormatSize;
        if(checkWholeIdentifier(1))
        {
            frames++;
            gettimeofday(&t2, NULL);
            // compute and print the elapsed time in millisec
            elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
            elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
            elaTime[i] += elapsedTime;
            elaTime[i] /= 2;

            printf("%16.3lf ms (%16.3Lf ms) -> %lf fps (%Lf fps) (%lu bytes -> %lu frames)\r", meanVal(), elapsedTime, 1.0 / (meanVal() / 1000), 1.0 / (elapsedTime / 1000), bitsRead/8, frames);
            fprintf(fp,"%lu, %Lf, %16.8Lf\n",frames,elapsedTime,1.0/(elapsedTime/1000.0));
            gettimeofday(&t1, NULL);
        }
        int lsize = 0;
        lsize += findNextStart();
        if(lsize == 0)
            break;
        bitsRead += lsize;

		fflush(stdout);

		if(i++ >= 10)
			i = 0;
	}

    fclose(fp);
	return 0;
}