# Explain how each write function inference to target file name & what content, & explain the process
- Assum filename = "test.txt"
- write(fd1, "Hello,", 6); => write to fd1 with 'Hello,', offset 6
- write(fd2. "world", 6); => write to fd2 with 'world', number offset is 6. Hello,world
- lseek(fd2, 0, SEEK_SET); => seek & move to the first offset of file
- write(fd1, "HELLO,", 6); => truncate(clear all content) & write to fd1, "HELLO," => fd1: "HELLO,", first 6 offsets occupied
- write(fd3, "Gidday", 6); => offer first 6 offsets & write "Gidday" to fd3 from offset 0 => "GiddayHELLO,"
# Note:
- the filename = "test.txt" will be shared for all proess & behaved depends on each action
- it also depends on the permission when fd is opened.