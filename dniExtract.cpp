// quick and dirty .DNI file extraction program by Ken Taylor (taylok2@rpi.edu) - 10/16/00
// works with the .dni files that come with the realMysttest.
// simply extracts the files inside the .dni file to a given directory.
//
// known bug: doesn't accept path names with spaces in them.
// workaround: use the DOS equivalent (ie Progra~1 instead of Program Files)
//             find the DOS equvialent in the properties dialog for a file.


// Also, this has only been tested to compile with MSVC++ 5 and 6.... i don't
// know if it'll compile in any other program (because of the _mkdir and _chdir
// calls)


// you are given permission to freely distribute, modify, and use pieces of
// this code in your own programs, If you modify this program or use pieces
// of it for your own code, you MUST:
//  1) include my name and email address in the source and any readme file
//     and give credit where credit is due
//  2) include a link to the original dniExtract program
//     (http://www.rpi.edu/~taylok2/dniExtract)
//  
// you don't have to inform me or ask my permission, but i would really like it
// if you did tell me about whatever you're doing with my code, because
// i'll just be genuinely curious and interested :)


// .dni file Format is:
// 0x00		'Dirt'
// 0x04		0x00000100 1c000000  (version number? 1.28?)
// 0x0c		abs offset to beginning of file table (1)
// 0x10		abs offset to beginning of name list (2)
// 0x14		abs offset to beginning of data (3)
// 0x18		abs offset to beginning of file table again
// 0x1c		directory list begins
//  DIRECTORY LIST FORMAT:
//  o0x00	UL (abs offset to name in name table)
//  o0x04       UL # of objects
//  o0x08..     UL abs offsets to object descriptors (in directory or file table)
// 0x(1)	file table begins
//  FILE TABLE FORMAT: each entry is 20 bytes
//  o0x00	UL (abs offset to name in name table)
//  o0x04	UL (abs offset to next entry in name table)
//                 (last one is 0000 0000)
//  o0x08	UL (length of data block)
//  o0x0c	UL (abs offset to data)
//  o0x10	UL 0000 0000
// 0x(2)    name list begins
//  NAME LIST FORMAT
//   - just null-terminated strings
// 0x(3)    data begins


// history:
// v0.01 -- 10/15/00 by Ken Taylor (taylok2@rpi.edu)
//           original release. extracted files from .dni into directories
//           works with realMyst test
//          10/16/00 cleaned it up a bit for public release
//            known problems: can't accept paths with spaces in them because i suck
//                            so use "Progra~1" instead of "Program Files" for example

#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#include<direct.h> // used for _mkdir and _chdir .. if you know a better way tell me!

void doDirectory(unsigned long Off, FILE * fun, unsigned long fileTableOff);
int getName(char * buf, unsigned long Off, FILE * fun);


int main()
{
	char dni[256], out[256];
	FILE *dnifile;

	printf("DNIExtract v0.01");
	printf("\n quick and dirty .DNI extractor by Ken Taylor (taylok2@rpi.edu)");
	printf("\n works with realMyst test\n\n");

	printf("Enter DNI file: ");
	scanf("%s", dni);

	dnifile = fopen(dni, "rb");

	if (dnifile == NULL)
	{
		printf("\n ERROR! cannot open .dni file \n");
		_getch();
		exit(1);
	}

    
	unsigned long dirttag;

	fseek(dnifile, 0, SEEK_SET);
	fread(&dirttag, sizeof(unsigned long), 1, dnifile);

	if (dirttag != 1953655108) // "Dirt"
	{
		printf("\nERROR! not a valid .dni file \n");
		_getch();
		exit(2);
	}

	printf("\nEnter output directory: ");
	scanf("%s", out);

	if( _chdir(out) )
	{
		if ( _mkdir(out) )
		{
			printf("\n ERROR! cannot make output directory \n");
			_getch();
			exit(3);
		}
		if ( _chdir(out) )
		{
			printf("\n ERROR! cannot change to output directory \n");
			_getch();
			exit(3);

		}
	}


    
	// step through directory, extracting files or making directories as
	// is fit.

	unsigned long directoryListOff, fileTableOff, nameListOff, dataOff, fileTableOff2;

	fseek(dnifile, 0x0c, SEEK_SET);
	directoryListOff = 0x1c;
	fread(&fileTableOff, sizeof(unsigned long), 1, dnifile);
	fread(&nameListOff, sizeof(unsigned long), 1, dnifile);
	fread(&dataOff, sizeof(unsigned long), 1, dnifile);
	fread(&fileTableOff2, sizeof(unsigned long), 1, dnifile);

	if(fileTableOff != fileTableOff2)
	{
		printf("\n ERROR in DNI file, fileTableOff mismatch \n");
		_getch();
		exit(4);

	}

	//printf("%lu\n%lu\n%lu\n%lu\n", fileTableOff, nameListOff, dataOff, fileTableOff2);

	doDirectory(directoryListOff, dnifile, fileTableOff);

	printf("\nDONE!\n");

	return 0;
}



void doDirectory(unsigned long currentOff, FILE * fun, unsigned long fileTableOff)
{
	char Name[256], objName[256];
	char writebuffer[1024];
	bool isRoot = false;
	unsigned long nameOff;
	unsigned long objectOff;
	unsigned long numObjects;
	
	fseek(fun, currentOff, SEEK_SET);
	fread(&nameOff, sizeof(unsigned long), 1, fun);

	getName(Name, nameOff, fun);

	_mkdir(Name);
	_chdir(Name);
	
	currentOff += 4;
	fseek(fun, currentOff, SEEK_SET);
	fread(&numObjects, sizeof(unsigned long), 1, fun);

	for(int i = 0; i < numObjects; i++)
	{
        currentOff += 4;
		fseek(fun, currentOff, SEEK_SET);
		fread(&objectOff, sizeof(unsigned long), 1, fun);
		if(objectOff < fileTableOff) 
		{
			// then it's a directory, recurse in
			doDirectory(objectOff, fun, fileTableOff);
		} else {
			// it's a file, and extract it.
			FILE *itsafile;
			unsigned long dataLen;
			unsigned long dataOff;
            
			fseek(fun, objectOff, SEEK_SET);
			fread(&nameOff, sizeof(unsigned long), 1, fun);

			getName(objName, nameOff, fun);
			itsafile = fopen(objName, "wb");

			if (itsafile == NULL)
			{ 
		        	printf("\n ERROR! can't open file for write \n");
		        	_getch();
				exit(1);
			}

			fseek(fun, objectOff+8, SEEK_SET);
			fread(&dataLen, sizeof(unsigned long), 1, fun);
			fread(&dataOff, sizeof(unsigned long), 1, fun);

			fseek(fun, dataOff, SEEK_SET);
			for(int j=0; j < (dataLen / 1024); j++)
			{
				fread(writebuffer, 1, 1024, fun);
				fwrite(writebuffer, 1, 1024, itsafile);
			}
			fread(writebuffer, 1, dataLen % 1024, fun);
			fwrite(writebuffer, 1, dataLen % 1024, itsafile);

			fclose(itsafile);
		}
	}

	_chdir("..");

}



int getName(char * buf, unsigned long Off, FILE * fun)
{
	fseek(fun, Off, SEEK_SET);
	int i = 0;

	do 
	{
		fread(buf + i, sizeof(unsigned char), 1, fun);
		i++;
	} while (buf[i-1] != 0);
	
	return i;
}
