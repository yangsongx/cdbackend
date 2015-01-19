#!/usr/bin/perl

$name;
$index=0;


open(FD, ">test1.csv") || die "failed write the file";
print FD "SEQUENTIAL\n";

while($index++ < 10000) {
    $name = "siptest" . $index;
    print FD "$name;[authentication username=$name password=111111]\n";
}

close(FD);

