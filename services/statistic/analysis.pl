#!/usr/bin/perl

##
# Perl script to parse user text data
# and give some statistic report
##

############################Constants############################
$LEADING_URL = "http://www.ip138.com:8080/search.asp?";

# .csv stored the existed num_list, whose format is like:
#
# 13022345678,Jiangsu
# 18900002345,Beijing
#
$PRELOAD = "preload.csv";

$FULLUSER = "fulluser.txt";
################################################################

# stored the latest mobile=>Province into .csv as preloaded for
# next time's parsing.
sub update_csvdata() {
}

# try fill up the num_list, and store it into a hash
sub get_num_list() {
    my ($l) = (@_); # input param is actually a reference of Hash!

    my $UA = LWP::UserAgent->new(keep_alive=>1);
    $UA->agent("Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/37.0.2062.124 Safari/537.36");

    #First try get all existed records from a preload csv file.
    open(CSV, $PRELOAD) || die "failed open preload csv";
    while(<CSV>){
        @lines = split(",");
        $l{$lines[0]} = $lines[1];
    }
    close(CSV);

    #next, read fulluser data file...
    open(FULL, $FULLUSER) || die "failed open full user data";
    while(<FULL>){
        $line = $_;
        chomp($line); # here store one mobile number, like 13022592323
        unless (exists $l{$line}) {
            printf("$line didn't existed in the preload.csv, will get it from internet...\n");
            my $url = $LEADING_URL . "mobile=" . $line . "&action=mobile";
            my $res = $UA->get($url);
            if($res->is_success) {
                my $html = $res->content;
            }
        }
    }
    close(FULL);
}

sub check_active(){
}


##
# Main Entry Point Start


##
# num_list :
#
# {13x xxxx xxxx} => "Province1"
# {13x xxxx xxxx} => "Province2"
# {13x xxxx xxxx} => "Province3"
# ....        ....       ....
our %num_list;

##
# prov_list - account how many provinces
# {Province1} => 30
# {Province2} => 98
# {Province3} => 3
#  ...          ...
our %prov_list;

&get_num_list(\%num_list);


#####ENDOF====
my %name;

$name{13815882359} = "Jiangsu";
$name{13022593515} = "BeiJing";

foreach $n (keys %name) {
    printf("$n ==> $name{$n}\n");
}

@keys = keys %name;
@all = values %name;

print @all;
