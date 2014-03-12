#include "File.h"
#include <string.h>
#include "../Core/OO.h"

static char FModes[5][5] = {"rb", "wb", "rwb+", "ab+", "wb+"};
static int Trash;

RCtor(File)
{
    This -> IsOpen = 0;
    This -> FilePtr = 0;
    RInit(File);
}

RDtor(File)
{
    if(This -> IsOpen)
        fclose(This -> BaseStream);
}

static void File_RefreshLength(File* This)
{
    fseek(This -> BaseStream, 0, SEEK_END);
    This -> Length = ftell(This -> BaseStream);
    fseek(This -> BaseStream, This -> FilePtr, SEEK_SET);
}

int File_Open(File* This, String* Path, OpenMode FileMode)
{
    return File_OpenChars(This, String_GetChars(Path), FileMode);
}

int File_OpenChars(File* This, char* Path, OpenMode FileMode)
{
    char* chars = Path;
    char* fmode = FModes[FileMode];
    This -> BaseStream = fopen(chars, fmode);

    This -> IsOpen = 1;
    if(This -> BaseStream == 0)
        This -> IsOpen = 0;
    This -> FilePtr = 0;

    if(This -> IsOpen)
        File_RefreshLength(This);
    return This -> IsOpen;
}

void File_Close(File* This)
{
    if(This -> IsOpen)
    {
        This -> IsOpen = 0;
        fclose(This -> BaseStream);
    }
}

int File_Flush(File* This)
{
    if(fflush(This -> BaseStream) == 0)
        return 1;
    return 0;
}

void File_SetPosition(File* This, int64_t Position)
{
    This -> FilePtr = Position;
    fseek(This -> BaseStream, This -> FilePtr, SEEK_SET);
}

int64_t File_GetPosition(File* This)
{
    return This -> FilePtr;
}

int64_t File_GetLength(File* This)
{
    return This -> Length;
}

#define File_Read_Array_Core \
    Trash = fread(&tmpchar, 1, 1, This -> BaseStream); \
    Array_Push(char, tmp, tmpchar); \
    This -> FilePtr ++; \
    fseek(This -> BaseStream, This -> FilePtr, SEEK_SET);

void File_Read_String(File* This, String* Dest)
{
    char tmpchar;
    Array_Gtor(char, tmp);
    
    do
    {
        File_Read_Array_Core
    }
    while(tmpchar != 0 && This -> FilePtr < This -> Length - 1);
    
    String_SetChars(Dest, tmp);
    File_SetPosition(This, This -> FilePtr);
    Array_Dtor(char, tmp);
}

int File_Read_Chars(File* This, char* Dest)
{
    int count = 0;
    char tmpchar;

    do
    {
        Trash = fread(&tmpchar, 1, 1, This -> BaseStream);
        Dest[count ++] = tmpchar;
        This -> FilePtr ++;
        fseek(This -> BaseStream, This -> FilePtr, SEEK_SET);
    }
    while(tmpchar != 0 && This -> FilePtr < This -> Length - 1);

    File_SetPosition(This, This -> FilePtr);
    return count;
}

void File_Read_Buffer(File* This, void* Dest, int64_t ByteCount)
{
    Trash = fread(Dest, 1, ByteCount, This -> BaseStream);
    This -> FilePtr += ByteCount;
    fseek(This -> BaseStream, This -> FilePtr, SEEK_SET);
}

void File_Write_String(File* This, String* Sorc)
{
    File_Write_Chars(This, String_GetChars(Sorc));
}

void File_Write_Chars(File* This, char* Sorc)
{
    int Len = strlen(Sorc);
    fwrite(Sorc, 1, Len, This -> BaseStream);
    This -> FilePtr += Len;
    fseek(This -> BaseStream, This -> FilePtr, SEEK_SET);
}

void File_Write_Buffer(File* This, char* Sorc, int64_t Length)
{
    fwrite(Sorc, 1, Length, This -> BaseStream);
    This -> FilePtr += Length;
    fseek(This -> BaseStream, This -> FilePtr, SEEK_SET);
}

void File_ReadWord(File* This, String* Dest)
{
    Array_Gtor(char, tmp);
    char tmpchar;

    Trash = fread(&tmpchar, 1, 1, This -> BaseStream);
    while(tmpchar == ' ' || tmpchar == '\t' ||
          tmpchar == '\n' || tmpchar == '\r')
    {
        This -> FilePtr ++;
        fseek(This -> BaseStream, This -> FilePtr, SEEK_SET);
        Trash = fread(&tmpchar, 1, 1, This -> BaseStream);
    }

    fseek(This -> BaseStream, This -> FilePtr, SEEK_SET);
    do
    {
        File_Read_Array_Core
    }
    while(tmpchar != ' ' && tmpchar != '\t' && tmpchar != '\n' && 
          tmpchar != '\r' && tmpchar != 0 && This -> FilePtr < This -> Length);

    tmp[tmp_Index] = 0;
    String_SetChars(Dest, tmp);
    Array_Dtor(char, tmp);
}

void File_ReadLine(File* This, String* Dest)
{
    Array_Gtor(char, tmp);
    char tmpchar;

    do
    {
        File_Read_Array_Core
    }
    while(tmpchar != '\n' && tmpchar != '\r' && tmpchar != 0 && 
          This -> FilePtr < This -> Length);

    tmp[tmp_Index] = 0;
    String_SetChars(Dest, tmp);
    Array_Dtor(char, tmp);
}

void File_ReadAll(File* This, String* Dest)
{
    String_AllocLength(Dest, This -> Length);
    fseek(This -> BaseStream, 0, SEEK_SET);
    Trash = fread(Dest -> Data, 1, This -> Length, This -> BaseStream);
    Dest -> Data_Index = This -> Length - 1;
}

void File_WriteLine(File* This, String* Sorc)
{
    File_Write_Chars(This, String_GetChars(Sorc));
    File_Write_Char(This, '\n');
}

//Template Reads & Writes
#if 0
#include "_File.rc"
#endif
#define _RTAddress "IO/_File.rc"
#define _ClassName
#define _Attr 1

#include "../Core/Include_T1AllTypes.h"

