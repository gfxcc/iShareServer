# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.2

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ubuntu/iShare

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/iShare

# Include any dependencies generated for this target.
include CMakeFiles/iShare.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/iShare.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/iShare.dir/flags.make

CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o: CMakeFiles/iShare.dir/flags.make
CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o: iShare.grpc.pb.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ubuntu/iShare/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o -c /home/ubuntu/iShare/iShare.grpc.pb.cc

CMakeFiles/iShare.dir/iShare.grpc.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iShare.dir/iShare.grpc.pb.cc.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ubuntu/iShare/iShare.grpc.pb.cc > CMakeFiles/iShare.dir/iShare.grpc.pb.cc.i

CMakeFiles/iShare.dir/iShare.grpc.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iShare.dir/iShare.grpc.pb.cc.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ubuntu/iShare/iShare.grpc.pb.cc -o CMakeFiles/iShare.dir/iShare.grpc.pb.cc.s

CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o.requires:
.PHONY : CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o.requires

CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o.provides: CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o.requires
	$(MAKE) -f CMakeFiles/iShare.dir/build.make CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o.provides.build
.PHONY : CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o.provides

CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o.provides.build: CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o

CMakeFiles/iShare.dir/iShare.pb.cc.o: CMakeFiles/iShare.dir/flags.make
CMakeFiles/iShare.dir/iShare.pb.cc.o: iShare.pb.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ubuntu/iShare/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/iShare.dir/iShare.pb.cc.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/iShare.dir/iShare.pb.cc.o -c /home/ubuntu/iShare/iShare.pb.cc

CMakeFiles/iShare.dir/iShare.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iShare.dir/iShare.pb.cc.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ubuntu/iShare/iShare.pb.cc > CMakeFiles/iShare.dir/iShare.pb.cc.i

CMakeFiles/iShare.dir/iShare.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iShare.dir/iShare.pb.cc.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ubuntu/iShare/iShare.pb.cc -o CMakeFiles/iShare.dir/iShare.pb.cc.s

CMakeFiles/iShare.dir/iShare.pb.cc.o.requires:
.PHONY : CMakeFiles/iShare.dir/iShare.pb.cc.o.requires

CMakeFiles/iShare.dir/iShare.pb.cc.o.provides: CMakeFiles/iShare.dir/iShare.pb.cc.o.requires
	$(MAKE) -f CMakeFiles/iShare.dir/build.make CMakeFiles/iShare.dir/iShare.pb.cc.o.provides.build
.PHONY : CMakeFiles/iShare.dir/iShare.pb.cc.o.provides

CMakeFiles/iShare.dir/iShare.pb.cc.o.provides.build: CMakeFiles/iShare.dir/iShare.pb.cc.o

CMakeFiles/iShare.dir/iShare_server.cc.o: CMakeFiles/iShare.dir/flags.make
CMakeFiles/iShare.dir/iShare_server.cc.o: iShare_server.cc
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ubuntu/iShare/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/iShare.dir/iShare_server.cc.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/iShare.dir/iShare_server.cc.o -c /home/ubuntu/iShare/iShare_server.cc

CMakeFiles/iShare.dir/iShare_server.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iShare.dir/iShare_server.cc.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ubuntu/iShare/iShare_server.cc > CMakeFiles/iShare.dir/iShare_server.cc.i

CMakeFiles/iShare.dir/iShare_server.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iShare.dir/iShare_server.cc.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ubuntu/iShare/iShare_server.cc -o CMakeFiles/iShare.dir/iShare_server.cc.s

CMakeFiles/iShare.dir/iShare_server.cc.o.requires:
.PHONY : CMakeFiles/iShare.dir/iShare_server.cc.o.requires

CMakeFiles/iShare.dir/iShare_server.cc.o.provides: CMakeFiles/iShare.dir/iShare_server.cc.o.requires
	$(MAKE) -f CMakeFiles/iShare.dir/build.make CMakeFiles/iShare.dir/iShare_server.cc.o.provides.build
.PHONY : CMakeFiles/iShare.dir/iShare_server.cc.o.provides

CMakeFiles/iShare.dir/iShare_server.cc.o.provides.build: CMakeFiles/iShare.dir/iShare_server.cc.o

CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o: CMakeFiles/iShare.dir/flags.make
CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o: MMGAPN/MMGSafariPayload.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ubuntu/iShare/CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o -c /home/ubuntu/iShare/MMGAPN/MMGSafariPayload.cpp

CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ubuntu/iShare/MMGAPN/MMGSafariPayload.cpp > CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.i

CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ubuntu/iShare/MMGAPN/MMGSafariPayload.cpp -o CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.s

CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o.requires:
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o.requires

CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o.provides: CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o.requires
	$(MAKE) -f CMakeFiles/iShare.dir/build.make CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o.provides.build
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o.provides

CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o.provides.build: CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o

CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o: CMakeFiles/iShare.dir/flags.make
CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o: MMGAPN/MMGAPNSConnection.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ubuntu/iShare/CMakeFiles $(CMAKE_PROGRESS_5)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o -c /home/ubuntu/iShare/MMGAPN/MMGAPNSConnection.cpp

CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ubuntu/iShare/MMGAPN/MMGAPNSConnection.cpp > CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.i

CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ubuntu/iShare/MMGAPN/MMGAPNSConnection.cpp -o CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.s

CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o.requires:
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o.requires

CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o.provides: CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o.requires
	$(MAKE) -f CMakeFiles/iShare.dir/build.make CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o.provides.build
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o.provides

CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o.provides.build: CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o

CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o: CMakeFiles/iShare.dir/flags.make
CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o: MMGAPN/MMGSSLConnection.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ubuntu/iShare/CMakeFiles $(CMAKE_PROGRESS_6)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o -c /home/ubuntu/iShare/MMGAPN/MMGSSLConnection.cpp

CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ubuntu/iShare/MMGAPN/MMGSSLConnection.cpp > CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.i

CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ubuntu/iShare/MMGAPN/MMGSSLConnection.cpp -o CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.s

CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o.requires:
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o.requires

CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o.provides: CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o.requires
	$(MAKE) -f CMakeFiles/iShare.dir/build.make CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o.provides.build
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o.provides

CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o.provides.build: CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o

CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o: CMakeFiles/iShare.dir/flags.make
CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o: MMGAPN/MMGFeedbackConnection.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ubuntu/iShare/CMakeFiles $(CMAKE_PROGRESS_7)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o -c /home/ubuntu/iShare/MMGAPN/MMGFeedbackConnection.cpp

CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ubuntu/iShare/MMGAPN/MMGFeedbackConnection.cpp > CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.i

CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ubuntu/iShare/MMGAPN/MMGFeedbackConnection.cpp -o CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.s

CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o.requires:
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o.requires

CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o.provides: CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o.requires
	$(MAKE) -f CMakeFiles/iShare.dir/build.make CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o.provides.build
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o.provides

CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o.provides.build: CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o

CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o: CMakeFiles/iShare.dir/flags.make
CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o: MMGAPN/MMGTools.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ubuntu/iShare/CMakeFiles $(CMAKE_PROGRESS_8)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o -c /home/ubuntu/iShare/MMGAPN/MMGTools.cpp

CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ubuntu/iShare/MMGAPN/MMGTools.cpp > CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.i

CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ubuntu/iShare/MMGAPN/MMGTools.cpp -o CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.s

CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o.requires:
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o.requires

CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o.provides: CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o.requires
	$(MAKE) -f CMakeFiles/iShare.dir/build.make CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o.provides.build
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o.provides

CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o.provides.build: CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o

CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o: CMakeFiles/iShare.dir/flags.make
CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o: MMGAPN/MMGDevice.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ubuntu/iShare/CMakeFiles $(CMAKE_PROGRESS_9)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o -c /home/ubuntu/iShare/MMGAPN/MMGDevice.cpp

CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ubuntu/iShare/MMGAPN/MMGDevice.cpp > CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.i

CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ubuntu/iShare/MMGAPN/MMGDevice.cpp -o CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.s

CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o.requires:
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o.requires

CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o.provides: CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o.requires
	$(MAKE) -f CMakeFiles/iShare.dir/build.make CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o.provides.build
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o.provides

CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o.provides.build: CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o

CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o: CMakeFiles/iShare.dir/flags.make
CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o: MMGAPN/MMGPayload.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ubuntu/iShare/CMakeFiles $(CMAKE_PROGRESS_10)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o -c /home/ubuntu/iShare/MMGAPN/MMGPayload.cpp

CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ubuntu/iShare/MMGAPN/MMGPayload.cpp > CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.i

CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ubuntu/iShare/MMGAPN/MMGPayload.cpp -o CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.s

CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o.requires:
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o.requires

CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o.provides: CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o.requires
	$(MAKE) -f CMakeFiles/iShare.dir/build.make CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o.provides.build
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o.provides

CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o.provides.build: CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o

CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o: CMakeFiles/iShare.dir/flags.make
CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o: MMGAPN/MMGIOSPayload.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/ubuntu/iShare/CMakeFiles $(CMAKE_PROGRESS_11)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o -c /home/ubuntu/iShare/MMGAPN/MMGIOSPayload.cpp

CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/ubuntu/iShare/MMGAPN/MMGIOSPayload.cpp > CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.i

CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/ubuntu/iShare/MMGAPN/MMGIOSPayload.cpp -o CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.s

CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o.requires:
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o.requires

CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o.provides: CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o.requires
	$(MAKE) -f CMakeFiles/iShare.dir/build.make CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o.provides.build
.PHONY : CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o.provides

CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o.provides.build: CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o

# Object files for target iShare
iShare_OBJECTS = \
"CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o" \
"CMakeFiles/iShare.dir/iShare.pb.cc.o" \
"CMakeFiles/iShare.dir/iShare_server.cc.o" \
"CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o" \
"CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o" \
"CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o" \
"CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o" \
"CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o" \
"CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o" \
"CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o" \
"CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o"

# External object files for target iShare
iShare_EXTERNAL_OBJECTS =

iShare: CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o
iShare: CMakeFiles/iShare.dir/iShare.pb.cc.o
iShare: CMakeFiles/iShare.dir/iShare_server.cc.o
iShare: CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o
iShare: CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o
iShare: CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o
iShare: CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o
iShare: CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o
iShare: CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o
iShare: CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o
iShare: CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o
iShare: CMakeFiles/iShare.dir/build.make
iShare: /usr/lib/x86_64-linux-gnu/libmysqlclient.a
iShare: CMakeFiles/iShare.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable iShare"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/iShare.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/iShare.dir/build: iShare
.PHONY : CMakeFiles/iShare.dir/build

CMakeFiles/iShare.dir/requires: CMakeFiles/iShare.dir/iShare.grpc.pb.cc.o.requires
CMakeFiles/iShare.dir/requires: CMakeFiles/iShare.dir/iShare.pb.cc.o.requires
CMakeFiles/iShare.dir/requires: CMakeFiles/iShare.dir/iShare_server.cc.o.requires
CMakeFiles/iShare.dir/requires: CMakeFiles/iShare.dir/MMGAPN/MMGSafariPayload.cpp.o.requires
CMakeFiles/iShare.dir/requires: CMakeFiles/iShare.dir/MMGAPN/MMGAPNSConnection.cpp.o.requires
CMakeFiles/iShare.dir/requires: CMakeFiles/iShare.dir/MMGAPN/MMGSSLConnection.cpp.o.requires
CMakeFiles/iShare.dir/requires: CMakeFiles/iShare.dir/MMGAPN/MMGFeedbackConnection.cpp.o.requires
CMakeFiles/iShare.dir/requires: CMakeFiles/iShare.dir/MMGAPN/MMGTools.cpp.o.requires
CMakeFiles/iShare.dir/requires: CMakeFiles/iShare.dir/MMGAPN/MMGDevice.cpp.o.requires
CMakeFiles/iShare.dir/requires: CMakeFiles/iShare.dir/MMGAPN/MMGPayload.cpp.o.requires
CMakeFiles/iShare.dir/requires: CMakeFiles/iShare.dir/MMGAPN/MMGIOSPayload.cpp.o.requires
.PHONY : CMakeFiles/iShare.dir/requires

CMakeFiles/iShare.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/iShare.dir/cmake_clean.cmake
.PHONY : CMakeFiles/iShare.dir/clean

CMakeFiles/iShare.dir/depend:
	cd /home/ubuntu/iShare && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/iShare /home/ubuntu/iShare /home/ubuntu/iShare /home/ubuntu/iShare /home/ubuntu/iShare/CMakeFiles/iShare.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/iShare.dir/depend

