program hello;
var a : integer;

    procedure p1();
    begin
        write('p1');
    end;

    procedure p2();
    begin
        p1();
    end;

begin
    a := 1;
    p2();
end.