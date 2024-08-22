This is a work in progress Dockerfile for Snac.

This readme documents the work done so far, and aims to make it easier to build on this work in the future.

I have made progess towards compiling SNAC but have not been able to yet. If you are reading this and find any errors in the Dockerfile, or have had success compiling Snac, please contact me at github.com/JarettBakerDunn.
------------------------
1. Make sure to set FFLAGS for mpich.

2. Download and install mpich according to the instructions in the mpich readme.

3. Set environment variables for Snac.

4. Clone the Snac repository.

5. The next two lines make it so that "gcc" and "cc" run with the -fcommon flag, which is necesary because snac will call gcc when compiling. This allows snac to run these commands without editing the Snac compile scripts to call -fcommon.

6. move configure.sh from docker into the required folder within snac. configure.sh is an edited configure file which will actually run, as opposed to the current snac configure file. In the current snac configure file, there is an issue with newline characters which prevented it from running. The new configure.sh also defines DL_DIR which allows the script to find a required library.