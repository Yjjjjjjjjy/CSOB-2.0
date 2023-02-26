#include <iostream>
#include <vector>
#include "./include/getbasic.h"

VectorSerialization<int> vec;
VectorSerialization<int> vec2;
 
void test_1(){
    vec2=VectorSerialization<int>(3,1);
    vec.insert(vec.end(),vec2.begin(),vec2.end());
    auto au=vec.begin();
    while (au!=vec.end()){
    std::cout<<*au<<'\n';
    au++;}
    std::cout<<"test-1"<<'\n';
    vec.clear();
}

void test_2(){
    vec2=VectorSerialization<int>(2,2);
    vec.insert(vec.end(),vec2.begin(),vec2.end());
    auto au=vec.begin();
    while (au!=vec.end()){
    std::cout<<*au<<'\n';
    au++;}
    std::cout<<"test-2"<<'\n';
}
int main() {
    
    
    test_1();
    test_2();
    return 0;
}