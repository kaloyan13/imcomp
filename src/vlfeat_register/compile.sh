export LD_LIBRARY_PATH=/data/mybin/vlfeat/vlfeat-0.9.20/bin/glnxa64/
PATH_TO_EIGEN="/home/tlm/dev/imcomp/src/relja_register/external_lib/eigen"
g++ -std=c++11 -fpermissive test_vlfeat_register.cc -o test_vlfeat_register -I$PATH_TO_EIGEN -I/data/mybin/vlfeat/vlfeat-0.9.20/ -L/data/mybin/vlfeat/vlfeat-0.9.20/bin/glnxa64/ -lvl

./test_vlfeat_register a.pgm b.pgm
