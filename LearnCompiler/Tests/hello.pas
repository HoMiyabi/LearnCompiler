program hello;
var hello_a: integer;

    procedure p1();
    var p1_a: integer;

        procedure p2();
        begin
            p1()
        end;

    begin
        if hello_a = 1 then
        begin
            hello_a := 2;
            p2();
        end;

        p1_a := 3;
        writeln(p1_a);
    end;

begin
    hello_a := 1;
    p1();
end.