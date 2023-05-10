# mysh-prompt

mysh (my simple shell) is a shell created using sytem calls in C. It supports basic commands like "ls" <br />
and some more complicated. <br />

**All commands need space between words to work.**

## Compile
There is a make file in order to compile the code. Just type `make` for compile. Then an executable is created "mysh". <br />
Type `./mysh` to run my shell. 

## Redirections
My shell supports redirections for input ("<") and output (">"). For example : <br />
`in-mysh-now:> myProgram > out.txt`<br />
`in-mysh-now:> sort < file1 > file2` <br />
`in-mysh-now:> sort < file1 > file2` <br />
<br />
You can also do concate to an existing file. <br />
`in-mysh-now:> cat file1 >> file2`

## Pipes
My shell supports pipes. For example : <br />
`in-mysh-now:> cat file1 file2 file3 | sort > file4`

## Background
You can execute commands in backgroung with '&' at the end of command. For example : <br />
`in-mysh-now:> sort file1 &` <br />
Whe a command runs in background then can run at the same time with next command you type.

## Wild Characters
My shell supports also wild characters. For example : <br />
`in-mysh-now:> ls file*.t?t`

## Aliases
Supports create and delete aliases with command `createalias` and `destroyalias`. For example : <br />
`in-mysh-now:> createalias ls “ls -l” <br />
`in-mysh-now:> destroyalias ls`




