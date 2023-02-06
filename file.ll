; ModuleID = 'mylang2ir'
declare i32 @printf(i8*, ...)
@print.str = constant [4 x i8] c"%d\0A\00"
define i32 @main() {
%n = alloca i32
%f0 = alloca i32
%f1 = alloca i32
%i = alloca i32
%t = alloca i32
store i32 0, i32* %n
store i32 0, i32* %f0
store i32 0, i32* %f1
store i32 0, i32* %i
store i32 0, i32* %t
store i32 10, i32* %n
store i32 0, i32* %f0
store i32 1, i32* %f1
store i32 2, i32* %i
br label %whcond
whcond:
%t1 = load i32* %n
%t2 = icmp ne i32 %t1, 0
br i1 %t2, label %whbody, label %whend
whbody:
%t3 = load i32* %f1
store i32 %t3, i32* %t
%t4 = load i32* %f0
%t5 = load i32* %f1
%t6 = add i32 %t4, %t5
store i32 %t6, i32* %f1
%t7 = load i32* %f1
call i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 %t7 )
%t8 = load i32* %t
store i32 %t8, i32* %f0
%t9 = load i32* %n
%t10 = sub i32 %t9, 1
store i32 %t10, i32* %n
br label %whcond
whend:
ret i32 0
}