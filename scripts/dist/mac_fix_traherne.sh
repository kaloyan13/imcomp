TRAHERNE_EXEC_DIR="/Users/tlm/dev/imcomp/bin"

#install_name_tool -change "/Users/tlm/deps/clang/imcomp/lib/lib/libMagick++-6.Q8.8.dylib" "@rpath/libMagick++-6.Q8.8.dylib" "$TRAHERNE_EXEC_DIR"/Traherne
install_name_tool -change "libMagick++-6.Q8.8.dylib" "@rpath/libMagick++-6.Q8.8.dylib" "$TRAHERNE_EXEC_DIR"/Traherne

#install_name_tool -change "libboost_filesystem.dylib" "@rpath/libboost_filesystem.dylib" "$TRAHERNE_EXEC_DIR"/Traherne
#install_name_tool -change "libboost_system.dylib" "@rpath/libboost_system.dylib" "$TRAHERNE_EXEC_DIR"/Traherne
#install_name_tool -change "libboost_thread.dylib" "@rpath/libboost_thread.dylib" "$TRAHERNE_EXEC_DIR"/Traherne
#install_name_tool -change "libboost_atomic.dylib" "@rpath/libboost_atomic.dylib" "$TRAHERNE_EXEC_DIR"/Traherne
#install_name_tool -change "libboost_chrono.dylib" "@rpath/libboost_chrono.dylib" "$TRAHERNE_EXEC_DIR"/Traherne
#install_name_tool -change "libboost_date_time.dylib" "@rpath/libboost_date_time.dylib" "$TRAHERNE_EXEC_DIR"/Traherne
install_name_tool -change "@loader_path/libvl.dylib" "@rpath/libvl.dylib" "$TRAHERNE_EXEC_DIR"/Traherne