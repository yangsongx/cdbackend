#!/usr/bin/perl

# Stress testing on UC login
#
#curl --header "Content-type:application/json" -d '{"number_mobile":"a22301-test1", "sim_id":"123", "hardware_uid":"hw", "caredearcode":"21k"}' 192.168.1.108:9002/uc/login


$name;
$i=$ARGV[0];
$max = $i + 500;
#`curl --header "Content-type:application/json" -d '{"number_mobile":"a22301-test2", "sim_id":"123", "hardware_uid":"hw", "caredearcode":"21k"}' 192.168.1.108:9002/uc/login`;

while($i++ < $max) {
    $name = "a22301-test" . $i;
#printf("$name\n");
    `curl --header "Content-type:application/json" -d '{"number_mobile":"$name", "sim_id":"123", "hardware_uid":"hw", "caredearcode":"21k"}' 192.168.1.108:9002/uc/login`
}

printf("totally $i user inserted\n");
