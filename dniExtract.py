# quick and dirty .DNI file extraction program by Ken Taylor (taylok2@rpi.edu) - 10/16/00
# works with the .dni files that come with the realMysttest.
# simply extracts the files inside the .dni file to a given directory.


# you are given permission to freely distribute, modify, and use pieces of
# this code in your own programs, If you modify this program or use pieces
# of it for your own code, you MUST:
#  1) include my dirname and email address in the source and any readme file
#     and give credit where credit is due
#  2) include a link to the original dniExtract program
#     (http:#www.rpi.edu/~taylok2/dniExtract)
#
# you don't have to inform me or ask my permission, but i would really like it
# if you did tell me about whatever you're doing with my code, because
# i'll just be genuinely curious and interested :)


# .dni file Format is:
# 0x00     'Dirt'
# 0x04     0x00000100 1c000000  (version number? 1.28?)
# 0x0c     abs offset to beginning of file table (1)
# 0x10     abs offset to beginning of dirname list (2)
# 0x14     abs offset to beginning of data (3)
# 0x18     abs offset to beginning of file table again
# 0x1c     directory list begins
#  DIRECTORY LIST FORMAT:
#  o0x00   UL (abs offset to dirname in dirname table)
#  o0x04       UL # of objects
#  o0x08..     UL abs offsets to object descriptors (in directory or file table)
# 0x(1)    file table begins
#  FILE TABLE FORMAT: each entry is 20 bytes
#  o0x00   UL (abs offset to dirname in dirname table)
#  o0x04   UL (abs offset to next entry in dirname table)
#                 (last one is 0000 0000)
#  o0x08   UL (length of data block)
#  o0x0c   UL (abs offset to data)
#  o0x10   UL 0000 0000
# 0x(2)    dirname list begins
#  dirname LIST FORMAT
#   - just null-terminated strings
# 0x(3)    data begins


# history:
# v0.01 -- 10/15/00 by Ken Taylor (taylok2@rpi.edu)
#           original release. extracted files from .dni into directories
#           works with realMyst test
#          10/16/00 cleaned it up a bit for public release
#            known problems: can't accept paths with spaces in them because i suck
#                            so use "Progra~1" instead of "Program Files" for example

import os
import sys


def main():
    print("DNIExtract v0.01")
    print(" quick and dirty .DNI extractor by Ken Taylor (taylok2@rpi.edu)")
    print(" Ported to Python by Nathaniel Case (ndc8551@rit.edu)")
    print(" works with realMyst test\n\n")

    dni = input("Enter DNI file: ")

    #try:
    with open(dni, "rb") as dnifile:
        dnifile.seek(0, os.SEEK_SET)
        dirttag = dnifile.read(4)

        if (dirttag != b"Dirt"):
            printf("ERROR! not a valid .dni file \n")
            return 2

        out = input("Enter output directory: ")

        if not os.path.exists(out):
            os.mkdir(out)
        os.chdir(out)

        # step through directory, extracting files or making directories as
        # is fit.
        dnifile.seek(0x0c, os.SEEK_SET)
        fileTableOff = int.from_bytes(dnifile.read(4), 'little')
        dirnameListOff = int.from_bytes(dnifile.read(4), 'little')
        dataOff = int.from_bytes(dnifile.read(4), 'little')
        fileTableOff2 = int.from_bytes(dnifile.read(4), 'little')

        if(fileTableOff != fileTableOff2):
            print(" ERROR in DNI file, fileTableOff mismatch")
            return 3

        do_directory(0x1c, dnifile, fileTableOff)

        print("DONE!")
        return 0

    #except:
    #    print(" ERROR! cannot open .dni file")
    #    return 1


def do_directory(currentOff, fun, fileTableOff):
    fun.seek(currentOff, os.SEEK_SET)
    dirnameOff = int.from_bytes(fun.read(4), 'little')

    dirname = get_dirname(dirnameOff, fun)

    if not os.path.exists(dirname):
        os.mkdir(dirname)
    os.chdir(dirname)

    currentOff += 4
    fun.seek(currentOff, os.SEEK_SET)
    numObjects = int.from_bytes(fun.read(4), 'little')

    for i in range(numObjects):
        currentOff += 4
        fun.seek(currentOff, os.SEEK_SET)
        objectOff = int.from_bytes(fun.read(4), 'little')
        if objectOff < fileTableOff:
            # then it's a directory, recurse in
            do_directory(objectOff, fun, fileTableOff)
        else:
            # it's a file, and extract it.
            fun.seek(objectOff, os.SEEK_SET)
            dirnameOff = int.from_bytes(fun.read(4), 'little')

            objdirname = get_dirname(dirnameOff, fun)
            with  open(objdirname, "wb") as itsafile:
                fun.seek(objectOff+8, os.SEEK_SET)
                dataLen = int.from_bytes(fun.read(4), 'little')
                dataOff = int.from_bytes(fun.read(4), 'little')

                fun.seek(dataOff, os.SEEK_SET)
                itsafile.write(fun.read(dataLen))

    os.chdir("..")


def get_dirname(off, fun):
    fun.seek(off, os.SEEK_SET)

    buf = b''
    while True:
        char = fun.read(1)
        if char == b'\x00':
            if not buf:
                continue
            break
        buf += char

    return buf


if __name__ == "__main__":
    sys.exit(main())
