project(psiwProj)
cmake_minimum_required(VERSION 3.5)

add_executable(psiwProj 
    main_pub_sub.c 
    #threaded_list.h threaded_list.c
    pub_sub_queue.h pub_sub_queue.c
    )
