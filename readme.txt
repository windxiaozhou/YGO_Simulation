# YGO实体卡牌对战场景识别

YGO_real_final_1.cpp 为没有多线程优化以及neon加速的源代码

YGO_real_final_2.cpp 为经过neon加速以及多线程优化后的源代码(适用于网络环境较差的情况)

YGO_real_final_3.cpp 为经过neon加速后的源代码(适用于网络环境良好的情况)



需要编译生成可执行文件，只需要修改CMakelists中的add_executable( YGO YGO_real_final_x.cpp )的x为1、2或者3即可编译对应版本的代码

当前目录下的可执行文件YGO的源码为YGO_real_final_3.cpp,适用于网络环境良好的情况下的对战场景识别。

pic文件夹中存放了用来对比的卡片，卡片数量可以增加。

decks.txt文件中存放的是本次识别所需要用到的卡片独有的卡片代码。



源码中的 offset 变量可以控制选择识别蓝方还是红方的战场，12为红方，其他值为蓝方。

cards_count变量可以控制读取decks.txt中存放的卡片的数量。

在连接摄像头之后，还需要上位机与下位机同时保证联网环境，才能保证数据正常传输。
