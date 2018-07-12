EXEC_NAME=$1

install_name_tool -change "libMagick++-6.Q8.8.dylib" "@rpath/libMagick++-6.Q8.8.dylib" $EXEC_NAME
install_name_tool -change "libboost_filesystem.dylib" "@rpath/libboost_filesystem.dylib" $EXEC_NAME
install_name_tool -change "libboost_system.dylib" "@rpath/libboost_system.dylib" $EXEC_NAME
install_name_tool -change "libboost_thread.dylib" "@rpath/libboost_thread.dylib" $EXEC_NAME
install_name_tool -change "libboost_atomic.dylib" "@rpath/libboost_atomic.dylib" $EXEC_NAME
install_name_tool -change "libboost_chrono.dylib" "@rpath/libboost_chrono.dylib" $EXEC_NAME
install_name_tool -change "libboost_date_time.dylib" "@rpath/libboost_date_time.dylib" $EXEC_NAME
install_name_tool -change "@loader_path/libvl.dylib" "@rpath/libvl.dylib" $EXEC_NAME