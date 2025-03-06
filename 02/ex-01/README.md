# Description:
Write the program that opens any file & use the O_APPEND flag, then performs a seek to the beginning
of the file & writes some data into it. 
# Result
- Output: the input data will appear at the end of the file's content.
- Why: due to initial reading thi file with flag O_APPEND (overwrite & append content to the end of original data line).

# Run
```bash
- x86_64-w64-mingw32-gcc main.c -o main.exe
- ./main.exe
```