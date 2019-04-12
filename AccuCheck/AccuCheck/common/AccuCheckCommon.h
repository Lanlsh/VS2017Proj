#pragma once
/*
    Desc: AccuCheck工程的公用数据的定义
*/

//释放内存
#define ACDelete(x)\
    if(x){delete x; x = nullptr;}
