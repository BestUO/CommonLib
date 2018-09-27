# 查找当前目录下的所有源文件并将名称保存到 DIR_LIB_SRCS 变量
aux_source_directory(. DIR_LIB_SRCS)

# 生成链接库
add_library(Commonlib ${DIR_LIB_SRCS})

#set linker_language
#set_target_properties(Commonlib PROPERTIES LINKER_LANGUAGE CXX)
