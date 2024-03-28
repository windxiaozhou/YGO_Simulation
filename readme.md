# YGO实体卡牌对战场景识别

## 简介

一个简单的YGO实体对战场景识别系统。能够对主要区域的卡图进行识别并传输至上位机。能够检测卡片横置(横置的方向是逆时针旋转90度)，还原YGO中的怪兽防守。

## 效果

![image-20240328211037779](C:\Users\mypc\Desktop\飞腾cup\CICC3244+天府旅游团+0414\软件源代码\assets\image-20240328211037779.png)

目前额外怪兽区域、墓地、额外卡组、除外区、卡组等因为摄像头像素原因还有待开发。

## 上位机

上位机用网页端(HTML+javascript)实现，位于文件夹HTML下，打开文件夹下名为YGO的h5文件即可。上位机显示界面即为上图中的星空背景板界面(提取于萌卡YGO_pro2素材)。

## 识别原理

![image-20240328211118155](C:\Users\mypc\Desktop\飞腾cup\CICC3244+天府旅游团+0414\软件源代码\assets\image-20240328211118155.png)

![image-20240328211151050](C:\Users\mypc\Desktop\飞腾cup\CICC3244+天府旅游团+0414\软件源代码\assets\image-20240328211151050.png)

![image-20240328211220067](C:\Users\mypc\Desktop\飞腾cup\CICC3244+天府旅游团+0414\软件源代码\assets\image-20240328211220067.png)

## 代码版本分布及注意事项

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

**没有搭建专门的服务器用于socket传输，目前使用了GoEasy的免费服务，如果有需要可以修改下位机与上位机的socket代码。**