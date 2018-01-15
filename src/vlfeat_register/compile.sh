rm test_vlfeat_register
rm result/*

export LD_LIBRARY_PATH=/data/mybin/vlfeat/vlfeat-0.9.20/bin/glnxa64/
PATH_TO_EIGEN="/home/tlm/dev/imcomp/src/relja_register/external_lib/eigen"
g++ -std=c++11 -fpermissive test_vlfeat_register.cc -o test_vlfeat_register -I$PATH_TO_EIGEN -I/data/mybin/vlfeat/vlfeat-0.9.20/ -L/data/mybin/vlfeat/vlfeat-0.9.20/bin/glnxa64/ -lvl `pkg-config --cflags --libs Magick++` -lboost_filesystem -lboost_system

#./test_vlfeat_register a.pgm b.pgm
./test_vlfeat_register im4_a.jpg im4_b.jpg # colorspace rgb and gray
#./test_vlfeat_register im2_a.png im2_b.jpg
#./test_vlfeat_register c.jpg d.jpg

#./test_vlfeat_register a.JPG b.JPG
#./test_vlfeat_register a_small.JPG b_small.JPG
