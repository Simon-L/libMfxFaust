#include <nfd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int openDspDialog(char* path);

int openDspDialog(char* path)
{
    NFD_Init();
    nfdu8char_t *outPath;
    nfdu8filteritem_t filters[1] = { { "Faust DSP", "dsp" } };
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = 1;
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result != NFD_OKAY)
    {
        NFD_Quit();
        return -1;
    }
    strcpy(path, outPath);
    NFD_FreePathU8(outPath);
    NFD_Quit();
    return 0;
}