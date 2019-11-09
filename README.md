# Database
Database implemented from scratch.

Heavily drawn from https://cstack.github.com/db_tutorial with my own changes.

```
.
├── LICENSE
├── Makefile
├── README.md
└── src
    ├── b-tree.h
    ├── datatypes.h
    ├── mm.h
    ├── parser.h
    ├── repl.c
    └── vm.h

1 directory, 9 files
```

## Install
```
$ make
```

## Usage

```
$ ./db 


 ############################## 
 ##  ________  ________      ##
 ## |\   ___ \|\   __  \     ##
 ## \ \  \_|\ \ \  \|\ /_    ##
 ##  \ \  \  \ \ \   __  \   ##
 ##   \ \  \__\ \ \  \|\  \  ##
 ##    \ \_______\ \_______\ ##
 ##     \|_______|\|_______| ##
 ##############################

Please select a database
?> [database]

?> create table [table name]

?> insert [id] [name] [value]

?> select
```
