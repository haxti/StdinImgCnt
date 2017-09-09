#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define READTIMEOUTSEC 3
#define MEANVALS 20
//#define DEBUG

long double elapsedTime;
double elaTime[MEANVALS];

static char BMP[] = {0x42, 0x4d};
static char PNG[] = {0x89, 0x50, 0x4e, 0x47};
char* Format = NULL;
int FormatSize = 0;

double meanVal()	//Gleitender Mittelwert
{
	int j;
	double res = 0;
	for(j = 0; j < MEANVALS; j++)
		res+=elaTime[j];
	return res/MEANVALS;
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
            if(checkWholeIdentifier(1) != 1)
            {
                Format = NULL;
                FormatSize = 0;
            }
        }
        else if(input == BMP[0])
        {
            // assume bmp
            Format = BMP;
            FormatSize = sizeof(BMP);
            if(checkWholeIdentifier(1) != 1)
            {
                Format = NULL;
                FormatSize = 0;
            }
        }

    }while(Format == NULL);
    printf("%.*s\n", FormatSize, Format);
    return;
}

/// Reads at max FormatSize chars to compare the identifier
/// \param start pos of identifier to compare
/// \return 0 if identifier was matched
int checkWholeIdentifier(unsigned int start)
{
    char input = 0;
    for(unsigned int i = start; i < FormatSize; i++)
    {
        fread(&input,1, 1,stdin);
        if(input != Format[i])
        {
            return 0;
        }
    }
    return 1;
}

/// Reads until it discovers first char of file identifier.
/// \return Valid identifier found.
int findNextStart()
{
    struct timeval lastSuccessfulRead, now;
    gettimeofday(&lastSuccessfulRead, NULL);
    char ch;
    int read = 0;
    while(Format != NULL)
    {
        read = fread(&ch, 1, 1, stdin);
        if(read == 0)       // if input ends
        {
            gettimeofday(&now, NULL);
            if((now.tv_sec - lastSuccessfulRead.tv_sec) >= READTIMEOUTSEC)
            {
                printf("\nInput interrupted! %d sec timout reached!\n",READTIMEOUTSEC);
                break;
            }

        } else
            gettimeofday(&lastSuccessfulRead, NULL);    // Update last successful read time

        if(ch == Format[0])
            if(checkWholeIdentifier(1))
                return 1;
    }
    return 0;
}

int main()
{
    unsigned long frames = 0;
    int i = 0;
    struct timeval t1, t2;

    FILE *fp = fopen("stats.csv", "w");
    fprintf(fp,"Frame, diff. Time, FPS\n");


#ifdef DEBUG
    sleep(10);
#endif
    getFormat();
    frames++;

    gettimeofday(&t1, NULL);
    while(findNextStart())
	{

        frames++;
        gettimeofday(&t2, NULL);
        // compute and print the elapsed time in millisec
        elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
        elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
        elaTime[i] += elapsedTime;
        elaTime[i] /= 2;

        printf("%16.3lf ms (%16.3Lf ms) -- %lf fps (%Lf fps) -- %lu frames\r", meanVal(), elapsedTime, 1.0 / (meanVal() / 1000), 1.0 / (elapsedTime / 1000), frames);
        fprintf(fp,"%lu, %Lf, %16.8Lf\n",frames,elapsedTime,1.0/(elapsedTime/1000.0));
        gettimeofday(&t1, NULL);

		fflush(stdout);

		if(i++ >= 10)
			i = 0;
	}

    fclose(fp);
	return 0;
}