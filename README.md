# NUAA PL/0 Compiler
南航 编译原理课设 PL/0语言
## 样例
```pascal
program testsqrt; # 求平方根
var a: f32;

    procedure abs(x: f32): f32
    begin
        if x < 0.0 then
            return -x;
        else
            return x;
    end

    procedure root(num: f32): f32
    const epsilon := 0.001;
    var x: f32;
    begin
        x := num;
        while abs(x * x - num) >= epsilon do
        begin
            x := 0.5 * (x + num / x);
        end
        return x;
    end

begin
    read(a);
    a := root(a);
    write(a);
end
```

## 运行
```
=====开始执行=====
5
2.23607
程序耗时: 3040ms
```

## 文法
```
<prog> -> <program> <id>; <block>
<block> -> [<condecl>] [<vardecl>] [<proc>] <body>

<condecl> -> const <condeclOne> {, <condeclOne>};
<condeclOne> -> <id> := <literal>

<type> -> i32 | f32
<idType> -> <id> : <type>
<vardecl> -> var <idType> {, <idType>};

<proc> -> procedure <id> ( [<idType> {, <idType>}] ) [: <id>] <block> [<proc>]

<body> -> begin <statement> {<statement>} end
<callProcedure> -> <id> ( [<exp>{, <exp>}] )
<statement> -> <id> := <exp>;
            | <callProcedure>;
            | if <lexp> then <statement> [else <statement>]
            | while <lexp> do <statement>
            | <body>
            | read ( <id>{，<id>} ) ;
            | write ( <exp>{, <exp>} ) ;
            | return [<exp>] ;

<lexp> -> <exp> <lop> <exp> | odd <exp>
<exp> -> [+|-] <term>{<aop> <term>}
<term> -> <factor>{<mop> <factor>}
<factor> -> <id>
         | <callProcedure>
         | <literal>
         | ( <exp> )
         | <type> ( <exp> )

<lop> -> = | <> | < | <= | > | >=
<aop> -> + | -
<mop> -> * | /

<id> -> <l>{<l>|<d>}
<literal> -> <int32> | <float32>
<int32> -> [+|-] <d>{<d>}
<float32> -> [+|-] <d>{<d>}.<d>{<d>}
<l> -> 大写字母和小写字母
<d> -> 数字0-9
```