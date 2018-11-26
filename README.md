# teradyne_ft17_logs_parser
Parser for logs of measurement stations
## To build type in project root
mkdir build && cd build && cmake .. && make -j 5
##To run
./parser.elf -t<teradyne log file1> -t<teradyne log file2> ... -t<teradyne log fileN> -f<ft17 log file1> -f<ft17 log file2> ... -f<ft17 log fileN> -o<output file name>.csv
