1. Running.

$ make

2.
    a) compareFS(2) for pthread:
    $ ./file <path1> <path2>

    b) compareFS(2) for openmp
    $ ./opfile <path1> <path2>

    c) compareFS(K) for pthread
    $ ./kfile <path1> <path2> ... <pathk>

    d) compareFS(K) for openmp
    $ ./opkfile <path1> <path2> ... <pathk>

3. It has been observed,

    $ ./opkfile

doesn't work immediately. On facing this issue, 2-3 succesive cycles of <Ctrl-c> and
./opkfile might help

4. q.c contain queue data structure and sha1.c contains function to calculate
    sha1 of file. The usage license can be found in sha1.c file.
    These files can be safely left untouched and let Makefile take care of them.
