cc = g++
target =  build/main.out
output =  main

source = src
include = include

main_source = ${source}/*.cpp
compile:
	${cc} -std=c++11 ${main_source} -o ${output} -I ${include}

generate_main_debug: 
	${cc} -std=c++11 -O0 -g ${main_source} -o ${target} -lm -I ${include}

upload:
	@scp -r . root@49.232.215.28:/root/server

debug: generate_main_debug
.phony: debug


help:
	@echo "this is a make file"
.phony: help