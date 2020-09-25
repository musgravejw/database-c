# Database

![GitHub tag (latest SemVer)](https://img.shields.io/github/v/tag/musgravejw/database)

Micro Database implementation.

Heavily drawn from https://cstack.github.com/db_tutorial with my own changes.

```shell
.
├── LICENSE
├── Makefile
├── README.md
├── dist
│   └── db
└── src
    ├── b-tree.h
    ├── datatypes.h
    ├── mm.h
    ├── parser.h
    ├── repl.c
    └── vm.h

2 directories, 10 files
```

## Install
```shell
$ make
```

## Usage

### Server
```shell
$ ./dist/db 


 ############################## 
 ##  ________  ________      ##
 ## |\   ___ \|\   __  \     ##
 ## \ \  \_|\ \ \  \|\ /_    ##
 ##  \ \  \  \ \ \   __  \   ##
 ##   \ \  \__\ \ \  \|\  \  ##
 ##    \ \_______\ \_______\ ##
 ##     \|_______|\|_______| ##
 ##############################

Starting server...
Listening on port 8080...

```

### REPL
```shell
$./dist/repl

Please select a database
?> [database]

?> create table [table name]

?> insert [id] [name] [value]

?> select
```

## License
 * This project is licensed under a BSD 3-Clause License

Copyright (c) 2019, John Musgrave All rights reserved.

