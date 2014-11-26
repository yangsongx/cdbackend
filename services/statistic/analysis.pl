#!/usr/bin/perl
use Encode;
use LWP;
use LWP::UserAgent;
use LWP::Simple;
use HTTP::Cookies;
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
    my ($myl) = (@_);
    # remmember to encode it as UTF-8
    open(CSV, ">:encoding(utf-8)", "$PRELOAD") or die "failed try writing $PRELOAD\n";
    foreach $k (keys $myl){
      $v = ${$myl}{$k};
      $val = decode("gbk", $v);
      print CSV "$k,$val\n";
    }
    close(CSV);
}

# try fill up the num_list, and store it into a hash
sub get_num_list() {
    my ($myh) = (@_); # input param is actually a reference of Hash!

    my $UA = LWP::UserAgent->new(keep_alive=>1);
    $UA->agent("Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/37.0.2062.124 Safari/537.36");

    #First try get all existed records from a preload csv file.
    open(CSV, "$PRELOAD") || die "failed open preload csv";
    while(<CSV>){
        @lines = split(",");
        ${$myh}{$lines[0]} = $lines[1];
    }
    close(CSV);

    printf("Finished the preload CSV reading...\n");

    #next, read fulluser data file...
    open(FULL, $FULLUSER) || die "failed open full user data";
    while(<FULL>){
        $line = $_;
        chomp($line); # here store one mobile number, like 13022592323
        printf("the phone number=$line\n");
        unless (exists ${$myh}{$line}) {
            printf("$line didn't existed in the preload.csv, will get it from internet...\n");
            my $url = $LEADING_URL . "mobile=" . $line . "&action=mobile";

            my $res = $UA->get($url);
            if($res->is_success) {
                my $html = $res->content;
                open(TMPFILE, ">", "get.html") or die "failed write the html";
                print TMPFILE $html;
                close(TMPFILE);

                open(TMPFILE, "get.html") || die "failed open the temp html";
                $TXT_ANCHOR = "ø®∫≈πÈ Ùµÿ";
                while(<TMPFILE>) {
                    if($_ =~ /\Q$TXT_ANCHOR\E.*class=tdc2\>\<.*--\>(.*)\<\/TD\>/) {
                        $h = $1;
                        if($h =~ /(.*)\&nbsp;(.*)/) {
                            ${$myh}{$line} = $1;
                            $o = ${$myh}{$line};
                            $out = decode("gbk", $o);
                            $o = encode("utf-8", $out);
                            printf("\t$line ==> $o\n");
                            last; #remember , last is C's break;
                        }
                    } elsif($_ =~ /noswap\>\Q$TXT_ANCHOR\E\<\/TD\>/ || $_ =~ /\Q$TXT_ANCHOR\E/) {
                        $newline = 1;#result stored at next new line
                    } elsif($newline == 1) {
                        $newline = 0;
                        if($_ =~ /\>(.*)\&nbsp;(.*)\<\/TD\>/){
                            ${$myh}{$line} = $1;
                            $o = ${$myh}{$line};
                            $out = decode("gbk", $o);
                            $o = encode("utf-8", $out);
                            printf("\t$line ==> $o\n");
                        }
                    }
                }
                close(TMPFILE);
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
$len = keys %num_list;
printf("After call get_num_list, the whole users info account = $len\n");

&update_csvdata(\%num_list);
printf("All user data stored in $PRELOAD\n");

